/* ****************************************************************************** *\

Copyright (C) 2008-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_session.cpp

\* ****************************************************************************** */

#include "mfx_common.h"
#include <mfx_session.h>

#include <vm_time.h>
#include <vm_sys_info.h>

#include <libmfx_core_factory.h>

#if defined (MFX_VA_WIN)
#include <libmfx_core_d3d9.h>
#include <atlbase.h>
#elif defined(MFX_VA_LINUX)
#include <libmfx_core_vaapi.h>
#endif

// static section of the file
namespace
{

mfxStatus mfxCOREGetCoreParam(mfxHDL pthis, mfxCoreParam *par)
{
    mfxSession session = (mfxSession) pthis;
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);
    MFX_CHECK(par, MFX_ERR_NULL_PTR);

    try
    {
        MFX_SCHEDULER_PARAM param;

        // reset the parameters
        memset(par, 0, sizeof(mfxCoreParam));

        // get the parameters of the current scheduler
        mfxRes = session->m_pScheduler->GetParam(&param);
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }

        // fill the structure
        mfxRes = MFXQueryIMPL(session, &(par->Impl));
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }
        mfxRes = MFXQueryVersion(session, &(par->Version));
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }
        par->NumWorkingThread = param.numberOfThreads;
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_pScheduler)
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
        else if (0 == par)
        {
            mfxRes = MFX_ERR_NULL_PTR;
        }
    }

    return mfxRes;

} // mfxStatus mfxCOREGetCoreParam(mfxHDL pthis, mfxCoreParam *par)

mfxStatus mfxCOREMapOpaqueSurface(mfxHDL pthis, mfxU32  num, mfxU32  type, mfxFrameSurface1 **op_surf)
{
    mfxSession session = (mfxSession) pthis;
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pCORE.get(), MFX_ERR_NOT_INITIALIZED);
    
    try
    {
        if (!op_surf)
            return MFX_ERR_MEMORY_ALLOC;
        
        if (!*op_surf)
            return MFX_ERR_MEMORY_ALLOC;

        mfxFrameAllocRequest  request;
        mfxFrameAllocResponse response;

        request.Type =        (mfxU16)type;
        request.NumFrameMin = request.NumFrameSuggested = (mfxU16)num;
        request.Info = op_surf[0]->Info;

        mfxRes = session->m_pCORE->AllocFrames(&request, &response, op_surf, num);
        return mfxRes;

    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_pScheduler)
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
    }

    return mfxRes;

} // mfxStatus mfxCOREMapOpaqueSurface(mfxHDL pthis, mfxU32  num, mfxU32  type, mfxFrameSurface1 **op_surf)
mfxStatus mfxCOREUnmapOpaqueSurface(mfxHDL pthis, mfxU32  num, mfxU32  type, mfxFrameSurface1 **op_surf)
{
    pthis; num; type; op_surf;
    // not yet supported
    return MFX_ERR_NONE;


} // mfxStatus mfxCOREUnmapOpaqueSurface(mfxHDL pthis, mfxU32  num, mfxU32  type, mfxFrameSurface1 **op_surf)

mfxStatus mfxCOREGetRealSurface(mfxHDL pthis, mfxFrameSurface1 *op_surf, mfxFrameSurface1 **surf)
{
    mfxSession session = (mfxSession) pthis;
    mfxStatus mfxRes = MFX_ERR_NONE;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pCORE.get(), MFX_ERR_NOT_INITIALIZED);

    try
    {
        *surf = session->m_pCORE->GetNativeSurface(op_surf);
        if (!*surf)
            return MFX_ERR_INVALID_HANDLE;

        return mfxRes;
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_pScheduler)
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
    }

    return mfxRes;
} // mfxStatus mfxCOREGetRealSurface(mfxHDL pthis, mfxFrameSurface1 *op_surf, mfxFrameSurface1 **surf)

mfxStatus mfxCOREGetOpaqueSurface(mfxHDL pthis, mfxFrameSurface1 *surf, mfxFrameSurface1 **op_surf)
{
    mfxSession session = (mfxSession) pthis;
    mfxStatus mfxRes = MFX_ERR_NONE;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pCORE.get(), MFX_ERR_NOT_INITIALIZED);
    
    try
    {
        *op_surf = session->m_pCORE->GetOpaqSurface(surf);
        if (!*op_surf)
            return MFX_ERR_INVALID_HANDLE;

        return mfxRes;
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_pScheduler)
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
    }

    return mfxRes;
}// mfxStatus mfxCOREGetOpaqueSurface(mfxHDL pthis, mfxFrameSurface1 *op_surf, mfxFrameSurface1 **surf)

