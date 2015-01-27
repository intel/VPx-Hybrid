/* ****************************************************************************** *\

Copyright (C) 2003-2007 Intel Corporation.  All rights reserved.

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

File Name: umc_video_decoder.cpp

\* ****************************************************************************** */

#include <string.h>
#include "umc_video_decoder.h"

namespace UMC
{

VideoDecoderParams::VideoDecoderParams(void)
{
    m_pData = NULL;
    memset(&info, 0, sizeof(sVideoStreamInfo));
    lFlags = 0;
    lTrickModesFlag= UMC_TRICK_MODES_NO;

    pPostProcessing = NULL;

    dPlaybackRate = 1;

    lpMemoryAllocator = NULL;

    pVideoAccelerator = NULL;
} // VideoDecoderParams::VideoDecoderParams(void)

VideoDecoderParams::~VideoDecoderParams(void)
{

} // VideoDecoderParams::~VideoDecoderParams(void)

VideoDecoder::~VideoDecoder(void)
{
  if (m_allocatedPostProcessing) {
    delete m_allocatedPostProcessing;
    m_allocatedPostProcessing = NULL;
  }
} // VideoDecoder::~VideoDecoder(void)

Status VideoDecoder::PreviewLastFrame(VideoData *out, BaseCodec *pPostProcessing)
{
  if (!pPostProcessing) {
    pPostProcessing = m_PostProcessing;
    if (!pPostProcessing) {
      return UMC_ERR_NOT_INITIALIZED;
    }
  }
  return pPostProcessing->GetFrame(&m_LastDecodedFrame, out);
} // void VideoDecoder::PreviewLastFrame()

Status VideoDecoder::GetInfo(BaseCodecParams *info)
{
    Status umcRes = UMC_OK;
    VideoDecoderParams *pParams = DynamicCast<VideoDecoderParams> (info);

    if (NULL == pParams)
        return UMC_ERR_NULL_PTR;

    pParams->info = m_ClipInfo;

    return umcRes;

} // Status VideoDecoder::GetInfo(BaseCodecParams *info)

Status VideoDecoder::SetParams(BaseCodecParams* params)
{
    Status umcRes = UMC_OK;
    VideoDecoderParams *pParams = DynamicCast<VideoDecoderParams>(params);

    if (NULL == pParams)
        return UMC_ERR_NULL_PTR;

    m_ClipInfo = pParams->info;

    return umcRes;

} // Status VideoDecoder::SetParams(BaseCodecParams* params)

} // end namespace UMC
