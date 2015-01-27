/* ****************************************************************************** *\

Copyright (C) 2011-2012 Intel Corporation.  All rights reserved.

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

File Name: libmfx_allocator_vaapi.h

\* ****************************************************************************** */

#include "mfx_common.h"

#if defined (MFX_VA_LINUX)

#ifndef _LIBMFX_ALLOCATOR_VAAPI_H_
#define _LIBMFX_ALLOCATOR_VAAPI_H_

#include <va/va.h>

#include "mfxvideo++int.h"
#include "libmfx_allocator.h"

// VAAPI Allocator internal Mem ID
struct vaapiMemIdInt
{
    VASurfaceID* m_surface;
    VAImage m_image;
    unsigned int m_fourcc;
};

// Internal Allocators 
namespace mfxDefaultAllocatorVAAPI
{
    mfxStatus AllocFramesHW(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
    mfxStatus LockFrameHW(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
    mfxStatus GetHDLHW(mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
    mfxStatus UnlockFrameHW(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr=0);
    mfxStatus FreeFramesHW(mfxHDL pthis, mfxFrameAllocResponse *response);

    class mfxWideHWFrameAllocator : public  mfxBaseWideFrameAllocator
    {
    public:
        mfxWideHWFrameAllocator(mfxU16 type, mfxHDL handle);
        virtual ~mfxWideHWFrameAllocator(void){};

        VADisplay* pVADisplay;

//        VASurfaceID* m_SrfQueue;
//        vaapiMemIdInt* m_SrfQueue;

        mfxU32 m_DecId;
    };

} //  namespace mfxDefaultAllocatorVAAPI

#endif // LIBMFX_ALLOCATOR_VAAPI_H_
#endif // (MFX_VA_LINUX)
/* EOF */
