/* ****************************************************************************** *\

Copyright (C) 2011-2014 Intel Corporation.  All rights reserved.

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

File Name: libmfx_core_vaapi.h

\* ****************************************************************************** */

#include "mfx_common.h"

#if defined (MFX_VA_LINUX)

#ifndef __LIBMFX_CORE__VAAPI_H__
#define __LIBMFX_CORE__VAAPI_H__

#include <memory>
#include "umc_structures.h"
#include "libmfx_core.h"
#include "libmfx_allocator_vaapi.h"
#include "libmfx_core_interface.h"

#include "mfx_platform_headers.h"

#include <va/va.h>

// disable the "conditional expression is constant" warning
#pragma warning(disable: 4127)

namespace UMC
{
    class DXVA2Accelerator;
    class LinuxVideoAccelerator;
    class ProtectedVA;
};


class VAAPIVideoCORE : public CommonCORE
{    
public:
    friend class FactoryCORE;
    class VAAPIAdapter : public VAAPIInterface
    {
    public:
        VAAPIAdapter(VAAPIVideoCORE *pVAAPICore):m_pVAAPICore(pVAAPICore)
        {
        };
    protected:
        VAAPIVideoCORE *m_pVAAPICore;
    
    };
    virtual ~VAAPIVideoCORE();

    virtual mfxStatus     SetHandle(mfxHandleType type, mfxHDL handle);

    virtual mfxStatus     AllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response, bool isNeedCopy = true);
    virtual void          GetVA(mfxHDL* phdl, mfxU16 type) 
    {
        (type & MFX_MEMTYPE_FROM_DECODE)?(*phdl = m_pVA.get()):(*phdl = 0);
    };
    // Get the current working adapter's number
    virtual mfxU32 GetAdapterNumber(void) {return m_adapterNum;}
    virtual eMFXPlatform  GetPlatformType() {return  MFX_PLATFORM_HARDWARE;}

    virtual mfxStatus DoFastCopy(mfxFrameSurface1 *pDst, mfxFrameSurface1 *pSrc);
    virtual mfxStatus DoFastCopyExtended(mfxFrameSurface1 *pDst, mfxFrameSurface1 *pSrc);
    virtual mfxStatus DoFastCopyWrapper(mfxFrameSurface1 *pDst, mfxU16 dstMemType, mfxFrameSurface1 *pSrc, mfxU16 srcMemType);

    mfxHDL * GetFastCompositingService();
    void SetOnFastCompositingService(void);
    bool IsFastCompositingEnabled(void) const;


    virtual eMFXHWType     GetHWType() { return m_HWType; }


    mfxStatus              CreateVA(mfxVideoParam * param, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
    // to check HW capatbilities
    virtual mfxStatus IsGuidSupported(const GUID guid, mfxVideoParam *par, bool isEncoder = false);

    virtual eMFXVAType   GetVAType() const {return MFX_HW_VAAPI; };
    virtual void* QueryCoreInterface(const MFX_GUID &guid);

    mfxStatus              GetVAService(VADisplay *pVADisplay);

protected:
    VAAPIVideoCORE(const mfxU32 adapterNum, const mfxU32 numThreadsAvailable, const mfxSession session = NULL);
    virtual void           Close();
    virtual mfxStatus      DefaultAllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);

    mfxStatus              CreateVideoAccelerator(mfxVideoParam * param, int profile, int NumOfRenderTarget, _mfxPlatformVideoSurface *RenderTargets);
    mfxStatus              ProcessRenderTargets(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response, mfxBaseWideFrameAllocator* pAlloc);
    mfxStatus              TraceFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response, mfxStatus sts);
    mfxStatus              OnDeblockingInWinRegistry(mfxU32 codecId);

    void                   ReleaseHandle();
    //std::auto_ptr<UMC::LinuxVideoAccelerator>       m_pVA;
    s_ptr<UMC::LinuxVideoAccelerator, true> m_pVA;
    VADisplay                           m_Display;

    const mfxU32                         m_adapterNum; // Ordinal number of adapter to work
    bool                                 m_bUseExtAllocForHWFrames;
    s_ptr<mfxDefaultAllocatorVAAPI::mfxWideHWFrameAllocator, true> m_pcHWAlloc;
    eMFXHWType                           m_HWType;

#if defined(MFX_ENABLE_CM)
    bool                                 m_bCmCopy;
    bool                                 m_bCmCopyAllowed;
    s_ptr<CmCopyWrapper, true>           m_pCmCopy;
#endif // #if defined(MFX_ENABLE_CM)

#if defined (ANDROID)
    VAConfigID m_va_config;
    VAContextID m_va_CM_context;
    bool m_bCM_Initialized;

#define CM_SRF_POOL_SIZE 20
    mfxU32 m_CM_CurIndex;
    struct
    {
        VASurfaceID srf;
        mfxHDL      key;
    } m_CM_SrfPool[CM_SRF_POOL_SIZE];
#endif
public: // aya: FIXME: private???   

    std::auto_ptr<VAAPIAdapter>            m_pAdapter;
};

bool IsSupported__VAEncMiscParameterPrivate(void);
bool IsSupported__VAHDCPEncryptionParameterBuffer(void);

#endif // __LIBMFX_CORE__VAAPI_H__
#endif // MFX_VA_LINUX
/* EOF */
