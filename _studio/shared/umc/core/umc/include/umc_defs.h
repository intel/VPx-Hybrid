/* ****************************************************************************** *\

Copyright (C) 2003-2014 Intel Corporation.  All rights reserved.

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

File Name: umc_defs.h

\* ****************************************************************************** */

#ifndef __UMC_DEFS_H__
#define __UMC_DEFS_H__

// This file contains defines which switch on/off support of
// codecs and renderers on application level

/*
// Windows on IA32
*/

#if defined(WIN32)

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FIO_READER
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    //#define UMC_ENABLE_DX_VIDEO_RENDER
    //#define UMC_ENABLE_BLT_VIDEO_RENDER
    //#define UMC_ENABLE_GDI_VIDEO_RENDER
    #define UMC_ENABLE_FW_VIDEO_RENDER

    // audio renderers
    //#define UMC_ENABLE_WINMM_AUDIO_RENDER
    //#define UMC_ENABLE_DSOUND_AUDIO_RENDER
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER
    //#define UMC_ENABLE_AVS_SPLITTER
    #define UMC_ENABLE_FLV_SPLITTER

    // video decoders
    //#define UMC_ENABLE_DV_VIDEO_DECODER
    //#define UMC_ENABLE_DV50_VIDEO_DECODER
    //#define UMC_ENABLE_DVHD_VIDEO_DECODER
    //#define UMC_ENABLE_H261_VIDEO_DECODER
    //#define UMC_ENABLE_H263_VIDEO_DECODER
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_H265_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    //#define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VP8_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER
    //#define UMC_ENABLE_AVS_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_DV_VIDEO_ENCODER
    #define UMC_ENABLE_DV50_VIDEO_ENCODER
    #define UMC_ENABLE_DVHD_VIDEO_ENCODER
    #define UMC_ENABLE_H261_VIDEO_ENCODER
    #define UMC_ENABLE_H263_VIDEO_ENCODER
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MVC_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_MJPEG_VIDEO_ENCODER
    //#define UMC_ENABLE_VC1_VIDEO_ENCODER

    //#define UMC_ENABLE_ME

    #define UMC_ENABLE_UMC_SCENE_ANALYZER

    // audio decoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_DECODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_DECODER
    #define UMC_ENABLE_AAC_AUDIO_DECODER
    //#define UMC_ENABLE_AC3_AUDIO_DECODER
    #define UMC_ENABLE_MP3_AUDIO_DECODER
    //#define UMC_ENABLE_AMR_AUDIO_DECODER
    //#define UMC_ENABLE_LPCM_AUDIO_DECODER
    //#define UMC_ENABLE_DTS_AUDIO_DECODER

    // audio encoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_ENCODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_ENCODER
    #define UMC_ENABLE_AAC_AUDIO_ENCODER
    //#define UMC_ENABLE_AC3_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_AUDIO_ENCODER
    //#define UMC_ENABLE_AMR_AUDIO_ENCODER

#endif // Win32 on IA32


/*
// WindowsCE on IA32
*/

#if defined(_WIN32_WCE) && (defined(x86) || defined(_X86_))

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    #define UMC_ENABLE_GDI_VIDEO_RENDER
    #define UMC_ENABLE_FW_VIDEO_RENDER