#define CORE_FUNC_IMPL(func_name, formal_param_list, actual_param_list) \
mfxStatus mfxCORE##func_name formal_param_list \
{ \
    mfxSession session = (mfxSession) pthis; \
    mfxStatus mfxRes; \
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE); \
    MFX_CHECK(session->m_pCORE.get(), MFX_ERR_NOT_INITIALIZED); \
    try \
    { \
        /* call the method */ \
        mfxRes = session->m_pCORE->func_name actual_param_list; \
    } \
    /* handle error(s) */ \
    catch(MFX_CORE_CATCH_TYPE) \
    { \
        /* set the default error value */ \
        if (NULL == session) \
        { \
            mfxRes = MFX_ERR_INVALID_HANDLE; \
        } \
        else if (NULL == session->m_pCORE.get()) \
        { \
            mfxRes = MFX_ERR_NOT_INITIALIZED; \
        } \
        else \
        { \
            mfxRes = MFX_ERR_NULL_PTR; \
        } \
    } \
    return mfxRes; \
} /* mfxStatus mfxCORE##func_name formal_param_list */

CORE_FUNC_IMPL(GetHandle, (mfxHDL pthis, mfxHandleType type, mfxHDL *handle), (type, handle))
CORE_FUNC_IMPL(IncreaseReference, (mfxHDL pthis, mfxFrameData *fd), (fd))
CORE_FUNC_IMPL(DecreaseReference, (mfxHDL pthis, mfxFrameData *fd), (fd))
CORE_FUNC_IMPL(CopyFrame, (mfxHDL pthis, mfxFrameSurface1 *dst, mfxFrameSurface1 *src), (dst, src))
CORE_FUNC_IMPL(CopyBuffer, (mfxHDL pthis, mfxU8 *dst, mfxU32 dst_size, mfxFrameSurface1 *src), (dst, dst_size, src))
CORE_FUNC_IMPL(CopyFrameEx, (mfxHDL pthis, mfxFrameSurface1 *dst, mfxU16  dstMemType, mfxFrameSurface1 *src, mfxU16  srcMemType), (dst, dstMemType, src, srcMemType))

#undef CORE_FUNC_IMPL

// exposed default allocator
mfxStatus mfxDefAllocFrames(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    MFX_CHECK_NULL_PTR1(pthis);
    VideoCORE *pCore = (VideoCORE*)pthis;
    return pCore->AllocFrames(request,response); 

} // mfxStatus mfxDefAllocFrames(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
mfxStatus mfxDefLockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    MFX_CHECK_NULL_PTR1(pthis);
    VideoCORE *pCore = (VideoCORE*)pthis;

    if (pCore->IsExternalFrameAllocator())
    {
        return pCore->LockExternalFrame(mid,ptr); 
    }

    return pCore->LockFrame(mid,ptr); 


} // mfxStatus mfxDefLockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
mfxStatus mfxDefGetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
{
    MFX_CHECK_NULL_PTR1(pthis);
    VideoCORE *pCore = (VideoCORE*)pthis;
    if (pCore->IsExternalFrameAllocator())
    {
        return pCore->GetExternalFrameHDL(mid, handle);
    }
    return pCore->GetFrameHDL(mid, handle);

} // mfxStatus mfxDefGetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
mfxStatus mfxDefUnlockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr=0)
{
    MFX_CHECK_NULL_PTR1(pthis);
    VideoCORE *pCore = (VideoCORE*)pthis;
    
    if (pCore->IsExternalFrameAllocator())
    {
        return pCore->UnlockExternalFrame(mid, ptr);
    }
    
    return pCore->UnlockFrame(mid, ptr); 

} // mfxStatus mfxDefUnlockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr=0)
mfxStatus mfxDefFreeFrames(mfxHDL pthis, mfxFrameAllocResponse *response)
{
    MFX_CHECK_NULL_PTR1(pthis);
    VideoCORE *pCore = (VideoCORE*)pthis;
    return pCore->FreeFrames(response); 

} // mfxStatus mfxDefFreeFrames(mfxHDL pthis, mfxFrameAllocResponse *response)



// exposed external allocator
mfxStatus mfxExtAllocFrames(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    MFX_CHECK_NULL_PTR1(pthis);
    if (request->Type & MFX_MEMTYPE_EXTERNAL_FRAME)
    {
        VideoCORE *pCore = (VideoCORE*)pthis;
        return pCore->AllocFrames(request,response); 
    }
    return MFX_ERR_UNSUPPORTED;

} // mfxStatus mfxExtAllocFrames(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
mfxStatus mfxExtLockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    VideoCORE *pCore = (VideoCORE*)pthis;
    return pCore->LockExternalFrame(mid,ptr); 

} // mfxStatus mfxExtLockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
mfxStatus mfxExtGetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
{
    VideoCORE *pCore = (VideoCORE*)pthis;
    return pCore->GetExternalFrameHDL(mid, handle);

} // mfxStatus mfxExtGetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
mfxStatus mfxExtUnlockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr=0)
{
    VideoCORE *pCore = (VideoCORE*)pthis;
    return pCore->UnlockExternalFrame(mid, ptr); 

} // mfxStatus mfxExtUnlockFrame(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr=0)
mfxStatus mfxExtFreeFrames(mfxHDL pthis, mfxFrameAllocResponse *response)
{
    VideoCORE *pCore = (VideoCORE*)pthis;
    return pCore->FreeFrames(response); 

} // mfxStatus mfxExtFreeFrames(mfxHDL pthis, mfxFrameAllocResponse *response)

