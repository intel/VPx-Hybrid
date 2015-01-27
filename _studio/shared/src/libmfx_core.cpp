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

File Name: libmfx_core.cpp

\* ****************************************************************************** */

// need for D3D external frames case
#if defined (MFX_DEVICE_CHECKING)
#include <d3d9.h>
#include <dxva2api.h>
#endif

#include <mfx_scheduler_core.h>
#include <libmfx_core_interface.h>
#include "mfx_session.h"
#include "libmfx_core.h"
#include "mfx_utils.h"

#include "mfx_common_int.h"
#include "vm_interlocked.h"

#include "ippi.h"

#include "mfx_umc_alloc_wrapper.h"

#include "vm_sys_info.h"

using namespace std;
//
// THE OTHER CORE FUNCTIONS HAVE IMPLICIT IMPLEMENTATION
//

FUNCTION_IMPL(CORE, SetBufferAllocator, (mfxSession session, mfxBufferAllocator *allocator), (allocator))
    FUNCTION_IMPL(CORE, SetFrameAllocator, (mfxSession session, mfxFrameAllocator *allocator), (allocator))
    FUNCTION_IMPL(CORE, SetHandle, (mfxSession session, mfxHandleType type, mfxHDL hdl), (type, hdl))
    FUNCTION_IMPL(CORE, GetHandle, (mfxSession session, mfxHandleType type, mfxHDL *hdl), (type, hdl))

#define MFX_CHECK_HDL(hdl) {if (!hdl) MFX_RETURN(MFX_ERR_INVALID_HANDLE);}

    mfxStatus CommonCORE::AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxHDL *mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    return (*m_bufferAllocator.bufferAllocator.Alloc)(m_bufferAllocator.bufferAllocator.pthis,nbytes, type, mid);
}
mfxStatus CommonCORE::LockBuffer(mfxHDL mid, mfxU8 **ptr)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    return (*m_bufferAllocator.bufferAllocator.Lock)(m_bufferAllocator.bufferAllocator.pthis, mid, ptr);
}
mfxStatus CommonCORE::UnlockBuffer(mfxHDL mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    return (*m_bufferAllocator.bufferAllocator.Unlock)(m_bufferAllocator.bufferAllocator.pthis,mid);
}
mfxStatus CommonCORE::FreeBuffer(mfxHDL mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    return (*m_bufferAllocator.bufferAllocator.Free)(m_bufferAllocator.bufferAllocator.pthis,mid);
}
mfxStatus  CommonCORE::CheckHandle()
{
#if defined (MFX_DEVICE_CHECKING)
    UMC::AutomaticUMCMutex guard(m_guard);
    if (m_hdl)
    {
        HANDLE hdl;
        HRESULT hr;
        IDirect3DDevice9 *pDev = 0;
        IDirect3DDeviceManager9* pDirect3DDeviceManager = (IDirect3DDeviceManager9*)m_hdl;
        hr = pDirect3DDeviceManager->OpenDeviceHandle(&hdl);
        if (FAILED(hr))
            return MFX_ERR_DEVICE_FAILED;
        hr = pDirect3DDeviceManager->LockDevice(hdl, &pDev, true);
        if (FAILED(hr))
            return MFX_ERR_DEVICE_FAILED;
        hr = pDev->TestCooperativeLevel();
        if (FAILED(hr))
        {
            pDirect3DDeviceManager->UnlockDevice(hdl, false);
            pDirect3DDeviceManager->CloseDeviceHandle(&hdl);
            return MFX_ERR_DEVICE_LOST;
        }
        hr = pDirect3DDeviceManager->UnlockDevice(hdl,false);
        if (FAILED(hr))
            return MFX_ERR_DEVICE_FAILED;
        hr = pDirect3DDeviceManager->CloseDeviceHandle(hdl);
        if (FAILED(hr))
            return MFX_ERR_DEVICE_FAILED;

        pDev->Release();

        return MFX_ERR_NONE;
    }
    else
        return MFX_ERR_NOT_FOUND;
#else
    return MFX_ERR_NONE;
#endif

} // mfxStatus  CommonCORE::CheckHandle()

mfxStatus CommonCORE::AllocFrames(mfxFrameAllocRequest *request,
                                  mfxFrameAllocResponse *response,
                                  mfxFrameSurface1 **pOpaqueSurface, 
                                  mfxU32 NumOpaqueSurface)
{
    m_bIsOpaqMode = true;

    mfxStatus sts;

    if (!pOpaqueSurface)
        return MFX_ERR_MEMORY_ALLOC;

    if (0 == NumOpaqueSurface)
        return MFX_ERR_MEMORY_ALLOC;

    if (!CheckOpaqueRequest(request, pOpaqueSurface, NumOpaqueSurface))
        return MFX_ERR_MEMORY_ALLOC;

    if (IsOpaqSurfacesAlreadyMapped(pOpaqueSurface, NumOpaqueSurface, response))
    {
        return MFX_ERR_NONE;
    }

    sts = AllocFrames(request, response);
    MFX_CHECK_STS(sts);

    for (mfxU32 i = 0; i < response->NumFrameActual; i++)
    {
        mfxFrameSurface1 surf = {};
        surf.Info = request->Info;
        //sts = LockFrame(response->mids[i], &surf.Data);
        //MFX_CHECK_STS(sts);
        surf.Data.MemId = response->mids[i];
        m_OpqTbl.insert(pair<mfxFrameSurface1 *, mfxFrameSurface1>(pOpaqueSurface[i], surf));
    }
    mfxFrameAllocResponse* pResp = new mfxFrameAllocResponse;
    *pResp = *response;

    m_RefCtrTbl.insert(pair<mfxFrameAllocResponse*, mfxU32>(pResp, 1));
    return sts;
}