//    #define UMC_ENABLE_DXWCE_VIDEO_RENDER

    // audio renderers
    #define UMC_ENABLE_WINMM_AUDIO_RENDER
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER
    #define UMC_ENABLE_AVS_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_FLV_SPLITTER

    // video decoders
    #define UMC_ENABLE_DV_VIDEO_DECODER
    #define UMC_ENABLE_DV50_VIDEO_DECODER
    #define UMC_ENABLE_DVHD_VIDEO_DECODER
    #define UMC_ENABLE_H261_VIDEO_DECODER
    #define UMC_ENABLE_H263_VIDEO_DECODER
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    #define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER
    #define UMC_ENABLE_AVS_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_DV_VIDEO_ENCODER
    #define UMC_ENABLE_DV50_VIDEO_ENCODER
    #define UMC_ENABLE_DVHD_VIDEO_ENCODER
    #define UMC_ENABLE_H261_VIDEO_ENCODER
    #define UMC_ENABLE_H263_VIDEO_ENCODER
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_MJPEG_VIDEO_ENCODER
    //#define UMC_ENABLE_VC1_VIDEO_ENCODER

    #define UMC_ENABLE_UMC_SCENE_ANALYZER

    // audio decoders
    #define UMC_ENABLE_AAC_INT_AUDIO_DECODER
    #define UMC_ENABLE_MP3_INT_AUDIO_DECODER
    #define UMC_ENABLE_AAC_AUDIO_DECODER
    #define UMC_ENABLE_AC3_AUDIO_DECODER
    #define UMC_ENABLE_MP3_AUDIO_DECODER
    #define UMC_ENABLE_LPCM_AUDIO_DECODER
    #define UMC_ENABLE_DTS_AUDIO_DECODER

    // audio encoders
    #define UMC_ENABLE_AAC_INT_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_INT_AUDIO_ENCODER
    #define UMC_ENABLE_AAC_AUDIO_ENCODER
    #define UMC_ENABLE_AC3_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_AUDIO_ENCODER
#endif // WinCE on IA32

/*
// Windows on EM64T
*/

#if defined(WIN64) && defined (_AMD64_)

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FIO_READER
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    #define UMC_ENABLE_GDI_VIDEO_RENDER
    #define UMC_ENABLE_FW_VIDEO_RENDER

    // audio renderers
    #define UMC_ENABLE_WINMM_AUDIO_RENDER
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER
    #define UMC_ENABLE_AVS_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_FLV_SPLITTER

    // video decoders
    #define UMC_ENABLE_DV_VIDEO_DECODER
    #define UMC_ENABLE_DV50_VIDEO_DECODER
    #define UMC_ENABLE_DVHD_VIDEO_DECODER
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_H265_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    #define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_H263_VIDEO_DECODER
    #define UMC_ENABLE_H261_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER
    #define UMC_ENABLE_AVS_VIDEO_DECODER
    #define UMC_ENABLE_VP8_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_DV_VIDEO_ENCODER
    #define UMC_ENABLE_DV50_VIDEO_ENCODER
    #define UMC_ENABLE_DVHD_VIDEO_ENCODER
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MVC_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_MJPEG_VIDEO_ENCODER
    #define UMC_ENABLE_H263_VIDEO_ENCODER
    #define UMC_ENABLE_H261_VIDEO_ENCODER
    //#define UMC_ENABLE_VC1_VIDEO_ENCODER

    #define UMC_ENABLE_UMC_SCENE_ANALYZER

    // audio decoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_DECODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_DECODER
    //#define UMC_ENABLE_AAC_AUDIO_DECODER
    //#define UMC_ENABLE_AC3_AUDIO_DECODER
    //#define UMC_ENABLE_MP3_AUDIO_DECODER
    //#define UMC_ENABLE_LPCM_AUDIO_DECODER
    //#define UMC_ENABLE_DTS_AUDIO_DECODER

    // audio encoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_ENCODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_ENCODER
    //#define UMC_ENABLE_AAC_AUDIO_ENCODER
    //#define UMC_ENABLE_AC3_AUDIO_ENCODER
    //#define UMC_ENABLE_MP3_AUDIO_ENCODER

#endif // Winx64 on EM64T

/*
// Windows on IA64
*/

