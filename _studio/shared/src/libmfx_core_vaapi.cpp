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

File Name: libmfx_core_vaapi.cpp

\* ****************************************************************************** */

#include <iostream>

#include "mfx_common.h"

#if defined (MFX_VA_LINUX)

#include "umc_va_linux.h"
#include "libmfx_core_vaapi.h"
#include "mfx_utils.h"
#include "mfx_session.h"
#include "ippi.h"
#include "mfx_common_decode_int.h"

#if defined(MFX_ENABLE_CM)
    #include "cm_mem_copy.h"
#endif // #if defined(MFX_ENABLE_CM)

#include <sys/ioctl.h>

#include "va/va.h"
#include <va/va_backend.h>

#if defined(ANDROID)
#include <va/va_tpi.h>
#include <va/va_backend_vpp.h>
#endif

#define MFX_CHECK_HDL(hdl) {if (!hdl) MFX_RETURN(MFX_ERR_INVALID_HANDLE);}
//#define MFX_GUID_CHECKING

/* Definitions below extracted from kernel DRM headers files */
/* START: IOCTLs structure definitions */
typedef struct drm_i915_getparam {
    int param;
    int *value;
} drm_i915_getparam_t;

#define I915_PARAM_CHIPSET_ID            4
#define DRM_I915_GETPARAM   0x06
#define DRM_IOCTL_BASE          'd'
#define DRM_COMMAND_BASE                0x40
#define DRM_IOWR(nr,type)       _IOWR(DRM_IOCTL_BASE,nr,type)
#define DRM_IOCTL_I915_GETPARAM         DRM_IOWR(DRM_COMMAND_BASE + DRM_I915_GETPARAM, drm_i915_getparam_t)

typedef struct {
    int device_id;
    eMFXHWType platform;
} mfx_device_item;

/* list of legal dev ID for Intel's graphics
 * Copied form i915_drv.c from linux kernel 3.9-rc7
 * */
const mfx_device_item listLegalDevIDs[] = {
    /*IVB*/
    { 0x0156, MFX_HW_IVB },   /* GT1 mobile */
    { 0x0166, MFX_HW_IVB },   /* GT2 mobile */
    { 0x0152, MFX_HW_IVB },   /* GT1 desktop */
    { 0x0162, MFX_HW_IVB },   /* GT2 desktop */
    { 0x015a, MFX_HW_IVB },   /* GT1 server */
    { 0x016a, MFX_HW_IVB },   /* GT2 server */
    /*HSW*/
    { 0x0402, MFX_HW_HSW },   /* GT1 desktop */
    { 0x0412, MFX_HW_HSW },   /* GT2 desktop */
    { 0x0422, MFX_HW_HSW },   /* GT2 desktop */
    { 0x041e, MFX_HW_HSW },   /* Core i3-4130 */
    { 0x040a, MFX_HW_HSW },   /* GT1 server */
    { 0x041a, MFX_HW_HSW },   /* GT2 server */
    { 0x042a, MFX_HW_HSW },   /* GT2 server */
    { 0x0406, MFX_HW_HSW },   /* GT1 mobile */
    { 0x0416, MFX_HW_HSW },   /* GT2 mobile */
    { 0x0426, MFX_HW_HSW },   /* GT2 mobile */
    { 0x0C02, MFX_HW_HSW },   /* SDV GT1 desktop */
    { 0x0C12, MFX_HW_HSW },   /* SDV GT2 desktop */
    { 0x0C22, MFX_HW_HSW },   /* SDV GT2 desktop */
    { 0x0C0A, MFX_HW_HSW },   /* SDV GT1 server */
    { 0x0C1A, MFX_HW_HSW },   /* SDV GT2 server */
    { 0x0C2A, MFX_HW_HSW },   /* SDV GT2 server */
    { 0x0C06, MFX_HW_HSW },   /* SDV GT1 mobile */
    { 0x0C16, MFX_HW_HSW },   /* SDV GT2 mobile */
    { 0x0C26, MFX_HW_HSW },   /* SDV GT2 mobile */
    { 0x0A02, MFX_HW_HSW },   /* ULT GT1 desktop */
    { 0x0A12, MFX_HW_HSW },   /* ULT GT2 desktop */
    { 0x0A22, MFX_HW_HSW },   /* ULT GT2 desktop */
    { 0x0A0A, MFX_HW_HSW },   /* ULT GT1 server */
    { 0x0A1A, MFX_HW_HSW },   /* ULT GT2 server */
    { 0x0A2A, MFX_HW_HSW },   /* ULT GT2 server */
    { 0x0A06, MFX_HW_HSW },   /* ULT GT1 mobile */
    { 0x0A16, MFX_HW_HSW },   /* ULT GT2 mobile */
    { 0x0A26, MFX_HW_HSW },   /* ULT GT2 mobile */
    { 0x0D02, MFX_HW_HSW },   /* CRW GT1 desktop */
    { 0x0D12, MFX_HW_HSW },   /* CRW GT2 desktop */
    { 0x0D22, MFX_HW_HSW },   /* CRW GT2 desktop */
    { 0x0D0A, MFX_HW_HSW },   /* CRW GT1 server */
    { 0x0D1A, MFX_HW_HSW },   /* CRW GT2 server */
    { 0x0D2A, MFX_HW_HSW },   /* CRW GT2 server */
    { 0x0D06, MFX_HW_HSW },   /* CRW GT1 mobile */
    { 0x0D16, MFX_HW_HSW },   /* CRW GT2 mobile */
    { 0x0D26, MFX_HW_HSW },   /* CRW GT2 mobile */
    /* this dev IDs added per HSD 5264859 request  */
    { 0x040B, MFX_HW_HSW }, /*HASWELL_B_GT1 *//* Reserved */
    { 0x041B, MFX_HW_HSW }, /*HASWELL_B_GT2*/
    { 0x042B, MFX_HW_HSW }, /*HASWELL_B_GT3*/
    { 0x040E, MFX_HW_HSW }, /*HASWELL_E_GT1*//* Reserved */
    { 0x041E, MFX_HW_HSW }, /*HASWELL_E_GT2*/
    { 0x042E, MFX_HW_HSW }, /*HASWELL_E_GT3*/

    { 0x0C0B, MFX_HW_HSW }, /*HASWELL_SDV_B_GT1*/ /* Reserved */
    { 0x0C1B, MFX_HW_HSW }, /*HASWELL_SDV_B_GT2*/
    { 0x0C2B, MFX_HW_HSW }, /*HASWELL_SDV_B_GT3*/
    { 0x0C0E, MFX_HW_HSW }, /*HASWELL_SDV_B_GT1*//* Reserved */
    { 0x0C1E, MFX_HW_HSW }, /*HASWELL_SDV_B_GT2*/
    { 0x0C2E, MFX_HW_HSW }, /*HASWELL_SDV_B_GT3*/

    { 0x0A0B, MFX_HW_HSW }, /*HASWELL_ULT_B_GT1*/ /* Reserved */
    { 0x0A1B, MFX_HW_HSW }, /*HASWELL_ULT_B_GT2*/
    { 0x0A2B, MFX_HW_HSW }, /*HASWELL_ULT_B_GT3*/
    { 0x0A0E, MFX_HW_HSW }, /*HASWELL_ULT_E_GT1*/ /* Reserved */
    { 0x0A1E, MFX_HW_HSW }, /*HASWELL_ULT_E_GT2*/
    { 0x0A2E, MFX_HW_HSW }, /*HASWELL_ULT_E_GT3*/

    { 0x0D0B, MFX_HW_HSW }, /*HASWELL_CRW_B_GT1*/ /* Reserved */
    { 0x0D1B, MFX_HW_HSW }, /*HASWELL_CRW_B_GT2*/
    { 0x0D2B, MFX_HW_HSW }, /*HASWELL_CRW_B_GT3*/
    { 0x0D0E, MFX_HW_HSW }, /*HASWELL_CRW_E_GT1*/ /* Reserved */
    { 0x0D1E, MFX_HW_HSW }, /*HASWELL_CRW_E_GT2*/
    { 0x0D2E, MFX_HW_HSW }, /*HASWELL_CRW_E_GT3*/

    /* VLV */
    { 0x0f30, MFX_HW_VLV },   /* VLV mobile */
    { 0x0f31, MFX_HW_VLV },   /* VLV mobile */
    { 0x0157, MFX_HW_VLV },
    { 0x0155, MFX_HW_VLV },

    /* BDW */
    /*GT3: */
    { 0x162D, MFX_HW_BDW},
    { 0x162A, MFX_HW_BDW},
    /*GT2: */
    { 0x161D, MFX_HW_BDW},
    { 0x161A, MFX_HW_BDW},
    /* GT1: */
    { 0x160D, MFX_HW_BDW},
    { 0x160A, MFX_HW_BDW},
    /* BDW-ULT */
    /* (16x2 - ULT, 16x6 - ULT, 16xB - Iris, 16xE - ULX) */
    /*GT3: */
    { 0x162E, MFX_HW_BDW},
    { 0x162B, MFX_HW_BDW},
    { 0x1626, MFX_HW_BDW},
    { 0x1622, MFX_HW_BDW},
    /* GT2: */
    { 0x161E, MFX_HW_BDW},
    { 0x161B, MFX_HW_BDW},
    { 0x1616, MFX_HW_BDW},
    { 0x1612, MFX_HW_BDW},
    /* GT1: */
    { 0x160E, MFX_HW_BDW},
    { 0x160B, MFX_HW_BDW},
    { 0x1606, MFX_HW_BDW},
    { 0x1602, MFX_HW_BDW}
};