mfxStatus CommonCORE::AllocFrames(mfxFrameAllocRequest *request,
                                  mfxFrameAllocResponse *response, bool isNeedCopy)
{
    MFX::AutoTimer timer("CommonCORE::AllocFrames");
    UMC::AutomaticUMCMutex guard(m_guard);
    mfxStatus sts = MFX_ERR_NONE;
    try
    {
        MFX_CHECK_NULL_PTR2(request, response);
        mfxFrameAllocRequest temp_request = *request;

        // external allocator doesn't know how to allocate opaque surfaces
        // we can treat opaque as internal
        if (temp_request.Type & MFX_MEMTYPE_OPAQUE_FRAME)
        {
            temp_request.Type -= MFX_MEMTYPE_OPAQUE_FRAME;
            temp_request.Type |= MFX_MEMTYPE_INTERNAL_FRAME;
        }
        

        if (!m_bFastCopy && isNeedCopy)
        {
            // initialize sse41 fast copy
            m_pFastCopy.reset(new FastCopy());
            m_pFastCopy.get()->Initialize();

            m_bFastCopy = true;
        }

        // external allocator
        if (m_bSetExtFrameAlloc)
        {
            sts = (*m_FrameAllocator.frameAllocator.Alloc)(m_FrameAllocator.frameAllocator.pthis, &temp_request, response);

            if (MFX_ERR_UNSUPPORTED == sts)
            {
                // Default Allocator is used for internal memory allocation only
                if (request->Type & MFX_MEMTYPE_EXTERNAL_FRAME)
                    return sts;

                return this->DefaultAllocFrames(request, response);
            }
            else if (MFX_ERR_NONE == sts)
            {
                sts = RegisterMids(response, request->Type, false);
                MFX_CHECK_STS(sts);
            }
        }
        else
        {
            // Default Allocator is used for internal memory allocation only
            if (request->Type & MFX_MEMTYPE_EXTERNAL_FRAME)
                return MFX_ERR_MEMORY_ALLOC;

            return this->DefaultAllocFrames(request, response);
        }
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
#if 0
    if (MFX_ERR_NONE == sts)
    {
        char descr[] = "?";
        if (request->Type & MFX_MEMTYPE_DXVA2_DECODER_TARGET) descr[0] = 'V';
        if (request->Type & MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET) descr[0] = 'P';
        if (request->Type & MFX_MEMTYPE_SYSTEM_MEMORY) descr[0] = 'S';
        timer.AddParam(descr, response->NumFrameActual);
    }
#endif
    MFX_LTRACE_I(MFX_TRACE_LEVEL_PARAMS, sts);
    return sts;
}
mfxStatus CommonCORE::DefaultAllocFrames(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    mfxStatus sts = MFX_ERR_NONE;
    if ((request->Type & MFX_MEMTYPE_DXVA2_DECODER_TARGET)||
        (request->Type & MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET))
        // should be SW
        return MFX_ERR_UNSUPPORTED;
    else //if (request->Type & MFX_MEMTYPE_INTERNAL_FRAME) // TBD - only internal frames can be allocated
    {
        mfxBaseWideFrameAllocator* pAlloc = GetAllocatorByReq(request->Type);
        // VPP, ENC, PAK can request frames for several times
        if (!pAlloc)
        {
            m_pcAlloc.reset(new mfxWideSWFrameAllocator(request->Type));
            pAlloc = m_pcAlloc.get();
        }
        else
            return MFX_ERR_MEMORY_ALLOC;

        // set frame allocator
        pAlloc->frameAllocator.pthis = pAlloc;
        // set buffer allocator for current frame single allocator
        pAlloc->wbufferAllocator.bufferAllocator = m_bufferAllocator.bufferAllocator;
        sts =  (*pAlloc->frameAllocator.Alloc)(pAlloc->frameAllocator.pthis, request, response);
        MFX_CHECK_STS(sts);
        sts = RegisterMids(response, request->Type, true, pAlloc);
        MFX_CHECK_STS(sts);
    }
    ++m_NumAllocators;
    m_pcAlloc.pop();
    return sts;
}
mfxStatus CommonCORE::LockFrame(mfxHDL mid, mfxFrameData *ptr)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    MFX::AutoTimer timer("CommonCORE::LockFrame");
    try
    {
        MFX_CHECK_HDL(mid);
        MFX_CHECK_NULL_PTR1(ptr);
        mfxFrameAllocator* pAlloc = GetAllocatorAndMid(mid);
        if (!pAlloc)
            return MFX_ERR_INVALID_HANDLE;
        return (*pAlloc->Lock)(pAlloc->pthis, mid, ptr);
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_INVALID_HANDLE;
    }
}
mfxStatus CommonCORE::GetFrameHDL(mfxHDL mid, mfxHDL* handle, bool ExtendedSearch)
{
    // mutex not needed here - mynikols
    mfxStatus sts;
    //UMC::AutomaticUMCMutex guard(m_guard);
    try 
    {
        MFX_CHECK_HDL(mid);
        MFX_CHECK_NULL_PTR1(handle);

        mfxFrameAllocator* pAlloc = GetAllocatorAndMid(mid);
        if (!pAlloc)
        {

            // we couldn't define behavior if external allocator did not set
            if (ExtendedSearch)// try to find in another cores
            {
                sts = m_session->m_pOperatorCore->DoFrameOperation(&VideoCORE::GetFrameHDL, mid, handle);
                if (MFX_ERR_NONE == sts)
                    return sts;
            }
            return MFX_ERR_UNDEFINED_BEHAVIOR;
        }
        else
            return (*pAlloc->GetHDL)(pAlloc->pthis, mid, handle);

    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
}
mfxStatus CommonCORE::UnlockFrame(mfxHDL mid, mfxFrameData *ptr)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    try
    {
        MFX_CHECK_HDL(mid);
        mfxFrameAllocator* pAlloc = GetAllocatorAndMid(mid);
        if (!pAlloc)
            return MFX_ERR_INVALID_HANDLE;
        return (*pAlloc->Unlock)(pAlloc->pthis, mid, ptr);
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_INVALID_HANDLE;
    }
}
mfxStatus CommonCORE::FreeFrames(mfxFrameAllocResponse *response, bool ExtendedSearch)
{
    mfxStatus sts = MFX_ERR_NONE;
    if (m_RefCtrTbl.size())
    {
        {
            UMC::AutomaticUMCMutex guard(m_guard);
            RefCtrTbl::iterator ref_it;
            for (ref_it = m_RefCtrTbl.begin(); ref_it != m_RefCtrTbl.end(); ref_it++)
            {
                if (IsEqual(*ref_it->first, *response))
                {
                    ref_it->second--;
                    if (0 == ref_it->second)
                    {
                        delete ref_it->first;
                        m_RefCtrTbl.erase(ref_it);
                        return InternalFreeFrames(response);
                    }
                    return sts;
                }
            }
        }
        if (ExtendedSearch)
        {
            sts = m_session->m_pOperatorCore->DoCoreOperation(&VideoCORE::FreeFrames, response);
            if (MFX_ERR_UNDEFINED_BEHAVIOR == sts) // no opaq surfaces found
                return InternalFreeFrames(response);
            else
                return sts;
        }
    }
    else if (ExtendedSearch)
    {
        sts = m_session->m_pOperatorCore->DoCoreOperation(&VideoCORE::FreeFrames, response);
        if (MFX_ERR_UNDEFINED_BEHAVIOR == sts) // no opaq surfaces found
            return InternalFreeFrames(response);
        else
            return sts;
    }
    return MFX_ERR_UNDEFINED_BEHAVIOR;
}
mfxStatus CommonCORE::InternalFreeFrames(mfxFrameAllocResponse *response)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    try
    {
        MFX_CHECK_NULL_PTR1(response);
        MFX_CHECK(response->NumFrameActual, MFX_ERR_NONE);
        mfxStatus sts = MFX_ERR_NONE;
        m_pMemId.reset(new mfxMemId[response->NumFrameActual]);
        mfxFrameAllocator* pAlloc;
        CorrespTbl::iterator ctbl_it;
        AllocQueue::iterator it;
        ctbl_it = m_CTbl.find(response->mids[0]);
        if (ctbl_it == m_CTbl.end())
            return MFX_ERR_INVALID_HANDLE;
        bool IsDefaultMem = ctbl_it->second.isDefaultMem;
        mfxMemId extMem = response->mids[0];
        mfxFrameAllocator* pFirstAlloc = GetAllocatorAndMid(extMem);
        // checking and correspond parameters
        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            extMem = response->mids[i];
            ctbl_it = m_CTbl.find(response->mids[i]);
            if (m_CTbl.end() == ctbl_it)
                return MFX_ERR_INVALID_HANDLE;
            m_pMemId.get()[i] = ctbl_it->second.InternalMid;
            pAlloc = GetAllocatorAndMid(extMem);
            // all frames should be allocated by one allocator
            if ((IsDefaultMem != ctbl_it->second.isDefaultMem)||
                (pAlloc != pFirstAlloc))
                return MFX_ERR_INVALID_HANDLE;
        }
        // save first mid. Need for future internal memory free
        extMem = response->mids[0];
        sts = FreeMidArray(pFirstAlloc, response);
        MFX_CHECK_STS(sts);
        // delete self allocator
        if (IsDefaultMem)
        {
            it = m_AllocatorQueue.find(extMem);
            if (m_AllocatorQueue.end() == it)
                return MFX_ERR_INVALID_HANDLE;
            if (it->second)
            {
                delete it->second;
                it->second = 0;
            }
        }
        // delete self queues
        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            ctbl_it = m_CTbl.find(response->mids[i]);
            if (IsDefaultMem)
                m_AllocatorQueue.erase(response->mids[i]);
            m_CTbl.erase(ctbl_it);
        }
        m_pMemId.reset(0);
        // we sure about response->mids
        delete[] response->mids;
        response->mids = 0;
        return sts;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
}
mfxStatus  CommonCORE::LockExternalFrame(mfxMemId mid, mfxFrameData *ptr, bool ExtendedSearch)
{
    mfxStatus sts;
    MFX::AutoTimer timer("CommonCORE::LockExternalFrame");
    try
    {
        {
            UMC::AutomaticUMCMutex guard(m_guard);

            // if exist opaque surface - take a look in them (internal surfaces)
            if (m_OpqTbl.size())
            {
                sts = LockFrame(mid, ptr);
                if (MFX_ERR_NONE == sts)
                    return sts;
            }
            MFX_CHECK_NULL_PTR1(ptr);

            if (m_bSetExtFrameAlloc)
            {
                mfxFrameAllocator* pAlloc = &m_FrameAllocator.frameAllocator;
                return (*pAlloc->Lock)(pAlloc->pthis, mid, ptr);
            }
        }
        // we couldn't define behavior if external allocator did not set
        // try to find in another cores
        if (ExtendedSearch)// try to find in another cores
        {
            sts = m_session->m_pOperatorCore->DoFrameOperation(&VideoCORE::LockExternalFrame, mid, ptr);
            if (MFX_ERR_NONE == sts)
                return sts;
        }
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_LOCK_MEMORY;
    }
}
mfxStatus  CommonCORE::GetExternalFrameHDL(mfxMemId mid, mfxHDL *handle, bool ExtendedSearch)
{
    // mutex not needed here - mynikols
    mfxStatus sts;
    //UMC::AutomaticUMCMutex guard(m_guard);
    try 
    {
        MFX_CHECK_NULL_PTR1(handle);

        // if exist opaque surface - take a look in them (internal surfaces)
        if (m_OpqTbl.size())
        {
            sts = GetFrameHDL(mid, handle);
            if (MFX_ERR_NONE == sts)
                return sts;
        }

        if (m_bSetExtFrameAlloc)
        {
            mfxFrameAllocator* pAlloc = &m_FrameAllocator.frameAllocator;
            return (*pAlloc->GetHDL)(pAlloc->pthis, mid, handle);
        }

        // we couldn't define behavior if external allocator did not set
        if (ExtendedSearch)// try to find in another cores
        {
            sts = m_session->m_pOperatorCore->DoFrameOperation(&VideoCORE::GetExternalFrameHDL, mid, handle);
            if (MFX_ERR_NONE == sts)
                return sts;
        }
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
}
mfxStatus  CommonCORE::UnlockExternalFrame(mfxMemId mid, mfxFrameData *ptr, bool ExtendedSearch)
{
    mfxStatus sts;
    try 
    {
        {
            UMC::AutomaticUMCMutex guard(m_guard);
            // if exist opaque surface - take a look in them (internal surfaces)
            if (m_OpqTbl.size())
            {
                sts = UnlockFrame(mid, ptr);
                if (MFX_ERR_NONE == sts)
                    return sts;
            }

            if (m_bSetExtFrameAlloc)
            {
                mfxFrameAllocator* pAlloc = &m_FrameAllocator.frameAllocator;
                return (*pAlloc->Unlock)(pAlloc->pthis, mid, ptr);
            }
        }
        // we couldn't define behavior if external allocator did not set
        if (ExtendedSearch)// try to find in another cores
        {
            sts = m_session->m_pOperatorCore->DoFrameOperation(&VideoCORE::UnlockExternalFrame, mid, ptr);
            if (MFX_ERR_NONE == sts)
                return sts;
        }
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        return MFX_ERR_LOCK_MEMORY;
    }

}
mfxMemId CommonCORE::MapIdx(mfxMemId mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    if (0 == mid)
        return 0;

    CorrespTbl::iterator ctbl_it;
    ctbl_it = m_CTbl.find(mid);
    if (m_CTbl.end() == ctbl_it)
        return 0;
    else
        return ctbl_it->second.InternalMid;
}
mfxFrameSurface1* CommonCORE::GetNativeSurface(mfxFrameSurface1 *pOpqSurface, bool ExtendedSearch)
{
    if (0 == pOpqSurface)
        return 0;

    {
        UMC::AutomaticUMCMutex guard(m_guard);
        OpqTbl::iterator oqp_it;
        oqp_it = m_OpqTbl.find(pOpqSurface);
        if (m_OpqTbl.end() != oqp_it)
            return &oqp_it->second;
    }

    if (ExtendedSearch)
        return m_session->m_pOperatorCore->GetSurface(&VideoCORE::GetNativeSurface, pOpqSurface);

    return 0;

}
mfxFrameSurface1* CommonCORE::GetOpaqSurface(mfxMemId mid, bool ExtendedSearch)
{
    if (0 == mid)
        return 0;

    {
        UMC::AutomaticUMCMutex guard(m_guard);
        OpqTbl::iterator oqp_it;
        for (oqp_it = m_OpqTbl.begin(); oqp_it != m_OpqTbl.end();oqp_it++)
        {
            if (oqp_it->second.Data.MemId == mid)
                return oqp_it->first;
        }
    }

    if (ExtendedSearch)
        return m_session->m_pOperatorCore->GetSurface(&VideoCORE::GetOpaqSurface, mid);

    return 0;
}
mfxStatus CommonCORE::FreeMidArray(mfxFrameAllocator* pAlloc, mfxFrameAllocResponse *response)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    MemIDMap::iterator it = m_RespMidQ.find(response->mids);
    if (m_RespMidQ.end() == it)
        return MFX_ERR_INVALID_HANDLE;
    else
    {
        mfxFrameAllocResponse sResponse = *response;
        mfxStatus sts;
        sResponse.mids = it->second;
        sts = (*pAlloc->Free)(pAlloc->pthis, &sResponse);
        MFX_CHECK_STS(sts);
        m_RespMidQ.erase(it);
        return sts;
    }
}

