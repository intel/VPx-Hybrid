/* ****************************************************************************** *\

Copyright (C) 2007-2008 Intel Corporation.  All rights reserved.

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

File Name: mfxstructures-int.h

\* ****************************************************************************** */
#ifndef __MFXSTRUCTURES_INT_H__
#define __MFXSTRUCTURES_INT_H__
#include "mfxstructures.h"

#ifdef __cplusplus
extern "C" {
#endif

// U32 CodecId;
enum {
    MFX_FOURCC_H263         =MFX_MAKEFOURCC('H','2','6','3'),
    MFX_FOURCC_MPEG4        =MFX_MAKEFOURCC('M','P','G','4'),
    MFX_FOURCC_AVS          =MFX_MAKEFOURCC('A','V','S',' '),
};

// mfxU8 CodecProfile, CodecLevel
// They are the same numbers as used in specific codec standards.
enum {
    // AVC Profiles & Levels
    MFX_PROFILE_AVC_HIGH10      =110,
    MFX_PROFILE_AVC_HIGH422     =122,

    // MPEG-2 Profiles & Levels
    MFX_PROFILE_MPEG2_SNR       =3,
    MFX_PROFILE_MPEG2_SPATIAL   =2,

    // MPEG-4 part 2 Profiles & Levels
    MFX_PROFILE_MPEG4_SIMPLE            =(0x00+1),
    MFX_PROFILE_MPEG4_ADVANCED_SIMPLE   =(0xF0+1),

    MFX_LEVEL_MPEG4_0           =(0x00+1),
    MFX_LEVEL_MPEG4_1           =(0x01+1),
    MFX_LEVEL_MPEG4_2           =(0x02+1),
    MFX_LEVEL_MPEG4_3           =(0x03+1),
    MFX_LEVEL_MPEG4_3b          =(0x07+1),
    MFX_LEVEL_MPEG4_4           =(0x04+1),
    MFX_LEVEL_MPEG4_5           =(0x05+1),
};

// mfxU8 TargetUsages: 1~7;
enum {
    // There are also other additional properties, such as the requirement of low latency, the recommendation of multipass, etc
    MFX_TARGETUSAGE_MULTIPASS        =0x20,
    MFX_TARGETUSAGE_LOW_LATENCY      =0x10,
};

// mfxU32 FourCC;
enum {
    MFX_FOURCC_IMC3         =MFX_MAKEFOURCC('I','M','C','3'),
    MFX_FOURCC_IMC4         =MFX_MAKEFOURCC('I','M','C','4'),
    MFX_FOURCC_Y216         =MFX_MAKEFOURCC('Y','2','1','6'),
    MFX_FOURCC_YV16         =MFX_MAKEFOURCC('Y','V','1','6'),
    MFX_FOURCC_YUYV         =MFX_MAKEFOURCC('Y','U','Y','V'),
};

// mfxU32 FourCC;
enum {
    MFX_CUC_AVC_MV_DCT      =MFX_MAKEFOURCC('C','U','X','0'),
    MFX_CUC_AVC_MV_TSQ      =MFX_MAKEFOURCC('C','U','Q','0'),

    MFX_CUC_VC1_MV_DCT      =MFX_MAKEFOURCC('C','U','X','1'),
    MFX_CUC_VC1_MV_TSQ      =MFX_MAKEFOURCC('C','U','Q','1'),

    MFX_CUC_MPEG2_MV_DCT    =MFX_MAKEFOURCC('C','U','X','2'),
    MFX_CUC_MPEG2_MV_TSQ    =MFX_MAKEFOURCC('C','U','Q','2'),
};

// VPP hints
enum {
    MFX_FOURCC_VPP_DEINTERLACE  =   MFX_MAKEFOURCC('D','I','T','L'),
};

/* VC-1 Extended Buffers */
typedef struct {
    mfxExtBuffer    Header;
    mfxU16          NumMv;
    mfxI16Pair*     Mv;
} mfxExtVc1MvData;

typedef struct {
    mfxExtBuffer    Header;
    mfxU16          NumMv;
    mfxU8*          MvDir;
} mfxExtVc1MvDir;

typedef struct {
    mfxExtBuffer    Header;
    mfxU16          NumMb;
    mfxI8*          RndCtrl;
} mfxExtVc1TrellisRndCtrl;

/* Slice Parameters for VC-1 */
typedef struct {
    mfxU32  CucId;
    mfxU32  CucSz;
    mfxU8   reserved8b[8];

    mfxU32  SliceDataOffset;
    mfxU32  SliceDataSize;

    mfxU16  BadSliceChopping;
    mfxU8   FirstMbX;
    mfxU8   FirstMbY;
    mfxU16  NumMb;
    mfxU16  SliceHeaderSize;

    mfxU8   SliceType;
    mfxU8   reserved6b[6];

    mfxU16  QScaleCode;

    mfxU8   reserved4b[4];
    mfxU16  SliceId;
} mfxSliceParamVC1;

/* Macroblock Codes for VC1 */
typedef struct {
    mfxU32      reserved1a[8];
    union {
        mfxU8   MbMode;
        struct {
            mfxU8   Skip8x8Flag     :4;
            mfxU8   MbScanMethod    :2;
            mfxU8   FirstMbFlag     :1;
            mfxU8   LastMbFlag      :1;
        };
    };

    union {
        mfxU8   MbType;
        struct {
            mfxU8   MbType5Bits     :5;
            mfxU8   IntraMbFlag     :1;
            mfxU8   FieldMbFlag     :1;
            mfxU8   TransformFlag   :1;
        };
    };

    union {
        mfxU8 MbFlag;
        struct {
            mfxU8   ResidDiffFlag   :1;
            mfxU8   AcPredFlag      :1;
            mfxU8   reserved2b      :2;
            mfxU8   OverlapFilter   :3;
            mfxU8   reserved3c      :1;
        };
    };

    mfxU8   NumPackedMv;
    mfxU8   MbXcnt;
    mfxU8   MbYcnt;
    mfxU16  CodedPattern4x4Y;
    mfxU16  CodedPattern4x4U;
    mfxU16  CodedPattern4x4V;


    mfxU8   reserved4d;
    mfxU8   QpScaleType;
    mfxU8   QpScaleCode;
    union   {
        mfxU8   HybridMvFlags;
        struct  {
            mfxU8   HybridMvDirectionFlags  :4;
            mfxU8   HybridMvPresentFlags    :4;
        };
    };

    struct {
        mfxU8   SubMbShape;
        mfxU8   SubMbPredMode;
        mfxU8   SubMbShapeU;
        mfxU8   SubMbShapeV;
        mfxU8   RefPicSelect[2][4];
    };

    union {
        mfxU8   MvDataFlag;
        struct {
            mfxU8   ExtMvDataFlag       :1;
            mfxU8   MvUnpackedFlag      :3;
            mfxU8   MbDataOffsetUnit    :4;
        };
    };

    mfxU8   MbDataSize128b;
    mfxU16  MbDataOffset;
    mfxI16  MV[8][2];
} mfxMbCodeVC1;

/* mfxU8 MbType */
enum {
    MFX_MBTYPE_INTRA_VC1          =0x3B,  /* VC-1 */
    MFX_MBTYPE_INTRA_FIELD_VC1    =0x7B,  /* VC-1 */
    MFX_MBTYPE_INTER_16X16_DIR    =0x17,  /* VC-1 */

    /* VC-1 */
    MFX_MBTYPE_INTER_8X8_0        =0x16,
    MFX_MBTYPE_INTER_8X8_1        =0x16,

    MFX_MBTYPE_INTER_FIELD_16X8_DIR   =0x57,  /* VC-1 */
    MFX_MBTYPE_INTER_FIELD_8X8_00     =0x56,  /* VC-1 */
    MFX_MBTYPE_INTER_MIX_INTRA        =0x18,  /* VC-1 */
};

/* mfxU32 CucId; */
enum {
    MFX_CUC_VC1_MBPARAM      =MFX_MAKEFOURCC('C','U','C','1'),
};

/* Frame Parameters for VC1 */
typedef struct {
    mfxU32  CucId;
    mfxU32  CucSz;
    mfxU8   FrameType;
    mfxU8   FrameType2nd;
    mfxU8   reserved8b;
    mfxU8   RecFrameLabel;
    mfxU8   DecFrameLabel;
    mfxU8   VppFrameLabel;
    mfxU16  NumMb;
    mfxU16  FrameWinMbMinus1;
    mfxU16  FrameHinMbMinus1;
    mfxU8   CurrFrameLabel;
    mfxU8   NumRefFrame;

    union {
        mfxU16  CodecFlags;
        struct {
            mfxU16  FieldPicFlag        :1;
            mfxU16  InterlacedFrameFlag :1;
            mfxU16  SecondFieldFlag     :1;
            mfxU16  BottomFieldFlag     :1;
            mfxU16  ChromaFormatIdc     :2;
            mfxU16  RefPicFlag          :1;
            mfxU16  BackwardPredFlag    :1;
            mfxU16  IsWMVA              :1;
            mfxU16  NoResidDiffs        :1;
            mfxU16  IntraResidUnsigned  :1;
            mfxU16  IsWMV9              :1;
            mfxU16  FrameMbsOnlyFlag    :1;
            mfxU16  BrokenLinkFlag      :1;
            mfxU16  CloseEntryFlag      :1;
            mfxU16  IntraPicFlag        :1;
        };
    };

    union {
        mfxU16  ExtraFlags;
        struct {
            mfxU16  Pic4MvAllowed           :1;
            mfxU16  SyncMarker              :1;
            mfxU16  PicResampling           :2;
            mfxU16  MvGridAndChroma         :4;
            mfxU16  ScanMethod              :2;
            mfxU16  ScanFixed               :1;
            mfxU16  RoundControl            :1;
            mfxU16  SpatialRes8             :1;
            mfxU16  ReferenceIndexed        :1;
            mfxU16  PicExtrapolation        :2;
        };
    };

    mfxU8   RangeMap;

    union {
        mfxU8   PicDeblock;
        struct {
            mfxU8   PicDeblocked        :6;
            mfxU8   PicDeblockConfined  :1;
            mfxU8   reserved1f          :1;
        };
    };

    mfxU8   LumaScale2;
    mfxU8   LumaScale;
    mfxU8   LumaShift2;
    mfxU8   LumaShift;

    union {
        mfxU8   RawCodingFlag;
        struct {
            mfxU8   MvTypeMB    :1;
            mfxU8   DirectMV    :1;
            mfxU8   SkipMB      :1;
            mfxU8   FieldTX     :1;
            mfxU8   ForwardMB   :1;
            mfxU8   ACpresd     :1;
            mfxU8   OverFlags   :1;
            mfxU8   HybridMV    :1;
        };
    };

    union {
        mfxU8   TransformFlags;         /* for VC-1 */
        struct {
            mfxU8   CodedBfraction  :5;
            mfxU8   TTMBF           :1;
            mfxU8   TTFRM           :2;
        };
    };

    union {
        mfxU16  PicQuantizer;       /* for VC-1 */
        struct {
            mfxU16  PQuant          :5;
            mfxU16  PQuantUniform   :1;
            mfxU16  HalfQP          :1;
            mfxU16  AltPQuant       :5;
            mfxU16  AltPQuantConfig :4;
        };
    };

    union {
        mfxU16  TableIndices;   /* for VC-1 */
        struct {
            mfxU16  CBPTable        :3;
            mfxU16  TransDCTable    :1;
            mfxU16  TransACTable    :2;
            union {
                struct {    /* for I-pictures & BI-pictures */
                    mfxU16  TransACTable2   :2;
                    mfxU16  reserved8l      :8;
                };
                struct {    /* for P-pictures & B-pictures */
                    mfxU16  TwoMVBPTable    :2;
                    mfxU16  MbModeTable     :3;
                    mfxU16  MvTable         :3;
                    mfxU16  FourMVBPTable   :2;
                };
            };
        };
    };

    union {
        mfxU16  MvReference;    /* for VC-1 */
        struct {    /* for P/B only */
            mfxU16  RefDistance         :4;
            mfxU16  NumRefPic           :1;
            mfxU16  RefFieldPicFlag     :1;
            mfxU16  FastUVMCflag        :1;
            mfxU16  FourMvSwitch        :1;
            mfxU16  UnifiedMvMode       :2;
            mfxU16  IntCompField        :2;
            mfxU16  ExtendedMvRange     :2;
            mfxU16  ExtendedDMvRange    :2;
        };
    };

    union{
        mfxU8   RefFrameListP[16];
        mfxU8   RefFrameListB[2][8];
    };

    mfxU32  MinFrameSize;
    mfxU32  MaxFrameSize;
} mfxFrameParamVC1;

/* mfxU32 CucId; */
enum {
    MFX_CUC_VC1_FRAMEPARAM       =MFX_MAKEFOURCC('C','U','F','1'),
};

/* mfxU32 CucId; */
enum {
    MFX_CUC_VC1_MVDATA      =MFX_MAKEFOURCC('C','U','X',7),
    MFX_CUC_VC1_MVDIR       =MFX_MAKEFOURCC('C','U','X',8),
    MFX_CUC_VC1_RNDCTRL     =MFX_MAKEFOURCC('C','U','X',9),
};

/* mfxU32 CucId; */
enum {
    MFX_CUC_VC1_SLICEPARAM       =MFX_MAKEFOURCC('C','U','S','1'),
};

/* mfxU32 FourCC; */
enum {
    MFX_CUC_VC1_MV          =MFX_MAKEFOURCC('C','U','C','1'),
    MFX_CUC_VC1_MV_RES      =MFX_MAKEFOURCC('C','U','R','1'),
};

/* mfxU8 SubMbShape; */
enum {
    MFX_SUBSHP_DCT_8X8      =0,
    MFX_SUBSHP_DCT_8X4      =1,
    MFX_SUBSHP_DCT_4X8      =2,
    MFX_SUBSHP_DCT_4X4      =3,
};

#ifdef __cplusplus
};
#endif

#endif // __MFXSTRUCTURES_H