/* END: IOCTLs definitions */

#define TMP_DEBUG

using namespace std;
using namespace UMC;

#pragma warning(disable: 4311) // in HWVideoCORE::TraceFrames(): pointer truncation from 'void*' to 'int'

static
VideoAccelerationHW ConvertMFXToUMCType(eMFXHWType type)
{
    VideoAccelerationHW umcType = VA_HW_UNKNOWN;

    switch(type)
    {
    case MFX_HW_LAKE:
        umcType = VA_HW_LAKE;
        break;

    case MFX_HW_LRB:
        umcType = VA_HW_LRB;
        break;
    case MFX_HW_SNB:
        umcType = VA_HW_SNB;
        break;
    case MFX_HW_IVB:
        umcType = VA_HW_IVB;
        break;
    case MFX_HW_HSW:
        umcType = VA_HW_HSW;
        break;
    default:
        break;
    }

    return umcType;
    
} // VideoAccelerationHW ConvertMFXToUMCType(eMFXHWType type)

static
eMFXHWType getPlatformType (VADisplay pVaDisplay)
{
    /* This is value by default */
    eMFXHWType retPlatformType = MFX_HW_UNKNOWN;
    int fd = 0, i = 0, listSize = 0;
    int devID = 0;
    int ret = 0;
    drm_i915_getparam_t gp;
    VADisplayContextP pDisplayContext_test = NULL;
    VADriverContextP  pDriverContext_test = NULL;

    pDisplayContext_test = (VADisplayContextP) pVaDisplay;
    pDriverContext_test  = pDisplayContext_test->pDriverContext;
    fd = *(int*) pDriverContext_test->drm_state;

    /* Now as we know real authenticated fd of VAAPI library,
     * we can call ioctl() to kernel mode driver,
     * get device ID and find out platform type
     * */
    gp.param = I915_PARAM_CHIPSET_ID;
    gp.value = &devID;

    ret = ioctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (!ret)
    {
        listSize = (sizeof(listLegalDevIDs)/sizeof(mfx_device_item));
        for (i = 0; i < listSize; ++i)
        {
            if (listLegalDevIDs[i].device_id == devID)
            {
                retPlatformType = listLegalDevIDs[i].platform;
                break;
            }
        }
    }

#if defined(ANDROID)
    /* Platforms corrections for Android:
     *  - treat VLV as IVB for now
     */
    /*if (MFX_HW_VLV == retPlatformType)*/ retPlatformType = MFX_HW_IVB;
#endif

    return retPlatformType;
} // eMFXHWType getPlatformType (VADisplay pVaDisplay)


VAAPIVideoCORE::VAAPIVideoCORE(
    const mfxU32 adapterNum,
    const mfxU32 numThreadsAvailable,
    const mfxSession session) :
            CommonCORE(numThreadsAvailable, session)
          , m_Display(0)
          , m_adapterNum(adapterNum)
          , m_bUseExtAllocForHWFrames(false)
          , m_HWType(MFX_HW_IVB) //MFX_HW_UNKNOWN
#if !defined(ANDROID)
#if defined(MFX_ENABLE_CM)
          , m_bCmCopy(false)
          , m_bCmCopyAllowed(false)
#endif
#else
          , m_bCmCopy(false)
          , m_bCmCopyAllowed(false)
          , m_va_config(VA_INVALID_ID)
          , m_va_CM_context(VA_INVALID_ID)
          , m_bCM_Initialized(false)
          , m_CM_CurIndex(0)
