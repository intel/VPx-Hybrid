/* ****************************************************************************** *\

Copyright (C) 2007-2012 Intel Corporation.  All rights reserved.

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

File Name: mfxstructurespro.h

\* ****************************************************************************** */
#ifndef __MFXSTRUCTURESPRO_H__
#define __MFXSTRUCTURESPRO_H__
#include "mfxstructures.h"

#pragma warning(disable: 4201)

#ifdef __cplusplus
extern "C"
{
#endif

/* Macroblock Codes for AVC */
typedef struct {
    mfxU32      reserved1a[8];
    union {
        mfxU8   MbMode;
        struct {
            mfxU8   Skip8x8Flag     :4;
            mfxU8   reserved2b      :2;
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
            union {
                mfxU8   CodedPatternDC  :3;
                struct {
                    mfxU8   DcBlockCodedYFlag   :1;
                    mfxU8   DcBlockCodedCbFlag  :1;
                    mfxU8   DcBlockCodedCrFlag  :1;
                };
            };
            union {
                mfxU8   FilterEdgeFlag  :3;
                struct {
                    mfxU8   FilteInternalEdgeFlag   :1;
                    mfxU8   FilterLeftEdgeFlag      :1;
                    mfxU8   FilterTopEdgeFlag       :1;
                };
            };
            mfxU8   reserved3c  :1;
        };
    };

    mfxU8   NumPackedMv;
    mfxU8   MbXcnt;
    mfxU8   MbYcnt;
    mfxU16  CodedPattern4x4Y;
    mfxU16  CodedPattern4x4U;
    mfxU16  CodedPattern4x4V;

    mfxU8   QpPrimeY;
    mfxU8   QpPrimeU;
    mfxU8   QpPrimeV;
    mfxU8   reserved4d;

    union {
        struct {    /* Inter */
            mfxU8   SubMbShape;
            mfxU8   SubMbPredMode;
            mfxU16  reserved5e;
            mfxU8   RefPicSelect[2][4];
        };
        struct {    /* Intra */
            mfxU16  LumaIntraModes[4];
            union {
                mfxU8   IntraStruct;
                struct {
                    mfxU8   ChromaIntraPredMode :2;
                    mfxU8   IntraPredAvailFlags :6;
                };
            };
        };
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

    union {
        mfxI16 MV[8][2];
        struct { /* if ExtMvDataFlag == 1 */
            mfxU32 MvDataOffset;
            mfxU32 MbUserOffset;
        };
    };
} mfxMbCodeAVC;

/* Macroblock Codes for MPEG-2 */
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
            mfxU8   reserved2b      :7;
        };
    };

    mfxU8   NumPackedMv;
    mfxU8   MbXcnt;
    mfxU8   MbYcnt;
    mfxU16  CodedPattern4x4Y;
    mfxU16  CodedPattern4x4U;
    mfxU16  CodedPattern4x4V;

    mfxU8   reserved3c;
    mfxU8   QpScaleType;
    mfxU8   QpScaleCode;

    mfxU16  reserved4d;
    mfxU8   SubMbPredMode;
    mfxU8   reserved5e[10];

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
} mfxMbCodeMPEG2;
/* Macroblock Codes */
typedef union {
    mfxMbCodeAVC    AVC;
    mfxMbCodeMPEG2  MPEG2;
} mfxMbCode;

/* MbType */
enum {
    /* For AVC intra cases: */
    MFX_MBTYPE_INTRA_16X16_000    =0x21,
    MFX_MBTYPE_INTRA_16X16_100    =0x22,
    MFX_MBTYPE_INTRA_16X16_200    =0x23,
    MFX_MBTYPE_INTRA_16X16_300    =0x24,
    MFX_MBTYPE_INTRA_16X16_010    =0x25,
    MFX_MBTYPE_INTRA_16X16_110    =0x26,
    MFX_MBTYPE_INTRA_16X16_210    =0x27,
    MFX_MBTYPE_INTRA_16X16_310    =0x28,
    MFX_MBTYPE_INTRA_16X16_020    =0x29,
    MFX_MBTYPE_INTRA_16X16_120    =0x2A,
    MFX_MBTYPE_INTRA_16X16_220    =0x2B,
    MFX_MBTYPE_INTRA_16X16_320    =0x2C,
    MFX_MBTYPE_INTRA_16X16_001    =0x2D,
    MFX_MBTYPE_INTRA_16X16_101    =0x2E,
    MFX_MBTYPE_INTRA_16X16_201    =0x2F,
    MFX_MBTYPE_INTRA_16X16_301    =0x30,
    MFX_MBTYPE_INTRA_16X16_011    =0x31,
    MFX_MBTYPE_INTRA_16X16_111    =0x32,
    MFX_MBTYPE_INTRA_16X16_211    =0x33,
    MFX_MBTYPE_INTRA_16X16_311    =0x34,
    MFX_MBTYPE_INTRA_16X16_021    =0x35,
    MFX_MBTYPE_INTRA_16X16_121    =0x36,
    MFX_MBTYPE_INTRA_16X16_221    =0x37,
    MFX_MBTYPE_INTRA_16X16_321    =0x38,