mfxStatus CommonCORE::RegisterMids(mfxFrameAllocResponse *response, mfxU16 memType, bool IsDefaultAlloc, mfxBaseWideFrameAllocator* pAlloc)
{
    //MFX::AutoTimer::SetFrames(response->mids, response->NumFrameActual);
    m_pMemId.reset(new mfxMemId[response->NumFrameActual]);
    mfxMemId mId;
    for (mfxU32 i = 0; i < response->NumFrameActual; i++)
    {
        MemDesc ds;
        if (!GetUniqID(mId))
        {
            return MFX_ERR_UNDEFINED_BEHAVIOR;
        }

        // add in queue only self allocators
        // need to define SW or HW allocation mode
        if (IsDefaultAlloc)
            m_AllocatorQueue.insert(pair<mfxMemId, mfxBaseWideFrameAllocator*>(mId, pAlloc));
        ds.InternalMid = response->mids[i];
        ds.isDefaultMem = IsDefaultAlloc;

        // set render target memory description
        ds.memType = memType;
        m_CTbl.insert(pair<mfxMemId, MemDesc>(mId, ds));
        m_pMemId.get()[i] = mId;
    }
    m_RespMidQ.insert(pair<mfxMemId*, mfxMemId*>(m_pMemId.get(), response->mids));
    response->mids = m_pMemId.get();
    m_pMemId.pop();
    return  MFX_ERR_NONE;
}