#endif
{
} // VAAPIVideoCORE::VAAPIVideoCORE(...)


VAAPIVideoCORE::~VAAPIVideoCORE()
{
#if defined(MFX_ENABLE_CM)
    if (m_bCmCopy)
    {
        m_pCmCopy.get()->Release();
        m_bCmCopy = false;
    }
#endif

    Close();
    
} // VAAPIVideoCORE::~VAAPIVideoCORE()


void VAAPIVideoCORE::Close()
{
    m_pVA.reset();
#if defined(ANDROID)
    CloseCM();
#endif
} // void VAAPIVideoCORE::Close()


mfxStatus
VAAPIVideoCORE::SetHandle(
    mfxHandleType type,
    mfxHDL hdl)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    try
    {
        mfxStatus sts = CommonCORE::SetHandle(type, hdl);

        MFX_CHECK_STS(sts);
        m_Display = (VADisplay)m_hdl;

        /* As we know right VA handle (pointer),
         * we can get real authenticated fd of VAAPI library(display),
         * and can call ioctl() to kernel mode driver,
         * to get device ID and find out platform type
         */
        m_HWType = getPlatformType(m_Display);

        return MFX_ERR_NONE;
    }
    catch (MFX_CORE_CATCH_TYPE)
    {
        ReleaseHandle();
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
}// mfxStatus VAAPIVideoCORE::SetHandle(mfxHandleType type, mfxHDL handle)


mfxStatus
VAAPIVideoCORE::TraceFrames(
    mfxFrameAllocRequest* request,
    mfxFrameAllocResponse* response,
    mfxStatus sts)
{
    return sts;
}

mfxStatus
VAAPIVideoCORE::AllocFrames(
    mfxFrameAllocRequest* request,
    mfxFrameAllocResponse* response,
    bool /* isNeedCopy */)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    try
    {
        MFX_CHECK_NULL_PTR2(request, response);
        mfxStatus sts = MFX_ERR_NONE;
        mfxFrameAllocRequest temp_request = *request;

        // external allocator doesn't know how to allocate opaque surfaces
        // we can treat opaque as internal
        if (temp_request.Type & MFX_MEMTYPE_OPAQUE_FRAME)
        {
            temp_request.Type -= MFX_MEMTYPE_OPAQUE_FRAME;
            temp_request.Type |= MFX_MEMTYPE_INTERNAL_FRAME;
        }
        
        if (!m_bFastCopy)
        {
            // initialize fast copy
            m_pFastCopy.reset(new FastCopy());
            m_pFastCopy.get()->Initialize();

            m_bFastCopy = true;
        }

#if defined(MFX_ENABLE_CM)
        if (!m_bCmCopy && m_bCmCopyAllowed && isNeedCopy && m_Display)
        {
            m_pCmCopy.reset(new CmCopyWrapper);
            if (!m_pCmCopy.get()->GetCmDevice(m_Display)){
                //!!!! WA: CM restricts multiple CmDevice creation from different device managers.
                //if failed to create CM device, continue without CmCopy
                m_bCmCopy = false;
                m_bCmCopyAllowed = false;
                m_pCmCopy.get()->Release();
                m_pCmCopy.reset();
                //return MFX_ERR_DEVICE_FAILED;
            }else{
                sts = m_pCmCopy.get()->Initialize();
                MFX_CHECK_STS(sts);
                m_bCmCopy = true;
            }
        }else if(m_bCmCopy){
            if(m_pCmCopy.get())
                m_pCmCopy.get()->ReleaseCmSurfaces();
            else
                m_bCmCopy = false;
        }
#endif // #if defined(MFX_ENABLE_CM)

        // use common core for sw surface allocation
        if (request->Type & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            sts = CommonCORE::AllocFrames(request, response);
            return TraceFrames(request, response, sts);
        } else
        {
            // external allocator
            if (m_bSetExtFrameAlloc)
            {
                // Default allocator should be used if D3D manager is not set and internal D3D buffers are required
                if (!m_Display && request->Type & MFX_MEMTYPE_INTERNAL_FRAME)
                {
                    m_bUseExtAllocForHWFrames = false;
                    sts = DefaultAllocFrames(request, response);
                    MFX_CHECK_STS(sts);

                    return TraceFrames(request, response, sts);
                }

                sts = (*m_FrameAllocator.frameAllocator.Alloc)(m_FrameAllocator.frameAllocator.pthis, &temp_request, response);

                // if external allocator cannot allocate d3d frames - use default memory allocator
                if (MFX_ERR_UNSUPPORTED == sts || MFX_ERR_MEMORY_ALLOC == sts)
                {
                    // Default Allocator is used for internal memory allocation only
                    if (request->Type & MFX_MEMTYPE_EXTERNAL_FRAME)
                        return sts;
                    m_bUseExtAllocForHWFrames = false;
                    sts = DefaultAllocFrames(request, response);
                    MFX_CHECK_STS(sts);

                    return TraceFrames(request, response, sts);
                }
                // let's create video accelerator 
                else if (MFX_ERR_NONE == sts)
                {
                    // Checking for unsupported mode - external allocator exist but Device handle doesn't set 
                    if (!m_Display) 
                        return MFX_ERR_UNSUPPORTED;

                    m_bUseExtAllocForHWFrames = true;
                    sts = ProcessRenderTargets(request, response, &m_FrameAllocator);
                    MFX_CHECK_STS(sts);
                   
                    return TraceFrames(request, response, sts);
                }
                // error situation
                else 
                {
                    m_bUseExtAllocForHWFrames = false;
                    return sts;
                }
            }
            else
            {
                // Default Allocator is used for internal memory allocation only
                if (request->Type & MFX_MEMTYPE_EXTERNAL_FRAME)
                    return MFX_ERR_MEMORY_ALLOC;

                m_bUseExtAllocForHWFrames = false;
                sts = DefaultAllocFrames(request, response);
                MFX_CHECK_STS(sts);

                return TraceFrames(request, response, sts);
            }
        }
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
    
} // mfxStatus VAAPIVideoCORE::AllocFrames(...)