    MFX_MBTYPE_INTRA_8X8          =0xA0,  /* AVC */
    MFX_MBTYPE_INTRA_4X4          =0x20,  /* AVC */
    MFX_MBTYPE_INTRA_PCM          =0x39,  /* AVC */
    MFX_MBTYPE_INTRA_MPEG2        =0x3A,  /* MPEG-2 */
    MFX_MBTYPE_INTRA_FIELD_MPEG2  =0x7A,  /* MPEG-2 */

    MFX_MBTYPE_SKIP_16X16_0       =0x01,  /* ALL */
    MFX_MBTYPE_SKIP_16X16_1       =0x02,  /* ALL */
    MFX_MBTYPE_SKIP_16X16_2       =0x03,  /* ALL */
    MFX_MBTYPE_SKIP_8X8           =0x16,  /* AVC */
    MFX_MBTYPE_SKIP_4X4           =0x16,  /* AVC */

    MFX_MBTYPE_INTER_16X16_0      =0x01,  /* ALL */
    MFX_MBTYPE_INTER_16X16_1      =0x02,  /* ALL */
    MFX_MBTYPE_INTER_16X16_2      =0x03,  /* ALL */

    /* ALL */
    MFX_MBTYPE_INTER_16X8_00      =0x04,
    MFX_MBTYPE_INTER_16X8_11      =0x06,
    MFX_MBTYPE_INTER_16X8_01      =0x08,
    MFX_MBTYPE_INTER_16X8_10      =0x0A,
    MFX_MBTYPE_INTER_16X8_02      =0x0C,
    MFX_MBTYPE_INTER_16X8_12      =0x0E,
    MFX_MBTYPE_INTER_16X8_20      =0x10,
    MFX_MBTYPE_INTER_16X8_21      =0x12,
    MFX_MBTYPE_INTER_16X8_22      =0x14,

    /* AVC */
    MFX_MBTYPE_INTER_8X16_00      =0x05,
    MFX_MBTYPE_INTER_8X16_11      =0x07,
    MFX_MBTYPE_INTER_8X16_01      =0x09,
    MFX_MBTYPE_INTER_8X16_10      =0x0B,
    MFX_MBTYPE_INTER_8X16_02      =0x0D,
    MFX_MBTYPE_INTER_8X16_12      =0x0F,
    MFX_MBTYPE_INTER_8X16_20      =0x11,
    MFX_MBTYPE_INTER_8X16_21      =0x13,
    MFX_MBTYPE_INTER_8X16_22      =0x15,

    /* MPEG-2 */
    MFX_MBTYPE_INTER_FIELD_16X8_00    =0x44,
    MFX_MBTYPE_INTER_FIELD_16X8_11    =0x46,
    MFX_MBTYPE_INTER_FIELD_16X8_01    =0x48,
    MFX_MBTYPE_INTER_FIELD_16X8_10    =0x4A,
    MFX_MBTYPE_INTER_FIELD_16X8_02    =0x4C,
    MFX_MBTYPE_INTER_FIELD_16X8_12    =0x4E,
    MFX_MBTYPE_INTER_FIELD_16X8_20    =0x50,
    MFX_MBTYPE_INTER_FIELD_16X8_21    =0x52,
    MFX_MBTYPE_INTER_FIELD_16X8_22    =0x54,
    MFX_MBTYPE_INTER_DUAL_PRIME       =0x19,  /* MPEG-2 */
    MFX_MBTYPE_INTER_OTHERS           =0x16   /* AVC */
};