CommonCORE::CommonCORE(const mfxU32 numThreadsAvailable, const mfxSession session) :
    m_ExtOptions(0),
    m_numThreadsAvailable(numThreadsAvailable),
    m_session(session),
    m_NumAllocators(0),
    m_hdl(NULL),
    m_DXVA2DecodeHandle(NULL),
    m_D3DDecodeHandle(NULL),
    m_D3DEncodeHandle(NULL),
    m_D3DVPPHandle(NULL),
    m_bSetExtBufAlloc(false),
    m_bSetExtFrameAlloc(false),
    m_bFastCopy(0),
    m_bUseExtManager(false),
    m_bIsOpaqMode(false),
    m_CoreId(0)
{
    m_bufferAllocator.bufferAllocator.pthis = &m_bufferAllocator;
    CheckTimingLog();
}

CommonCORE::~CommonCORE()
{
    Close();
}

void CommonCORE::Close()
{
    m_CTbl.clear();
    m_AllocatorQueue.clear();
    m_OpqTbl.clear();
    MemIDMap::iterator it;
    while(m_RespMidQ.size())
    {
        // now its NOT a mistake situation
        // all mids should be freed already except opaque shared surfaces
        it = m_RespMidQ.begin();
        delete[] it->first;
        m_RespMidQ.erase(it);
    }
    if (m_bUseExtManager && m_hdl)
    {
#if defined(_WIN32) || defined(_WIN64)
        ((IUnknown*)m_hdl)->Release();
#endif
        m_bUseExtManager = false;
    }
}

mfxStatus CommonCORE::GetHandle(mfxHandleType type, mfxHDL *handle)
{
    MFX_CHECK_NULL_PTR1(handle);
    UMC::AutomaticUMCMutex guard(m_guard);

#if defined(_WIN32) || defined(_WIN64)
    // check error(s)
    if (MFX_HANDLE_DIRECT3D_DEVICE_MANAGER9 == type || 
        MFX_HANDLE_D3D11_DEVICE == type)
    {
        if (m_hdl)
        {
            *handle = m_hdl;
            return MFX_ERR_NONE;
        }
        // not exist handle yet
        else
            return MFX_ERR_NOT_FOUND;
    }
    else if (MFX_HANDLE_DECODER_DRIVER_HANDLE == type)
    {
        if (m_DXVA2DecodeHandle)
        {
            *handle = m_DXVA2DecodeHandle;
            return MFX_ERR_NONE;
        }
        // not exist handle yet
        else
            return MFX_ERR_NOT_FOUND;
    }
    else if (MFX_HANDLE_VIDEO_DECODER == type )
    {
        if (m_D3DDecodeHandle)
        {
            *handle = m_D3DDecodeHandle;
            return MFX_ERR_NONE;
        }
        // not exist handle yet
        else
            return MFX_ERR_NOT_FOUND;

    }

#endif // #if defined(_WIN32) || defined(_WIN64)
#if defined(LINUX32) || defined(LINUX64) || defined(MFX_VA_LINUX)
    if (MFX_HANDLE_VA_DISPLAY == type )
    {
        if (m_hdl)
        {
            *handle = m_hdl;
            return MFX_ERR_NONE;
        }
        // not exist handle yet
        else
            return MFX_ERR_NOT_FOUND;
    }
#endif
    // if wrong type
    return MFX_ERR_UNDEFINED_BEHAVIOR;

} // mfxStatus CommonCORE::GetHandle(mfxHandleType type, mfxHDL *handle)

mfxStatus CommonCORE::SetHandle(mfxHandleType type, mfxHDL hdl)
{
    MFX_CHECK_NULL_PTR1(hdl);
    UMC::AutomaticUMCMutex guard(m_guard);

    // Need to call at once
    switch ((mfxU32)type)
    {
#if defined(_WIN32) || defined(_WIN64)
    case MFX_HANDLE_DIRECT3D_DEVICE_MANAGER9:
    case MFX_HANDLE_D3D11_DEVICE:
        // if device manager already set
        if (m_hdl)
            return MFX_ERR_UNDEFINED_BEHAVIOR;
        // set external handle
        m_hdl = hdl;
        ((IUnknown*)m_hdl)->AddRef();
        m_bUseExtManager = true;
        break;

#ifdef MFX_DEBUG_TOOLS
    case MFX_HANDLE_TIMING_LOG:
        return (MFX::AutoTimer::Init((TCHAR*)hdl, 1) ? MFX_ERR_INVALID_HANDLE : MFX_ERR_NONE);
    case MFX_HANDLE_TIMING_SUMMARY:
        return (MFX::AutoTimer::Init((TCHAR*)hdl, 2) ? MFX_ERR_INVALID_HANDLE : MFX_ERR_NONE);
    case MFX_HANDLE_TIMING_TAL:
        return (MFX::AutoTimer::Init((TCHAR*)hdl, 3) ? MFX_ERR_INVALID_HANDLE : MFX_ERR_NONE);
    case MFX_HANDLE_EXT_OPTIONS:
        m_ExtOptions |= *(mfxU32*)hdl;
        return MFX_ERR_NONE;
#endif
#endif // #if defined(_WIN32) || defined(_WIN64)
#if defined(LINUX32) || defined(LINUX64) || defined(MFX_VA_LINUX)
    case MFX_HANDLE_VA_DISPLAY:
        // if device manager already set
        if (m_hdl)
            return MFX_ERR_UNDEFINED_BEHAVIOR;
        // set external handle
        m_hdl = hdl;
        m_bUseExtManager = true;
        break;
#endif

    default:
        // wrong input type
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
    return MFX_ERR_NONE;
}// mfxStatus CommonCORE::SetHandle(mfxHandleType type, mfxHDL handle)

mfxStatus CommonCORE::SetBufferAllocator(mfxBufferAllocator *allocator)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    if (!allocator)
        return MFX_ERR_NONE;

    if (!m_bSetExtBufAlloc)
    {
        m_bufferAllocator.bufferAllocator = *allocator;
        m_bSetExtBufAlloc = true;
        return MFX_ERR_NONE;
    }
    else
        return MFX_ERR_UNDEFINED_BEHAVIOR;
}
mfxFrameAllocator* CommonCORE::GetAllocatorAndMid(mfxMemId& mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    CorrespTbl::iterator ctbl_it = m_CTbl.find(mid);
    if (m_CTbl.end() == ctbl_it)
        return 0;
    if (!ctbl_it->second.isDefaultMem)
    {
        if (m_bSetExtFrameAlloc)
        {
            mid = ctbl_it->second.InternalMid;
            return &m_FrameAllocator.frameAllocator;
        }
        else // error
        {
            mid = 0;
            return 0;
        }
    }
    else
    {
        AllocQueue::iterator it = m_AllocatorQueue.find(mid);
        if (it == m_AllocatorQueue.end())
        {
            mid = 0;
            return 0;
        }
        else
        {
            mid = ctbl_it->second.InternalMid;
            return &it->second->frameAllocator;
        }

    }
}
mfxBaseWideFrameAllocator* CommonCORE::GetAllocatorByReq(mfxU16 type) const
{
    AllocQueue::const_iterator it = m_AllocatorQueue.begin();
    while (it != m_AllocatorQueue.end())
    {
        // external frames should be allocated at once
        // internal frames can be allocated many times
        if ((it->second->type == type)&&
            (it->second->type & MFX_MEMTYPE_EXTERNAL_FRAME))
            return it->second;
        ++it;
    }
    return 0;
}
mfxStatus CommonCORE::SetFrameAllocator(mfxFrameAllocator *allocator)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    if (!allocator)
        return MFX_ERR_NONE;

    if (!m_bSetExtFrameAlloc)
    {
        m_FrameAllocator.frameAllocator = *allocator;
        m_bSetExtFrameAlloc = true;
        m_session->m_coreInt.FrameAllocator = *allocator;
        return MFX_ERR_NONE;
    }
    else
        return MFX_ERR_UNDEFINED_BEHAVIOR;

}