mfxStatus
VAAPIVideoCORE::DefaultAllocFrames(
    mfxFrameAllocRequest* request,
    mfxFrameAllocResponse* response)
{
    mfxStatus sts = MFX_ERR_NONE;

    if ((request->Type & MFX_MEMTYPE_DXVA2_DECODER_TARGET)||
        (request->Type & MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET)) // SW - TBD !!!!!!!!!!!!!!
    {
        if (!m_Display)
            return MFX_ERR_NOT_INITIALIZED;

        mfxBaseWideFrameAllocator* pAlloc = GetAllocatorByReq(request->Type);
        // VPP, ENC, PAK can request frames for several times
        if (pAlloc && (request->Type & MFX_MEMTYPE_FROM_DECODE))
            return MFX_ERR_MEMORY_ALLOC;

        if (!pAlloc)
        {
            m_pcHWAlloc.reset(new mfxDefaultAllocatorVAAPI::mfxWideHWFrameAllocator(request->Type, m_Display));
            pAlloc = m_pcHWAlloc.get();
        }
        // else ???

        pAlloc->frameAllocator.pthis = pAlloc;
        sts = (*pAlloc->frameAllocator.Alloc)(pAlloc->frameAllocator.pthis,request, response);
        MFX_CHECK_STS(sts);
        sts = ProcessRenderTargets(request, response, pAlloc);
        MFX_CHECK_STS(sts);
        
    }
    else 
    {
        return CommonCORE::DefaultAllocFrames(request, response);
    }
    ++m_NumAllocators;
    
    return sts;
    
} // mfxStatus VAAPIVideoCORE::DefaultAllocFrames(...)


mfxStatus
VAAPIVideoCORE::CreateVA(
    mfxVideoParam* param,
    mfxFrameAllocRequest* request,
    mfxFrameAllocResponse* response)
{
    mfxStatus sts = MFX_ERR_NONE;

    if (!(request->Type & MFX_MEMTYPE_FROM_DECODE) ||
        !(request->Type & MFX_MEMTYPE_DXVA2_DECODER_TARGET))
        return MFX_ERR_NONE;

    int profile = UMC::VA_VLD;

    // video accelerator is needed for decoders only 
    switch (param->mfx.CodecId)
    {
    case MFX_CODEC_VC1:
        profile |= VA_VC1;
        break;
    case MFX_CODEC_MPEG2:
        profile |= VA_MPEG2;
        break;
    case MFX_CODEC_AVC:
        profile |= VA_H264;
        break;
    case MFX_CODEC_VP8:
        profile |= VA_VP8;
        break;
    default:
        return MFX_ERR_UNSUPPORTED;
    }

    VASurfaceID RenderTargets[128]; // 32 should be enough

    for (mfxU32 i = 0; i < response->NumFrameActual; i++)
    {
        mfxMemId InternalMid = response->mids[i];
        mfxFrameAllocator* pAlloc = GetAllocatorAndMid(InternalMid);
        VASurfaceID *pSurface = NULL;
        if (pAlloc)
            pAlloc->GetHDL(pAlloc->pthis, InternalMid, (mfxHDL*)&pSurface);
        else
            return MFX_ERR_UNDEFINED_BEHAVIOR;

        RenderTargets[i] = *pSurface;
    }

    m_pVA.reset(new LinuxVideoAccelerator); //aya must be fixed late???

    sts = CreateVideoAccelerator(param, profile, response->NumFrameActual, RenderTargets);

    return sts;
    
} // mfxStatus VAAPIVideoCORE::CreateVA(...)


mfxStatus
VAAPIVideoCORE::ProcessRenderTargets(
    mfxFrameAllocRequest* request,
    mfxFrameAllocResponse* response,
    mfxBaseWideFrameAllocator* pAlloc)
{
    if (response->NumFrameActual > 128)
        return MFX_ERR_UNSUPPORTED;

    RegisterMids(response, request->Type, !m_bUseExtAllocForHWFrames, pAlloc);
    m_pcHWAlloc.pop();
    
    return MFX_ERR_NONE;
    
} // mfxStatus VAAPIVideoCORE::ProcessRenderTargets(

mfxStatus
VAAPIVideoCORE::GetVAService(
    VADisplay*  pVADisplay)
{
    // check if created already
    if (m_Display)
    {
        if (pVADisplay)
        {
            *pVADisplay = m_Display;
        }
        return MFX_ERR_NONE;
    }

    return MFX_ERR_NOT_INITIALIZED;
    
} // mfxStatus VAAPIVideoCORE::GetVAService(...)

