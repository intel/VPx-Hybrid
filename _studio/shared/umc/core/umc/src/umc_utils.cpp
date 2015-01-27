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

File Name: umc_utils.cpp

\* ****************************************************************************** */
#include "vm_strings.h"
#include "umc_structures.h"
#include "umc_va_base.h"

#define UMC_SAME_NAME(X) \
  { UMC::X, VM_STRING(#X) }

namespace UMC
{
  typedef struct {
    int code;
    const vm_char *string;
  } CodeStringTable;

  static CodeStringTable StringsOfError[] =
  {
    { UMC::UMC_OK,                    VM_STRING("No any errors") },
    { UMC::UMC_ERR_FAILED,            VM_STRING("General operation fault") },
    { UMC::UMC_ERR_NOT_INITIALIZED,   VM_STRING("Object was not initialized before usage") },
    { UMC::UMC_ERR_TIMEOUT,           VM_STRING("Timeout") },
    { UMC::UMC_ERR_NOT_ENOUGH_DATA,   VM_STRING("Not enough data") },
    { UMC::UMC_ERR_NULL_PTR,          VM_STRING("Zero pointer was passed as param") },
    { UMC::UMC_ERR_INIT,              VM_STRING("Failed to initialize codec") },
    { UMC::UMC_ERR_SYNC,              VM_STRING("Required suncronization code was not found") },
    { UMC::UMC_ERR_NOT_ENOUGH_BUFFER, VM_STRING("Buffer size is not enough") },
    { UMC::UMC_ERR_END_OF_STREAM,     VM_STRING("End of stream") },
    { UMC::UMC_ERR_OPEN_FAILED,       VM_STRING("Device/file open error") },
    { UMC::UMC_ERR_ALLOC,             VM_STRING("Failed to allocate memory") },
    { UMC::UMC_ERR_INVALID_STREAM,    VM_STRING("Invalid stream") },
    { UMC::UMC_ERR_UNSUPPORTED,       VM_STRING("Unsupported") },
    { UMC::UMC_ERR_NOT_IMPLEMENTED,   VM_STRING("Not implemented yet") },
    { UMC::UMC_ERR_INVALID_PARAMS,    VM_STRING("Incorrect parameters") },
  };

  static CodeStringTable StringsOfVideoRenderType[] = {
    { UMC::DEF_VIDEO_RENDER,    VM_STRING("DEFAULT") },
    { UMC::DX_VIDEO_RENDER,     VM_STRING("DX") },
    { UMC::BLT_VIDEO_RENDER,    VM_STRING("BLT") },
    { UMC::GDI_VIDEO_RENDER,    VM_STRING("GDI") },
    { UMC::GX_VIDEO_RENDER,     VM_STRING("GX") },
    { UMC::SDL_VIDEO_RENDER,    VM_STRING("SDL") },
    { UMC::AGL_VIDEO_RENDER,    VM_STRING("MACOS AGL") },
    { UMC::FB_VIDEO_RENDER,     VM_STRING("FB") },
    { UMC::FW_VIDEO_RENDER,     VM_STRING("FILE WRITER") },
    { UMC::NULL_VIDEO_RENDER,   VM_STRING("NULL") },
    { UMC::MTWREG_VIDEO_RENDER, VM_STRING("MTWREG") },
    { UMC::OVL2_VIDEO_RENDER,   VM_STRING("OVL2") },
    { UMC::DXWCE_VIDEO_RENDER,  VM_STRING("DXWCE") },
    { UMC::AGL_VIDEO_RENDER,    VM_STRING("AGL") },
    { UMC::NO_VIDEO_RENDER,     VM_STRING("NO_RENDER") },
    { UMC::D3D_VIDEO_RENDER,    VM_STRING("D3D") },
  };

  static CodeStringTable StringsOfAudioRenderType[] = {
    { UMC::DEF_AUDIO_RENDER,    VM_STRING("DEFAULT") },
    { UMC::DSOUND_AUDIO_RENDER, VM_STRING("DSOUND") },
    { UMC::WINMM_AUDIO_RENDER,  VM_STRING("WINMM") },
    { UMC::ALSA_AUDIO_RENDER,   VM_STRING("ALSA") },
    { UMC::OSS_AUDIO_RENDER,    VM_STRING("OSS") },
    { UMC::SDL_AUDIO_RENDER,    VM_STRING("SDL") },
    { UMC::COREAUDIO_RENDER,    VM_STRING("MACOS COREAUDIO") },
    { UMC::NULL_AUDIO_RENDER,   VM_STRING("NULL") },
    { UMC::FW_AUDIO_RENDER,     VM_STRING("FILE WRITER") },
  };

  static CodeStringTable StringsOfFormatType[] = {
    UMC_SAME_NAME(YV12),
    UMC_SAME_NAME(NV12),
    UMC_SAME_NAME(YUY2),
    UMC_SAME_NAME(UYVY),
    UMC_SAME_NAME(YUV411),
    UMC_SAME_NAME(YUV420),
    UMC_SAME_NAME(YUV422),
    UMC_SAME_NAME(YUV444),
    UMC_SAME_NAME(YUV_VC1),
    UMC_SAME_NAME(Y411),
    UMC_SAME_NAME(Y41P),
    UMC_SAME_NAME(RGB32),
    UMC_SAME_NAME(RGB24),
    UMC_SAME_NAME(RGB565),
    UMC_SAME_NAME(RGB555),
    UMC_SAME_NAME(RGB444),
    UMC_SAME_NAME(GRAY),
    UMC_SAME_NAME(YUV420A),
    UMC_SAME_NAME(YUV422A),
    UMC_SAME_NAME(YUV444A),
    UMC_SAME_NAME(YVU9),
    UMC_SAME_NAME(D3D_SURFACE)
  };
#if 1
  static CodeStringTable StringsOfVideoAcceleration[] = {
    UMC_SAME_NAME(UNKNOWN),
    UMC_SAME_NAME(VC1_MC),
    UMC_SAME_NAME(VC1_VLD),
    UMC_SAME_NAME(MPEG2_VLD),
    UMC_SAME_NAME(H264_VLD),
  };
#endif
  static CodeStringTable StringsOfAudioType[] = {
    { UMC::UNDEF_AUDIO,         VM_STRING("UNDEF") },
    { UMC::PCM_AUDIO,           VM_STRING("PCM") },
    { UMC::LPCM_AUDIO,          VM_STRING("LPCM") },
    { UMC::AC3_AUDIO,           VM_STRING("AC3") },
    { UMC::TWINVQ_AUDIO,        VM_STRING("TWINVQ") },
    { UMC::MPEG1_AUDIO,         VM_STRING("MPEG1") },
    { UMC::MPEG2_AUDIO,         VM_STRING("MPEG2") },
    { UMC::MPEG_AUDIO_LAYER1,   VM_STRING("MPEGxL1") },
    { UMC::MPEG_AUDIO_LAYER2,   VM_STRING("MPEGxL2") },
    { UMC::MPEG_AUDIO_LAYER3,   VM_STRING("MPEGxL3") },
    { UMC::MP1L1_AUDIO,         VM_STRING("MP1L1") },
    { UMC::MP1L2_AUDIO,         VM_STRING("MP1L2") },
    { UMC::MP1L3_AUDIO,         VM_STRING("MP1L3") },
    { UMC::MP2L1_AUDIO,         VM_STRING("MP2L1") },
    { UMC::MP2L2_AUDIO,         VM_STRING("MP2L2") },
    { UMC::MP2L3_AUDIO,         VM_STRING("MP2L3") },
    { UMC::VORBIS_AUDIO,        VM_STRING("VORBIS") },
    { UMC::AAC_AUDIO,           VM_STRING("AAC") },
//    { UMC::AAC_FMT_UNDEF,        VM_STRING("AAC_FMT_UNDEF") },
//    { UMC::AAC_FMT_RAW,          VM_STRING("AAC_FMT_RAW") },
//    { UMC::AAC_FMT_EX_GA,        VM_STRING("AAC_FMT_EX_GA") },
    { UMC::AAC_MPEG4_STREAM,    VM_STRING("AAC_MP4") },
    { UMC::AMR_NB_AUDIO,        VM_STRING("ARM-NB") },
    { UMC::ALAW_AUDIO,          VM_STRING("A-LAW") },
    { UMC::MULAW_AUDIO,         VM_STRING("MU-LAW") },
  };

  static CodeStringTable StringsOfVideoType[] = {
    { UMC::UNDEF_VIDEO,         VM_STRING("UNDEF") },
    { UMC::UNCOMPRESSED_VIDEO,  VM_STRING("UNCOMP.") },
    { UMC::MPEG1_VIDEO,         VM_STRING("MPEG1") },
    { UMC::MPEG2_VIDEO,         VM_STRING("MPEG2") },
    { UMC::MPEG4_VIDEO,         VM_STRING("MPEG4") },
    { UMC::H261_VIDEO,          VM_STRING("H261") },
    { UMC::H263_VIDEO,          VM_STRING("H263") },
    { UMC::H264_VIDEO,          VM_STRING("H264") },
    { UMC::DIGITAL_VIDEO_SD,    VM_STRING("DV_SD") },
    { UMC::DIGITAL_VIDEO_HD,    VM_STRING("DV_HD") },
    { UMC::WMV_VIDEO,           VM_STRING("WMV") },
    { UMC::MJPEG_VIDEO,         VM_STRING("MJPEG") },
    { UMC::VC1_VIDEO,           VM_STRING("VC1_VIDEO") },
    { UMC::DIGITAL_VIDEO_50,    VM_STRING("DV_50") },
    { UMC::MPEG2_VIDEO,         VM_STRING("M2") }, // for umc_video_enc_con
    { UMC::MPEG4_VIDEO,         VM_STRING("M4") }, // for umc_video_enc_con
    { UMC::VC1_VIDEO,           VM_STRING("VC1") }, // for umc_video_enc_con
    { UMC::AVS_VIDEO,           VM_STRING("AVS") },
    { UMC::VP8_VIDEO,           VM_STRING("VP8") },
    { UMC::HEVC_VIDEO,          VM_STRING("HEVC") },
  };

  static CodeStringTable StringsOfVideoSubType[] = {
    { UMC::UNDEF_VIDEO_SUBTYPE,        VM_STRING("UNDEF") },
    { UMC::MPEG4_VIDEO_DIVX5,          VM_STRING("DIVX5") },
    { UMC::MPEG4_VIDEO_QTIME,          VM_STRING("QTIME") },
    { UMC::DIGITAL_VIDEO_TYPE_1,       VM_STRING("DV_TYPE1") },
    { UMC::DIGITAL_VIDEO_TYPE_2,       VM_STRING("DV_TYPE2") },
    { UMC::MPEG4_VIDEO_DIVX3,          VM_STRING("DIVX3") },
    { UMC::MPEG4_VIDEO_DIVX4,          VM_STRING("DIVX4") },
    { UMC::MPEG4_VIDEO_XVID,           VM_STRING("XVID") },
    { UMC::AVC1_VIDEO,                 VM_STRING("AVC1") },
    { UMC::H263_VIDEO_SORENSON,        VM_STRING("H263_SORENSON") },
    { UMC::VC1_VIDEO_RCV,              VM_STRING("VC1_RCV") },
    { UMC::VC1_VIDEO_VC1,              VM_STRING("VC1") },
    { UMC::WVC1_VIDEO,                 VM_STRING("WVC1") },
    { UMC::WMV3_VIDEO,                 VM_STRING("WMV3") },
  };

  static CodeStringTable StringsOfStreamType[] = {
    { UMC::UNDEF_STREAM,               VM_STRING("UNDEF") },
    { UMC::AVI_STREAM,                 VM_STRING("AVI") },
    { UMC::MP4_ATOM_STREAM,            VM_STRING("MP4ATOM") },
    { UMC::ASF_STREAM,                 VM_STRING("ASF") },
    { UMC::H26x_PURE_VIDEO_STREAM,     VM_STRING("H26x") },
    { UMC::H261_PURE_VIDEO_STREAM,     VM_STRING("H261PV") },
    { UMC::H263_PURE_VIDEO_STREAM,     VM_STRING("H263PV") },
    { UMC::H264_PURE_VIDEO_STREAM,     VM_STRING("H264PV") },
    { UMC::MPEGx_SYSTEM_STREAM,        VM_STRING("MPEGx") },
    { UMC::MPEG1_SYSTEM_STREAM,        VM_STRING("MPEG1") },
    { UMC::MPEG2_SYSTEM_STREAM,        VM_STRING("MPEG2") },
    { UMC::MPEG4_SYSTEM_STREAM,        VM_STRING("MPEG4") },
    { UMC::MPEGx_PURE_VIDEO_STREAM,    VM_STRING("MPEGxPV") },
    { UMC::MPEGx_PURE_AUDIO_STREAM,    VM_STRING("MPEGxPA") },
    { UMC::MPEGx_PES_PACKETS_STREAM,   VM_STRING("MPEGxPES") },
    { UMC::MPEGx_PROGRAMM_STREAM,      VM_STRING("MPEGxP") },
    { UMC::MPEGx_TRANSPORT_STREAM,     VM_STRING("MPEGxT") },
    { UMC::MPEG1_PURE_VIDEO_STREAM,    VM_STRING("MPEG1PV") },
    { UMC::MPEG1_PURE_AUDIO_STREAM,    VM_STRING("MPEG1PA") },
    { UMC::MPEG1_PES_PACKETS_STREAM,   VM_STRING("MPEG1PES") },
    { UMC::MPEG1_PROGRAMM_STREAM,      VM_STRING("MPEG1P") },
    { UMC::MPEG2_PURE_VIDEO_STREAM,    VM_STRING("MPEG2PV") },
    { UMC::MPEG2_PURE_AUDIO_STREAM,    VM_STRING("MPEG2PA") },
    { UMC::MPEG2_PES_PACKETS_STREAM,   VM_STRING("MPEG2PES") },
    { UMC::MPEG2_PROGRAMM_STREAM,      VM_STRING("MPEG2P") },
    { UMC::MPEG2_TRANSPORT_STREAM,     VM_STRING("MPEG2T") },
    { UMC::MPEG2_TRANSPORT_STREAM_TTS, VM_STRING("MPEG2TTS") },
    { UMC::MPEG4_PURE_VIDEO_STREAM,    VM_STRING("MPEG4PV") },
    { UMC::VC1_PURE_VIDEO_STREAM,      VM_STRING("VC1PV") },
    { UMC::WAVE_STREAM,                VM_STRING("WAVE") },
    { UMC::AVS_PURE_VIDEO_STREAM,      VM_STRING("AVSPV") },
    { UMC::FLV_STREAM,                 VM_STRING("FLV") },
    { UMC::IVF_STREAM,                 VM_STRING("IVF") },
    //    { UMC::WEB_CAM_STREAM,                VM_STRING("WEB_CAM") },
  };

  //////////////////////////////////////////////////////////////////////////////

  static const vm_char* umcCodeToString(CodeStringTable *table,
                                        int table_size,
                                        int code)
  {
    int i;
    for (i = 0; i < table_size; i++) {
      if (table[i].code == code) {
        return table[i].string;
      }
    }
    return VM_STRING("UNDEF");
  }

  static Status umcStringToCode(CodeStringTable *table,
                                int table_size,
                                const vm_char* string,
                                int *code)
  {
    int i;
    for (i = 0; i < table_size; i++) {
      if (!vm_string_stricmp(table[i].string, string)) {
        *code = table[i].code;
        return UMC_OK;
      }
    }
    return UMC_ERR_INVALID_PARAMS;
  }

  //////////////////////////////////////////////////////////////////////////////

#define UMC_CODE_TO_STRING(table, code) \
  umcCodeToString(table, sizeof(table)/sizeof(CodeStringTable), code)

#define UMC_STRING_TO_CODE(table, string, code, type) \
  { \
    Status res; \
    int tmp_code = -1; \
    res = umcStringToCode(table, sizeof(table)/sizeof(CodeStringTable), string, &tmp_code); \
    if (UMC_OK == res) *code = (type)tmp_code; \
    return res; \
  }

  //////////////////////////////////////////////////////////////////////////////

  const vm_char* GetErrString(Status code)
  {
    return UMC_CODE_TO_STRING(StringsOfError, code);
  }
  const vm_char* GetStreamTypeString(SystemStreamType code)
  {
    return UMC_CODE_TO_STRING(StringsOfStreamType, code);
  }
  const vm_char* GetFormatTypeString(ColorFormat code)
  {
    return UMC_CODE_TO_STRING(StringsOfFormatType, code);
  }
  const vm_char* GetAudioTypeString(AudioStreamType code)
  {
    return UMC_CODE_TO_STRING(StringsOfAudioType, code);
  }
  const vm_char* GetVideoTypeString(VideoStreamType code)
  {
    return UMC_CODE_TO_STRING(StringsOfVideoType, code);
  }
  const vm_char* GetVideoSubTypeString(VideoStreamSubType code)
  {
      return UMC_CODE_TO_STRING(StringsOfVideoSubType, code);
  }
  const vm_char* GetVideoRenderTypeString(VideoRenderType code)
  {
    return UMC_CODE_TO_STRING(StringsOfVideoRenderType, code);
  }
  const vm_char* GetAudioRenderTypeString(AudioRenderType code)
  {
    return UMC_CODE_TO_STRING(StringsOfAudioRenderType, code);
  }

  Status GetFormatType(const vm_char *string, ColorFormat *code)
  {
    UMC_STRING_TO_CODE(StringsOfFormatType, string, code, ColorFormat);
  }
  Status GetStreamType(const vm_char *string, SystemStreamType *code)
  {
    UMC_STRING_TO_CODE(StringsOfStreamType, string, code, SystemStreamType);
  }
  Status GetAudioType(const vm_char *string, AudioStreamType *code)
  {
    UMC_STRING_TO_CODE(StringsOfAudioType, string, code, AudioStreamType);
  }
  Status GetVideoType(const vm_char *string, VideoStreamType *code)
  {
    UMC_STRING_TO_CODE(StringsOfVideoType, string, code, VideoStreamType);
  }
  Status GetVideoSubType(const vm_char *string, VideoStreamSubType *code)
  {
      UMC_STRING_TO_CODE(StringsOfVideoSubType, string, code, VideoStreamSubType);
  }
  Status GetAudioRenderType(const vm_char *string, AudioRenderType *code)
  {
    UMC_STRING_TO_CODE(StringsOfAudioRenderType, string, code, AudioRenderType);
  }
  Status GetVideoRenderType(const vm_char *string, VideoRenderType *code)
  {
    UMC_STRING_TO_CODE(StringsOfVideoRenderType, string, code, VideoRenderType);
  }
  const vm_char* GetVideoAccelerationString(VideoAccelerationProfile code)
  {
    return UMC_CODE_TO_STRING(StringsOfVideoAcceleration, code);
  }
  Status GetVideoAccelerationProfile(const vm_char *string, VideoAccelerationProfile *code)
  {
    UMC_STRING_TO_CODE(StringsOfVideoAcceleration, string, code, VideoAccelerationProfile);
  }

}; // namespace UMC