// no care about surface, opaq and all round. Just increasing reference
mfxStatus CommonCORE::IncreasePureReference(mfxU16& Locked)
{
    //MFX_CHECK_NULL_PTR1(ptr);
    UMC::AutomaticUMCMutex guard(m_guard);
    if (Locked > 65534)
    {
        return MFX_ERR_LOCK_MEMORY;
    }
    else
    {
        vm_interlocked_inc16((volatile Ipp16u*)&Locked);
        return MFX_ERR_NONE;
    }
}// CommonCORE::IncreasePureReference(mfxFrameData *ptr)

// no care about surface, opaq and all round. Just increasing reference
mfxStatus CommonCORE::DecreasePureReference(mfxU16& Locked)
{
    //MFX_CHECK_NULL_PTR1(ptr);
    UMC::AutomaticUMCMutex guard(m_guard);
    if (Locked < 1)
    {
        return MFX_ERR_LOCK_MEMORY;
    }
    else
    {
        vm_interlocked_dec16((volatile Ipp16u*)&Locked);
        return MFX_ERR_NONE;
    }
}// CommonCORE::IncreasePureReference(mfxFrameData *ptr)

mfxStatus CommonCORE::IncreaseReference(mfxFrameData *ptr, bool ExtendedSearch)
{
    MFX_CHECK_NULL_PTR1(ptr);
    if (ptr->Locked > 65534)
    {
        return MFX_ERR_LOCK_MEMORY;
    }
    else
    {
        {
            UMC::AutomaticUMCMutex guard(m_guard);
            // Opaque surface syncronization
            if (m_bIsOpaqMode)
            {
                OpqTbl::iterator oqp_it;
                for (oqp_it = m_OpqTbl.begin(); oqp_it != m_OpqTbl.end(); oqp_it++)
                {
                    if (&oqp_it->second.Data == ptr)
                    {
                        vm_interlocked_inc16((volatile Ipp16u*)&(oqp_it->first->Data.Locked));
                        vm_interlocked_inc16((volatile Ipp16u*)&ptr->Locked);
                        return MFX_ERR_NONE;
                    }
                }
            }
        }

        // we dont find in self queue let find in neigb cores
        if (ExtendedSearch)
        {
            // makes sence to remove ans tay only error return
            if (MFX_ERR_NONE != m_session->m_pOperatorCore->DoCoreOperation(&VideoCORE::IncreaseReference, ptr))
                return IncreasePureReference(ptr->Locked);
            else
                return MFX_ERR_NONE;


        }
        return MFX_ERR_INVALID_HANDLE;
    }
}

mfxStatus CommonCORE::DecreaseReference(mfxFrameData *ptr, bool ExtendedSearch)
{
    MFX_CHECK_NULL_PTR1(ptr);
    // should be positive
    if (ptr->Locked < 1)
    {
        return MFX_ERR_LOCK_MEMORY;
    }
    else
    {
        {
            UMC::AutomaticUMCMutex guard(m_guard);
            // Opaque surface syncronization
            if (m_bIsOpaqMode)
            {
                OpqTbl::iterator oqp_it;
                for (oqp_it = m_OpqTbl.begin(); oqp_it != m_OpqTbl.end(); oqp_it++)
                {
                    if (&oqp_it->second.Data == ptr)
                    {
                        vm_interlocked_dec16((volatile Ipp16u*)&(oqp_it->first->Data.Locked));
                        vm_interlocked_dec16((volatile Ipp16u*)&ptr->Locked);
                        return MFX_ERR_NONE;
                    }
                }
            }
        }

        // we dont find in self queue let find in neigb cores
        if (ExtendedSearch)
        {
            // makes sence to remove ans tay only error return
            if (MFX_ERR_NONE != m_session->m_pOperatorCore->DoCoreOperation(&VideoCORE::DecreaseReference, ptr))
                return DecreasePureReference(ptr->Locked);
            else
                return MFX_ERR_NONE;
        }
        return MFX_ERR_INVALID_HANDLE;
    }
}

void CommonCORE::INeedMoreThreadsInside(const void *pComponent)
{
    if ((m_session) &&
        (m_session->m_pScheduler))
    {
        m_session->m_pScheduler->ResetWaitingStatus(pComponent);
    }

} // void CommonCORE::INeedMoreThreadsInside(const void *pComponent)

bool CommonCORE::IsExternalFrameAllocator() const
{
    return m_bSetExtFrameAlloc;
}

mfxStatus CommonCORE::DoFastCopyWrapper(mfxFrameSurface1 *pDst, mfxU16 dstMemType, mfxFrameSurface1 *pSrc, mfxU16 srcMemType)
{
    mfxStatus sts = MFX_ERR_NONE;

    mfxFrameSurface1 srcTempSurface, dstTempSurface;

    mfxMemId srcMemId, dstMemId;

    memset(&srcTempSurface, 0, sizeof(mfxFrameSurface1));
    memset(&dstTempSurface, 0, sizeof(mfxFrameSurface1));

    // save original mem ids
    srcMemId = pSrc->Data.MemId;
    dstMemId = pDst->Data.MemId;

    srcTempSurface.Info = pSrc->Info;
    dstTempSurface.Info = pDst->Info;

    srcTempSurface.Data.MemId = srcMemId;
    dstTempSurface.Data.MemId = dstMemId;

    bool isSrcLocked = false;
    bool isDstLocked = false;

    if (srcMemType & MFX_MEMTYPE_EXTERNAL_FRAME)
    {
        //if (srcMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            if (NULL == pSrc->Data.Y)
            {
                // only if pointers are absence
                sts = LockExternalFrame(srcMemId, &srcTempSurface.Data);
                MFX_CHECK_STS(sts);

                isSrcLocked = true;
            }
            else
            {
                srcTempSurface.Data = pSrc->Data;
            }

            srcTempSurface.Data.MemId = 0;
        }
    }
    else if (srcMemType & MFX_MEMTYPE_INTERNAL_FRAME)
    {
        //if (srcMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
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
            }

            srcTempSurface.Data.MemId = 0;
        }
    }

    if (dstMemType & MFX_MEMTYPE_EXTERNAL_FRAME)
    {
        //if (dstMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            if (NULL == pDst->Data.Y)
            {
                // only if pointers are absence
                sts = LockExternalFrame(dstMemId, &dstTempSurface.Data);
                MFX_CHECK_STS(sts);

                isDstLocked = true;
            }
            else
            {
                dstTempSurface.Data = pDst->Data;
            }

            dstTempSurface.Data.MemId = 0;
        }
    }
    else if (dstMemType & MFX_MEMTYPE_INTERNAL_FRAME)
    {
        if (dstMemType & MFX_MEMTYPE_SYSTEM_MEMORY)
        {
            if (NULL == pDst->Data.Y)
            {
                // only if pointers are absence
                sts = LockFrame(dstMemId, &dstTempSurface.Data);
                MFX_CHECK_STS(sts);

                isDstLocked = true;
            }
            else
            {
                dstTempSurface.Data = pDst->Data;
            }

            dstTempSurface.Data.MemId = 0;
        }
    }

    sts = DoFastCopyExtended(&dstTempSurface, &srcTempSurface);
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
}