mfxStatus
VAAPIVideoCORE::CreateVideoAccelerator(
    mfxVideoParam* param,
    int profile,
    int NumOfRenderTarget,
    VASurfaceID* RenderTargets)
{
    mfxStatus sts = MFX_ERR_NONE;

    Status st;
    UMC::LinuxVideoAcceleratorParams params;
    mfxFrameInfo *pInfo = &(param->mfx.FrameInfo);

    //m_pVA.reset(new LinuxVideoAccelerator); aya must be fixed late

    if (!m_Display)
        return MFX_ERR_NOT_INITIALIZED;

    UMC::VideoStreamInfo VideoInfo;
    VideoInfo.clip_info.width = pInfo->Width;
    VideoInfo.clip_info.height = pInfo->Height;

    m_pVA.get()->m_Platform = UMC::VA_LINUX;
    m_pVA.get()->m_Profile = (VideoAccelerationProfile)profile;

#if 0
    // Find DXVA2 configuration
    st = m_pVA->FindConfiguration(&VideoInfo, m_ExtOptions);
    if(UMC_OK != st)
    {
        m_pVA.reset();
        return MFX_ERR_UNSUPPORTED;
    }
#endif

    // Init Accelerator
    params.m_Display = m_Display;
    params.m_pVideoStreamInfo = &VideoInfo;
    params.m_iNumberSurfaces = NumOfRenderTarget;
    params.isExt = true;
    params.m_surf = (void **)RenderTargets;

    st = m_pVA.get()->Init(&params); //, m_ExtOptions);

    if(UMC_OK != st)
    {
        //m_pVA.reset(); aya must be fixed late
        return MFX_ERR_UNSUPPORTED;
    }

#if 0
    if (IS_PROTECTION_ANY(param->Protected))
    {
        DXVA2_DecodeExtensionData DecodeExtension;
        DecodeExtension.Function = DXVA2_DECODE_GET_DRIVER_HANDLE;
        DecodeExtension.pPrivateInputData = NULL;
        DecodeExtension.PrivateInputDataSize = 0;
        DecodeExtension.pPrivateOutputData = &m_DXVA2DecodeHandle;
        DecodeExtension.PrivateOutputDataSize = sizeof(m_DXVA2DecodeHandle);
        
        st = m_pVA->ExecuteExtensionBuffer(&DecodeExtension);
        if(UMC_OK != st)
        {
            m_pVA.reset();
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else
        m_DXVA2DecodeHandle = NULL;
#endif

    m_pVA.get()->m_HWPlatform = ConvertMFXToUMCType(m_HWType);

    return sts;
    
} // mfxStatus VAAPIVideoCORE::CreateVideoAccelerator(...)


mfxStatus
VAAPIVideoCORE::DoFastCopyWrapper(
    mfxFrameSurface1* pDst,
    mfxU16 dstMemType,
    mfxFrameSurface1* pSrc,
    mfxU16 srcMemType)
{
    mfxStatus sts;

    mfxHDL srcHandle, dstHandle;
    mfxMemId srcMemId, dstMemId;

    mfxFrameSurface1 srcTempSurface, dstTempSurface;

    memset(&srcTempSurface, 0, sizeof(mfxFrameSurface1));
    memset(&dstTempSurface, 0, sizeof(mfxFrameSurface1));

    // save original mem ids
    srcMemId = pSrc->Data.MemId;
    dstMemId = pDst->Data.MemId;

    srcTempSurface.Info = pSrc->Info;
    dstTempSurface.Info = pDst->Info;

    bool isSrcLocked = false;
    bool isDstLocked = false;

    if (srcMemType & MFX_MEMTYPE_EXTERNAL_FRAME)
    {
        if (srcMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            if (NULL == pSrc->Data.Y)
            {
                sts = LockExternalFrame(srcMemId, &srcTempSurface.Data);
                MFX_CHECK_STS(sts);
                    
                isSrcLocked = true;
            }
            else
            {
                srcTempSurface.Data = pSrc->Data;
                srcTempSurface.Data.MemId = 0;
            }
        }
        else if (srcMemType & MFX_MEMTYPE_DXVA2_DECODER_TARGET)
        {
            sts = GetExternalFrameHDL(srcMemId, &srcHandle);
            MFX_CHECK_STS(sts);

            srcTempSurface.Data.MemId = srcHandle;
        }
    }
    else if (srcMemType & MFX_MEMTYPE_INTERNAL_FRAME)
    {
        if (srcMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            if (NULL == pSrc->Data.Y)
            {
                sts = LockFrame(srcMemId, &srcTempSurface.Data);
                MFX_CHECK_STS(sts);

                isSrcLocked = true;
            }
            else
            {
                srcTempSurface.Data = pSrc->Data;
                srcTempSurface.Data.MemId = 0;
            }
        }
        else if (srcMemType & MFX_MEMTYPE_DXVA2_DECODER_TARGET)
        {
            sts = GetFrameHDL(srcMemId, &srcHandle);
            MFX_CHECK_STS(sts);

            srcTempSurface.Data.MemId = srcHandle;
        }
    }

    if (dstMemType & MFX_MEMTYPE_EXTERNAL_FRAME)
    {
        if (dstMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            if (NULL == pDst->Data.Y)
            {
                sts = LockExternalFrame(dstMemId, &dstTempSurface.Data);
                MFX_CHECK_STS(sts);

                isDstLocked = true;
            }
            else
            {
                dstTempSurface.Data = pDst->Data;
                dstTempSurface.Data.MemId = 0;
            }
        }
        else if (dstMemType & MFX_MEMTYPE_DXVA2_DECODER_TARGET)
        {
            sts = GetExternalFrameHDL(dstMemId, &dstHandle);
            MFX_CHECK_STS(sts);

            dstTempSurface.Data.MemId = dstHandle;
        }
    }
    else if (dstMemType & MFX_MEMTYPE_INTERNAL_FRAME)
    {
        if (dstMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            if (NULL == pDst->Data.Y)
            {
                sts = LockFrame(dstMemId, &dstTempSurface.Data);
                MFX_CHECK_STS(sts);

                isDstLocked = true;
            }
            else
            {
                dstTempSurface.Data = pDst->Data;
                dstTempSurface.Data.MemId = 0;
            }
        }
        else if (dstMemType & MFX_MEMTYPE_DXVA2_DECODER_TARGET)
        {
            sts = GetFrameHDL(dstMemId, &dstHandle);
            MFX_CHECK_STS(sts);

            dstTempSurface.Data.MemId = dstHandle;
        }
    }

    sts = DoFastCopyExtended(&dstTempSurface, &srcTempSurface);

    if (MFX_ERR_DEVICE_FAILED == sts && 0 != dstTempSurface.Data.Corrupted)
    {
        // complete task even if frame corrupted
        pDst->Data.Corrupted = dstTempSurface.Data.Corrupted;
        sts = MFX_ERR_NONE;
    }

    MFX_CHECK_STS(sts);

    if (true == isSrcLocked)
    {
        if (srcMemType & MFX_MEMTYPE_EXTERNAL_FRAME)
        {
            sts = UnlockExternalFrame(srcMemId, &srcTempSurface.Data);
            MFX_CHECK_STS(sts);
        }
        else if (srcMemType & MFX_MEMTYPE_INTERNAL_FRAME)
        {
            sts = UnlockFrame(srcMemId, &srcTempSurface.Data);
            MFX_CHECK_STS(sts);
        }
    }

    if (true == isDstLocked)
    {
        if (dstMemType & MFX_MEMTYPE_EXTERNAL_FRAME)
        {
            sts = UnlockExternalFrame(dstMemId, &dstTempSurface.Data);
            MFX_CHECK_STS(sts);
        }
        else if (dstMemType & MFX_MEMTYPE_INTERNAL_FRAME)
        {
            sts = UnlockFrame(dstMemId, &dstTempSurface.Data);
            MFX_CHECK_STS(sts);
        }
    }

    return MFX_ERR_NONE;
    
} // mfxStatus VAAPIVideoCORE::DoFastCopyWrapper(...)


mfxStatus
VAAPIVideoCORE::DoFastCopyExtended(
    mfxFrameSurface1* pDst,
    mfxFrameSurface1* pSrc)
{
    // check that only memId or pointer are passed
    // otherwise don't know which type of memory copying is requested
    if (
        (NULL != pDst->Data.Y && NULL != pDst->Data.MemId) ||
        (NULL != pSrc->Data.Y && NULL != pSrc->Data.MemId)
        )
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    IppiSize roi = {IPP_MIN(pSrc->Info.Width, pDst->Info.Width), IPP_MIN(pSrc->Info.Height, pDst->Info.Height)};
    
    // check that region of interest is valid
    if (0 == roi.width || 0 == roi.height)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    FastCopy *pFastCopy = m_pFastCopy.get();
#if defined(MFX_ENABLE_CM)
    CmCopyWrapper *pCmCopy = m_pCmCopy.get();
#endif

    mfxU16 srcPitch = pSrc->Data.Pitch;
    mfxU16 dstPitch = pDst->Data.Pitch;

    if (NULL != pSrc->Data.MemId && NULL != pDst->Data.MemId)
    {
#if 0
        //should be init m_hDirectXHandle ???

        hRes = m_pDirect3DDeviceManager->LockDevice(m_hDirectXHandle, &m_pDirect3DDevice, true);
        MFX_CHECK(SUCCEEDED(hRes), MFX_ERR_DEVICE_FAILED);

        const tagRECT rect = {0, 0, roi.width, roi.height};

        hRes = m_pDirect3DDevice->StretchRect((IDirect3DSurface9*) pSrc->Data.MemId, &rect, 
                                              (IDirect3DSurface9*) pDst->Data.MemId, &rect, 
                                               D3DTEXF_LINEAR);

        MFX_CHECK(SUCCEEDED(hRes), MFX_ERR_DEVICE_FAILED);

        hRes = m_pDirect3DDeviceManager->UnlockDevice(m_hDirectXHandle, false);
        MFX_CHECK(SUCCEEDED(hRes), MFX_ERR_DEVICE_FAILED);

        m_pDirect3DDevice->Release();
#endif
#if defined(MFX_ENABLE_CM)
        if (m_bCmCopy == true && pDst->Info.FourCC != MFX_FOURCC_YV12 && CM_SUPPORTED_COPY_SIZE(roi))
        {
            sts = pCmCopy->CopyVideoToVideoMemoryAPI(pDst->Data.MemId, pSrc->Data.Y, roi);
            MFX_CHECK_STS(sts);
        }
        else
#endif // #if defined(MFX_ENABLE_CM)
        {
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (NULL != pSrc->Data.MemId && NULL != pDst->Data.Y)
    {
        VASurfaceID *va_surface = (VASurfaceID*)(pSrc->Data.MemId);
        VAImage va_image;
        VAStatus va_sts;
        void *pBits = NULL;

        MFX_CHECK(m_Display,MFX_ERR_NOT_INITIALIZED);

#if defined(ANDROID)
        if((((unsigned long)pDst->Data.Y & (64 - 1)) == 0)
            && (pDst->Info.FourCC == MFX_FOURCC_NV12))
        // make CM fast copy only in case of output memory aligment equal to 64
        // and MFX_FOURCC_NV12 color format
        {
            if (!m_bCM_Initialized)
            {
                va_sts = InitCM(pDst->Info.Width, pDst->Info.Height);
                MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);
            }

            VASurfaceID va_dstSurface = CM_FindInPool(pDst->Data.Y);
            if (VA_INVALID_SURFACE == va_dstSurface)
            {
                VASurfaceAttributeTPI attribute_tpi;

                attribute_tpi.size = pDst->Info.Width * pDst->Info.Height * 3 / 2;
                attribute_tpi.luma_stride = pDst->Info.Width;
                attribute_tpi.buffers = (unsigned int*) pDst->Data.Y;
                attribute_tpi.tiling = 0;
                attribute_tpi.pixel_format = VA_FOURCC_NV12;
                attribute_tpi.type = VAExternalMemoryUserPointer;

                // Create surface
                va_sts = vaCreateSurfacesWithAttribute(
                           m_Display,
                           pDst->Info.Width,
                           pDst->Info.Height,
                           VA_RT_FORMAT_YUV420,
                           1,
                           &va_dstSurface,
                           &attribute_tpi
                          );
                MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);

                mfxStatus mfx_sts = CM_AddToPool(va_dstSurface, pDst->Data.Y);
                MFX_CHECK_STS(mfx_sts);
            }

            //CM fast copy call
            va_sts = CM_FastCopy(m_Display, m_va_CM_context, va_dstSurface, *va_surface, pDst->Info.Width, pDst->Info.Height);
            MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);

            return MFX_ERR_NONE;
        }
#endif

        va_sts = vaDeriveImage(m_Display, *va_surface, &va_image);
        MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);

        // vaMapBuffer
        va_sts = vaMapBuffer(m_Display, va_image.buf, (void **) &pBits);
        MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);
        
        // copy data
        {
            mfxI64 verticalPitch = (mfxI64)(pDst->Data.UV - pDst->Data.Y);
            verticalPitch = (verticalPitch % pDst->Data.Pitch)? 0 : verticalPitch / pDst->Data.Pitch;

#if defined(MFX_ENABLE_CM)
            if (m_bCmCopy == true && CM_ALIGNED(pDst->Data.Pitch) && pDst->Info.FourCC != MFX_FOURCC_YV12 && CM_ALIGNED(pDst->Data.Y) && CM_ALIGNED(pDst->Data.UV) && CM_SUPPORTED_COPY_SIZE(roi) && verticalPitch >= pDst->Info.Height && verticalPitch <= 16384)
            {
                sts = pCmCopy->CopyVideoToSystemMemoryAPI(pDst->Data.Y, pDst->Data.Pitch, (mfxU32)verticalPitch, pSrc->Data.MemId, 0, roi);
                MFX_CHECK_STS(sts);
            }
            else
#endif
            {

                Ipp32u srcPitch = va_image.pitches[0];
                Ipp32u Height =  va_image.height;

                mfxStatus sts;
                mfxU8* ptrY  = (mfxU8 *)pBits + va_image.offsets[0];
                mfxU8* ptrUV = (mfxU8 *)pBits + va_image.offsets[1];

                MFX_CHECK((pDst->Data.Y == 0) == (pDst->Data.UV == 0), MFX_ERR_UNDEFINED_BEHAVIOR);
                MFX_CHECK(dstPitch < 0x8000, MFX_ERR_UNDEFINED_BEHAVIOR);
                MFX_CHECK(srcPitch < 0x8000, MFX_ERR_UNDEFINED_BEHAVIOR);

                switch (pDst->Info.FourCC)
                {
                    case MFX_FOURCC_NV12:

                        sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, ptrY, srcPitch, roi);
                        MFX_CHECK_STS(sts);

                        roi.height >>= 1;

                        sts = pFastCopy->Copy(pDst->Data.UV, dstPitch, ptrUV, srcPitch, roi);
                        MFX_CHECK_STS(sts);

                        break;

                    case MFX_FOURCC_YV12:

                        sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, (mfxU8 *)pBits, srcPitch, roi);
                        MFX_CHECK_STS(sts);

                        roi.width >>= 1;
                        roi.height >>= 1;

                        srcPitch >>= 1;
                        dstPitch >>= 1;

                        sts = pFastCopy->Copy(pDst->Data.V, dstPitch, (mfxU8 *)pBits + (Height * srcPitch * 5) / 4, srcPitch, roi);
                        MFX_CHECK_STS(sts);

                        sts = pFastCopy->Copy(pDst->Data.U, dstPitch, (mfxU8 *)pBits + Height * srcPitch, srcPitch, roi);
                        MFX_CHECK_STS(sts);
                    
                        break;

                    case MFX_FOURCC_YUY2:
                    
                        roi.width *= 2;

                        sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, (mfxU8 *)pBits, srcPitch, roi);
                        MFX_CHECK_STS(sts);

                        break;

                    case MFX_FOURCC_RGB3:
                    {
                        MFX_CHECK_NULL_PTR1(pDst->Data.R);
                        MFX_CHECK_NULL_PTR1(pDst->Data.G);
                        MFX_CHECK_NULL_PTR1(pDst->Data.B);

                        mfxU8* ptrDst = IPP_MIN(IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B);
                    
                        roi.width *= 3;

                        sts = pFastCopy->Copy(ptrDst, dstPitch, (mfxU8 *)pBits, srcPitch, roi);
                        MFX_CHECK_STS(sts);
                    }
                        break;

                    case MFX_FOURCC_RGB4:
                    {
                        MFX_CHECK_NULL_PTR1(pDst->Data.R);
                        MFX_CHECK_NULL_PTR1(pDst->Data.G);
                        MFX_CHECK_NULL_PTR1(pDst->Data.B);

                        mfxU8* ptrDst = IPP_MIN(IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B);

                        roi.width *= 4;

                        sts = pFastCopy->Copy(ptrDst, dstPitch, (mfxU8 *)pBits, srcPitch, roi);
                        MFX_CHECK_STS(sts);
                    }
                        break;

                    case MFX_FOURCC_P8:

                        sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, (mfxU8 *)pBits, srcPitch, roi);
                        MFX_CHECK_STS(sts);

                        break;

                    default:

                        return MFX_ERR_UNSUPPORTED;
                }

            }
        }

        // vaUnmapBuffer
        va_sts = vaUnmapBuffer(m_Display, va_image.buf);
        MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);

        // vaDestroyImage
        va_sts = vaDestroyImage(m_Display, va_image.image_id);
        MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);
    
    }
    else if (NULL != pSrc->Data.Y && NULL != pDst->Data.Y)
    {
        // system memories were passed
        // use common way to copy frames

        MFX_CHECK((pSrc->Data.Y == 0) == (pSrc->Data.UV == 0), MFX_ERR_UNDEFINED_BEHAVIOR);
        MFX_CHECK((pDst->Data.Y == 0) == (pDst->Data.UV == 0), MFX_ERR_UNDEFINED_BEHAVIOR);
        MFX_CHECK(dstPitch < 0x8000, MFX_ERR_UNDEFINED_BEHAVIOR);
        MFX_CHECK(srcPitch < 0x8000, MFX_ERR_UNDEFINED_BEHAVIOR);

        switch (pDst->Info.FourCC)
        {
            case MFX_FOURCC_NV12:

                ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);
                
                roi.height >>= 1;

                ippiCopy_8u_C1R(pSrc->Data.UV, srcPitch, pDst->Data.UV, dstPitch, roi);

                break;

            case MFX_FOURCC_YV12:

                ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

                roi.width >>= 1;
                roi.height >>= 1;

                srcPitch >>= 1;
                dstPitch >>= 1;

                ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, pDst->Data.V, dstPitch, roi);

                ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, pDst->Data.U, dstPitch, roi);
                
                break;

            case MFX_FOURCC_YUY2:
                
                roi.width *= 2;

                ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

                break;

            case MFX_FOURCC_RGB3:
            {
                MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                mfxU8* ptrSrc = IPP_MIN(IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B);

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN(IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B);

                roi.width *= 3;

                ippiCopy_8u_C1R(ptrSrc, srcPitch, ptrDst, dstPitch, roi);
            }
                break;

            case MFX_FOURCC_RGB4:
            {
                MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                mfxU8* ptrSrc = IPP_MIN(IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B);

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN(IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B);

                roi.width *= 4;

                ippiCopy_8u_C1R(ptrSrc, srcPitch, ptrDst, dstPitch, roi);
            }
                break;

            case MFX_FOURCC_P8:

                ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

                break;

            default:

                return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (NULL != pSrc->Data.Y && NULL != pDst->Data.MemId)
    {
        mfxI64 verticalPitch = (mfxI64)(pSrc->Data.UV - pSrc->Data.Y);
        verticalPitch = (verticalPitch % pSrc->Data.Pitch)? 0 : verticalPitch / pSrc->Data.Pitch;

#if defined(MFX_ENABLE_CM)
        if (m_bCmCopy == true && CM_ALIGNED(pSrc->Data.Pitch) && pDst->Info.FourCC != MFX_FOURCC_YV12 && CM_ALIGNED(pSrc->Data.Y) && CM_ALIGNED(pSrc->Data.UV) && CM_SUPPORTED_COPY_SIZE(roi) && verticalPitch >= pSrc->Info.Height && verticalPitch <= 16384)
        {
            sts = pCmCopy->CopySystemToVideoMemoryAPI(pDst->Data.MemId, 0, pSrc->Data.Y, pSrc->Data.Pitch,(mfxU32)verticalPitch, roi);
            MFX_CHECK_STS(sts);
        }
        else
#endif
        {
            VAStatus va_sts = VA_STATUS_SUCCESS;
            VASurfaceID *va_surface = (VASurfaceID*)(size_t)pDst->Data.MemId;
            VAImage va_image;
            void *pBits = NULL;

            MFX_CHECK(m_Display, MFX_ERR_NOT_INITIALIZED);

            va_sts = vaDeriveImage(m_Display, *va_surface, &va_image);
            MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);

            // vaMapBuffer
            va_sts = vaMapBuffer(m_Display, va_image.buf, (void **) &pBits);
            MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);

            Ipp32u dstPitch = va_image.pitches[0];

            MFX_CHECK((pSrc->Data.Y == 0) == (pSrc->Data.UV == 0), MFX_ERR_UNDEFINED_BEHAVIOR);
            MFX_CHECK(dstPitch < 0x8000, MFX_ERR_UNDEFINED_BEHAVIOR);
            MFX_CHECK(srcPitch < 0x8000, MFX_ERR_UNDEFINED_BEHAVIOR);

            switch (pDst->Info.FourCC)
            {
                case MFX_FOURCC_NV12:

                    ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, (mfxU8 *)pBits + va_image.offsets[0], dstPitch, roi);

                    roi.height >>= 1;

                    ippiCopy_8u_C1R(pSrc->Data.UV, srcPitch, (mfxU8 *)pBits + va_image.offsets[1], dstPitch, roi);

                    break;

                case MFX_FOURCC_YV12:

                    ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, (mfxU8 *)pBits + va_image.offsets[0], dstPitch, roi);

                    roi.width >>= 1;
                    roi.height >>= 1;

                    srcPitch >>= 1;
                    dstPitch >>= 1;

                    ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, (mfxU8 *)pBits + va_image.offsets[1], dstPitch, roi);

                    ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, (mfxU8 *)pBits + va_image.offsets[2], dstPitch, roi);

                    break;
                case MFX_FOURCC_YUY2:
                
                    roi.width *= 2;

                    ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, (mfxU8 *)pBits + va_image.offsets[0], dstPitch, roi);

                    break;

                case MFX_FOURCC_RGB3:
                {
                    MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                    MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                    MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                    mfxU8* ptrSrc = IPP_MIN(IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B);

                    roi.width *= 3;

                    ippiCopy_8u_C1R(ptrSrc, srcPitch, (mfxU8 *)pBits + va_image.offsets[0], dstPitch, roi);
                }
                    break;

                case MFX_FOURCC_RGB4:
                {
                    MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                    MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                    MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                    mfxU8* ptrSrc = IPP_MIN(IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B);

                    roi.width *= 4;

                    ippiCopy_8u_C1R(ptrSrc, srcPitch, (mfxU8 *)pBits + va_image.offsets[0], dstPitch, roi);
                }
                    break;

                case MFX_FOURCC_P8:
                    ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, (mfxU8 *)pBits + va_image.offsets[0], dstPitch, roi);
                    break;

                default:

                    return MFX_ERR_UNSUPPORTED;
            }

            // vaUnmapBuffer
            va_sts = vaUnmapBuffer(m_Display, va_image.buf);
            MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);

            // vaDestroyImage
            va_sts = vaDestroyImage(m_Display, va_image.image_id);
            MFX_CHECK(VA_STATUS_SUCCESS == va_sts, MFX_ERR_DEVICE_FAILED);
        }
    }
    else
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
    
    return MFX_ERR_NONE;

} // mfxStatus VAAPIVideoCORE::DoFastCopyExtended(mfxFrameSurface1 *pDst, mfxFrameSurface1 *pSrc)