#if defined(WIN64) && !defined (_AMD64_)

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FIO_READER
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    //#define UMC_ENABLE_GDI_VIDEO_RENDER
    #define UMC_ENABLE_FW_VIDEO_RENDER

    // audio renderers
    //#define UMC_ENABLE_WINMM_AUDIO_RENDER
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER
    //#define UMC_ENABLE_AVS_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_FLV_SPLITTER

    // video decoders
    //#define UMC_ENABLE_DV_VIDEO_DECODER
    //#define UMC_ENABLE_DV50_VIDEO_DECODER
    //#define UMC_ENABLE_DVHD_VIDEO_DECODER
    //#define UMC_ENABLE_H261_VIDEO_DECODER
    //#define UMC_ENABLE_H263_VIDEO_DECODER
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_H265_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    //#define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER
    //#define UMC_ENABLE_AVS_VIDEO_DECODER
    #define UMC_ENABLE_VP8_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_DV_VIDEO_ENCODER
    #define UMC_ENABLE_DV50_VIDEO_ENCODER
    #define UMC_ENABLE_DVHD_VIDEO_ENCODER
    #define UMC_ENABLE_H261_VIDEO_ENCODER
    #define UMC_ENABLE_H263_VIDEO_ENCODER
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MVC_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_MJPEG_VIDEO_ENCODER
    //#define UMC_ENABLE_VC1_VIDEO_ENCODER

    #define UMC_ENABLE_UMC_SCENE_ANALYZER

    // audio decoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_DECODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_DECODER
    #define UMC_ENABLE_AAC_AUDIO_DECODER
    //#define UMC_ENABLE_AC3_AUDIO_DECODER
    #define UMC_ENABLE_MP3_AUDIO_DECODER
    //#define UMC_ENABLE_AMR_AUDIO_DECODER
    //#define UMC_ENABLE_LPCM_AUDIO_DECODER
    //#define UMC_ENABLE_DTS_AUDIO_DECODER

    // audio encoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_ENCODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_ENCODER
    #define UMC_ENABLE_AAC_AUDIO_ENCODER
    //#define UMC_ENABLE_AC3_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_AUDIO_ENCODER
    //#define UMC_ENABLE_AMR_AUDIO_ENCODER

#endif // Win64 on IA64

/*
// WindowsCE on IXP4xx
*/

#if defined(_WIN32_WCE) && (defined (ARM) || defined(_ARM_))

    // readers/writers
    #define UMC_ENABLE_FIO_READER
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    #define UMC_ENABLE_MTWGX_VIDEO_RENDER
    #define UMC_ENABLE_FW_VIDEO_RENDER

    // audio renderers
    #define UMC_ENABLE_WINMM_AUDIO_RENDER
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER
    #define UMC_ENABLE_AVS_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_FLV_SPLITTER

    // video decoders
    #define UMC_ENABLE_DV_VIDEO_DECODER
    #define UMC_ENABLE_DV50_VIDEO_DECODER
    #define UMC_ENABLE_DVHD_VIDEO_DECODER
    #define UMC_ENABLE_H261_VIDEO_DECODER
    #define UMC_ENABLE_H263_VIDEO_DECODER
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    #define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER
    #define UMC_ENABLE_AVS_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_DV_VIDEO_ENCODER
    #define UMC_ENABLE_DV50_VIDEO_ENCODER
    #define UMC_ENABLE_DVHD_VIDEO_ENCODER
    #define UMC_ENABLE_H261_VIDEO_ENCODER
    #define UMC_ENABLE_H263_VIDEO_ENCODER
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_MJPEG_VIDEO_ENCODER
    //#define UMC_ENABLE_VC1_VIDEO_ENCODER

    #define UMC_ENABLE_UMC_SCENE_ANALYZER

    // audio decoders
    #define UMC_ENABLE_AAC_INT_AUDIO_DECODER
    #define UMC_ENABLE_MP3_INT_AUDIO_DECODER
    #define UMC_ENABLE_LPCM_AUDIO_DECODER

    // audio encoders
    #define UMC_ENABLE_AAC_INT_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_INT_AUDIO_ENCODER

#endif // WinCE on IXP4xx

/*
// Linux on IA32
*/

#if (defined(LINUX32) || defined(__APPLE__)) && !(defined (_ARM_) || defined(ARM))

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FIO_READER
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
#ifdef __APPLE__
# ifndef OSX64
   #define UMC_ENABLE_AGL_VIDEO_RENDER
