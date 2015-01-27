/* ****************************************************************************** *\

Copyright (C) 2008-2012 Intel Corporation.  All rights reserved.

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

File Name: mfx_enc_common.h

\* ****************************************************************************** */
#ifndef _MFX_ENC_COMMON_H_
#define _MFX_ENC_COMMON_H_

#include "ippdefs.h"
#include "mfxdefs.h"
#include "mfxstructures.h"
#include "umc_structures.h"
#include "mfx_common.h"
#include "mfx_common_int.h"

class InputSurfaces
{
private:
    bool                    m_bOpaq;
    bool                    m_bSysMemFrames;
    VideoCORE*              m_pCore;
    mfxFrameAllocRequest    m_request;
    mfxFrameAllocResponse   m_response;
    bool                    m_bInitialized;
    mfxFrameInfo            m_Info;


public:
    InputSurfaces(VideoCORE* pCore):
      m_bOpaq(false),
          m_bSysMemFrames(false),
          m_pCore(pCore),
          m_bInitialized (false)
      {
          memset(&m_request,  0, sizeof(mfxFrameAllocRequest));
          memset(&m_response, 0, sizeof (mfxFrameAllocResponse));  
          memset(&m_Info,0,sizeof(mfxFrameInfo));
      }
     virtual ~InputSurfaces()
     {
         Close();     
     }
     mfxStatus Reset(mfxVideoParam *par, mfxU16 numFrameMin);

     mfxStatus Close();
    

     inline bool isOpaq() {return  m_bOpaq;}
     inline bool isSysMemFrames () {return m_bSysMemFrames;}
     inline bool CheckInputFrame(mfxFrameInfo* pFrameInfo)
     {
         return (m_Info.Width == pFrameInfo->Width && m_Info.Height == pFrameInfo->Height);     
     }

     inline mfxFrameSurface1 *GetOriginalSurface(mfxFrameSurface1 *surface)
     {
         return m_bOpaq ? m_pCore->GetNativeSurface(surface) : surface;
     }

     inline mfxFrameSurface1 *GetOpaqSurface(mfxFrameSurface1 *surface)
     {
         return m_bOpaq ? m_pCore->GetOpaqSurface(surface->Data.MemId) : surface;
     } 
};

//----MFX data -> UMC data--------------------------------------------
Ipp8u  CalculateMAXBFrames (mfxU8 GopRefDist);
Ipp16u CalculateUMCGOPLength (mfxU16 GOPSize, mfxU8 targetUsage);

bool SetPROParameters (mfxU8 TargetUsages,Ipp8u &MESpeed, bool &UseFB, bool &FastFB,
                       bool &bIntensityCompensation, bool &bChangeInterpolationType,
                       bool &bChangeVLCTables,
                       bool &bTrellisQuantization, bool &bUsePadding,
                       bool &bVSTransform, bool &deblocking, mfxU8 &smoothing, bool &fastUVMC);
bool SetUFParameters(mfxU8 TargetUsages, bool& mixed,Ipp32u& twoRef );

Ipp32u CalculateUMCBitrate(mfxU16    TargetKbps);

Ipp64f CalculateUMCFramerate(mfxU32 FrameRateExtN, mfxU32 FrameRateExtD);
void CalculateMFXFramerate(Ipp64f framerate, mfxU32* FrameRateExtN, mfxU32* FrameRateExtD);
void ConvertFrameRateMPEG2(mfxU32 FrameRateExtD, mfxU32 FrameRateExtN, mfxI32 &frame_rate_code, mfxI32 &frame_rate_extension_n, mfxI32 &frame_rate_extension_d);
//void ConvertFrameRateMPEG2(mfxU32 FrameRateExtD, mfxU32 FrameRateExtN, mfxI32 &frame_rate_code, mfxI32 &frame_rate_extension_n, mfxI32 &frame_rate_extension_d);