/* IntraMbFlag */
enum {
    MFX_MBTYPE_INTRA_FLAG   =0x20
};

/* FieldMbFlag */
enum {
    MFX_MBTYPE_FIELD_FLAG   =0x40
};

/* TransformFlag */
enum {
    MFX_MBTYPE_TX8X8_FLAG   =0x80
};

/* SubMbShape */
enum {
    MFX_SUBSHP_NO_SPLIT     =0,
    MFX_SUBSHP_TWO_8X4      =1,
    MFX_SUBSHP_TWO_4X8      =2,
    MFX_SUBSHP_FOUR_4X4     =3
};

/* SubMbPredMode */
enum {
    MFX_SUBDIR_REF_0        =0,
    MFX_SUBDIR_REF_1        =1,
    MFX_SUBDIR_BIDIR        =2,
    MFX_SUBDIR_INTRA        =3
};

/* SubMbType */
enum {
    MFX_MBTYPE_SKIP_16X8_TOP_FLAG   =0x30,
    MFX_MBTYPE_SKIP_16X8_BTM_FLAG   =0xC0
};

/* MvUnpackedFlag */
enum {
    MFX_MVPACK_UNPACKED_16X16_P  =1,
    MFX_MVPACK_UNPACKED_16X16_B  =2,
    MFX_MVPACK_UNPACKED_8X8_P    =3,
    MFX_MVPACK_UNPACKED_8X8_B    =4,
    MFX_MVPACK_UNPACKED_4X4_P    =5,
    MFX_MVPACK_UNPACKED_4X4_B    =6,
    MFX_MVPACK_PACKED            =7
};

typedef struct {
    mfxU32      CucId;
    mfxU32      CucSz;
    mfxU32      NumMb;
    mfxMbCode   *Mb;
} mfxMbParam;

/* CucId */
enum {
    MFX_CUC_AVC_MBPARAM      =MFX_MAKEFOURCC('C','U','C','0'),
    MFX_CUC_MPEG2_MBPARAM    =MFX_MAKEFOURCC('C','U','C','2')
};

/* MbScanMethod */
enum {
    MFX_SCANMETHOD_MPEG2_ZIGZAG         =0,
    MFX_SCANMETHOD_MPEG2_ALTERNATIVE    =1,
    MFX_SCANMETHOD_ARBITRARY            =3
};

/* Frame Parameters for AVC */
typedef struct {
    mfxU32  CucId;
    mfxU32  CucSz;
    mfxU8   FrameType;
    mfxU8   FrameType2nd;
    mfxU8   reserved1a;
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
            mfxU16  FieldPicFlag    :1;
            union {
                mfxU16  MbaffFrameFlag      :1;
                mfxU16  SecondFieldPicFlag  :1;
            };
            mfxU16  ResColorTransFlag   :1;
            mfxU16  SPforSwitchFlag     :1;
            mfxU16  ChromaFormatIdc     :2;
            mfxU16  RefPicFlag          :1;
            mfxU16  ConstraintIntraFlag :1;
            mfxU16  WeightedPredFlag    :1;
            mfxU16  WeightedBipredIdc   :2;
            mfxU16  MbsConsecutiveFlag  :1;
            mfxU16  FrameMbsOnlyFlag    :1;
            mfxU16  Transform8x8Flag    :1;
            mfxU16  NoMinorBipredFlag   :1;
            mfxU16  IntraPicFlag        :1;
        };
    };

    union {
        mfxU16  ExtraFlags;
        struct {
            mfxU16  EntropyCodingModeFlag   :1;
            mfxU16  Direct8x8InferenceFlag  :1;
            mfxU16  TransCoeffFlag          :1;
            mfxU16  reserved2b              :1;
            mfxU16  ZeroDeltaPicOrderFlag   :1;
            mfxU16  GapsInFrameNumAllowed   :1;
            mfxU16  reserved3c              :2;
            mfxU16  PicOrderPresent         :1;
            mfxU16  RedundantPicCntPresent  :1;
            mfxU16  ScalingListPresent      :1;
            mfxU16  SliceGroupMapPresent    :1;
            mfxU16  ILDBControlPresent      :1;
            mfxU16  MbCodePresent           :1;
            mfxU16  MvDataPresent           :1;
            mfxU16  ResDataPresent          :1;
        };
    };

    mfxI8   PicInitQpMinus26;
    mfxI8   PicInitQsMinus26;

    mfxI8   ChromaQp1stOffset;
    mfxI8   ChromaQp2ndOffset;
    mfxU8   NumRefIdxL0Minus1;
    mfxU8   NumRefIdxL1Minus1;

    mfxU8   BitDepthLumaMinus8;
    mfxU8   BitDepthChromaMinus8;

    mfxU8   Log2MaxFrameCntMinus4;
    mfxU8   PicOrderCntType;

    mfxU8   Log2MaxPicOrdCntMinus4;
    mfxU8   NumSliceGroupMinus1;

    mfxU8   SliceGroupMapType;
    mfxU8   SliceGroupXRateMinus1;

    union {
        mfxU8   RefFrameListP[16];
        mfxU8   RefFrameListB[2][8];
    };

    mfxU32  MinFrameSize;
    mfxU32  MaxFrameSize;
} mfxFrameParamAVC;

