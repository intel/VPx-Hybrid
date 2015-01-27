/* ****************************************************************************** *\

Copyright (C) 2006 - 2014 Intel Corporation.  All rights reserved.

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

File Name: umc_va_base.h

\* ****************************************************************************** */

#ifndef __UMC_VA_BASE_H__
#define __UMC_VA_BASE_H__

#ifndef UMC_RESTRICTED_CODE_VA

#include <stdio.h>
#include <vector>
#include "vm_types.h"
#include "vm_debug.h"
#include "vm_time.h"
#include "ipps.h"

#if defined (UMC_VA) || defined (MFX_VA)

#ifndef UMC_VA
#   define UMC_VA
#endif

#ifndef MFX_VA
#   define MFX_VA
#endif

#if defined(LINUX32) || defined(LINUX64) || defined(LINUX_VA_EMULATION)
#   ifndef UMC_VA_LINUX
#       define UMC_VA_LINUX          // HW acceleration through Linux VA
#   endif
#elif defined(_WIN32) || defined(_WIN64)
#   ifndef UMC_VA_DXVA
#       define UMC_VA_DXVA           // HW acceleration through DXVA
#   endif
#elif defined(__APPLE__) 
#   ifndef UMC_VA_OSX
#      define UMC_VA_OSX
#   endif
#else
    #error unsupported platform
#endif

#endif //  defined (MFX_VA) || defined (UMC_VA)

#ifdef UMC_VA_SIMULATION
    #define UMC_VA_DXVA
#endif

#ifdef UMC_STREAM_ANALYZER
    #define UMC_VA_DXVA
    #define UMC_VA_SIMULATION
#endif

#include "ipps.h"
#include "vm_types.h"

#ifdef  __cplusplus
#include "umc_structures.h"
#include "umc_dynamic_cast.h"

#pragma warning(disable : 4201)

#ifdef UMC_VA_LINUX
#include <va/va.h>//aya
#include <va/va_dec_vp8.h>
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p) (p);
#endif
#endif


#ifdef UMC_VA_DXVA
#include <windows.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <initguid.h>
#endif

#ifdef UMC_VA_DXVA
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>
#endif


