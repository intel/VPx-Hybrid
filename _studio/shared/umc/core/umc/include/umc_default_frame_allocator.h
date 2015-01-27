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

File Name: umc_default_frame_allocator.h

\* ****************************************************************************** */

#ifndef __UMC_DEFAULT_FRAME_ALLOCATOR_H__
#define __UMC_DEFAULT_FRAME_ALLOCATOR_H__

#include "umc_frame_allocator.h"
#include "umc_default_memory_allocator.h"

#include <vector>

namespace UMC
{

class FrameInformation;

class DefaultFrameAllocator : public FrameAllocator
{
    DYNAMIC_CAST_DECL(DefaultFrameAllocator, FrameAllocator)

public:
    DefaultFrameAllocator(void);
    virtual ~DefaultFrameAllocator(void);

    // Initiates object
    virtual Status Init(FrameAllocatorParams *pParams);

    // Closes object and releases all allocated memory
    virtual Status Close();

    virtual Status Reset();

    // Allocates or reserves physical memory and returns unique ID
    // Sets lock counter to 0
    virtual Status Alloc(FrameMemID *pNewMemID, const VideoDataInfo * info, Ipp32u flags);

    // Lock() provides pointer from ID. If data is not in memory (swapped)
    // prepares (restores) it. Increases lock counter
    virtual const FrameData* Lock(FrameMemID mid);

    // Unlock() decreases lock counter
    virtual Status Unlock(FrameMemID mid);

    // Notifies that the data won't be used anymore. Memory can be freed.
    virtual Status IncreaseReference(FrameMemID mid);

    // Notifies that the data won't be used anymore. Memory can be freed.
    virtual Status DecreaseReference(FrameMemID mid);

protected:

    Status Free(FrameMemID mid);

    std::vector<FrameInformation *> m_frames;

};

} // namespace UMC

#endif // __UMC_DEFAULT_FRAME_ALLOCATOR_H__
