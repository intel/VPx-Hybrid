/* ****************************************************************************** *\

Copyright (C) 2007-2014 Intel Corporation.  All rights reserved.

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

File Name: mfxvideo++int.h

\* ****************************************************************************** */
#ifndef __MFXVIDEOPLUSPLUS_INTERNAL_H
#define __MFXVIDEOPLUSPLUS_INTERNAL_H

#include "mfxvideo.h"
#include "mfxvideopro.h"
#include <mfx_task_threading_policy.h>
#include <mfx_interface.h>
#include "mfxmvc.h"
#include "mfxsvc.h"
#include "mfxjpeg.h"
#include "mfxvp8.h"
#include "mfxplugin.h"


#if defined(_WIN32) || defined(_WIN64)
#include <guiddef.h>
#else
#ifndef GUID_TYPE_DEFINED

#include <string.h>
// TODO: temporary workaround for linux (aya: see below)
typedef struct {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;

static int operator==(const GUID & guidOne, const GUID & guidOther)
{
    return !memcmp(&guidOne, &guidOne, sizeof(GUID));
}
#define GUID_TYPE_DEFINED
#endif


// GUIDs from DDI spec 0.73
static const GUID DXVA2_Intel_Encode_AVC = 
{ 0x97688186, 0x56a8, 0x4094, { 0xb5, 0x43, 0xfc, 0x9d, 0xaa, 0xa4, 0x9f, 0x4b } };
static const GUID DXVA2_Intel_Encode_VP8 = 
{ 0x2364d06a, 0xf67f, 0x4186, { 0xae, 0xd0, 0x62, 0xb9, 0x9e, 0x17, 0x84, 0xf1 } };
static const GUID DXVA2_Intel_Encode_MPEG2 = 
{ 0xc346e8a3, 0xcbed, 0x4d27, { 0x87, 0xcc, 0xa7, 0xe, 0xb4, 0xdc, 0x8c, 0x27 } };
static const GUID DXVA2_Intel_Encode_SVC = 
{ 0xd41289c2, 0xecf3, 0x4ede, { 0x9a, 0x04, 0x3b, 0xbf, 0x90, 0x68, 0xa6, 0x29 } };

static const GUID sDXVA2_Intel_IVB_ModeJPEG_VLD_NoFGT =
{ 0x91cd2d6e, 0x897b, 0x4fa1, { 0xb0, 0xd7, 0x51, 0xdc, 0x88, 0x01, 0x0e, 0x0a } };

static const GUID sDXVA2_ModeMPEG2_VLD =
{ 0xee27417f, 0x5e28, 0x4e65, { 0xbe, 0xea, 0x1d, 0x26, 0xb5, 0x08, 0xad, 0xc9 } };

static const GUID sDXVA2_Intel_ModeVC1_D_Super =
{ 0xE07EC519, 0xE651, 0x4cd6, { 0xAC, 0x84, 0x13, 0x70, 0xCC, 0xEE, 0xC8, 0x51 } };

static const GUID sDXVA2_Intel_EagleLake_ModeH264_VLD_NoFGT =
{ 0x604f8e68, 0x4951, 0x4c54, { 0x88, 0xfe, 0xab, 0xd2, 0x5c, 0x15, 0xb3, 0xd6 } };

static const GUID sDXVA_ModeH264_VLD_SVC_Scalable_Constrained_Baseline =
{ 0x9b8175d4, 0xd670, 0x4cf2, { 0xa9, 0xf0, 0xfa, 0x56, 0xdf, 0x71, 0xa1, 0xae } };

static const GUID sDXVA_ModeH264_VLD_SVC_Scalable_Constrained_High =
{ 0x8efa5926, 0xbd9e, 0x4b04, { 0x8b, 0x72, 0x8f, 0x97, 0x7d, 0xc4, 0x4c, 0x36 } };

static const GUID sDXVA_ModeH264_VLD_Multiview_NoFGT =
{ 0x705b9d82, 0x76cf, 0x49d6, { 0xb7, 0xe6, 0xac, 0x88, 0x72, 0xdb, 0x01, 0x3c } };

static const GUID sDXVA_ModeH264_VLD_Stereo_Progressive_NoFGT =
{ 0xd79be8da, 0x0cf1, 0x4c81, { 0xb8, 0x2a, 0x69, 0xa4, 0xe2, 0x36, 0xf4, 0x3d } };

static const GUID sDXVA_ModeH264_VLD_Stereo_NoFGT =
{ 0xf9aaccbb, 0xc2b6, 0x4cfc, { 0x87, 0x79, 0x57, 0x07, 0xb1, 0x76, 0x05, 0x52 } };

static const GUID sDXVA2_ModeH264_VLD_NoFGT =
{ 0x1b81be68, 0xa0c7, 0x11d3, { 0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5 } };

static const GUID sDXVA_Intel_ModeH264_VLD_MVC =
{ 0xe296bf50, 0x8808, 0x4ff8, { 0x92, 0xd4, 0xf1, 0xee, 0x79, 0x9f, 0xc3, 0x3c } };

static const GUID DXVA_Intel_ModeHEVC_VLD_MainProfile =
{ 0x8c56eb1e, 0x2b47, 0x466f, { 0x8d, 0x33, 0x7d, 0xbc, 0xd6, 0x3f, 0x3d, 0xf2 } };

static const GUID DXVA_ModeHEVC_VLD_Main =
{ 0x5b11d51b, 0x2f4c, 0x4452, { 0xbc, 0xc3, 0x9,  0xf2, 0xa1, 0x16, 0xc,  0xc0 } };

#endif

#if defined(_WIN32) || defined(_WIN64)
#undef MFX_DEBUG_TOOLS
#define MFX_DEBUG_TOOLS

#if defined(DEBUG) || defined(_DEBUG)
#undef  MFX_DEBUG_TOOLS // to avoid redefinition
#define MFX_DEBUG_TOOLS
#endif
#endif // #if defined(_WIN32) || defined(_WIN64)

//#ifdef MFX_DEBUG_TOOLS
#define SKIP_EXT_BUFFERS_CHECK // (mynikols) check of supported extended buffers disabled in trunk
//#endif


// This is the include file for Media SDK component development.
enum eMFXPlatform
{
    MFX_PLATFORM_SOFTWARE      = 0,
    MFX_PLATFORM_HARDWARE      = 1,
};

enum eMFXVAType
{
    MFX_HW_NO       = 0,
    MFX_HW_D3D9     = 1,
    MFX_HW_D3D11    = 2,
    //MFX_HW_VA       = 3,//aya???
    MFX_HW_VAAPI    = 4,// Linux VA-API
    MFX_HW_VDAAPI   = 5,// OS X VDA-API

};

typedef enum
{
    MFX_ERR_MORE_DATA_RUN_TASK  = -10000,  /* MFX_ERR_MORE_DATA but async task should be added to Session */
} mfxInternalErrors;

enum eMFXHWType
{
    MFX_HW_UNKNOWN   = 0,
    MFX_HW_LAKE      = 0x100000,
    MFX_HW_LRB       = 0x200000,
    MFX_HW_SNB       = 0x300000,

    MFX_HW_IVB       = 0x400000,

    MFX_HW_HSW       = 0x500000,
    MFX_HW_HSW_ULT   = 0x500001,

    MFX_HW_VLV       = 0x600000,

    MFX_HW_BDW       = 0x700000,

    MFX_HW_CHV       = 0x800000,

    MFX_HW_SCL       = 0x900000
};

enum
{
    MFX_FOURCC_IMC3         = MFX_MAKEFOURCC('I','M','C','3'),
    MFX_FOURCC_YUV400       = MFX_MAKEFOURCC('4','0','0','P'),
    MFX_FOURCC_YUV411       = MFX_MAKEFOURCC('4','1','1','P'),
    MFX_FOURCC_YUV422H      = MFX_MAKEFOURCC('4','2','2','H'),
    MFX_FOURCC_YUV422V      = MFX_MAKEFOURCC('4','2','2','V'),
    MFX_FOURCC_YUV444       = MFX_MAKEFOURCC('4','4','4','P'),
    MFX_FOURCC_RGBP         = MFX_MAKEFOURCC('R','G','B','P')
};

#ifdef MFX_DEBUG_TOOLS
// commented since running behavior tests w/o catch is very annoying
// #define MFX_CORE_CATCH_TYPE     int**** // to disable catch
#define MFX_CORE_CATCH_TYPE     ...
#else
#define MFX_CORE_CATCH_TYPE     ...
#endif

// Forward declaration of used classes
struct MFX_ENTRY_POINT;

// VideoCORE
class VideoCORE {
public:

    virtual ~VideoCORE(void) {};

    // imported to external API
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL *handle) = 0;
    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL handle) = 0;
    virtual mfxStatus SetBufferAllocator(mfxBufferAllocator *allocator) = 0;
    virtual mfxStatus SetFrameAllocator(mfxFrameAllocator *allocator) = 0;

    // Internal interface only
    // Utility functions for memory access
    virtual mfxStatus  AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxMemId *mid) = 0;
    virtual mfxStatus  LockBuffer(mfxMemId mid, mfxU8 **ptr) = 0;
    virtual mfxStatus  UnlockBuffer(mfxMemId mid) = 0;
    virtual mfxStatus  FreeBuffer(mfxMemId mid) = 0;

    // Function checks D3D device for I/O D3D surfaces
    // If external allocator exists means that component can obtain device handle
    // If I/O surfaces in system memory  returns MFX_ERR_NONE
    virtual mfxStatus  CheckHandle() = 0;

    virtual mfxStatus  GetFrameHDL(mfxMemId mid, mfxHDL *handle, bool ExtendedSearch = true) = 0;
        
    virtual mfxStatus  AllocFrames(mfxFrameAllocRequest *request, 
                                   mfxFrameAllocResponse *response, bool isNeedCopy = true) = 0;

    virtual mfxStatus  AllocFrames(mfxFrameAllocRequest *request, 
                                   mfxFrameAllocResponse *response, 
                                   mfxFrameSurface1 **pOpaqueSurface, 
                                   mfxU32 NumOpaqueSurface) = 0;

    virtual mfxStatus  LockFrame(mfxMemId mid, mfxFrameData *ptr) = 0;
    virtual mfxStatus  UnlockFrame(mfxMemId mid, mfxFrameData *ptr=0) = 0;
    virtual mfxStatus  FreeFrames(mfxFrameAllocResponse *response, bool ExtendedSearch = true) = 0;

    virtual mfxStatus  LockExternalFrame(mfxMemId mid, mfxFrameData *ptr, bool ExtendedSearch = true) = 0;
    virtual mfxStatus  GetExternalFrameHDL(mfxMemId mid, mfxHDL *handle, bool ExtendedSearch = true) = 0;
    virtual mfxStatus  UnlockExternalFrame(mfxMemId mid, mfxFrameData *ptr=0, bool ExtendedSearch = true) = 0;

    virtual mfxMemId MapIdx(mfxMemId mid) = 0;
    virtual mfxFrameSurface1* GetNativeSurface(mfxFrameSurface1 *pOpqSurface, bool ExtendedSearch = true) = 0;
    virtual mfxFrameSurface1* GetOpaqSurface(mfxMemId mid, bool ExtendedSearch = true) = 0;

    // Increment Surface lock
    virtual mfxStatus  IncreaseReference(mfxFrameData *ptr, bool ExtendedSearch = true) = 0;
    // Decrement Surface lock
    virtual mfxStatus  DecreaseReference(mfxFrameData *ptr, bool ExtendedSearch = true) = 0;

        // no care about surface, opaq and all round. Just increasing reference
    virtual mfxStatus IncreasePureReference(mfxU16 &) = 0;
    // no care about surface, opaq and all round. Just decreasing reference
    virtual mfxStatus DecreasePureReference(mfxU16 &) = 0;

    // Check HW property
    virtual void  GetVA(mfxHDL* phdl, mfxU16 type) = 0;
    virtual mfxStatus CreateVA(mfxVideoParam * , mfxFrameAllocRequest *, mfxFrameAllocResponse *) = 0;
    // Get the current working adapter's number
    virtual mfxU32 GetAdapterNumber(void) = 0;

    // Get Video Processing
    virtual void  GetVideoProcessing(mfxHDL* phdl) = 0;
    virtual mfxStatus CreateVideoProcessing(mfxVideoParam *) = 0;

    virtual eMFXPlatform GetPlatformType() = 0;

    // Get the current number of working threads
    virtual mfxU32 GetNumWorkingThreads(void) = 0;
    virtual void INeedMoreThreadsInside(const void *pComponent) = 0;

    // need for correct video accelerator creation
    virtual mfxStatus DoFastCopy(mfxFrameSurface1 *dst, mfxFrameSurface1 *src) = 0;
    virtual mfxStatus DoFastCopyExtended(mfxFrameSurface1 *dst, mfxFrameSurface1 *src) = 0;
    virtual mfxStatus DoFastCopyWrapper(mfxFrameSurface1 *dst, mfxU16 dstMemType, mfxFrameSurface1 *src, mfxU16 srcMemType) = 0;
    virtual bool IsFastCopyEnabled(void) = 0;

    virtual bool IsExternalFrameAllocator(void) const = 0;

    virtual eMFXHWType   GetHWType() = 0;

    virtual bool         SetCoreId(mfxU32 Id) = 0;
    virtual eMFXVAType   GetVAType() const = 0;
    virtual
    mfxStatus CopyFrame(mfxFrameSurface1 *dst, mfxFrameSurface1 *src) = 0;
    virtual
    mfxStatus CopyBuffer(mfxU8 *dst, mfxU32 dst_size, mfxFrameSurface1 *src) = 0;

    virtual
    mfxStatus CopyFrameEx(mfxFrameSurface1 *pDst, mfxU16 dstMemType, mfxFrameSurface1 *pSrc, mfxU16 srcMemType) = 0;

    virtual mfxStatus IsGuidSupported(const GUID guid, mfxVideoParam *par, bool isEncoder = false) = 0;
    virtual bool CheckOpaqueRequest(mfxFrameAllocRequest *request, mfxFrameSurface1 **pOpaqueSurface, mfxU32 NumOpaqueSurface, bool ExtendedSearch = true) = 0;
        //function checks if surfaces already allocated and mapped and request is consistent. Fill response if surfaces are correct
    virtual bool IsOpaqSurfacesAlreadyMapped(mfxFrameSurface1 **pOpaqueSurface, mfxU32 NumOpaqueSurface, mfxFrameAllocResponse *response, bool ExtendedSearch = true) = 0;

    virtual void* QueryCoreInterface(const MFX_GUID &guid) = 0;
    virtual mfxSession GetSession() = 0;

    virtual void SetWrapper(void* pWrp) = 0;

    virtual mfxU16 GetAutoAsyncDepth() = 0;

    virtual bool IsCompatibleForOpaq() = 0;
};