void InitCoreInterface(mfxCoreInterface *pCoreInterface,
                       const mfxSession session)
{
    // reset the structure
    memset(pCoreInterface, 0, sizeof(mfxCoreInterface));


     // fill external allocator
    pCoreInterface->FrameAllocator.pthis = session->m_pCORE.get();
    pCoreInterface->FrameAllocator.Alloc = &mfxDefAllocFrames;
    pCoreInterface->FrameAllocator.Lock = &mfxDefLockFrame;
    pCoreInterface->FrameAllocator.GetHDL = &mfxDefGetHDL;
    pCoreInterface->FrameAllocator.Unlock = &mfxDefUnlockFrame;
    pCoreInterface->FrameAllocator.Free = &mfxDefFreeFrames;

    // fill the methods
    pCoreInterface->pthis = (mfxHDL) session;
    pCoreInterface->GetCoreParam = &mfxCOREGetCoreParam;
    pCoreInterface->GetHandle = &mfxCOREGetHandle;
    pCoreInterface->IncreaseReference = &mfxCOREIncreaseReference;
    pCoreInterface->DecreaseReference = &mfxCOREDecreaseReference;
    pCoreInterface->CopyFrame = &mfxCORECopyFrame;
    pCoreInterface->CopyBuffer = &mfxCORECopyBuffer;
    pCoreInterface->MapOpaqueSurface = &mfxCOREMapOpaqueSurface;
    pCoreInterface->UnmapOpaqueSurface = &mfxCOREUnmapOpaqueSurface;
    pCoreInterface->GetRealSurface = &mfxCOREGetRealSurface;
    pCoreInterface->GetOpaqueSurface = &mfxCOREGetOpaqueSurface;


} // void InitCoreInterface(mfxCoreInterface *pCoreInterface,

} // namespace

_mfxSession::_mfxSession(const mfxU32 adapterNum) :
    m_adapterNum(adapterNum)
{
#if defined (MFX_VA)
    m_currentPlatform = MFX_PLATFORM_HARDWARE;
#else
    m_currentPlatform = MFX_PLATFORM_SOFTWARE;
#endif

    Clear();
    m_bIsHWENCSupport = false;
    m_version.Version = 0;

} // _mfxSession::_mfxSession(const mfxU32 adapterNum) :

_mfxSession::~_mfxSession(void)
{
    Release();

} // _mfxSession::~_mfxSession(void)

void _mfxSession::Clear(void)
{
    m_pScheduler = NULL;
    m_pSchedulerAllocated = NULL;

    m_priority = MFX_PRIORITY_NORMAL;
    m_bIsHWENCSupport = false;
    //m_coreInt.ExternalSurfaceAllocator = 0;

} // void _mfxSession::Clear(void)

void _mfxSession::Release(void)
{
    if (m_pScheduler)
    {
        m_pScheduler->Release();
    }
    if (m_pSchedulerAllocated)
    {
        m_pSchedulerAllocated->Release();
    }

    // unregister plugin before closing
    if (m_plgGen.get())
    {
        m_plgGen->PluginClose();
    }
    if (m_plgEnc.get())
    {
        m_plgEnc->PluginClose();
    }
    if (m_plgDec.get())
    {
        m_plgDec->PluginClose();
    }
    if (m_plgVPP.get())
    {
        m_plgVPP->PluginClose();
    }
    // release the components the excplicit way.
    // do not relay on default deallocation order,
    // somebody could change it.
    m_plgGen.reset();
    m_pBRC.reset();
    m_pPAK.reset();
    m_pENC.reset();
    m_pVPP.reset();
    m_pDECODE.reset();
    m_pENCODE.reset();
    m_pCORE.reset();

    //delete m_coreInt.ExternalSurfaceAllocator;
    Clear();

} // void _mfxSession::Release(void)

