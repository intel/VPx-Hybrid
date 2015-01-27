/* ****************************************************************************** *\

Copyright (C) 2008 - 2012 Intel Corporation.  All rights reserved.

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

File Name: mfx_common_decode_int.h

\* ****************************************************************************** */
#ifndef __MFX_COMMON_DECODE_INT_H__
#define __MFX_COMMON_DECODE_INT_H__

#include <vector>
#include "mfx_common_int.h"
#include "umc_video_decoder.h"

class MFXMediaDataAdapter : public UMC::MediaData
{
public:
    MFXMediaDataAdapter(mfxBitstream *pBitstream = 0);

    void Load(mfxBitstream *pBitstream);
    void Save(mfxBitstream *pBitstream);
};

mfxStatus ConvertUMCStatusToMfx(UMC::Status status);

void ConvertMFXParamsToUMC(mfxVideoParam *par, UMC::VideoDecoderParams *umcVideoParams);
void ConvertMFXParamsToUMC(mfxVideoParam *par, UMC::VideoStreamInfo *umcVideoParams);

bool IsNeedChangeVideoParam(mfxVideoParam *par);

inline mfxU32 ExtractProfile(mfxU32 profile)
{
    return profile & 0xFF;
}

inline bool IsMVCProfile(mfxU32 profile)
{
    profile = ExtractProfile(profile);
    return (profile == MFX_PROFILE_AVC_MULTIVIEW_HIGH || profile == MFX_PROFILE_AVC_STEREO_HIGH);
}

inline bool IsSVCProfile(mfxU32 profile)
{
    profile = ExtractProfile(profile);
    return (profile == MFX_PROFILE_AVC_SCALABLE_BASELINE || profile == MFX_PROFILE_AVC_SCALABLE_HIGH);
}

template <class T>
mfxStatus CheckIntelDataPrivateReport(T *pConfig, mfxVideoParam *par)
{

    if ((par->mfx.FrameInfo.Width > pConfig->ConfigMBcontrolRasterOrder) ||
        (par->mfx.FrameInfo.Height > pConfig->ConfigResidDiffHost))
    {
        return MFX_WRN_PARTIAL_ACCELERATION;
    }

    bool isArbitraryCroppingSupported = pConfig->ConfigDecoderSpecific & 0x01;


    mfxExtSVCSeqDesc * svcDesc = (mfxExtSVCSeqDesc*)GetExtendedBuffer(par->ExtParam, par->NumExtParam, MFX_EXTBUFF_SVC_SEQ_DESC);
    if (svcDesc && IsSVCProfile(par->mfx.CodecProfile) && !isArbitraryCroppingSupported)
    {
        bool isBaselineConstraints = true;

        for (Ipp32u i = 0; i < sizeof(svcDesc->DependencyLayer)/sizeof(svcDesc->DependencyLayer[0]); i++)
        {
            if (svcDesc->DependencyLayer[i].Active)
            {
                if ((svcDesc->DependencyLayer[i].ScaledRefLayerOffsets[0] & 0xf) || (svcDesc->DependencyLayer[i].ScaledRefLayerOffsets[1] & 0xf))
                {
                    isBaselineConstraints = false;
                    break;
                }

                Ipp32u depD = svcDesc->DependencyLayer[i].RefLayerDid;
                if (depD >= 8)
                    continue;
                Ipp16u scaledW = (Ipp16u)(svcDesc->DependencyLayer[i].Width - svcDesc->DependencyLayer[i].ScaledRefLayerOffsets[0]-svcDesc->DependencyLayer[i].ScaledRefLayerOffsets[2]);
                Ipp16u scaledH = (Ipp16u)(svcDesc->DependencyLayer[i].Height - svcDesc->DependencyLayer[i].ScaledRefLayerOffsets[1]-svcDesc->DependencyLayer[i].ScaledRefLayerOffsets[3]);
                if ( ! ((scaledW == svcDesc->DependencyLayer[depD].Width &&
                    scaledH == svcDesc->DependencyLayer[depD].Height) ||

                    (scaledW == 2*svcDesc->DependencyLayer[depD].Width &&
                    scaledH == 2*svcDesc->DependencyLayer[depD].Height) ||

                    (scaledW == (3*svcDesc->DependencyLayer[depD].Width)/2 &&
                    scaledH == (3*svcDesc->DependencyLayer[depD].Height)/2) ))
                {
                    isBaselineConstraints = false;
                    break;
                }
            }
        }

        if (!isBaselineConstraints)
        {
            return MFX_WRN_PARTIAL_ACCELERATION;
        }
    }
    return MFX_ERR_NONE;
} // mfxStatus GetMaxSizes

#endif