mfxStatus CommonCORE::DoFastCopy(mfxFrameSurface1 *dst, mfxFrameSurface1 *src)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    mfxStatus sts;

    IppiSize roi = {IPP_MIN(src->Info.Width, dst->Info.Width), IPP_MIN(src->Info.Height, dst->Info.Height)};
    if (!roi.width || !roi.height)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    mfxU8 *pDst;
    mfxU8 *pSrc;

    Ipp32u srcPitch;
    Ipp32u dstPitch;

    FastCopy *pFastCopy = m_pFastCopy.get();

    if (FAST_COPY_SSE41 == m_pFastCopy.get()->GetSupportedMode())
    {
        pDst = dst->Data.Y;
        pSrc = src->Data.Y;

        if (NULL == pDst || NULL == pSrc)
        {
            return MFX_ERR_NULL_PTR;
        }

        srcPitch = src->Data.PitchLow + ((mfxU32)src->Data.PitchHigh << 16);
        dstPitch = dst->Data.PitchLow + ((mfxU32)dst->Data.PitchHigh << 16);

        switch (dst->Info.FourCC)
        {
        case MFX_FOURCC_NV12:

            sts = pFastCopy->Copy(pDst, dstPitch, pSrc, srcPitch, roi);

            roi.height >>= 1;

            pSrc = src->Data.UV;
            pDst = dst->Data.UV;

            if (NULL == pDst || NULL == pSrc)
            {
                return MFX_ERR_NULL_PTR;
            }

            sts = pFastCopy->Copy(pDst, dstPitch, pSrc, srcPitch, roi);

            break;

        case MFX_FOURCC_YV12:

            sts = pFastCopy->Copy(pDst, dstPitch, pSrc, srcPitch, roi);

            roi.height >>= 1;

            pSrc = src->Data.U;
            pDst = dst->Data.U;

            if (NULL == pDst || NULL == pSrc)
            {
                return MFX_ERR_NULL_PTR;
            }

            roi.width >>= 1;

            srcPitch >>= 1;
            dstPitch >>= 1;

            sts = pFastCopy->Copy((mfxU8 *) pDst, dstPitch, (mfxU8 *) pSrc, srcPitch, roi);

            pSrc = src->Data.V;
            pDst = dst->Data.V;

            if (NULL == pDst || NULL == pSrc)
            {
                return MFX_ERR_NULL_PTR;
            }

            sts = pFastCopy->Copy((mfxU8 *) pDst, dstPitch, (mfxU8 *) pSrc, srcPitch, roi);

            break;

        case MFX_FOURCC_YUY2:

            roi.width *= 2;

            sts = pFastCopy->Copy(pDst, dstPitch, pSrc, srcPitch, roi);

            break;

        case MFX_FOURCC_P8:

            sts = pFastCopy->Copy(pDst, dstPitch, pSrc, srcPitch, roi);

            break;

        default:

            return MFX_ERR_UNSUPPORTED;
        }
    }
    else
    {
        pDst = dst->Data.Y;
        pSrc = src->Data.Y;

        if (NULL == pDst || NULL == pSrc)
        {
            return MFX_ERR_NULL_PTR;
        }

        srcPitch = src->Data.PitchLow + ((mfxU32)src->Data.PitchHigh << 16);
        dstPitch = dst->Data.PitchLow + ((mfxU32)dst->Data.PitchHigh << 16);

        switch (dst->Info.FourCC)
        {
        case MFX_FOURCC_NV12:

            ippiCopy_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi);

            roi.height >>= 1;

            pSrc = src->Data.UV;
            pDst = dst->Data.UV;

            if (NULL == pDst || NULL == pSrc)
            {
                return MFX_ERR_NULL_PTR;
            }

            ippiCopy_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi);

            break;

        case MFX_FOURCC_YV12:

            ippiCopy_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi);

            pSrc = src->Data.U;
            pDst = dst->Data.U;

            if (NULL == pDst || NULL == pSrc)
            {
                return MFX_ERR_NULL_PTR;
            }

            roi.width >>= 1;
            roi.height >>= 1;

            srcPitch >>= 1;
            dstPitch >>= 1;

            ippiCopy_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi);

            pSrc = src->Data.V;
            pDst = dst->Data.V;

            if (NULL == pDst || NULL == pSrc)
            {
                return MFX_ERR_NULL_PTR;
            }

            ippiCopy_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi);

            break;

        case MFX_FOURCC_YUY2:

            roi.width *= 2;

            ippiCopy_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi);

            break;

        case MFX_FOURCC_P8:

            ippiCopy_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi);

            break;

        default:

            return MFX_ERR_UNSUPPORTED;
        }
    }

    return MFX_ERR_NONE;
}
mfxStatus CommonCORE::DoFastCopyExtended(mfxFrameSurface1 *pDst, mfxFrameSurface1 *pSrc)
{
    // up mutex
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxStatus sts;

    sts = CheckFrameData(pSrc);
    MFX_CHECK_STS(sts);

    sts = CheckFrameData(pDst);
    MFX_CHECK_STS(sts);

    // CheckFrameData should be added

    // check that only memId or pointer are passed
    // otherwise don't know which type of memory copying is requested
    if (
        (NULL != pDst->Data.Y && NULL != pDst->Data.MemId) ||
        (NULL != pSrc->Data.Y && NULL != pSrc->Data.MemId)
        )
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    // check that external allocator was set
    if ((NULL != pDst->Data.MemId || NULL != pSrc->Data.MemId) &&  false == m_bSetExtFrameAlloc)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    IppiSize roi = {IPP_MIN(pSrc->Info.Width, pDst->Info.Width), IPP_MIN(pSrc->Info.Height, pDst->Info.Height)};

    // check that region of interest is valid
    if (0 == roi.width || 0 == roi.height)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    Ipp32u srcPitch = pSrc->Data.PitchLow + ((mfxU32)pSrc->Data.PitchHigh << 16);
    Ipp32u dstPitch = pDst->Data.PitchLow + ((mfxU32)pDst->Data.PitchHigh << 16);

    FastCopy *pFastCopy = m_pFastCopy.get();

    if (NULL != pSrc->Data.Y && NULL != pDst->Data.Y)
    {
        // system memories were passed
        // use common way to copy frames

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

            ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, pDst->Data.U, dstPitch, roi);
            ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, pDst->Data.V, dstPitch, roi);
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

                break;
            }
        case MFX_FOURCC_RGB4:
        case MFX_FOURCC_A2RGB10:
            {
                MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                mfxU8* ptrSrc = IPP_MIN(IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B);

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN( IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B );

                roi.width *= 4;

                ippiCopy_8u_C1R(ptrSrc, srcPitch, ptrDst, dstPitch, roi);

                break;
            }
        case MFX_FOURCC_P8:

            ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

            break;

            /*case MFX_FOURCC_IMC3:

            ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

            roi.width >>= 1;
            roi.height >>= 1;

            ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, pDst->Data.U, dstPitch, roi);
            ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, pDst->Data.V, dstPitch, roi);
            break;*/

        default:
            return MFX_ERR_UNSUPPORTED;
        }
    }
    else if (NULL != pSrc->Data.Y && NULL != pDst->Data.MemId)
    {
        // source are placed in system memory, destination is in video memory
        // use common way to copy frames from system to video, most faster

        sts = LockExternalFrame(pDst->Data.MemId, &pDst->Data);
        MFX_CHECK_STS(sts);

        dstPitch = pDst->Data.PitchLow + ((mfxU32)pDst->Data.PitchHigh << 16);

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

            ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, pDst->Data.U, dstPitch, roi);

            ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, pDst->Data.V, dstPitch, roi);

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

                mfxU8* ptrSrc = IPP_MIN( IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B );

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN( IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B );

                roi.width *= 3;

                ippiCopy_8u_C1R(ptrSrc, srcPitch, ptrDst, dstPitch, roi);

                break;
            }
        case MFX_FOURCC_RGB4:
        case MFX_FOURCC_A2RGB10:
            {
                MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                mfxU8* ptrSrc = IPP_MIN( IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B );

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN( IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B );

                roi.width *= 4;

                ippiCopy_8u_C1R(ptrSrc, srcPitch, ptrDst, dstPitch, roi);
                break;
            }
        case MFX_FOURCC_P8:
            ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);
            break;

            /*case MFX_FOURCC_IMC3:
            ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

            roi.width >>= 1;
            roi.height >>= 1;

            ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, pDst->Data.U, dstPitch, roi);
            ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, pDst->Data.V, dstPitch, roi);
            break;*/

        default:
            return MFX_ERR_UNSUPPORTED;
        }

        // unlock external frame
        sts = UnlockExternalFrame(pDst->Data.MemId, &pDst->Data);
        MFX_CHECK_STS(sts);
    }
    else if (NULL != pSrc->Data.MemId && NULL != pDst->Data.Y)
    {
        // source are placed in video memory, destination is in system memory
        // use fast copy sse approach

        // lock external frame
        sts = LockExternalFrame(pSrc->Data.MemId, &pSrc->Data);
        MFX_CHECK_STS(sts);

        srcPitch = pSrc->Data.PitchLow + ((mfxU32)pSrc->Data.PitchHigh << 16);

        switch (pDst->Info.FourCC)
        {
        case MFX_FOURCC_NV12:

            sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, pSrc->Data.Y, srcPitch, roi);
            MFX_CHECK_STS(sts);

            roi.height >>= 1;

            sts = pFastCopy->Copy(pDst->Data.UV, dstPitch, pSrc->Data.UV, srcPitch, roi);
            MFX_CHECK_STS(sts);

            break;

        case MFX_FOURCC_YV12:

            sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, pSrc->Data.Y, srcPitch, roi);
            MFX_CHECK_STS(sts);

            roi.width >>= 1;
            roi.height >>= 1;

            srcPitch >>= 1;
            dstPitch >>= 1;

            sts = pFastCopy->Copy(pDst->Data.U, dstPitch, pSrc->Data.U, srcPitch, roi);
            MFX_CHECK_STS(sts);

            sts = pFastCopy->Copy(pDst->Data.V, dstPitch, pSrc->Data.V, srcPitch, roi);
            MFX_CHECK_STS(sts);

            break;

        case MFX_FOURCC_YUY2:

            roi.width *= 2;

            sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, pSrc->Data.Y, srcPitch, roi);
            MFX_CHECK_STS(sts);

            break;

        case MFX_FOURCC_RGB3:
            {
                MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                mfxU8* ptrSrc = IPP_MIN( IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B );

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN( IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B );

                roi.width *= 3;

                sts = pFastCopy->Copy(ptrDst, dstPitch, ptrSrc, srcPitch, roi);
                MFX_CHECK_STS(sts);

                break;
            }
        case MFX_FOURCC_RGB4:
            {
                MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                mfxU8* ptrSrc = IPP_MIN( IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B );

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN( IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B );

                roi.width *= 4;

                sts = pFastCopy->Copy(ptrDst, dstPitch, ptrSrc, srcPitch, roi);
                MFX_CHECK_STS(sts);

                break;
            }
        case MFX_FOURCC_P8:

            sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, pSrc->Data.Y, srcPitch, roi);
            MFX_CHECK_STS(sts);
            break;

            /*case MFX_FOURCC_IMC3:

            sts = pFastCopy->Copy(pDst->Data.Y, dstPitch, pSrc->Data.Y, srcPitch, roi);
            MFX_CHECK_STS(sts);

            roi.width >>= 1;
            roi.height >>= 1;

            sts = pFastCopy->Copy(pDst->Data.U, dstPitch, pSrc->Data.U, srcPitch, roi);
            MFX_CHECK_STS(sts);

            sts = pFastCopy->Copy(pDst->Data.V, dstPitch, pSrc->Data.V, srcPitch, roi);
            MFX_CHECK_STS(sts);
            break;*/

        default:
            return MFX_ERR_UNSUPPORTED;
        }

        // unlock external frame
        sts = UnlockExternalFrame(pSrc->Data.MemId, &pSrc->Data);
        MFX_CHECK_STS(sts);
    }
    else if (NULL != pSrc->Data.MemId && NULL != pDst->Data.MemId)
    {
        // the both frames are placed in video memory
        // use common memcpy approach

        // lock external frame
        sts = LockExternalFrame(pSrc->Data.MemId, &pSrc->Data);
        MFX_CHECK_STS(sts);

        // lock external frame
        sts = LockExternalFrame(pDst->Data.MemId, &pDst->Data);
        MFX_CHECK_STS(sts);

        srcPitch = pSrc->Data.PitchLow + ((mfxU32)pSrc->Data.PitchHigh << 16);
        dstPitch = pDst->Data.PitchLow + ((mfxU32)pDst->Data.PitchHigh << 16);

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

            ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, pDst->Data.U, dstPitch, roi);

            ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, pDst->Data.V, dstPitch, roi);

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

                mfxU8* ptrSrc = IPP_MIN( IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B );

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN( IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B );

                roi.width *= 3;

                ippiCopy_8u_C1R(ptrSrc, srcPitch, ptrDst, dstPitch, roi);

                break;
            }
        case MFX_FOURCC_RGB4:
            {
                MFX_CHECK_NULL_PTR1(pSrc->Data.R);
                MFX_CHECK_NULL_PTR1(pSrc->Data.G);
                MFX_CHECK_NULL_PTR1(pSrc->Data.B);

                mfxU8* ptrSrc = IPP_MIN( IPP_MIN(pSrc->Data.R, pSrc->Data.G), pSrc->Data.B );

                MFX_CHECK_NULL_PTR1(pDst->Data.R);
                MFX_CHECK_NULL_PTR1(pDst->Data.G);
                MFX_CHECK_NULL_PTR1(pDst->Data.B);

                mfxU8* ptrDst = IPP_MIN( IPP_MIN(pDst->Data.R, pDst->Data.G), pDst->Data.B );

                roi.width *= 4;

                ippiCopy_8u_C1R(ptrSrc, srcPitch, ptrDst, dstPitch, roi);

                break;
            }
        case MFX_FOURCC_P8:

            ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

            break;

            /*case MFX_FOURCC_IMC3:
            ippiCopy_8u_C1R(pSrc->Data.Y, srcPitch, pDst->Data.Y, dstPitch, roi);

            roi.width >>= 1;
            roi.height >>= 1;

            ippiCopy_8u_C1R(pSrc->Data.U, srcPitch, pDst->Data.U, dstPitch, roi);
            ippiCopy_8u_C1R(pSrc->Data.V, srcPitch, pDst->Data.V, dstPitch, roi);
            break;*/


        default:
            return MFX_ERR_UNSUPPORTED;
        }

        // unlock external frame
        sts = UnlockExternalFrame(pDst->Data.MemId, &pDst->Data);
        MFX_CHECK_STS(sts);

        // unlock external frame
        sts = UnlockExternalFrame(pSrc->Data.MemId, &pSrc->Data);
        MFX_CHECK_STS(sts);

        // return partial acceleration because of there is no possibility
        // to copy video to video in fastest way by directx interface
        return MFX_WRN_PARTIAL_ACCELERATION;
    }

    return MFX_ERR_NONE;
}