class VideoBRC
{
public:
    // Destructor
    virtual
    ~VideoBRC(void) {}

    virtual
    mfxStatus Init(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Reset(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;

    virtual
    mfxStatus FrameENCUpdate(mfxFrameCUC *cuc) = 0;
    virtual
    mfxStatus FramePAKRefine(mfxFrameCUC *cuc) = 0;
    virtual
    mfxStatus FramePAKRecord(mfxFrameCUC *cuc) = 0;

};

class VideoENC
{
public:
    // Destructor
    virtual
    ~VideoENC(void){}

    virtual
    mfxStatus Init(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Reset(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetVideoParam(mfxVideoParam *par) = 0;
    virtual
    mfxStatus GetFrameParam(mfxFrameParam *par) = 0;

    // THIS METHOD SHOULD BECOME PURE VIRTUAL TOO,
    // BUT FOR COMPATIBILITY REASONS LET IT INTACT
    virtual
    mfxStatus RunFrameVmeENCCheck(mfxFrameCUC *cuc,
                                  MFX_ENTRY_POINT *pEntryPoint)
    {
        cuc = cuc;
        pEntryPoint = pEntryPoint;
        return MFX_ERR_NONE;
    }
    virtual
    mfxStatus RunFrameVmeENC(mfxFrameCUC *cuc) = 0;

};

class VideoPAK
{
public:
    // Destructor
    virtual
    ~VideoPAK(void) {}

    virtual
    mfxStatus Init(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Reset(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetVideoParam(mfxVideoParam *par) = 0;
    virtual
    mfxStatus GetFrameParam(mfxFrameParam *par) = 0;

    virtual
    mfxStatus RunSeqHeader(mfxFrameCUC *cuc) = 0;

    // THIS METHOD SHOULD BECOME PURE VIRTUAL TOO,
    // BUT FOR COMPATIBILITY REASONS LET IT INTACT
    virtual
    mfxStatus RunFramePAKCheck(mfxFrameCUC *cuc, MFX_ENTRY_POINT *pEntryPoint)
    {
        cuc = cuc;
        pEntryPoint = pEntryPoint;
        return MFX_ERR_NONE;
    }
    virtual
    mfxStatus RunFramePAK(mfxFrameCUC *cuc) = 0;

};

class VideoBSD {
public:
    // Destructor
    virtual ~VideoBSD(void) {}

    virtual mfxStatus Reset(mfxVideoParam *par)=0;
    virtual mfxStatus Close(void)=0;

    virtual mfxStatus GetVideoParam(mfxVideoParam *par)=0;
    virtual mfxStatus GetFrameParam(mfxFrameParam *par)=0;
    virtual mfxStatus GetSliceParam(mfxSliceParam *par)=0;

    virtual mfxStatus RunVideoParam(mfxBitstream *bs, mfxVideoParam *par)=0;
    virtual mfxStatus RunFrameParam(mfxBitstream *bs, mfxFrameParam *par)=0;
    virtual mfxStatus RunSliceParam(mfxBitstream *bs, mfxSliceParam *par)=0;

    virtual mfxStatus RunSliceBSD(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunSliceMFX(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunFrameBSD(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunFrameMFX(mfxFrameCUC *cuc)=0;
    virtual mfxStatus ExtractUserData(mfxBitstream *bs,mfxU8 *ud,mfxU32 *sz,mfxU64 *ts)=0;
};

class VideoDEC {
public:
    // Destructor
    virtual ~VideoDEC(void) {}

    virtual mfxStatus Init(mfxVideoParam *par)=0;
    virtual mfxStatus Reset(mfxVideoParam *par)=0;
    virtual mfxStatus Close(void)=0;

    virtual mfxStatus GetVideoParam(mfxVideoParam *par)=0;
    virtual mfxStatus GetFrameParam(mfxFrameParam *par)=0;
    virtual mfxStatus GetSliceParam(mfxSliceParam *par)=0;

    virtual mfxStatus RunFrameFullDEC(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunFramePredDEC(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunFrameIQT(mfxFrameCUC *cuc, mfxU8 scan)=0;
    virtual mfxStatus GetFrameRecon(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunFrameILDB(mfxFrameCUC *cuc)=0;

    virtual mfxStatus RunSliceFullDEC(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunSlicePredDEC(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunSliceIQT(mfxFrameCUC *cuc, mfxU8 scan)=0;
    virtual mfxStatus GetSliceRecon(mfxFrameCUC *cuc)=0;
    virtual mfxStatus RunSliceILDB(mfxFrameCUC *cuc)=0;
};

// mfxEncodeInternalParams
typedef enum
{

    MFX_IFLAG_ADD_HEADER = 1,  // MPEG2: add SeqHeader before this frame
    MFX_IFLAG_ADD_EOS = 2,     // MPEG2: add EOS after this frame
    MFX_IFLAG_BWD_ONLY = 4     // MPEG2: only backward prediction for this frame

} MFX_ENCODE_INTERNAL_FLAGS;


typedef struct _mfxEncodeInternalParams : public mfxEncodeCtrl
{
    mfxU32              FrameOrder;
    mfxU32              InternalFlags; //MFX_ENCODE_INTERNAL_FLAGS
    mfxFrameSurface1    *surface;
} mfxEncodeInternalParams;

class VideoENCODE
{
public:
    // Destructor
    virtual
    ~VideoENCODE(void) {}

    virtual
    mfxStatus Init(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Reset(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetVideoParam(mfxVideoParam *par) = 0;
    virtual
    mfxStatus GetFrameParam(mfxFrameParam *par) = 0;
    virtual
    mfxStatus GetEncodeStat(mfxEncodeStat *stat) = 0;
    virtual
    mfxStatus EncodeFrameCheck(mfxEncodeCtrl *ctrl,
                               mfxFrameSurface1 *surface,
                               mfxBitstream *bs,
                               mfxFrameSurface1 **reordered_surface,
                               mfxEncodeInternalParams *pInternalParams,
                               MFX_ENTRY_POINT *pEntryPoint)
    {
        pEntryPoint = pEntryPoint;
        return EncodeFrameCheck(ctrl, surface, bs, reordered_surface, pInternalParams);
    }
    virtual
    mfxStatus EncodeFrameCheck(mfxEncodeCtrl *ctrl,
                               mfxFrameSurface1 *surface,
                               mfxBitstream *bs,
                               mfxFrameSurface1 **reordered_surface,
                               mfxEncodeInternalParams *pInternalParams,
                               MFX_ENTRY_POINT pEntryPoints[],
                               mfxU32 &numEntryPoints)
    {
        mfxStatus mfxRes;

        // call the overweighted version
        mfxRes = EncodeFrameCheck(ctrl, surface, bs, reordered_surface, pInternalParams, pEntryPoints);
        numEntryPoints = 1;

        return mfxRes;
    }
    virtual
    mfxStatus EncodeFrameCheck(mfxEncodeCtrl *ctrl,
                               mfxFrameSurface1 *surface,
                               mfxBitstream *bs,
                               mfxFrameSurface1 **reordered_surface,
                               mfxEncodeInternalParams *pInternalParams,
                               MFX_ENTRY_POINT pEntryPoints[],
                               mfxU32 &numEntryPoints,
                               mfxExtVppAuxData * (&aux))
    {
        mfxStatus mfxRes;

        // call the overweighted version
        mfxRes = EncodeFrameCheck(ctrl, surface, bs, reordered_surface, pInternalParams, pEntryPoints, numEntryPoints);
        aux = 0;

        return mfxRes;
    }
    virtual
    mfxStatus EncodeFrameCheck(mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxFrameSurface1 **reordered_surface, mfxEncodeInternalParams *pInternalParams) = 0;
    virtual
    mfxStatus EncodeFrame(mfxEncodeCtrl *ctrl, mfxEncodeInternalParams *pInternalParams, mfxFrameSurface1 *surface, mfxBitstream *bs) = 0;
    virtual
    mfxStatus CancelFrame(mfxEncodeCtrl *ctrl, mfxEncodeInternalParams *pInternalParams, mfxFrameSurface1 *surface, mfxBitstream *bs) = 0;

};

class VideoDECODE
{
public:
    // Destructor
    virtual
    ~VideoDECODE(void) {}

    virtual
    mfxStatus Init(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Reset(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetVideoParam(mfxVideoParam *par) = 0;
    virtual
    mfxStatus GetDecodeStat(mfxDecodeStat *stat) = 0;
    virtual
    mfxStatus DecodeFrameCheck(mfxBitstream *bs,
                               mfxFrameSurface1 *surface_work,
                               mfxFrameSurface1 **surface_out,
                               MFX_ENTRY_POINT *pEntryPoint)
    {
        pEntryPoint = pEntryPoint;
        return DecodeFrameCheck(bs, surface_work, surface_out);
    }
    virtual
    mfxStatus DecodeFrameCheck(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out) = 0;
    virtual
    mfxStatus DecodeFrame(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 *surface_out) = 0;
    virtual
    mfxStatus SetSkipMode(mfxSkipMode mode) {mode=mode;return MFX_ERR_UNSUPPORTED;};
    virtual mfxStatus GetPayload(mfxU64 *ts, mfxPayload *payload) = 0;

};

class VideoVPP
{
public:
    // Destructor
    virtual
    ~VideoVPP(void) {}

    virtual
    mfxStatus Init(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Reset(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetVideoParam(mfxVideoParam *par) = 0;
    virtual
    mfxStatus GetVPPStat(mfxVPPStat *stat) = 0;
    virtual
    mfxStatus VppFrameCheck(mfxFrameSurface1 *in,
                            mfxFrameSurface1 *out,
                            mfxExtVppAuxData *aux,
                            MFX_ENTRY_POINT *pEntryPoint)
    {
        pEntryPoint = pEntryPoint;
        aux = aux;
        return VppFrameCheck(in, out);
    }

    virtual
    mfxStatus VppFrameCheck(mfxFrameSurface1 *in,
                            mfxFrameSurface1 *out,
                            mfxExtVppAuxData *aux,
                            MFX_ENTRY_POINT pEntryPoints[],
                            mfxU32 &numEntryPoints)
    {
        mfxStatus mfxRes;

        // call the overweighted version
        mfxRes = VppFrameCheck(in, out, aux, pEntryPoints);
        numEntryPoints = 1;

        return mfxRes;
    }

    virtual
    mfxStatus VppFrameCheck(mfxFrameSurface1 *in, mfxFrameSurface1 *out) = 0;
    virtual
    mfxStatus RunFrameVPP(mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux) = 0;

};

// forward declaration of used types
struct mfxPlugin;
struct mfxCoreInterface;

class VideoUSER
{
public:
    // Destructor
    virtual
    ~VideoUSER(void) {};

    // Initialize the user's plugin
    virtual
    mfxStatus PluginInit(const mfxPlugin *pParam,
                   mfxSession session,
                   mfxU32 type = MFX_PLUGINTYPE_VIDEO_GENERAL) = 0;
    // Release the user's plugin
    virtual
    mfxStatus PluginClose(void) = 0;
    // Get the plugin's threading policy
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    // Check the parameters to start a new tasl
    virtual
    mfxStatus Check(const mfxHDL *in, mfxU32 in_num,
                    const mfxHDL *out, mfxU32 out_num,
                    MFX_ENTRY_POINT *pEntryPoint) = 0;

};

class VideoCodecUSER
    : public VideoUSER
{
public:
    //statically exposed for mediasdk components but are plugin dependent
    virtual mfxStatus Init(mfxVideoParam *par) = 0;
    virtual mfxStatus QueryIOSurf(VideoCORE *core, mfxVideoParam *par, mfxFrameAllocRequest *in, mfxFrameAllocRequest *out) = 0;
    virtual mfxStatus Query(VideoCORE *core, mfxVideoParam *in, mfxVideoParam *out) = 0;
    virtual mfxStatus DecodeHeader(VideoCORE *core, mfxBitstream *bs, mfxVideoParam *par) = 0;
    virtual mfxStatus VPPFrameCheck(mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, MFX_ENTRY_POINT *ep) = 0;

    //expose new encoder/decoder view
    virtual VideoENCODE* GetEncodePtr() = 0;
    virtual VideoDECODE* GetDecodePtr() = 0;
    virtual VideoVPP* GetVPPPtr() = 0;
};

#endif // __MFXVIDEOPLUSPLUS_INTERNAL_H