mfxStatus CheckFrameRateMPEG2(mfxU32 &FrameRateExtD, mfxU32 &FrameRateExtN);
mfxStatus CheckAspectRatioMPEG2 (mfxU16 &aspectRatioW, mfxU16 &aspectRatioH, mfxU32 frame_width, mfxU32 frame_heigth, mfxU16 cropW, mfxU16 cropH);


bool IsFrameRateMPEG2Supported(mfxU32 FrameRateExtD, mfxU32 FrameRateExtN);
bool IsAspectRatioMPEG2Supported (mfxU32 aspectRatioW, mfxU32 aspectRatioH, mfxU32 frame_width, mfxU32 frame_heigth, mfxU32 cropW, mfxU32 cropH);
mfxU8 GetAspectRatioCode (mfxU32 dispAspectRatioW, mfxU32 dispAspectRatioH);
bool RecalcFrameMPEG2Rate (mfxU32 FrameRateExtD, mfxU32 FrameRateExtN, mfxU32 &OutFrameRateExtD, mfxU32 &OutFrameRateExtN);
mfxU32 TranslateMfxFRCodeMPEG2(mfxFrameInfo *info, mfxU32 *codeN, mfxU32* codeD); // returns mpeg2 fr code

mfxExtBuffer*       GetExtBuffer       (mfxExtBuffer** ebuffers, mfxU32 nbuffers, mfxU32 BufferId);
mfxExtCodingOption* GetExtCodingOptions(mfxExtBuffer** ebuffers, mfxU32 nbuffers);
mfxExtVideoSignalInfo* GetExtVideoSignalInfo(mfxExtBuffer** ebuffers, mfxU32 nbuffers);

mfxStatus CheckExtVideoSignalInfo(mfxExtVideoSignalInfo * videoSignalInfo);

inline mfxI32 min4(mfxI32 a, mfxI32 b,mfxI32 c,mfxI32 d)
{
    if (a>b)
    {
        if (c<d)
        {
            return (b<c)? b:c;
        }
        else
        {
             return (b<d)? b:d;

        }

    }
    else
    {
        if (c<d)
        {
            return (a<c)? a:c;
        }
        else
        {
             return (a<d)? a:d;

        }
    }
}

UMC::FrameType GetFrameType (mfxU16 FrameOrder, mfxInfoMFX* info);

//----UMC data -> MFX data--------------------------------------------
mfxU16 CalculateMFXGOPLength (Ipp16u GOPSize);
mfxU8 CalculateGopRefDist(mfxU8 BNum);

//----------------------------------------------
//mfxI16    GetExBufferIndex        (mfxFrameCUC *cuc, mfxU32 cucID);
//----------------------------------------------
inline bool isIntra(mfxU8 FrameType)
{
    return (FrameType & MFX_FRAMETYPE_I);
}
mfxStatus CopyFrame(mfxFrameData *pIn, mfxFrameData *pOut, mfxFrameInfo *pInfoIn, mfxFrameInfo *pInfoOut=0);
void CorrectProfileLevelMpeg2(mfxU16 &profile, mfxU16 & level, mfxU32 w, mfxU32 h, mfxF64 frame_rate, mfxU32 bitrate);


inline mfxI64 CalcDTSForRefFrameMpeg2(mfxI64 PTS, mfxI32 lastRefDist, mfxU32 maxRefDist, mfxF64 frameRate)
{
    return (maxRefDist == 1 || PTS == -1) ? PTS : PTS - (mfxI64)((1.0/frameRate)*(lastRefDist > 0 ? lastRefDist : 1)*90000);
}
inline mfxI64 CalcDTSForNonRefFrameMpeg2(mfxI64 PTS)
{
    return PTS;
}

struct Rational {mfxU64 n, d;};

#define D3DFMT_NV12 (D3DFORMAT)(MFX_MAKEFOURCC('N', 'V', '1', '2'))
#define D3DDDIFMT_NV12 (D3DDDIFORMAT)(MFX_MAKEFOURCC('N', 'V', '1', '2'))
#define D3DDDIFMT_YU12 (D3DDDIFORMAT)(MFX_MAKEFOURCC('Y', 'U', '1', '2'))

#endif //_MFX_ENC_COMMON_H_