/* Frame Parameters for MPEG-2 */
typedef struct {
    mfxU32  CucId;
    mfxU32  CucSz;
    mfxU8   FrameType;
    mfxU8   FrameType2nd;
    mfxU8   reserved1a;
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
            mfxU16  ForwardPredFlag     :1;
            mfxU16  NoResidDiffs        :1;
            mfxU16  reserved3c          :2;
            mfxU16  FrameMbsOnlyFlag    :1;
            mfxU16  BrokenLinkFlag      :1;
            mfxU16  CloseEntryFlag      :1;
            mfxU16  IntraPicFlag        :1;
        };
    };

    union {
        mfxU16  ExtraFlags;
        struct {
            mfxU16  reserved4d          :4;
            mfxU16  MvGridAndChroma     :4;
            mfxU16  reserved5e          :8;
        };
    };

    mfxU16  reserved6f;
    mfxU16  BitStreamFcodes;

    union {
        mfxU16  BitStreamPCEelement;
        struct {
            mfxU16    reserved7g            :3;
            mfxU16    ProgressiveFrame      :1;
            mfxU16    Chroma420type         :1;
            mfxU16    RepeatFirstField      :1;
            mfxU16    AlternateScan         :1;
            mfxU16    IntraVLCformat        :1;
            mfxU16    QuantScaleType        :1;
            mfxU16    ConcealmentMVs        :1;
            mfxU16    FrameDCTprediction    :1;
            mfxU16    TopFieldFirst         :1;
            mfxU16    PicStructure          :2;
            mfxU16    IntraDCprecision      :2;
        };
    };

    mfxU8   BSConcealmentNeed;
    mfxU8   BSConcealmentMethod;
    mfxU16  TemporalReference;
    mfxU32  VBVDelay;

    union{
        mfxU8   RefFrameListP[16];
        mfxU8   RefFrameListB[2][8];
    };

    mfxU32  MinFrameSize;
    mfxU32  MaxFrameSize;
} mfxFrameParamMPEG2;

typedef union {
    mfxFrameParamAVC    AVC;
    mfxFrameParamMPEG2  MPEG2;
} mfxFrameParam;

/* CucId */
enum {
    MFX_CUC_AVC_FRAMEPARAM       =MFX_MAKEFOURCC('C','U','F','0'),
    MFX_CUC_MPEG2_FRAMEPARAM     =MFX_MAKEFOURCC('C','U','F','2')
};

/* RefIndexListP[16] */
/* RefIndexListB[2][8] */
enum {
    MFX_REFLABEL_BTM_FIELD      =0x80,
    MFX_REFLABEL_UNUSED         =0xFF
};

/* AVC Extended Buffers */
typedef struct {
    mfxExtBuffer    Header;
    mfxI16Pair*     Mv;
} mfxExtAvcMvData;

typedef struct {
    mfxExtBuffer    Header;
    mfxU8   ScalingLists4x4[6][16];
    mfxU8   ScalingLists8x8[2][64];
} mfxExtAvcQMatrix;

