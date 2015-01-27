/* ****************************************************************************** *\

Copyright (C) 2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_vp8_encode_hybrid_bse.h

\* ****************************************************************************** */

#ifndef _MFX_VP8_ENCODE_HYBRID_BSE_H_
#define _MFX_VP8_ENCODE_HYBRID_BSE_H_

#include "mfx_common.h"
#include "mfx_vp8_encode_utils_hw.h"
#include "mfx_vp8_encode_utils_hybrid_hw.h"

#if defined(MFX_ENABLE_VP8_VIDEO_ENCODE_HW) && defined(MFX_VA)

namespace MFX_VP8ENC
{
    class Vp8CoreBSP
    {
    public:
        static Vp8CoreBSP* Create();
    protected:
        Vp8CoreBSP() {};
    public:
        mfxStatus Init(const mfxVideoParam &video);
        mfxStatus Reset(const mfxVideoParam &video);
        mfxStatus SetNextFrame(mfxFrameSurface1 * pSurface, bool bExternal, const sFrameParams &frameParams, mfxU32 frameNum);
        mfxStatus RunBSP(bool bIVFHeaders, bool bAddSH, mfxBitstream *bs, TaskHybridDDI *pTask, MBDATA_LAYOUT const & layout, mfxCoreInterface * pCore);
        VP8HybridCosts GetUpdatedCosts();
    };
}

#endif
#endif
