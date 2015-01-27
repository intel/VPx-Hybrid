/* ****************************************************************************** *\

Copyright (C) 2003-2010 Intel Corporation.  All rights reserved.

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

File Name: umc_video_decoder.h

\* ****************************************************************************** */

#ifndef __UMC_VIDEO_DECODER_H__
#define __UMC_VIDEO_DECODER_H__

#include "umc_structures.h"
#include "umc_video_data.h"
#include "umc_base_codec.h"
#include "umc_base_color_space_converter.h"

namespace UMC
{

class VideoAccelerator;

class VideoDecoderParams : public BaseCodecParams
{
    DYNAMIC_CAST_DECL(VideoDecoderParams, BaseCodecParams)

public:
    // Default constructor
    VideoDecoderParams();
    // Destructor
    virtual ~VideoDecoderParams();

    VideoStreamInfo         info;                           // (VideoStreamInfo) compressed video info
    Ipp32u                  lFlags;                         // (Ipp32u) decoding flag(s)
    Ipp32u                  lTrickModesFlag;                // (Ipp32u) trick modes

    Ipp64f                  dPlaybackRate;

    BaseCodec               *pPostProcessing;               // (BaseCodec*) pointer to post processing

    VideoAccelerator        *pVideoAccelerator;             // pointer to video accelerator
};

/******************************************************************************/

class VideoDecoder : public BaseCodec
{
    DYNAMIC_CAST_DECL(VideoDecoder, BaseCodec)

public:
    VideoDecoder(void) :
        m_PostProcessing(NULL),
        m_allocatedPostProcessing(NULL)
    {}

    // Destructor
    virtual ~VideoDecoder(void);

    // BaseCodec methods
    // Get codec working (initialization) parameter(s)
    virtual Status GetInfo(BaseCodecParams *info);
    // Set new working parameter(s)
    virtual Status SetParams(BaseCodecParams *params);

    // Additional methods
    // Reset skip frame counter
    virtual Status ResetSkipCount() = 0;
    // Increment skip frame counter
    virtual Status SkipVideoFrame(Ipp32s) = 0;
    // Get skip frame counter statistic
    virtual Ipp32u GetNumOfSkippedFrames() = 0;
    // Preview last decoded frame
    virtual Status PreviewLastFrame(VideoData *out, BaseCodec *pPostProcessing = NULL);

    // returns closed capture data
    virtual Status GetUserData(MediaData* /*pCC*/)
    {
        return UMC_ERR_NOT_IMPLEMENTED;
    }

protected:

    VideoStreamInfo         m_ClipInfo;                         // (VideoStreamInfo) clip info
    VideoData               m_LastDecodedFrame;                 // (VideoData) last decoded frame
    BaseCodec               *m_PostProcessing;                  // (BaseCodec*) pointer to post processing
    BaseCodec               *m_allocatedPostProcessing;         // (BaseCodec*) pointer to default post processing allocated by decoder

private:
    // Declare private copy constructor to avoid accidental assignment
    // and klocwork complaining.
    VideoDecoder(const VideoDecoder &);
    VideoDecoder & operator = (const VideoDecoder &);
};

} // end namespace UMC

#endif // __UMC_VIDEO_DECODER_H__
