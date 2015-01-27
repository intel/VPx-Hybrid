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

File Name: vaapi_ext_interface.h

\* ****************************************************************************** */

#ifndef __VAAPI_EXT_INTERFACE_H__
#define __VAAPI_EXT_INTERFACE_H__

#define VAAPI_DRIVER_VPG

// Misc parameter for encoder
#define  VAEncMiscParameterTypePrivate     -2
// encryption parameters for PAVP
#define  VAEncryptionParameterBufferType   -3

#define EXTERNAL_ENCRYPTED_SURFACE_FLAG   (1<<16)
#define VA_CODED_BUF_STATUS_PRIVATE_DATA_HDCP (1<<8)
#define VA_HDCP_ENABLED (1<<12)

typedef struct _VAEncMiscParameterPrivate
{
    unsigned int target_usage; // Valid values 1-7 for AVC & MPEG2.
    unsigned int reserved[7];  // Reserved for future use.
} VAEncMiscParameterPrivate;

/*VAEncrytpionParameterBuffer*/
typedef struct _VAEncryptionParameterBuffer
{
    //Not used currently
    unsigned int encryptionSupport;
    //Not used currently
    unsigned int hostEncryptMode;
    // For IV, Counter input
    unsigned int pavpAesCounter[2][4];
    // not used currently
    unsigned int pavpIndex;
    // PAVP mode, CTR, CBC, DEDE etc
    unsigned int pavpCounterMode;
    unsigned int pavpEncryptionType;
    // not used currently
    unsigned int pavpInputSize[2];
    // not used currently
    unsigned int pavpBufferSize[2];
    // not used currently
    VABufferID   pvap_buf;
    // set to TRUE if protected media
    unsigned int pavpHasBeenEnabled;
    // not used currently
    unsigned int IntermmediatedBufReq;
    // not used currently
    unsigned int uiCounterIncrement;
    // AppId: PAVP sessin Index from application
    unsigned int app_id;

} VAEncryptionParameterBuffer;

typedef struct _VAHDCPEncryptionParameterBuffer {
    unsigned int       status;
    int                bEncrypted;
    unsigned int       counter[4];
} VAHDCPEncryptionParameterBuffer;

// structure for encoder capability
typedef struct _VAEncQueryCapabilities
{
    /* these 2 fields should be preserved over all versions of this structure */
    unsigned int version; /* [in] unique version of the sturcture interface  */
    unsigned int size;    /* [in] size of the structure*/
    union
    {
        struct
        {
            unsigned int SliceStructure    :3; /* 1 - SliceDividerSnb; 2 - SliceDividerHsw; 3 - SliceDividerBluRay; the other - SliceDividerOneSlice  */
            unsigned int NoInterlacedField :1;
        } bits;
        unsigned int CodingLimits;
    } EncLimits;
    unsigned int MaxPicWidth;
    unsigned int MaxPicHeight;
    unsigned int MaxNum_ReferenceL0;
    unsigned int MaxNum_ReferenceL1;
    unsigned int MBBRCSupport;
} VAEncQueryCapabilities;

// VAAPI Extension to support querying encoder capability
typedef VAStatus (*vaExtQueryEncCapabilities)(
    VADisplay                   dpy,
    VAProfile                   profile,
    VAEncQueryCapabilities      *pVaEncCaps);

#define VPG_EXT_QUERY_ENC_CAPS  "vpgExtQueryEncCapabilities"


/* below is private VAAPI for VP8 Hybrid encoder */

// entry point for ENC + hybrid PAK
#define VAEntrypointHybridEncSlice          -1

// structure discribing MB data layout returned from driver
typedef struct _VAEncMbDataLayout
{
    unsigned char   MB_CODE_size;
    unsigned int    MB_CODE_offset;
    unsigned int    MB_CODE_stride;
    unsigned char   MV_number;
    unsigned int    MV_offset;
    unsigned int    MV_stride;
} VAEncMbDataLayout;

// this buffer representing VAEncMbDataLayout for function vpgQueryBufferAttributes()
#define VAEncMbDataBufferType               -4

#define FUNC_QUERY_BUFFER_ATTRIBUTES "hybridQueryBufferAttributes"
//#define FUNC_QUERY_BUFFER_ATTRIBUTES "media_QueryBufferAttributes"
//#define FUNC_QUERY_BUFFER_ATTRIBUTES "vpgQueryBufferAttributes"

// VAAPI private extension for quering buffer attributes for different buffer types
// at the moment supports only VAEncMbDataBufferType 
typedef VAStatus (*hybridQueryBufferAttributes)(
     VADisplay      dpy, 
     VAContextID    context, 
     VABufferType   bufferType,
     void           *outputData,
     unsigned int   *outputDataLen);

#define VAEncMiscParameterTypeVP8HybridFrameUpdate -3

typedef struct _VAEncMiscParameterVP8HybridFrameUpdate
{
    // Previous frame bitstream size in bytes used when BRC is enabled.
    // If it is unknown, like for the first couple frames, it is set to 0.
    unsigned int prev_frame_size;
    // Depending on whether app is running in synchronous or asynchronouse mode,
    // it may have the immediate previous frame size or the 2 prior frame size.
    // BRC algorithm is different for the 2 due to different frame types, below flag
    // indicates if prev_frame_size is from 2 previous frames.
    bool         two_prev_frame_flag;

    // Previous frame costs to use for current frame.  If set to 0 the driver will use default values.
    // MBs encoded by Intra(0), LastRef(1), GoldRef(2), or AltRef(3).
    unsigned short ref_frame_cost[4];
    // Mode costs, one per segment. INTRA_NONPRED(0), INTRA_16x16(1), INTRA_8x8(2), INTRA_4x4(3)
    // Note 0-3 correspond to Universal VME Input Message DW2.0.  See BSPEC for details.
    unsigned short intra_mode_cost[4];
    // MB's encoded by 16x16(0), 16x8(1), 8x8(2), 4x4(3).
    unsigned short inter_mode_cost[4];
    // one per segment
    unsigned short intra_non_dc_penalty_16x16;
    // one per segment
    unsigned short intra_non_dc_penalty_4x4;

    // quantization index for reference frames:
    //  ref_q_index[0] - LastRef
    //  ref_q_index[1] - GoldRef
    //  ref_q_index[2] - AltRef
    unsigned char  ref_q_index[3];
} VAEncMiscParameterVP8HybridFrameUpdate;


// Segment map parameters needed by BRC when app provides segmentation map.
#define VAEncMiscParameterTypeVP8SegmentMapParams   -4

typedef struct _VAEncMiscParameterVP8SegmentMapParams
{
    // Specifies the QIndex delta of Y1_ac value for BRC to use, one per segment,
    // when the application provides the VP8 MB Segmentation Buffer using
    // VAEncMacroblockMapBufferType.
    char yac_quantization_index_delta[4];
} VAEncMiscParameterVP8SegmentMapParams;

#endif // __VAAPI_EXT_INTERFACE_H__
