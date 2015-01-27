/* ****************************************************************************** *\

Copyright (C) 2006-2014 Intel Corporation.  All rights reserved.

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

File Name: umc_memory_allocator.h

\* ****************************************************************************** */

#ifndef __UMC_MEMORY_ALLOCATOR_H__
#define __UMC_MEMORY_ALLOCATOR_H__

#include "ippdefs.h"
#include "umc_structures.h"
#include "umc_dynamic_cast.h"
#include "umc_mutex.h"

#define MID_INVALID 0

namespace UMC
{

typedef size_t MemID;

enum UMC_ALLOC_FLAGS
{
    UMC_ALLOC_PERSISTENT  = 0x01,
    UMC_ALLOC_FUNCLOCAL   = 0x02
};

class MemoryAllocatorParams
{
     DYNAMIC_CAST_DECL_BASE(MemoryAllocatorParams)

public:
    MemoryAllocatorParams(){}
    virtual ~MemoryAllocatorParams(){}
};

class MemoryAllocator
{
    DYNAMIC_CAST_DECL_BASE(MemoryAllocator)

public:
    MemoryAllocator(void){}
    virtual ~MemoryAllocator(void){}

    // Initiates object
    virtual Status Init(MemoryAllocatorParams *pParams) = 0;

    // Closes object and releases all allocated memory
    virtual Status Close() = 0;

    // Allocates or reserves physical memory and returns unique ID
    // Sets lock counter to 0
    virtual Status Alloc(MemID *pNewMemID, size_t Size, Ipp32u Flags, Ipp32u Align = 16) = 0;

    // Lock() provides pointer from ID. If data is not in memory (swapped)
    // prepares (restores) it. Increases lock counter
    virtual void* Lock(MemID MID) = 0;

    // Unlock() decreases lock counter
    virtual Status Unlock(MemID MID) = 0;

    // Notifies that the data won't be used anymore. Memory can be freed.
    virtual Status Free(MemID MID) = 0;

    // Immediately deallocates memory regardless of whether it is in use (locked) or no
    virtual Status DeallocateMem(MemID MID) = 0;

protected:
    Mutex m_guard;

private:
    // Declare private copy constructor to avoid accidental assignment
    // and klocwork complaining.
    MemoryAllocator(const MemoryAllocator &);
    MemoryAllocator & operator = (const MemoryAllocator &);
};

} //namespace UMC

#endif //__UMC_MEMORY_ALLOCATOR_H__