# endif
#else
   #define UMC_ENABLE_FB_VIDEO_RENDER
//   #define UMC_ENABLE_SDL_VIDEO_RENDER
#endif /* __APPLE__ */
    #define UMC_ENABLE_FW_VIDEO_RENDER

    // audio renderers
#ifdef __APPLE__
    #define UMC_ENABLE_COREAUDIO_RENDER
#else
    #define UMC_ENABLE_ALSA_AUDIO_RENDER
    #define UMC_ENABLE_OSS_AUDIO_RENDER
//    #define UMC_ENABLE_SDL_AUDIO_RENDER
#endif /* __APPLE__ */
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER
    //#define UMC_ENABLE_AVS_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_FLV_SPLITTER

    // video decoders
    //#define UMC_ENABLE_DV_VIDEO_DECODER
    //#define UMC_ENABLE_DV50_VIDEO_DECODER
    //#define UMC_ENABLE_DVHD_VIDEO_DECODER
    //#define UMC_ENABLE_H261_VIDEO_DECODER
    //#define UMC_ENABLE_H263_VIDEO_DECODER

    //#define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VP8_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER
    #define UMC_ENABLE_H264_VIDEO_DECODER
    //#define UMC_ENABLE_H265_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER

    //#define UMC_ENABLE_AVS_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_DV_VIDEO_ENCODER
    #define UMC_ENABLE_DV50_VIDEO_ENCODER
    #define UMC_ENABLE_DVHD_VIDEO_ENCODER
    #define UMC_ENABLE_H261_VIDEO_ENCODER
    #define UMC_ENABLE_H263_VIDEO_ENCODER
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MVC_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_MJPEG_VIDEO_ENCODER
    //#define UMC_ENABLE_VC1_VIDEO_ENCODER

    #define UMC_ENABLE_UMC_SCENE_ANALYZER

    // audio decoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_DECODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_DECODER
    #define UMC_ENABLE_AAC_AUDIO_DECODER
    //#define UMC_ENABLE_AC3_AUDIO_DECODER
    #define UMC_ENABLE_MP3_AUDIO_DECODER
    //#define UMC_ENABLE_LPCM_AUDIO_DECODER
    //#define UMC_ENABLE_DTS_AUDIO_DECODER

    // audio encoders
    //#define UMC_ENABLE_AAC_INT_AUDIO_ENCODER
    //#define UMC_ENABLE_MP3_INT_AUDIO_ENCODER
    #define UMC_ENABLE_AAC_AUDIO_ENCODER
    //#define UMC_ENABLE_AC3_AUDIO_ENCODER
    //#define UMC_ENABLE_MP3_AUDIO_ENCODER

#endif // Linux on IA32

/*
// Linux on EM64T
*/

/*
// Linux on IA64
*/

/*
// Linux on IXP4xx
*/

#if defined(LINUX32) && (defined (ARM) || defined(_ARM_))

    // readers/writers
    #define UMC_ENABLE_FIO_READER
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    #define UMC_ENABLE_FB_VIDEO_RENDER
    #define UMC_ENABLE_FW_VIDEO_RENDER
//  #define UMC_ENABLE_SDL_VIDEO_RENDER

    // audio renderers
    #define UMC_ENABLE_ALSA_AUDIO_RENDER
    #define UMC_ENABLE_OSS_AUDIO_RENDER
