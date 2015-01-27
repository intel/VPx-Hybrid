/* ****************************************************************************** *\

Copyright (C) 2011-2013 Intel Corporation.  All rights reserved.

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

File Name: mfxsvc.h

\* ****************************************************************************** */
#ifndef __MFXSVC_H__
#define __MFXSVC_H__

#include "mfxdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CodecProfile, CodecLevel */
enum {
    MFX_PROFILE_AVC_SCALABLE_BASELINE = 83,
    MFX_PROFILE_AVC_SCALABLE_HIGH = 86
};

/* Extended Buffer Ids */
enum {
    MFX_EXTBUFF_SVC_SEQ_DESC        =   MFX_MAKEFOURCC('S','V','C','D'),
    MFX_EXTBUFF_SVC_RATE_CONTROL    =   MFX_MAKEFOURCC('S','V','C','R'),
    MFX_EXTBUFF_SVC_TARGET_LAYER    =   MFX_MAKEFOURCC('S','V','C','T'),
    MFX_EXTBUFF_VPP_SVC_DOWNSAMPLING  = MFX_MAKEFOURCC('D','W','N','S')
};

typedef struct {
    mfxExtBuffer    Header;

    mfxU16  RateControlMethod;
    mfxU16  reserved1[10];


    mfxU16  NumLayers;
    struct mfxLayer {
        mfxU16  TemporalId;
        mfxU16  DependencyId;
        mfxU16  QualityId;
        mfxU16  reserved2[5];

        union{
            struct{
                mfxU32  TargetKbps;
                mfxU32  InitialDelayInKB;
                mfxU32  BufferSizeInKB;
                mfxU32  MaxKbps;
                mfxU32  reserved3[4];
            } CbrVbr;

            struct{
                mfxU16  QPI;
                mfxU16  QPP;
                mfxU16  QPB;
            }Cqp;

            struct{
                mfxU32  TargetKbps;
                mfxU32  Convergence;
                mfxU32  Accuracy;
            }Avbr;
        };
    }Layer[1024];
} mfxExtSVCRateControl;

typedef struct {
    mfxU16    Active;

    mfxU16    Width;
    mfxU16    Height;

    mfxU16    CropX;
    mfxU16    CropY;
    mfxU16    CropW;
    mfxU16    CropH;

    mfxU16    RefLayerDid;
    mfxU16    RefLayerQid;

    mfxU16    GopPicSize;
    mfxU16    GopRefDist;
    mfxU16    GopOptFlag;
    mfxU16    IdrInterval;

    mfxU16    BasemodePred; /* four-state option, UNKNOWN/ON/OFF/ADAPTIVE */
    mfxU16    MotionPred;   /* four-state option, UNKNOWN/ON/OFF/ADAPTIVE */
    mfxU16    ResidualPred; /* four-state option, UNKNOWN/ON/OFF/ADAPTIVE */

    mfxU16    DisableDeblockingFilter;  /* tri -state option */
    mfxI16    ScaledRefLayerOffsets[4];
    mfxU16    ScanIdxPresent; /* tri -state option */
    mfxU16    reserved2[8];

    mfxU16   TemporalNum;
    mfxU16   TemporalId[8];

    mfxU16   QualityNum;

    struct mfxQualityLayer {
            mfxU16 ScanIdxStart; 
            mfxU16 ScanIdxEnd;

            mfxU16 TcoeffPredictionFlag; /* tri -state option */
            mfxU16 reserved3[5];
    } QualityLayer[16];

}  mfxExtSVCDepLayer;

typedef struct {
    mfxExtBuffer        Header;

    mfxU16              TemporalScale[8];
    mfxU16              RefBaseDist;
    mfxU16              reserved1[3];

    mfxExtSVCDepLayer   DependencyLayer[8];
} mfxExtSVCSeqDesc;

typedef struct {
    mfxExtBuffer    Header;

    mfxU16  TargetTemporalID;
    mfxU16  TargetDependencyID;
    mfxU16  TargetQualityID;
    mfxU16  reserved[9];
} mfxExtSvcTargetLayer ;

enum {
   MFX_DWNSAMPLING_ALGM_BEST_QUALITY    = 0x0001,
   MFX_DWNSAMPLING_ALGM_BEST_SPEED      = 0x0002
};

typedef struct {
    mfxExtBuffer    Header;
    mfxU16  Algorithm;
    mfxU16  reserved[11];
} mfxExtSVCDownsampling;

#ifdef __cplusplus
} // extern "C"
#endif

#endif