bool CommonCORE::IsFastCopyEnabled()
{
    return (FAST_COPY_SSE41 == m_pFastCopy.get()->GetSupportedMode());
}

#define REG_ROOT                    HKEY_CURRENT_USER
#define REG_PATH_MEDIASDK           TEXT("Software\\Intel\\MediaSDK")
#define REG_KEY_TIMING_LOG          TEXT("TimingLog")
#define REG_KEY_TIMING_PER_FRAME    TEXT("TimingPerFrame")

mfxStatus CommonCORE::CheckTimingLog()
{
#ifdef MFX_DEBUG_TOOLS
    HKEY hKey;
    TCHAR timing_filename[MAX_PATH] = TEXT("");
    DWORD dwPerFrameStatistic = 0;
    DWORD type1, type2;
    DWORD size1 = sizeof(timing_filename);
    DWORD size2 = sizeof(DWORD);

    if (ERROR_SUCCESS != RegOpenKeyEx(REG_ROOT, REG_PATH_MEDIASDK, 0, KEY_QUERY_VALUE, &hKey))
    {
        // not enough permissions
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    // read "TimingPerFrame" key
    RegQueryValueEx(hKey, REG_KEY_TIMING_PER_FRAME, NULL, &type2, (LPBYTE)&dwPerFrameStatistic, &size2);

    if (ERROR_SUCCESS == RegQueryValueEx(hKey, REG_KEY_TIMING_LOG, NULL, &type1, (LPBYTE)timing_filename, &size1))
    {
        if (REG_SZ == type1)
        {
            MFX::AutoTimer::Init(timing_filename, dwPerFrameStatistic);
        }
    }

    RegCloseKey(hKey);
#endif

    return MFX_ERR_NONE;
}


bool CommonCORE::CheckOpaqueRequest(mfxFrameAllocRequest *request,
                                    mfxFrameSurface1 **pOpaqueSurface,
                                    mfxU32 NumOpaqueSurface,
                                    bool ExtendedSearch)
{
    if (pOpaqueSurface &&
        request->NumFrameMin != NumOpaqueSurface)
        return false;

    if (!(request->Type  & MFX_MEMTYPE_OPAQUE_FRAME))
        return false;

    if (m_OpqTbl.size())
    {
        OpqTbl::iterator oqp_it;
        bool isOpaqAllocated = false;
        mfxU32 i = 0;
        // each opaq surface has correspondence in already allocated OR
        // no one opaq surface sholud already allocated
        for (;i < request->NumFrameMin;i++)
        {
            oqp_it = m_OpqTbl.find(pOpaqueSurface[i]);
            if (oqp_it != m_OpqTbl.end())
            {
                isOpaqAllocated = true;
            }
            else if (isOpaqAllocated)
                return false;
        }
    }
    if (ExtendedSearch)// try to find in another cores
    {
        bool sts = m_session->m_pOperatorCore->CheckOpaqRequest(request, pOpaqueSurface, NumOpaqueSurface);
        return sts;
    }

    return true;

}
bool CommonCORE::IsOpaqSurfacesAlreadyMapped(mfxFrameSurface1 **pOpaqueSurface,
                                             mfxU32 NumOpaqueSurface, 
                                             mfxFrameAllocResponse *response,
                                             bool ExtendedSearch)
{
    if (m_OpqTbl.size())
    {
        mfxU32 i = 0;
        OpqTbl::iterator oqp_it;
        oqp_it = m_OpqTbl.find(pOpaqueSurface[i]);
        // consistent already checked in CheckOpaqueRequest function
        if (oqp_it != m_OpqTbl.end())
        {
            UMC::AutomaticUMCMutex guard(m_guard);

            CorrespTbl::iterator ctbl_it;

            m_pMemId.reset(new mfxMemId[2*NumOpaqueSurface]);
            response->mids = m_pMemId.get();
            m_pMemId.pop();

            for (; i < NumOpaqueSurface; i++)
            {
                oqp_it = m_OpqTbl.find(pOpaqueSurface[i]);
                if (oqp_it != m_OpqTbl.end())
                {
                    ctbl_it = m_CTbl.find(oqp_it->second.Data.MemId);
                    if (m_CTbl.end() == ctbl_it)
                        return false;
                    response->mids[i] = oqp_it->second.Data.MemId;
                    response->mids[i+NumOpaqueSurface] = ctbl_it->second.InternalMid;
                }
                else
                    return false;
            }

            response->NumFrameActual = (mfxU16)NumOpaqueSurface;


            RefCtrTbl::iterator ref_it;
            for (ref_it = m_RefCtrTbl.begin(); ref_it != m_RefCtrTbl.end(); ref_it++)
            {
                if (IsEqual(*ref_it->first, *response))
                {
                    ref_it->second++;
                    m_RespMidQ.insert(pair<mfxMemId*, mfxMemId*>(response->mids, response->mids+NumOpaqueSurface));
                    return true;
                }
            }
            return false; // unexpected behavior

        }
        else if (ExtendedSearch)
        {
            bool sts = m_session->m_pOperatorCore->IsOpaqSurfacesAlreadyMapped(pOpaqueSurface, NumOpaqueSurface, response);
            return sts;
        }
    }
    else if (ExtendedSearch) // try to find in another cores
    {
        bool sts = m_session->m_pOperatorCore->IsOpaqSurfacesAlreadyMapped(pOpaqueSurface, NumOpaqueSurface, response);
        return sts;
    }

    return false;

}

// 15 bits - uniq surface Id
// 15 bits score Id
bool CommonCORE::GetUniqID(mfxMemId& id)
{
    size_t count = 1;
    CorrespTbl::iterator ctbl_it;
    for (; count < (1 << 15); count++)
    {
        ctbl_it = m_CTbl.find((mfxMemId)(count | (m_CoreId << 15)));
        if (ctbl_it == m_CTbl.end())
        {
            id = (mfxMemId)(count | (m_CoreId << 15));
            return true;
        }
    }          
    return false;
}

bool  CommonCORE::SetCoreId(mfxU32 Id) 
{
    if (m_CoreId < (1 << 15)) 
    {
        m_CoreId = Id;
        return true;
    }
    return false;
}

void* CommonCORE::QueryCoreInterface(const MFX_GUID &guid)
{
    if (MFXIVideoCORE_GUID == guid)
        return (void*) this;

    return NULL;
}

void CommonCORE::SetWrapper(void* pWrp)
{
    m_pWrp = (mfx_UMC_FrameAllocator *)pWrp;
}

mfxU16 CommonCORE::GetAutoAsyncDepth() 
{
    return (mfxU16)vm_sys_info_get_cpu_num();
}

