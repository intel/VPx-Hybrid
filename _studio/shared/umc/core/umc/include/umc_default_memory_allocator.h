/* ****************************************************************************** *\

Copyright (C) 2006-2007 Intel Corporation.  All rights reserved.

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

File Name: umc_default_memory_allocator.h

\* ****************************************************************************** */

#ifndef __UMC_DEFAULT_MEMORY_ALLOCATOR_H__
#define __UMC_DEFAULT_MEMORY_ALLOCATOR_H__

#include "umc_memory_allocator.h"

namespace UMC
{

struct MemoryInfo;

class DefaultMemoryAllocator : public MemoryAllocator
{
    DYNAMIC_CAST_DECL(DefaultMemoryAllocator, MemoryAllocator)

public:
    DefaultMemoryAllocator(void);
    virtual
    ~DefaultMemoryAllocator(void);

    // Initiates object
    virtual Status Init(MemoryAllocatorParams *pParams);

    // Closes object and releases all allocated memory
    virtual Status Close();

    // Allocates or reserves physical memory and return unique ID
    // Sets lock counter to 0
    virtual Status Alloc(MemID *pNewMemID, size_t Size, Ipp32u Flags, Ipp32u Align = 16);

    // Lock() provides pointer from ID. If data is not in memory (swapped)
    // prepares (restores) it. Increases lock counter
    virtual void *Lock(MemID MID);

    // Unlock() decreases lock counter
    virtual Status Unlock(MemID MID);

    // Notifies that the data won’t be used anymore. Memory can be freed.
    virtual Status Free(MemID MID);

    // Immediately deallocates memory regardless of whether it is in use (locked) or no
    virtual Status DeallocateMem(MemID MID);

protected:
    MemoryInfo* memInfo;  // memory blocks descriptors
    Ipp32s      memCount; // number of allocated descriptors
    Ipp32s      memUsed;  // number of used descriptors
    MemID       lastMID;  // last value assigned to descriptor
};

} // namespace UMC

#endif // __UMC_DEFAULT_MEMORY_ALLOCATOR_H
