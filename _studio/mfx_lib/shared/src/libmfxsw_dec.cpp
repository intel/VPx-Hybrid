/* ****************************************************************************** *\

Copyright (C) 2008-2009 Intel Corporation.  All rights reserved.

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

File Name: libmfxsw_dec.cpp

\* ****************************************************************************** */
#include "libmfxsw_core.h"
#include "mfx_common.h"

#if defined (MFX_ENABLE_MPEG2_VIDEO_DEC)
#include "mfx_mpeg2_dec.h"
#endif

#if defined (MFX_ENABLE_VC1_VIDEO_DEC)
#include "mfx_vc1_dec.h"
#endif

#if defined (MFX_ENABLE_H264_VIDEO_DEC)
#include "mfx_h264_dec.h"
#endif

struct SWDEC:private DispatchSession {
    SWCORE*     core;
    VideoDEC*   dec;
    mfxU32      CodecId;
};

mfxStatus MFXVideoDEC_Create(mfxHDL core, mfxHDL *dec) {
    if (!core) return MFX_ERR_INVALID_HANDLE;
    if (!dec) return MFX_ERR_NULL_PTR;

    try {
        SWDEC *pDEC=new SWDEC;
        if (!pDEC) return MFX_ERR_MEMORY_ALLOC;
        pDEC->core=(SWCORE *)core;
        pDEC->dec=0;
        pDEC->CodecId=0;
        *dec=(mfxHDL)pDEC;
        return MFX_ERR_NONE;
    } catch (...) {
        return MFX_ERR_UNKNOWN;
    }
}

mfxStatus MFXVideoDEC_Destroy(mfxHDL dec) {
    try {
        if (!dec) return MFX_ERR_INVALID_HANDLE;
        MFXVideoDEC_Close(dec);
        delete (SWDEC *)dec;
        return MFX_ERR_NONE;
    } catch (...) {
        return MFX_ERR_UNKNOWN;
    }
}

static mfxStatus CreateCodecSpecificClass(mfxU32 CodecId, VideoCORE *core, VideoDEC **pDEC) {
    mfxStatus sts=MFX_ERR_UNSUPPORTED;
    switch (CodecId) {
#if defined (MFX_ENABLE_VC1_VIDEO_DEC)
    case MFX_CODEC_VC1:
        *pDEC=new MFXVideoDECVC1(core,&sts);
        break;
#endif
#if defined (MFX_ENABLE_MPEG2_VIDEO_DEC)
    case MFX_CODEC_MPEG2:
        *pDEC=new MFXVideoDECMPEG2(core,&sts);
        break;
#endif
#if defined (MFX_ENABLE_H264_VIDEO_DEC)
    case MFX_CODEC_AVC:
        *pDEC=new MFXVideoDECH264(core,&sts);
        break;
#endif
    default:
        return sts;
    }
    if (!(*pDEC)) return MFX_ERR_MEMORY_ALLOC;
    if (sts>MFX_ERR_NONE) {
        delete *pDEC; *pDEC=0;
    }
    return sts;
}

mfxStatus MFXVideoDEC_Query(mfxHDL dec, mfxVideoParam *in, mfxVideoParam *out) {
    try {
        SWDEC *pDEC=(SWDEC *)dec;
        if (!pDEC) return MFX_ERR_INVALID_HANDLE;
        if (!out) return MFX_ERR_NULL_PTR;

        VideoDEC *tdec=pDEC->dec;
        if (tdec) return tdec->Query(in,out);
        mfxStatus sts=CreateCodecSpecificClass(out->mfx.CodecId,pDEC->core->core,&tdec);
        if (sts>MFX_ERR_NONE) return sts;
        sts=tdec->Query(in,out);
        delete tdec;
        return sts;
    } catch (...) {
        return MFX_ERR_UNKNOWN;
    }
}

mfxStatus MFXVideoDEC_Init(mfxHDL dec, mfxVideoParam *par) {
    try {
        if (!par) return MFX_ERR_NULL_PTR;
        if (par->Version.Major) return MFX_ERR_UNSUPPORTED;

        SWDEC *pDEC=(SWDEC *)dec;
        if (!pDEC) return MFX_ERR_INVALID_HANDLE;

        if (pDEC->dec && pDEC->CodecId!=par->mfx.CodecId) {
            delete pDEC->dec;
            pDEC->dec=0;
            pDEC->CodecId=0;
        }

        if (!pDEC->dec) {
            mfxStatus sts=CreateCodecSpecificClass(par->mfx.CodecId,pDEC->core->core,&pDEC->dec);
            if (sts>MFX_ERR_NONE) return sts;
            pDEC->CodecId=par->mfx.CodecId;
        }
        return pDEC->dec->Init(par);
    } catch (...) {
        return MFX_ERR_UNKNOWN;
    }
}

mfxStatus MFXVideoDEC_Close(mfxHDL dec) {
    try {
        SWDEC *pDEC=(SWDEC *)dec;
        if (!pDEC) return MFX_ERR_INVALID_HANDLE;
        if (!pDEC->dec) return MFX_ERR_NONE;
        mfxStatus sts=pDEC->dec->Close();
        delete pDEC->dec;
        pDEC->dec=0;
        return sts;
    } catch (...) {
        return MFX_ERR_UNKNOWN;
    }
}

#undef FUNCTION
#define FUNCTION(RETVAL,FUNC1,PROTO,FUNC2,ARGS) \
    RETVAL FUNC1 PROTO {    \
        try {   \
            SWDEC *pDEC=(SWDEC *)dec;   \
            if (!pDEC) return MFX_ERR_INVALID_HANDLE;     \
            if (!pDEC->dec) return MFX_ERR_NOT_INITIALIZED;    \
            return pDEC->dec-> FUNC2 ARGS;  \
        } catch (...) {     \
            return MFX_ERR_UNKNOWN;     \
        }   \
    }

FUNCTION(mfxStatus,MFXVideoDEC_Reset,(mfxHDL dec, mfxVideoParam *par),Reset,(par))

FUNCTION(mfxStatus,MFXVideoDEC_GetVideoParam,(mfxHDL dec, mfxVideoParam *par),GetVideoParam,(par))
FUNCTION(mfxStatus,MFXVideoDEC_GetFrameParam,(mfxHDL dec, mfxFrameParam *par),GetFrameParam,(par))
FUNCTION(mfxStatus,MFXVideoDEC_GetSliceParam,(mfxHDL dec, mfxSliceParam *par),GetSliceParam,(par))

FUNCTION(mfxStatus,MFXVideoDEC_RunFrameFullDEC,(mfxHDL dec, mfxFrameCUC *cuc),RunFrameFullDEC,(cuc))
FUNCTION(mfxStatus,MFXVideoDEC_RunFramePredDEC,(mfxHDL dec, mfxFrameCUC *cuc),RunFramePredDEC,(cuc))
FUNCTION(mfxStatus,MFXVideoDEC_RunFrameIQT,(mfxHDL dec, mfxFrameCUC *cuc, mfxU8 scan),RunFrameIQT,(cuc, scan))
FUNCTION(mfxStatus,MFXVideoDEC_RunFrameILDB,(mfxHDL dec, mfxFrameCUC *cuc),RunFrameILDB,(cuc))
FUNCTION(mfxStatus,MFXVideoDEC_GetFrameRecon,(mfxHDL dec, mfxFrameCUC *cuc),GetFrameRecon,(cuc))

FUNCTION(mfxStatus,MFXVideoDEC_RunSliceFullDEC,(mfxHDL dec, mfxFrameCUC *cuc),RunSliceFullDEC,(cuc))
FUNCTION(mfxStatus,MFXVideoDEC_RunSlicePredDEC,(mfxHDL dec, mfxFrameCUC *cuc),RunSlicePredDEC,(cuc))
FUNCTION(mfxStatus,MFXVideoDEC_RunSliceIQT,(mfxHDL dec, mfxFrameCUC *cuc, mfxU8 scan),RunSliceIQT,(cuc,scan))
FUNCTION(mfxStatus,MFXVideoDEC_RunSliceILDB,(mfxHDL dec, mfxFrameCUC *cuc),RunSliceILDB,(cuc))
FUNCTION(mfxStatus,MFXVideoDEC_GetSliceRecon,(mfxHDL dec, mfxFrameCUC *cuc),GetSliceRecon,(cuc))

