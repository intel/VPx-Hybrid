/* ****************************************************************************** *\

Copyright (C) 2012-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_vp8_encode_ddi_hw.h

\* ****************************************************************************** */
#include "mfx_common.h"
#if defined(MFX_ENABLE_VP8_VIDEO_ENCODE_HW) && defined(MFX_VA)

#ifndef _MFX_VP8_ENCODE_DDI_HW_H_
#define _MFX_VP8_ENCODE_DDI_HW_H_

#include <vector>
#include "mfx_vp8_encode_utils_hw.h"
#include "mfx_platform_defs.h"

#if defined (MFX_VA_LINUX)
#include "mfx_vp8_vaapi_ext.h"
#endif

namespace MFX_VP8ENC
{

#if defined (MFX_VA_LINUX)
    typedef struct tagENCODE_CAPS_VP8 
    { 
        union { 
            struct { 
                UINT CodingLimitSet      : 1; 
                UINT Color420Only        : 1; 
                UINT SegmentationAllowed : 1; 
                UINT CoeffPartitionLimit : 2; 
                UINT FrameLevelRateCtrl  : 1; 
                UINT BRCReset            : 1; 
                UINT                     : 25; 
            }; 
            UINT CodingLimits;
        }; 

        union { 
            struct { 
                BYTE EncodeFunc    : 1; 
                BYTE HybridPakFunc : 1; // Hybrid Pak function for BDW 
                BYTE               : 6; 
            }; 
            BYTE CodingFunction; 
        }; 

        UINT MaxPicWidth; 
        UINT MaxPicHeight; 
    } ENCODE_CAPS_VP8;

    typedef struct
    {
        SHORT  x;
        SHORT  y;
    } ENCODE_MV_DATA;
#endif

    typedef VAEncMbDataLayout MBDATA_LAYOUT;

    class DriverEncoder;

    mfxStatus QueryHwCaps(mfxCoreInterface * pCore, ENCODE_CAPS_VP8 & caps);
    mfxStatus CheckVideoParam(mfxVideoParam const & par, ENCODE_CAPS_VP8 const &caps);

    DriverEncoder* CreatePlatformVp8Encoder();

    class TaskHybridDDI;

    class DriverEncoder
    {
    public:

        virtual ~DriverEncoder(){}

        virtual mfxStatus CreateAuxilliaryDevice(
                        mfxCoreInterface * pCore,
                        GUID               guid,
                        mfxU32             width,
                        mfxU32             height) = 0;

        virtual
        mfxStatus CreateAccelerationService(
            mfxVideoParam const & par) = 0;

        virtual
        mfxStatus Reset(
            mfxVideoParam const & par) = 0;

        virtual 
        mfxStatus Register(
            mfxFrameAllocResponse & response,
            D3DDDIFORMAT            type) = 0;

        virtual 
        mfxStatus Execute(
            TaskHybridDDI const &task, 
            mfxHDL surface) = 0;

        virtual
        mfxStatus QueryCompBufferInfo(
            D3DDDIFORMAT           type,
            mfxFrameAllocRequest & request,
            mfxU32 frameWidth,
            mfxU32 frameHeight) = 0;

        virtual
        mfxStatus QueryEncodeCaps(
            ENCODE_CAPS_VP8 & caps) = 0;

        virtual
        mfxStatus QueryMBLayout(
            MBDATA_LAYOUT &layout) = 0;

        virtual
        mfxStatus QueryStatus( 
            Task & task ) = 0;

        virtual
            mfxU32 GetReconSurfFourCC() = 0;

        virtual
        mfxStatus Destroy() = 0;
    };

#if defined (MFX_VA_LINUX)

#include <va/va_enc_vp8.h>
    
#define MFX_DESTROY_VABUFFER(vaBufferId, vaDisplay)    \
do {                                               \
    if ( vaBufferId != VA_INVALID_ID)              \
    {                                              \
        vaDestroyBuffer(vaDisplay, vaBufferId);    \
        vaBufferId = VA_INVALID_ID;                \
    }                                              \
} while (0)

#define D3DDDIFMT_INTELENCODE_BITSTREAMDATA     (D3DDDIFORMAT)164
#define D3DDDIFMT_INTELENCODE_MBDATA            (D3DDDIFORMAT)165
#define D3DDDIFMT_INTELENCODE_SEGMENTMAP        (D3DDDIFORMAT)178
#define D3DDDIFMT_INTELENCODE_COEFFPROB         (D3DDDIFORMAT)179
#define D3DDDIFMT_INTELENCODE_DISTORTIONDATA    (D3DDDIFORMAT)180