typedef struct {
    mfxExtBuffer    Header;
    mfxU16  FirstMBInSlice;
    mfxU16  NumMbInSlice;
    mfxU8   RefPicList[2][32];
    mfxI8   Weights[2][32][3][2];
} mfxExtAvcRefSliceInfo;

typedef struct {
    mfxExtBuffer    Header;
    mfxMbParam*     MbParam;
    mfxExtAvcMvData*MvData;
    mfxI32          FieldOrderCnt[2];
    mfxI32          FieldOrderCntList[16][2];
    mfxU16          FrameNumList[16];
    mfxU16          NumSlice;
    mfxExtAvcRefSliceInfo*  SliceInfo;
} mfxExtAvcRefPicInfo;

typedef struct {
    mfxExtBuffer    Header;
    union {
        mfxU8   CodecFlags;
        struct {
            mfxU8   FieldPicFlag    :1;
            mfxU8   MbaffFrameFlag  :1;
            mfxU8   IsTopShortRef   :1;
            mfxU8   IsBottomShortRef:1;
            mfxU8   IsTopLongRef    :1;
            mfxU8   IsBottomLongRef :1;
        };
    };
    mfxI32  FrameNum;
    mfxI32  LongFrameIndex;
    mfxExtAvcRefPicInfo PicInfo[2];
} mfxExtAvcRefFrameParam;

typedef struct {
    mfxExtBuffer    Header;
    mfxExtAvcRefFrameParam* RefList[16];
} mfxExtAvcRefList;

/* MPEG-2 Extended Buffers */
typedef struct {
    mfxExtBuffer    Header;
    mfxU8   IntraQMatrix[64];
    mfxU8   InterQMatrix[64];
    mfxU8   ChromaIntraQMatrix[64];
    mfxU8   ChromaInterQMatrix[64];
} mfxExtMpeg2QMatrix;

typedef struct {
    mfxExtBuffer    Header;
    mfxI16*         Coeffs;
} mfxExtMpeg2Coeffs;

typedef struct {
    mfxExtBuffer    Header;
    mfxU16          Pitch;
    mfxI16*         Residual;
} mfxExtMpeg2Residual;

typedef struct {
    mfxExtBuffer    Header;
    mfxU16          Pitch;
    mfxU8*          Prediction;
} mfxExtMpeg2Prediction;

/* CucId */
enum {
    MFX_CUC_AVC_MVDATA      =MFX_MAKEFOURCC('C','U','X',1),
    MFX_CUC_AVC_QMATRIX     =MFX_MAKEFOURCC('C','U','X',2),
    MFX_CUC_AVC_REFPIC      =MFX_MAKEFOURCC('C','U','X',3),
    MFX_CUC_AVC_REFFRAME    =MFX_MAKEFOURCC('C','U','X',4),
    MFX_CUC_AVC_REFSLICE    =MFX_MAKEFOURCC('C','U','X',5),
    MFX_CUC_AVC_REFLIST     =MFX_MAKEFOURCC('C','U','X',6),

    MFX_CUC_MPEG2_MVDATA    =MFX_MAKEFOURCC('C','U','X',10),
    MFX_CUC_MPEG2_RESPIXEL  =MFX_MAKEFOURCC('C','U','X',11),
    MFX_CUC_MPEG2_QMATRIX   =MFX_MAKEFOURCC('C','U','X',12),
    MFX_CUC_MPEG2_RESCOEFF  =MFX_MAKEFOURCC('C','U','X',13)
};

/* Slice Parameters for AVC */
typedef struct {
    mfxU32  CucId;
    mfxU32  CucSz;
    mfxU16  FrameCnt;
    mfxU16  IdrPictureId;
    mfxU32  reserved;

    mfxU32  SliceDataOffset;
    mfxU32  SliceDataSize;

    mfxU16  BadSliceChopping;
    mfxU8   FirstMbX;
    mfxU8   FirstMbY;
    mfxU16  NumMb;
    mfxU16  SliceHeaderSize;

    mfxU8   SliceType;
    mfxU8   Log2WeightDenomLuma;
    mfxU8   Log2WeightDenomChroma;
    mfxU8   NumRefIdxL0Minus1;
    mfxU8   NumRefIdxL1Minus1;
    mfxI8   DeblockAlphaC0OffsetDiv2;
    mfxI8   DeblockBetaOffsetDiv2;
    mfxU8   BsInsertionFlag;

    mfxI8   SliceQsDelta;
    mfxI8   SliceQpDelta;

    mfxU8   RedundantPicCnt;
    mfxU8   DirectPredType;
    mfxU8   CabacInitIdc;
    mfxU8   DeblockDisableIdc;
    mfxU16  SliceId;
} mfxSliceParamAVC;