mfxStatus _mfxSession::Init(mfxIMPL implInterface, mfxVersion *ver)
{
    mfxStatus mfxRes;
    MFX_SCHEDULER_PARAM schedParam;
    mfxU32 maxNumThreads;

    // release the object before initialization
    Release();

    if (ver)
    {
        m_version = *ver;
    }
    else
    {
        mfxStatus sts = MFXQueryVersion(this, &m_version);
        if (sts != MFX_ERR_NONE)
            return sts;
    }

    // save working HW interface
    switch (implInterface)
    {
        // if nothing is set, nothing is returned
    case MFX_IMPL_UNSUPPORTED:
        m_implInterface = MFX_IMPL_UNSUPPORTED;
        break;
#if defined(MFX_VA_WIN)        
        // D3D9 is only one supported interface
    case MFX_IMPL_VIA_ANY:
#endif    
    case MFX_IMPL_VIA_D3D9:
        m_implInterface = MFX_IMPL_VIA_D3D9;
        break;
    case MFX_IMPL_VIA_D3D11:
        m_implInterface = MFX_IMPL_VIA_D3D11;
        break;
        
#if defined(MFX_VA_LINUX)        
        // VAAPI is only one supported interface
    case MFX_IMPL_VIA_ANY:
    case MFX_IMPL_VIA_VAAPI:
        m_implInterface = MFX_IMPL_VIA_VAAPI;
        break;
#endif        

        // unknown hardware interface
    default:
        if (MFX_PLATFORM_HARDWARE == m_currentPlatform)
            return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    }

    // get the number of available threads
    maxNumThreads = vm_sys_info_get_cpu_num();

    // allocate video core
    if (MFX_PLATFORM_SOFTWARE == m_currentPlatform)
    {
        m_pCORE.reset(FactoryCORE::CreateCORE(MFX_HW_NO, 0, maxNumThreads, this));
    }
#if defined(MFX_VA_WIN)
    else
    {
        if (MFX_IMPL_VIA_D3D11 == m_implInterface)
        {
#if defined (MFX_D3D11_ENABLED)
            m_pCORE.reset(FactoryCORE::CreateCORE(MFX_HW_D3D11, m_adapterNum, maxNumThreads, this));
#else
            return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
#endif
        }
        else
            m_pCORE.reset(FactoryCORE::CreateCORE(MFX_HW_D3D9, m_adapterNum, maxNumThreads, this));
        
    }
#elif defined(MFX_VA_LINUX)
    else
    {
        m_pCORE.reset(FactoryCORE::CreateCORE(MFX_HW_VAAPI, m_adapterNum, maxNumThreads, this));
    }
    
#elif defined(MFX_VA_OSX)
    
    else
    {
        m_pCORE.reset(FactoryCORE::CreateCORE(MFX_HW_VDAAPI, m_adapterNum, maxNumThreads, this));
    }
#endif

    // initialize the core interface
    InitCoreInterface(&m_coreInt, this);

    // query the scheduler interface
    m_pScheduler = QueryInterface<MFXIScheduler> (m_pSchedulerAllocated,
                                                  MFXIScheduler_GUID);
    if (NULL == m_pScheduler)
    {
        return MFX_ERR_UNKNOWN;
    }
    memset(&schedParam, 0, sizeof(schedParam));
    schedParam.flags = MFX_SCHEDULER_DEFAULT;
    schedParam.numberOfThreads = maxNumThreads;
    schedParam.pCore = m_pCORE.get();
    mfxRes = m_pScheduler->Initialize(&schedParam);
    if (MFX_ERR_NONE != mfxRes)
    {
        return mfxRes;
    }

    m_pOperatorCore = new OperatorCORE(m_pCORE.get());

    return MFX_ERR_NONE;

} // mfxStatus _mfxSession::Init(mfxIMPL implInterface)

mfxStatus _mfxSession::RestoreScheduler(void)
{
    if(m_pSchedulerAllocated)
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    // leave the current scheduler
    if (m_pScheduler)
    {
        m_pScheduler->Release();
        m_pScheduler = NULL;
    }

    // query the scheduler interface
    m_pScheduler = QueryInterface<MFXIScheduler> (m_pSchedulerAllocated,
                                                  MFXIScheduler_GUID);
    if (NULL == m_pScheduler)
    {
        return MFX_ERR_UNKNOWN;
    }

    return MFX_ERR_NONE;

} // mfxStatus _mfxSession::RestoreScheduler(void)

mfxStatus _mfxSession::ReleaseScheduler(void)
{
    if(m_pScheduler)
        m_pScheduler->Release();
    
    if(m_pSchedulerAllocated)
    m_pSchedulerAllocated->Release();

    m_pScheduler = NULL;
    m_pSchedulerAllocated = NULL;
    
    return MFX_ERR_NONE;

} // mfxStatus _mfxSession::RestoreScheduler(void)


namespace MFX
{
    unsigned int CreateUniqId()
    {
        static volatile mfxU32 g_tasksId = 0;
        return (unsigned int)vm_interlocked_inc32(&g_tasksId);
    }
}