mfxStatus
VAAPIVideoCORE::DoFastCopy(
    mfxFrameSurface1* dst,
    mfxFrameSurface1* src)
{
    CommonCORE::DoFastCopy(dst, src);
    
    return MFX_ERR_NONE;

} // mfxStatus VAAPIVideoCORE::DoFastCopy(...)


void VAAPIVideoCORE::ReleaseHandle()
{
    
} // void VAAPIVideoCORE::ReleaseHandle()

mfxStatus VAAPIVideoCORE::IsGuidSupported(const GUID /*guid*/,
                                         mfxVideoParam *par, bool isEncoder) 
{
    if (!par)
        return MFX_WRN_PARTIAL_ACCELERATION;

    if (IsMVCProfile(par->mfx.CodecProfile) || IsSVCProfile(par->mfx.CodecProfile))
        return MFX_WRN_PARTIAL_ACCELERATION;

    switch (par->mfx.CodecId)
    {
    case MFX_CODEC_VC1:
        break;
    case MFX_CODEC_AVC:
        break;
    case MFX_CODEC_HEVC:
        return MFX_WRN_PARTIAL_ACCELERATION;
        break;
    case MFX_CODEC_MPEG2:
        if (par->mfx.FrameInfo.Width  > 2048 || par->mfx.FrameInfo.Height > 2048) //MPEG2 decoder doesn't support resolution bigger than 2K
            return MFX_WRN_PARTIAL_ACCELERATION;
        break;
    case MFX_CODEC_JPEG:
        return MFX_WRN_PARTIAL_ACCELERATION;
    case MFX_CODEC_VP8:
        return MFX_WRN_PARTIAL_ACCELERATION;
    default:
        return MFX_ERR_UNSUPPORTED;
    }

    if (MFX_HW_IVB == m_HWType || MFX_HW_HSW == m_HWType)
    {

        if (par->mfx.FrameInfo.Width > 4096 || par->mfx.FrameInfo.Height > 4096)
        {
            return MFX_WRN_PARTIAL_ACCELERATION;
        }
        else
        {
            return MFX_ERR_NONE;
        }
    }
    else // for other platforms decision is based on SNB/ELK assumption
    {
        if (par->mfx.FrameInfo.Width > 1920 || par->mfx.FrameInfo.Height > 1200)
        {
            return MFX_WRN_PARTIAL_ACCELERATION;
        }
    }
    return MFX_ERR_NONE;
}