    enum {
        MFX_FOURCC_VP8_NV12    = MFX_MAKEFOURCC('V','P','8','N'),
        MFX_FOURCC_VP8_MBDATA  = MFX_MAKEFOURCC('V','P','8','M'),
        MFX_FOURCC_VP8_SEGMAP  = MFX_MAKEFOURCC('V','P','8','S'),
    };

    /* Convert MediaSDK into DDI */

    void FillSpsBuffer(mfxVideoParam const & par, 
        VAEncSequenceParameterBufferVP8 & sps);

    mfxStatus FillPpsBuffer(Task const &TaskHybridDDI, mfxVideoParam const & par, 
        VAEncPictureParameterBufferVP8 & pps);

    mfxStatus FillQuantBuffer(Task const &TaskHybridDDI, mfxVideoParam const & par,
        VAQMatrixBufferVP8 & quant);

    typedef struct
    {
        VASurfaceID surface;
        mfxU32 number;
        mfxU32 idxBs;

    } ExtVASurface;

    class VAAPIEncoder : public DriverEncoder
    {
    public:
        VAAPIEncoder();

        virtual
        ~VAAPIEncoder();

        virtual
        mfxStatus CreateAuxilliaryDevice(
            mfxCoreInterface* core,
            GUID       guid,
            mfxU32     width,
            mfxU32     height);

        virtual
        mfxStatus CreateAccelerationService(
            mfxVideoParam const & par);

        virtual
        mfxStatus Reset(
            mfxVideoParam const & par);

        // empty  for Lin
        virtual
        mfxStatus Register(
            mfxMemId memId,
            D3DDDIFORMAT type);

        // 2 -> 1
        virtual
        mfxStatus Register(
            mfxFrameAllocResponse& response,
            D3DDDIFORMAT type);

        // (mfxExecuteBuffers& data)
        virtual
        mfxStatus Execute(
            TaskHybridDDI const &task, 
            mfxHDL surface);

        // recomendation from HW
        virtual
        mfxStatus QueryCompBufferInfo(
            D3DDDIFORMAT type,
            mfxFrameAllocRequest& request,
            mfxU32 frameWidth,
            mfxU32 frameHeight);

        virtual
        mfxStatus QueryEncodeCaps(
            ENCODE_CAPS_VP8& caps);

        virtual
        mfxStatus QueryMBLayout(
            MBDATA_LAYOUT &layout);

        virtual
        mfxStatus QueryStatus(
            Task & task);

        virtual
            mfxU32 GetReconSurfFourCC();

        virtual
        mfxStatus Destroy();

    private:
        VAAPIEncoder(const VAAPIEncoder&); // no implementation
        VAAPIEncoder& operator=(const VAAPIEncoder&); // no implementation

        mfxCoreInterface * m_pmfxCore;
        VP8MfxParam   m_video;
        MBDATA_LAYOUT m_layout;

        // encoder specific. can be encapsulated by auxDevice class
        VADisplay    m_vaDisplay;
        VAContextID  m_vaContextEncode;
        VAConfigID   m_vaConfig;

        // encode params (extended structures)
        VAEncSequenceParameterBufferVP8        m_sps;
        VAEncPictureParameterBufferVP8         m_pps;
        VAQMatrixBufferVP8                     m_quant;
        VAEncMiscParameterVP8HybridFrameUpdate m_frmUpdate;
        VAEncMiscParameterVP8SegmentMapParams  m_segMapPar;
        VAEncMiscParameterFrameRate            m_frameRate;

        // encode buffer to send vaRender()
        VABufferID m_spsBufferId;
        VABufferID m_ppsBufferId;
        VABufferID m_qMatrixBufferId;
        VABufferID m_frmUpdateBufferId;
        VABufferID m_segMapParBufferId;
        VABufferID m_frameRateBufferId;
        VABufferID m_rateCtrlBufferId;
        VABufferID m_hrdBufferId;
      
        std::vector<ExtVASurface> m_feedbackCache;
        std::vector<ExtVASurface> m_mbDataQueue;
        std::vector<ExtVASurface> m_reconQueue;
        std::vector<ExtVASurface> m_segMapQueue;
        
        static const mfxU32 MAX_CONFIG_BUFFERS_COUNT = 9; // sps, pps, quant, seg_map, per segment par, frame update data, frame rate, rate ctrl, hrd

        mfxU32 m_width;
        mfxU32 m_height;
        ENCODE_CAPS_VP8 m_caps;
        UMC::Mutex                      m_guard;
    };
#endif


}
#endif
#endif