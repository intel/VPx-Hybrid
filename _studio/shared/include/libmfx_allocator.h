/* ****************************************************************************** *\

Copyright (C) 2007-2011 Intel Corporation.  All rights reserved.

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

File Name: libmfx_allocator.h

\* ****************************************************************************** */

#ifndef _LIBMFX_ALLOCATOR_H_
#define _LIBMFX_ALLOCATOR_H_

#include <vector>
#include "mfxvideo.h"


// Internal Allocators
namespace mfxDefaultAllocator
{
    mfxStatus AllocBuffer(mfxHDL pthis, mfxU32 nbytes, mfxU16 type, mfxMemId *mid);
    mfxStatus LockBuffer(mfxHDL pthis, mfxMemId mid, mfxU8 **ptr);
    mfxStatus UnlockBuffer(mfxHDL pthis, mfxMemId mid);
    mfxStatus FreeBuffer(mfxHDL pthis, mfxMemId mid);

    mfxStatus AllocFrames(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
    mfxStatus LockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
    mfxStatus GetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
    mfxStatus UnlockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr=0);
    mfxStatus FreeFrames(mfxHDL pthis, mfxFrameAllocResponse *response);

    struct BufferStruct
    {
        mfxHDL      allocator;
        mfxU32      id;
        mfxU32      nbytes;
        mfxU16      type;
    };
    struct FrameStruct
    {
        mfxU32          id;
        mfxFrameInfo    info;
    };
}

class mfxWideBufferAllocator
{
public:
    std::vector<mfxDefaultAllocator::BufferStruct*> m_bufHdl;
    mfxWideBufferAllocator(void);
    ~mfxWideBufferAllocator(void);
    mfxBufferAllocator bufferAllocator;
};

class mfxBaseWideFrameAllocator
{
public:
    mfxBaseWideFrameAllocator(mfxU16 type = 0);
    virtual ~mfxBaseWideFrameAllocator();
    mfxFrameAllocator       frameAllocator;
    mfxWideBufferAllocator  wbufferAllocator;
    mfxU32                  NumFrames;
    std::vector<mfxHDL>     m_frameHandles;
    // Type of this allocator
    mfxU16                  type;
};
class mfxWideSWFrameAllocator : public  mfxBaseWideFrameAllocator
{
public:
    mfxWideSWFrameAllocator(mfxU16 type);
    virtual ~mfxWideSWFrameAllocator(void) {};
};

#endif