/* Slice Parameters for MPEG-2 */
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
} mfxSliceParamMPEG2;

typedef union {
    mfxSliceParamAVC    AVC;
    mfxSliceParamMPEG2  MPEG2;
} mfxSliceParam;

/* CucId */
enum {
    MFX_CUC_AVC_SLICEPARAM       =MFX_MAKEFOURCC('C','U','S','0'),
    MFX_CUC_MPEG2_SLICEPARAM     =MFX_MAKEFOURCC('C','U','S','2')
};

/* BadSliceChopping */
enum {
    MFX_SLICECHOP_NONE      =0,
    MFX_SLICECHOP_TAIL      =1,
    MFX_SLICECHOP_HEAD      =2,
    MFX_SLICECHOP_HEADTAIL  =3
};

/* SliceType */
enum {
    MFX_SLICETYPE_I         =1,
    MFX_SLICETYPE_P         =2,
    MFX_SLICETYPE_B         =4,

    MFX_SLICETYPE_TOP_FIELD =0x10,
    MFX_SLICETYPE_BTM_FIELD =0x20,
    MFX_SLICETYPE_REFERENCE =0x40,
    MFX_SLICETYPE_IDR       =0x80
};

/* BsInsertionFlags */
enum {
    MFX_BSINSERTION_EMULATION   =1,
    MFX_BSINSERTION_CABACZERO   =2
};

/* Frame Surface */
typedef struct {
    mfxU32  reserved[4];
    mfxU16  NumFrameData;
    mfxFrameInfo    Info;
    mfxFrameData    **Data;
} mfxFrameSurface;

/* Frame CUCs */
typedef struct {
    mfxU32      FourCC;         /* See the same variable description in mfxVideoParam section */
    mfxU32      FrameCucSz;

    mfxU16      FrameCnt;       /* The counter of the current frame modulo 0x1000, used as an identifier */
    mfxU16      NumMb;          /* The total number of macroblocks of the current frame */
    mfxU16      FieldId;        /* See below */
    mfxU16      SliceId;        /* The counter of the current slice. */
    mfxU16      NumSlice;       /* Number of slices divided in a frame */
    mfxU16      NumExtBuffer;   /* The number of extra extended data buffers pointed in the current CUC */

    mfxFrameParam*      FrameParam;
    mfxSliceParam*      SliceParam;
    mfxMbParam*         MbParam;

    mfxFrameSurface*    FrameSurface;
    mfxBitstream*       Bitstream;

    mfxExtBuffer**      ExtBuffer;
} mfxFrameCUC;

/* FourCC */
enum {
    MFX_CUC_AVC_MV          =MFX_MAKEFOURCC('C','U','C','0'),
    MFX_CUC_AVC_MV_RES      =MFX_MAKEFOURCC('C','U','R','0'),

    MFX_CUC_MPEG2_MV        =MFX_MAKEFOURCC('C','U','C','2'),
    MFX_CUC_MPEG2_MV_RES    =MFX_MAKEFOURCC('C','U','R','2')
};

/* FieldId */
enum {
    MFX_FIELDMODE_FRAME         =0,
    MFX_FIELDMODE_TFF_TOP       =1,
    MFX_FIELDMODE_TFF_BTM       =2,
    MFX_FIELDMODE_TFF           =3,
    MFX_FIELDMODE_MBAFF         =4,
    MFX_FIELDMODE_BFF_BTM       =5,
    MFX_FIELDMODE_BFF_TOP       =6,
    MFX_FIELDMODE_BFF           =7
};

/* mfxU16 SyncOption */
enum {
    MFX_SYNCOPT_NOTEXPORT_MBCODE   = 0x01
};

/* mfxU16 FrameType */
enum {
    MFX_FRAMETYPE_TFF = 0x10,
    MFX_FRAMETYPE_BFF = 0x20
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif

