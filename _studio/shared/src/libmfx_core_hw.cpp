/* ****************************************************************************** *\

Copyright (C) 2007-2013 Intel Corporation.  All rights reserved.

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

File Name: libmfx_core_hw.cpp

\* ****************************************************************************** */

#include "mfx_common.h"
#if defined (MFX_VA_WIN)

#include "libmfx_core_hw.h"
#include "umc_va_dxva2.h"
#include "mfx_common_decode_int.h"
#include "mfx_enc_common.h"

using namespace UMC;

VideoAccelerationHW ConvertMFXToUMCType(eMFXHWType type)
{
    VideoAccelerationHW umcType = VA_HW_UNKNOWN;

    switch(type)
    {
    case MFX_HW_LAKE:
        umcType = VA_HW_LAKE;
        break;

    case MFX_HW_LRB:
        umcType = VA_HW_LRB;
        break;
    case MFX_HW_SNB:
        umcType = VA_HW_SNB;
        break;
    case MFX_HW_IVB:
        umcType = VA_HW_IVB;
        break;

    case MFX_HW_HSW:
        umcType = VA_HW_HSW;
        break;
    case MFX_HW_HSW_ULT:
        umcType = VA_HW_HSW_ULT;
        break;

    case MFX_HW_BDW:
        umcType = VA_HW_BDW;
        break;

    case MFX_HW_SCL:
        umcType = VA_HW_SCL;
        break;

    case MFX_HW_VLV:
        umcType = VA_HW_VLV;
        break;
    }

    return umcType;
}

mfxU32 ChooseProfile(mfxVideoParam * param, eMFXHWType hwType)
{
    mfxU32 profile = UMC::VA_VLD;

    // video accelerator is needed for decoders only 
    switch (param->mfx.CodecId)
    {
    case MFX_CODEC_VC1:
        profile |= VA_VC1;
        break;
    case MFX_CODEC_MPEG2:
        profile |= VA_MPEG2;
        break;
    case MFX_CODEC_AVC:
        {
        profile |= VA_H264;

        mfxU32 profile_idc = ExtractProfile(param->mfx.CodecProfile);

        if (IsMVCProfile(profile_idc))
        {
            if (hwType == MFX_HW_HSW || hwType == MFX_HW_HSW_ULT || hwType == MFX_HW_BDW || hwType == MFX_HW_SCL)
            {
#if 0 // Intel  MVC GUID
                profile |= H264_VLD_MVC;
                if (param->mfx.FrameInfo.PicStruct == MFX_PICSTRUCT_PROGRESSIVE)
                    profile |= VA_SHORT_SLICE_MODE;
#else // or MS MVC GUID
                mfxExtMVCSeqDesc * points = (mfxExtMVCSeqDesc *)GetExtendedBuffer(param->ExtParam, param->NumExtParam, MFX_EXTBUFF_MVC_SEQ_DESC);

                if (profile_idc == MFX_PROFILE_AVC_MULTIVIEW_HIGH && points && points->NumView > 2)
                    profile |= VA_PROFILE_MVC_MV;
                else
                    profile |= param->mfx.FrameInfo.PicStruct == MFX_PICSTRUCT_PROGRESSIVE ? VA_PROFILE_MVC_STEREO_PROG : VA_PROFILE_MVC_STEREO;

                //if (param->mfx.FrameInfo.PicStruct == MFX_PICSTRUCT_PROGRESSIVE)
                //    profile |= VA_SHORT_SLICE_MODE;
#endif
            }
            else
            {
                profile |= VA_LONG_SLICE_MODE;
            }
        }

        if (IsSVCProfile(profile_idc))
        {
            profile |= (profile_idc == MFX_PROFILE_AVC_SCALABLE_HIGH) ? VA_PROFILE_SVC_HIGH : VA_PROFILE_SVC_BASELINE;
        }
        }
        break;
    case MFX_CODEC_JPEG:
        profile |= VA_JPEG;
        break;
    case MFX_CODEC_VP8:
        profile |= VA_VP8;
        break;
    case MFX_CODEC_HEVC:
        profile |= VA_H265;
        break;


    default:
        return 0;
    }

    return profile;
}

bool IsHwMvcEncSupported()
{
    return true;
}

#endif 
/* EOF */
