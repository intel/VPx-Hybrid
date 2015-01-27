/* ****************************************************************************** *\

Copyright (C) 2003-2013 Intel Corporation.  All rights reserved.

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

File Name: umc_config.h

\* ****************************************************************************** */

#ifndef __UMC_CONFIG_H__
#define __UMC_CONFIG_H__

// This file contains defines which switch on/off support of various components

#if defined(_MSC_VER)
#pragma warning(disable:4206) // warning C4206: nonstandard extension used : translation unit is empty
#endif

/*
// Windows* on IA-32 or Intel(R) 64
*/

#if defined(WIN32) || defined (WIN64)
    // Enables MediaSDK based tools and samples
    // You need MediaSDK library, DirectX SDK and Windows SDK 7.1 to use them
    // You also need to manually add proper paths to includes and libs folders and libmfx.lib library to project
//    #define UMC_ENABLE_MSDK_INTERFACE

    // system interfaces
    #define UMC_ENABLE_SYS_WIN // allow use of Win32 window system

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FILE_READER_MMAP
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    #define UMC_ENABLE_FW_VIDEO_RENDER
#ifdef UMC_ENABLE_SYS_WIN // window context is required for these renders
    #define UMC_ENABLE_OPENGL_VIDEO_RENDER
    #define UMC_ENABLE_GDI_VIDEO_RENDER
#endif

    // audio renderers
#ifdef UMC_ENABLE_SYS_WIN // window context is required for DirectSound render
#if (_MSC_VER >= 1600)
    #define UMC_ENABLE_DSOUND_AUDIO_RENDER
#endif
#endif
    #define UMC_ENABLE_FW_AUDIO_RENDER
    #define UMC_ENABLE_WINMM_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER

    // muxers
    #define UMC_ENABLE_MPEG2_MUXER
    #define UMC_ENABLE_MP4_MUXER

    // video decoders
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    #define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_VC1_VIDEO_ENCODER

    // audio decoders
//    #define UMC_ENABLE_AAC_AUDIO_DECODER
  //  #define UMC_ENABLE_MP3_AUDIO_DECODER
    //#define UMC_ENABLE_AC3_AUDIO_DECODER

    // audio encoders
    #define UMC_ENABLE_AAC_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_AUDIO_ENCODER
#endif

/*
// Unix* on IA-32 or Intel(R) 64
*/

#if defined OSX32
    // system interfaces
    //#define UMC_ENABLE_SYS_GLX // you need to manually add paths to X11 sdk with glx lib and header

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FILE_READER_MMAP
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    #define UMC_ENABLE_FW_VIDEO_RENDER
#ifdef UMC_ENABLE_SYS_GLX
    #define UMC_ENABLE_OPENGL_VIDEO_RENDER
#endif

    // audio renderers
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER

    // muxers
    #define UMC_ENABLE_MPEG2_MUXER
    #define UMC_ENABLE_MP4_MUXER

    // video decoders
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    #define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_VC1_VIDEO_ENCODER

    // audio decoders
    #define UMC_ENABLE_AAC_AUDIO_DECODER
    #define UMC_ENABLE_MP3_AUDIO_DECODER
    #define UMC_ENABLE_AC3_AUDIO_DECODER

    // audio encoders
    #define UMC_ENABLE_AAC_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_AUDIO_ENCODER

#elif defined(LINUX32) || defined(LINUX64)
    // system interfaces
    #define UMC_ENABLE_SYS_GLX // allow use of X Window system

    // readers/writers
    #define UMC_ENABLE_FILE_READER
    #define UMC_ENABLE_FILE_READER_MMAP
    #define UMC_ENABLE_FILE_WRITER

    // video renderers
    #define UMC_ENABLE_FW_VIDEO_RENDER
#ifdef UMC_ENABLE_SYS_GLX
    #define UMC_ENABLE_OPENGL_VIDEO_RENDER
#endif

    // audio renderers
    #define UMC_ENABLE_OSS_AUDIO_RENDER
    #define UMC_ENABLE_FW_AUDIO_RENDER

    // splitters
    #define UMC_ENABLE_AVI_SPLITTER
    #define UMC_ENABLE_MPEG2_SPLITTER
    #define UMC_ENABLE_MP4_SPLITTER
    #define UMC_ENABLE_H264_SPLITTER
    #define UMC_ENABLE_VC1_SPLITTER

    // muxers
    #define UMC_ENABLE_MPEG2_MUXER
    #define UMC_ENABLE_MP4_MUXER

    // video decoders
    #define UMC_ENABLE_H264_VIDEO_DECODER
    #define UMC_ENABLE_MPEG2_VIDEO_DECODER
    #define UMC_ENABLE_MPEG4_VIDEO_DECODER
    #define UMC_ENABLE_MJPEG_VIDEO_DECODER
    #define UMC_ENABLE_VC1_VIDEO_DECODER

    // video encoders
    #define UMC_ENABLE_H264_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG2_VIDEO_ENCODER
    #define UMC_ENABLE_MPEG4_VIDEO_ENCODER
    #define UMC_ENABLE_VC1_VIDEO_ENCODER

    // audio decoders
    #define UMC_ENABLE_AAC_AUDIO_DECODER
    #define UMC_ENABLE_MP3_AUDIO_DECODER
    #define UMC_ENABLE_AC3_AUDIO_DECODER

    // audio encoders
    #define UMC_ENABLE_AAC_AUDIO_ENCODER
    #define UMC_ENABLE_MP3_AUDIO_ENCODER
#endif

#ifdef UMC_ENABLE_FILE_READER_MMAP
#define UMC_ENABLE_FILE_READER
#endif

#endif