#if defined(LINUX32) || defined(LINUX64) || defined(__APPLE__)
#ifndef GUID_TYPE_DEFINED
typedef struct {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;
#define GUID_TYPE_DEFINED
#endif
#endif


namespace UMC
{

#define VA_COMBINATIONS(CODEC) \
    CODEC##_VLD     = VA_##CODEC | VA_VLD

enum VideoAccelerationProfile
{
    UNKNOWN         = 0,

    // Codecs
    VA_CODEC        = 0x00ff,
    VA_MPEG2        = 0x0001,
    VA_MPEG4        = 0x0002,
    VA_H264         = 0x0003,
    VA_VC1          = 0x0004,
    VA_JPEG         = 0x0005,
    VA_VP8          = 0x0006,
    VA_H265            = 0x0007,

    // Entry points
    VA_ENTRY_POINT  = 0xff00,
    VA_SW           = 0x0100, // no acceleration
    VA_MC           = 0x0200,
    VA_IT           = 0x0300,
    VA_VLD          = 0x0400,
    VA_DEBLOCK      = 0x1000,

    VA_PROFILE                  = 0xf000,
    VA_PROFILE_SVC_HIGH         = 0x2000,
    VA_PROFILE_SVC_BASELINE     = 0x3000,
    VA_PROFILE_MVC              = 0x4000,
    VA_PROFILE_MVC_MV           = 0x5000,
    VA_PROFILE_MVC_STEREO       = 0x6000,
    VA_PROFILE_MVC_STEREO_PROG  = 0x7000,
    VA_PROFILE_INTEL            = 0x8000,

    // configurations
    VA_CONFIGURATION    = 0x00ff0000,
    VA_LONG_SLICE_MODE  = 0x00010000,
    VA_SHORT_SLICE_MODE = 0x00020000,
    VA_ANY_SLICE_MODE   = 0x00030000,

    VC1_MC          = VA_VC1 | VA_MC,
    MPEG2_IT        = VA_MPEG2 | VA_IT,

    MPEG2_VLD       = VA_MPEG2 | VA_VLD,
    H264_VLD        = VA_H264 | VA_VLD,
    H265_VLD        = VA_H265 | VA_VLD,
    VC1_VLD         = VA_VC1 | VA_VLD,
    JPEG_VLD        = VA_JPEG | VA_VLD,
    VP8_VLD         = VA_VP8 | VA_VLD,
    HEVC_VLD        = VA_H265 | VA_VLD,

    H264_VLD_MVC            = VA_H264 | VA_VLD | VA_PROFILE_MVC,
    H264_VLD_SVC_BASELINE   = VA_H264 | VA_VLD | VA_PROFILE_SVC_BASELINE,
    H264_VLD_SVC_HIGH       = VA_H264 | VA_VLD | VA_PROFILE_SVC_HIGH,

    H264_VLD_MVC_MULTIVIEW      = VA_H264 | VA_VLD | VA_PROFILE_MVC_MV,
    H264_VLD_MVC_STEREO         = VA_H264 | VA_VLD | VA_PROFILE_MVC_STEREO,
    H264_VLD_MVC_STEREO_PROG    = VA_H264 | VA_VLD | VA_PROFILE_MVC_STEREO_PROG
};

#define MAX_BUFFER_TYPES    32
enum VideoAccelerationPlatform
{
    VA_UNKNOWN_PLATFORM = 0,

    VA_PLATFORM  = 0x0f0000,
    VA_DXVA1     = 0x010000,
    VA_DXVA2     = 0x020000,
    VA_LINUX     = 0x030000,
    VA_SOFTWARE  = 0x040000,

    // Flags
    VA_SIMULATOR = 0x100000,
    VA_ANALYZER  = 0x200000
};

enum VideoAccelerationHW
{
    VA_HW_UNKNOWN = 0,

    VA_HW_LAKE  = 0x010000,
    VA_HW_LRB   = 0x020000,
    VA_HW_SNB   = 0x030000,
    VA_HW_IVB   = 0x040000,

    VA_HW_HSW   = 0x050000,
    VA_HW_HSW_ULT   = 0x050001,

    VA_HW_VLV       = 0x060000,

    VA_HW_BDW       = 0x700000,
    VA_HW_SCL       = 0x800000
};

class VideoData;
class UMCVACompBuffer;
class VACompBuffer;
class ProtectedVA;

// Convert codec type from enum VideoStreamType to enum VideoAccelerationProfile
VideoAccelerationProfile VideoType2VAProfile(VideoStreamType video_type);
const vm_char* GetVideoAccelerationString(VideoAccelerationProfile code);
Status GetVideoAccelerationProfile(const vm_char *Name, VideoAccelerationProfile *Code);

inline bool IsVaSvcProfile(Ipp32u profile)
{
    profile = profile & VA_PROFILE;
    return (profile == VA_PROFILE_SVC_HIGH) || (profile == VA_PROFILE_SVC_BASELINE);
}

inline bool IsVaMvcProfile(Ipp32u profile)
{
    profile = profile & VA_PROFILE;
    return (profile == VA_PROFILE_MVC) || (profile == VA_PROFILE_MVC_MV) || (profile == VA_PROFILE_MVC_STEREO) || (profile == VA_PROFILE_MVC_STEREO_PROG);
}

enum eUMC_DirectX_Status
{
    E_FRAME_LOCKED = 0xC0262111
};

enum eUMC_VA_Status
{
    UMC_ERR_DEVICE_FAILED         = -2000,
    UMC_ERR_DEVICE_LOST           = -2001,
    UMC_ERR_FRAME_LOCKED          = -2002
};

///////////////////////////////////////////////////////////////////////////////////

class VideoAcceleratorParams
{
    DYNAMIC_CAST_DECL_BASE(VideoAcceleratorParams);
public:
    VideoAcceleratorParams(void)
    {
        m_pVideoStreamInfo = NULL;
        m_iNumberSurfaces  = 0; // 0 means use default value
        m_SurfaceWidth     = 0; // 0 means use default value
        m_SurfaceHeight    = 0; // 0 means use default value
    }
    virtual ~VideoAcceleratorParams(void){}

    VideoStreamInfo *m_pVideoStreamInfo;
    Ipp32s          m_iNumberSurfaces;
    Ipp32s          m_SurfaceWidth;
    Ipp32s          m_SurfaceHeight;

    // if extended surfaces exist
    bool   isExt;
    void** m_surf;
};


class VideoAccelerator
{
    DYNAMIC_CAST_DECL_BASE(VideoAccelerator);
public:
    VideoAccelerator() :
        m_Profile(UNKNOWN),
        m_Platform(VA_UNKNOWN_PLATFORM),
        m_HWPlatform(VA_HW_UNKNOWN),
        m_bH264ShortSlice(false),
        m_bH264MVCSupport(false),
        m_isUseStatuReport(true)
    {
    }

    virtual ~VideoAccelerator()
    {
        Close();
    }

    virtual Status FindConfiguration(VideoStreamInfo *pVideoInfo) = 0; // Check configuration
    virtual Status Init(VideoAcceleratorParams* pInfo) = 0; // Initilize and allocate all resources
    virtual Status GetInfo(VideoAcceleratorParams* /*pInfo*/) { return UMC_ERR_UNSUPPORTED; }
    virtual Status Close(void);
    virtual Status Reset(void);

    virtual Status BeginFrame   (Ipp32s  index) = 0; // Begin decoding for specified index
    virtual void*  GetCompBuffer(Ipp32s            buffer_type,
                                 UMCVACompBuffer** buf   = NULL,
                                 Ipp32s            size  = -1,
                                 Ipp32s            index = -1) = 0; // request buffer
    virtual Status Execute      (void) = 0;          // execute decoding
    virtual Status ExecuteExtensionBuffer(void * buffer) = 0;
    virtual Status ExecuteStatusReportBuffer(void * buffer, Ipp32s size) = 0;
    virtual Status SyncTask(Ipp32s index) = 0;
    virtual Status QueryTaskStatus(Ipp32s index, void * status, void * error) = 0;
    virtual Status ReleaseBuffer(Ipp32s type) = 0;   // release buffer
    virtual Status EndFrame     (void * handle = 0) = 0;          // end frame
    // output frame (with reordering)
    virtual Status DisplayFrame (Ipp32s index, VideoData *pOutputVideoData = NULL) = 0;

    bool IsSimulate() const { return ((m_Platform & VA_SIMULATOR) != 0); }
    virtual bool IsIntelCustomGUID() const = 0;
    /* TODO: is used on Linux only? On Linux there are isues with signed/unsigned return value. */
    virtual Ipp32s GetSurfaceID(Ipp32s idx) { return idx; }

    bool IsLongSliceControl() const { return (!m_bH264ShortSlice); };
    bool IsMVCSupport() const {return m_bH264MVCSupport; };
    bool IsUseStatusReport() { return m_isUseStatuReport; }
    void SetStatusReportUsing(bool isUseStatuReport) { m_isUseStatuReport = isUseStatuReport; }

    virtual void GetVideoDecoder(void **handle) = 0;

    VideoAccelerationProfile    m_Profile;          // entry point
    VideoAccelerationPlatform   m_Platform;         // DXVA, LinuxVA, etc
    VideoAccelerationHW         m_HWPlatform;

protected:

    bool            m_bH264ShortSlice;
    bool            m_bH264MVCSupport;
    bool            m_isUseStatuReport;
};

///////////////////////////////////////////////////////////////////////////////////

class UMCVACompBuffer //: public MediaData
{
    friend class VACompBuffer;

public:
    UMCVACompBuffer()
    {
        type = -1;
        BufferSize = 0;
        DataSize = 0;
        ptr = NULL;
        PVPState = NULL;

        FirstMb = -1;
        NumOfMB = -1;
        FirstSlice = -1;
    }
    virtual ~UMCVACompBuffer(){}

    // Set
    virtual Status SetBufferPointer(Ipp8u *_ptr, size_t bytes)
    {
        ptr = _ptr;
        BufferSize = (Ipp32s)bytes;
        return UMC_OK;
    }
    virtual void SetDataSize(Ipp32s size);
    virtual void SetNumOfItem(Ipp32s num);
    virtual Status SetPVPState(void *buf, Ipp32u size);

    // Get
    Ipp32s  GetType()       const { return type; }
    void*   GetPtr()        const { return ptr; }
    Ipp32s  GetBufferSize() const { return BufferSize; }
    Ipp32s  GetDataSize()   const { return DataSize; }
    void*   GetPVPStatePtr()const { return PVPState; }
    Ipp32s   GetPVPStateSize()const { return (NULL == PVPState ? 0 : sizeof(PVPStateBuf)); }

    // public fields
    Ipp32s      FirstMb;
    Ipp32s      NumOfMB;
    Ipp32s      FirstSlice;
    Ipp32s      type;

protected:
    Ipp8u       PVPStateBuf[16];
    void*       ptr;
    void*       PVPState;
    Ipp32s      BufferSize;
    Ipp32s      DataSize;
};

} // namespace UMC

#endif // __cplusplus

#pragma warning(default : 4201)

#endif // UMC_RESTRICTED_CODE_VA
#endif // __UMC_VA_BASE_H__
