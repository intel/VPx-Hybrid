/* ****************************************************************************** *\

Copyright (C) 2006-2009 Intel Corporation.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
- Neither the name of Intel Corporation nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL INTEL CORPORATION BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

File Name: umc_default_memory_allocator.cpp

\* ****************************************************************************** */

#include "umc_default_memory_allocator.h"
#include "ipps.h"

namespace UMC
{

  // structure to describe one memory block
  struct MemoryInfo
  {
    void *pMemory;          // allocated memory block
    size_t Size;            // allocated size
    MemID  MID;             // MID_INVALID if unused
    Ipp32s Alignment;       // requested alignment
    Ipp32s LocksCount;      // lock counter
    Ipp32s InvalidatedFlag; // set after Free()

    void Init()
    {
      pMemory = 0;
      Size = 0;
      MID = MID_INVALID;
    }
    // Released descriptor with not released memory of suitable size
    Ipp32s CanReuse(size_t s_Size, Ipp32s s_Alignment)
    {
      if(MID == MID_INVALID &&
        pMemory != 0 &&
        s_Size+(align_pointer<Ipp8u*>(pMemory, s_Alignment) - (Ipp8u*)pMemory) <= Size)
        return 1;
      return 0;

    }
    // assign memory to descriptor
    Status Alloc(size_t s_Size, Ipp32s s_Alignment)
    {
      if(!CanReuse(s_Size, s_Alignment)) { // can't reuse
        pMemory = ippsMalloc_8u((Ipp32s)s_Size+s_Alignment);
        if(pMemory == 0) {
          vm_debug_trace1(VM_DEBUG_ERROR, VM_STRING("failed to allocate %d bytes"), (Ipp32s)Size);
          return UMC_ERR_ALLOC;
        }
        Size = s_Size+s_Alignment; // align to be done on Lock() call
      }
      Alignment = s_Alignment;
      LocksCount = 0;
      InvalidatedFlag = 0;
      return UMC_OK;
    }
    // mark as no more used checking the state
    void Clear()
    {
      if(pMemory != 0) {
        if(InvalidatedFlag == 0) {
          vm_debug_trace2(VM_DEBUG_MEMORY, VM_STRING("Mem block ID:%d size:%d wasn't released"),
            (Ipp32s)MID, (Ipp32s)Size);
          InvalidatedFlag = 1;
        }
        if(LocksCount != 0) {
          vm_debug_trace2(VM_DEBUG_MEMORY, VM_STRING("Mem block ID:%d size:%d has bad lock counter"),
            (Ipp32s)MID, (Ipp32s)Size);
          LocksCount = 0;
        }
        MID = MID_INVALID;
      }
    }
    // release memory allocated to descriptor
    void Release()
    {
      if(pMemory != 0) {
        Clear();
        ippsFree(pMemory);
        pMemory = 0;
      }

    }
  };


DefaultMemoryAllocator::DefaultMemoryAllocator(void)
{
    memInfo = 0;
    memCount = 0; // allocate only on call
    memUsed = 0;
    lastMID = MID_INVALID;
    Init(NULL);
}

DefaultMemoryAllocator::~DefaultMemoryAllocator(void)
{
    DefaultMemoryAllocator::Close();
}

Status DefaultMemoryAllocator::Init(MemoryAllocatorParams* /*pParams*/)
{
    AutomaticUMCMutex guard(m_guard);

    DefaultMemoryAllocator::Close();

    return UMC_OK;
}

Status DefaultMemoryAllocator::Close()
{
  AutomaticUMCMutex guard(m_guard);

  Ipp32s i;
  for(i=0; i<memUsed; i++) {
    memInfo[i].Release();
  }
  if(memInfo != 0) {
    ippsFree(memInfo);
    memInfo = 0;
  }
  memUsed = 0;
  memCount = 0;

  return UMC_OK;
}

Status DefaultMemoryAllocator::Alloc(MemID *pNewMemID, size_t Size, Ipp32u /*Flags*/, Ipp32u Align/*=16*/)
{
  Ipp32s i;
  MemoryInfo* pmem = 0;
  MemoryInfo* pmemtofree = 0;

  AutomaticUMCMutex guard(m_guard);

  if (pNewMemID == NULL)
    return UMC_ERR_NULL_PTR;

  if (Size == 0 || Align == 0)
    return UMC_ERR_INVALID_PARAMS;

  *pNewMemID = MID_INVALID;


  for (i = 1; i <= (1 << 20); i <<= 1) {
    if (i & Align) {
      break; // stop at nonzero bit
    }
  }

  if (i != (Ipp32s)Align) // no 1 in 20 ls bits or more than 1 nonzero bit
    return UMC_ERR_INVALID_PARAMS;

  for (i=0; i<memUsed; i++) { // search unused or free
    if (memInfo[i].pMemory == 0 || memInfo[i].CanReuse(Size, Align)) {
      pmem = &memInfo[i];
      break;
    } else if(memInfo[i].MID == MID_INVALID)
      pmemtofree = &memInfo[i];
  }
  if (pmem == 0 && memUsed < memCount) { // take from never used
    pmem = &memInfo[memUsed];
    memUsed ++;
  }
  if(pmem == 0 && pmemtofree != 0) { // release last unsuitable
    pmemtofree->Release();
    pmem = pmemtofree;
  }
  if (pmem == 0) { // relocate all descriptors
    Ipp32s newcount = IPP_MAX(8, memCount*2);
    MemoryInfo* newmem = (MemoryInfo*)ippsMalloc_8u((int)(newcount*sizeof(MemoryInfo)));
    if (newmem == 0) {
      vm_debug_trace1(VM_DEBUG_ERROR, VM_STRING("failed to allocate %d bytes"), newcount*sizeof(MemoryInfo));
      return UMC_ERR_ALLOC;
    }

    for (i=0; i<memCount; i++) // copy existing
      newmem[i] = memInfo[i];

    // free old descriptors
    if (memInfo)
        ippsFree(memInfo);

    memInfo = newmem;
    memCount = newcount;
    for (; i<memCount; i++)
      memInfo[i].Init(); // init new

    pmem = &memInfo[memUsed]; // take first in new
    memUsed ++;
  }

  if(UMC_OK != pmem->Alloc(Size, Align))
    return UMC_ERR_ALLOC;
  lastMID++;
  pmem->MID = lastMID;
  *pNewMemID = lastMID;

  return UMC_OK;
}

void* DefaultMemoryAllocator::Lock(MemID MID)
{
  Ipp32s i;

  if( MID == MID_INVALID )
    return NULL;

  AutomaticUMCMutex guard(m_guard);

  for (i=0; i<memUsed; i++) {
    if (memInfo[i].MID == MID) {
      if(memInfo[i].pMemory == 0 || memInfo[i].InvalidatedFlag)
        return NULL; // no memory or invalidated
      memInfo[i].LocksCount ++;
      // return with aligning
      return align_pointer<Ipp8u*>(memInfo[i].pMemory, memInfo[i].Alignment);
    }
  }

  return NULL;
}

Status DefaultMemoryAllocator::Unlock(MemID MID)
{
  Ipp32s i;

  if( MID == MID_INVALID )
    return UMC_ERR_FAILED;

  AutomaticUMCMutex guard(m_guard);

  for (i=0; i<memUsed; i++) {
    if (memInfo[i].MID == MID) {
      if(memInfo[i].pMemory == 0 || memInfo[i].LocksCount <= 0)
        return UMC_ERR_FAILED; // no mem or lock /unlock mismatch
      memInfo[i].LocksCount --;
      if(memInfo[i].LocksCount == 0 && memInfo[i].InvalidatedFlag)
        memInfo[i].Clear(); //  no more use
      return UMC_OK;
    }
  }

  return UMC_ERR_FAILED;
}

Status DefaultMemoryAllocator::Free(MemID MID)
{
  Ipp32s i;

  if( MID == MID_INVALID )
    return UMC_ERR_FAILED;

  AutomaticUMCMutex guard(m_guard);

  for (i=0; i<memUsed; i++) {
    if (memInfo[i].MID == MID) {
      if(memInfo[i].pMemory == 0 || memInfo[i].InvalidatedFlag != 0)
        return UMC_ERR_FAILED; // no mem or re-free
      memInfo[i].InvalidatedFlag = 1;
      if(memInfo[i].LocksCount == 0)
        memInfo[i].Clear(); // not in use
      return UMC_OK;
    }
  }

  return UMC_ERR_FAILED;
}

Status DefaultMemoryAllocator::DeallocateMem(MemID MID)
{
  Ipp32s i;

  if( MID == MID_INVALID )
    return UMC_ERR_FAILED;

  AutomaticUMCMutex guard(m_guard);

  for (i=0; i<memUsed; i++) {
    if (memInfo[i].MID == MID) {
      if(memInfo[i].pMemory == 0)
        return UMC_ERR_FAILED; // no memory
      memInfo[i].InvalidatedFlag = 1;
      memInfo[i].Clear();
      return UMC_OK;
    }
  }

  return UMC_OK;
}

} // namespace UMC