//    #define UMC_ENABLE_SDL_AUDIO_RENDER
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER
    #define UMC_ENABLE_AVS_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_FLV_SPLITTER

    // video decoders
    #define UMC_ENABLE_DV_VIDEO_DECODER
    #define UMC_ENABLE_DV50_VIDEO_DECODER
    #define UMC_ENABLE_DVHD_VIDEO_DECODER
    #define UMC_ENABLE_H261_VIDEO_DECODER
    #define UMC_ENABLE_H263_VIDEO_DECODER
    #define UMC_ENABLE_H264_VIDEO_DECODER
    //#define UMC_ENABLE_H265_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    #define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER
    #define UMC_ENABLE_AVS_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_DV_VIDEO_ENCODER
    #define UMC_ENABLE_DV50_VIDEO_ENCODER
    #define UMC_ENABLE_DVHD_VIDEO_ENCODER
    #define UMC_ENABLE_H261_VIDEO_ENCODER
    #define UMC_ENABLE_H263_VIDEO_ENCODER
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MVC_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_MJPEG_VIDEO_ENCODER
    //#define UMC_ENABLE_VC1_VIDEO_ENCODER

    #define UMC_ENABLE_UMC_SCENE_ANALYZER

    // audio decoders
    #define UMC_ENABLE_AAC_INT_AUDIO_DECODER
    #define UMC_ENABLE_MP3_INT_AUDIO_DECODER
    #define UMC_ENABLE_LPCM_AUDIO_DECODER

    // audio encoders
    #define UMC_ENABLE_AAC_INT_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_INT_AUDIO_ENCODER

#endif // Linux on IXP4xx

#ifdef __cplusplus

// forward declarations
namespace UMC
{
class Splitter;
class BaseCodec;
class VideoDecoder;
class VideoEncoder;
class AudioCodec;
class Muxer;
class MediaBuffer;

// create instance of particular component
// should be deleted via delete operator

Splitter *CreateMPEG2Splitter();
Splitter *CreateMPEG2SplitterNT();
Splitter *CreateMPEG4Splitter();
Splitter *CreateAVISplitter();
Splitter *CreateVC1Splitter();
Splitter *CreateASFSplitter();

VideoDecoder *CreateMPEG2Decoder();
VideoDecoder *CreateMPEG4Decoder();
VideoDecoder *CreateH261Decoder();
VideoDecoder *CreateH263Decoder();
VideoDecoder *CreateH264Decoder();
VideoDecoder *CreateVC1Decoder();
VideoDecoder *CreateDVDecoder();
VideoDecoder *CreateDV50Decoder();
VideoDecoder *CreateDV100Decoder();

BaseCodec *CreateVideoProcessing(); // full video processing
BaseCodec *CreateD3DVideoProcessing();
BaseCodec *CreateDeinterlacing();
BaseCodec *CreateColorSpaceConversion();
BaseCodec *CreateVideoResizing();

VideoEncoder *CreateMPEG2Encoder();
VideoEncoder *CreateMPEG4Encoder();
VideoEncoder *CreateH261Encoder();
VideoEncoder *CreateH263Encoder();
VideoEncoder *CreateH264Encoder();
VideoEncoder *CreateVC1Encoder();
VideoEncoder *CreateDVEncoder();
VideoEncoder *CreateDV50Encoder();
VideoEncoder *CreateDV100Encoder();

AudioCodec *CreateAACDecoder();
AudioCodec *CreateAACEncoder();
AudioCodec *CreateMP3Decoder();
AudioCodec *CreateMP3Encoder();
AudioCodec *CreateAC3Decoder();
AudioCodec *CreatePCMDecoder();

// version without floating point operations
AudioCodec *CreateAACDecoderInt();
AudioCodec *CreateAACEncoderInt();
AudioCodec *CreateMP3DecoderInt();
AudioCodec *CreateMP3EncoderInt();

Muxer *CreateMPEG2Muxer();
Muxer *CreateMPEG4Muxer();
Muxer *CreateAVIMuxer();

MediaBuffer *CreateLinearBuffer();
MediaBuffer *CreateSampleBuffer();
MediaBuffer *CreateVideoBuffer();

// Convert decoder to asynchronous decoder.
VideoDecoder *CreateAsynchronousVideoDecoder(VideoDecoder *pDecoder);

} // namespace UMC

#endif //__cplusplus

#include "ippdefs.h"
#define MFX_INTERNAL_CPY(dst, src, size) ippsCopy_8u((const Ipp8u *)(src), (Ipp8u *)(dst), (int)size)

/******************************************************************************/

#endif // __UMC_DEFS_H__