void* VAAPIVideoCORE::QueryCoreInterface(const MFX_GUID &guid)
{
    if(MFXIVideoCORE_GUID == guid)
    {
        return (void*) this;
    }
    else if( MFXICOREVAAPI_GUID == guid )
    {
        return (void*) m_pAdapter.get();
    }
    else if (MFXIHWCAPS_GUID == guid)
    {
        return (void*) &m_encode_caps;
    }
#if defined(MFX_ENABLE_CM)
    else if (MFXICORECM_GUID == guid)
    {
        CmDevice* pCmDevice = NULL;
        if (!m_bCmCopy)
        {
            m_pCmCopy.reset(new CmCopyWrapper);
            pCmDevice = m_pCmCopy.get()->GetCmDevice(m_Display);
            if (!pCmDevice)
                return NULL;
            if (MFX_ERR_NONE != m_pCmCopy.get()->Initialize())
                return NULL;
            m_bCmCopy = true;
        }
        else
        {
            pCmDevice =  m_pCmCopy.get()->GetCmDevice(m_Display);
        }
        return (void*)pCmDevice;
    }
#endif
    else
    {
        return NULL;
    }
    
} // void* VAAPIVideoCORE::QueryCoreInterface(const MFX_GUID &guid)

bool IsHwMvcEncSupported()
{
    return false;
}

#endif 
/* EOF */
