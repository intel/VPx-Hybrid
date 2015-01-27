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

File Name: mfx_umc_alloc_wrapper.cpp

\* ****************************************************************************** */
#include "umc_defs.h"

#include "mfx_umc_alloc_wrapper.h"
#include "mfx_common.h"
#include "libmfx_core.h"
#include "mfx_common_int.h"

#include "ippi.h"
#include "ippcc.h"
#include "ipps.h"
#if defined (MFX_VA_WIN)
#include "dxgi.h"
#endif
#include "vm_file.h"

#if defined (MFX_VA_WIN)
#include "umc_d3d_video_processing.h"

#include "mfx_vpp_jpeg_d3d9.h"

#endif

mfx_UMC_MemAllocator::mfx_UMC_MemAllocator():m_pCore(NULL)
{
}

mfx_UMC_MemAllocator::~mfx_UMC_MemAllocator()
{
}

UMC::Status mfx_UMC_MemAllocator::InitMem(UMC::MemoryAllocatorParams *, VideoCORE* mfxCore)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    UMC::Status Sts = UMC::UMC_OK;
    if(!mfxCore)
        return UMC::UMC_ERR_NULL_PTR;
    m_pCore = mfxCore;
    return Sts;
}

UMC::Status mfx_UMC_MemAllocator::Close()
{
    UMC::AutomaticUMCMutex guard(m_guard);

    UMC::Status sts = UMC::UMC_OK;
    m_pCore = 0;
    return sts;
}

UMC::Status mfx_UMC_MemAllocator::Alloc(UMC::MemID *pNewMemID, size_t Size, Ipp32u , Ipp32u )
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxMemId memId;
    mfxStatus Sts = m_pCore->AllocBuffer((mfxU32)Size, /*MFX_MEMTYPE_PERSISTENT_MEMORY*/ MFX_MEMTYPE_SYSTEM_MEMORY, &memId);
    MFX_CHECK_UMC_STS(Sts);
    *pNewMemID = ((UMC::MemID)memId + 1);
    return UMC::UMC_OK;
}

void* mfx_UMC_MemAllocator::Lock(UMC::MemID MID)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxStatus Sts = MFX_ERR_NONE;

    mfxU8 *ptr;
    Sts = m_pCore->LockBuffer((mfxHDL)(MID - 1), &ptr);
    if (Sts < MFX_ERR_NONE)
        return 0;

    return ptr;
}

UMC::Status mfx_UMC_MemAllocator::Unlock(UMC::MemID MID)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    UMC::Status sts = UMC::UMC_OK;
    m_pCore->UnlockBuffer((mfxHDL)(MID - 1));
    return sts;
}

UMC::Status mfx_UMC_MemAllocator::Free(UMC::MemID MID)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    m_pCore->FreeBuffer((mfxHDL)(MID - 1));
    return UMC::UMC_OK;
}

UMC::Status mfx_UMC_MemAllocator::DeallocateMem(UMC::MemID )
{
    UMC::Status sts = UMC::UMC_OK;
    return sts;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// mfx_UMC_FrameAllocator implementation
////////////////////////////////////////////////////////////////////////////////////////////////
mfx_UMC_FrameAllocator::FrameInformation::FrameInformation()
    : m_locks(0)
    , m_referenceCounter(0)
{
}

void mfx_UMC_FrameAllocator::FrameInformation::Reset()
{
    m_locks = 0;
    m_referenceCounter = 0;
}

mfx_UMC_FrameAllocator::mfx_UMC_FrameAllocator()
    : m_curIndex(-1),
      m_pCore(0),
      m_externalFramesResponse(0),
      m_isSWDecode(false)
{
}

mfx_UMC_FrameAllocator::~mfx_UMC_FrameAllocator()
{
    Close();
}

UMC::Status mfx_UMC_FrameAllocator::InitMfx(UMC::FrameAllocatorParams *,
                                            VideoCORE* mfxCore,
                                            const mfxVideoParam *params,
                                            const mfxFrameAllocRequest *request,
                                            mfxFrameAllocResponse *response,
                                            bool isUseExternalFrames,
                                            bool isSWplatform)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    m_isSWDecode = isSWplatform;

    if (!mfxCore || !params)
        return UMC::UMC_ERR_NULL_PTR;

    if (!isUseExternalFrames && (!request || !response))
        return UMC::UMC_ERR_NULL_PTR;

    m_pCore = mfxCore;
    m_IsUseExternalFrames = isUseExternalFrames;

    Ipp32s bit_depth = params->mfx.FrameInfo.FourCC == MFX_FOURCC_P010 ? 10 : 8;

    UMC::ColorFormat color_format;

    switch (params->mfx.FrameInfo.FourCC)
    {
    case MFX_FOURCC_NV12:
        color_format = UMC::NV12;
        break;
    case MFX_FOURCC_P010:
        color_format = UMC::NV12;
        break;
    case MFX_FOURCC_RGB4:
        color_format = UMC::RGB32;
        break;
    case MFX_FOURCC_YV12:
        color_format = UMC::YUV420;
        break;
    case MFX_FOURCC_YUY2:
        color_format = UMC::YUY2;
        break;
#if defined (MFX_VA_WIN)
    case DXGI_FORMAT_AYUV:
        color_format = UMC::RGB32;
        break;
#endif
    default:
        return UMC::UMC_ERR_UNSUPPORTED;
    }

    UMC::Status umcSts;

    if (MFX_CODEC_JPEG != params->mfx.CodecId || MFX_ROTATION_0 == params->mfx.Rotation || MFX_ROTATION_180 == params->mfx.Rotation)
    {
        umcSts = m_info.Init(params->mfx.FrameInfo.Width, params->mfx.FrameInfo.Height, color_format, bit_depth);

        m_surface.Info.Width = params->mfx.FrameInfo.Width;
        m_surface.Info.Height = params->mfx.FrameInfo.Height;
    }
    else
    {
        umcSts = m_info.Init(params->mfx.FrameInfo.Height, params->mfx.FrameInfo.Width, color_format, bit_depth);

        m_surface.Info.Width = params->mfx.FrameInfo.Height;
        m_surface.Info.Height = params->mfx.FrameInfo.Width;
    }

    if (umcSts != UMC::UMC_OK)
        return umcSts;

    if (!m_IsUseExternalFrames || !m_isSWDecode)
    {
        m_frameData.resize(response->NumFrameActual);
        m_extSurfaces.resize(response->NumFrameActual);

        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            m_frameData[i].first.Data.MemId = response->mids[i];

            memcpy_s(&m_frameData[i].first.Info, sizeof(mfxFrameInfo), &request->Info, sizeof(mfxFrameInfo));

            // fill UMC frameData
            FrameInformation * frameMID = &m_frameData[i].second;
            frameMID->Reset();
            UMC::FrameData* frameData = &frameMID->m_frame;

            // set correct width & height to planes
            frameData->Init(&m_info, (UMC::FrameMemID)i, this);
        }
    }

    mfxCore->SetWrapper(this);

    return UMC::UMC_OK;
}


UMC::Status mfx_UMC_FrameAllocator::Close()
{
    UMC::AutomaticUMCMutex guard(m_guard);

    Reset();
    m_frameData.clear();
    m_extSurfaces.clear();
    return UMC::UMC_OK;
}

void mfx_UMC_FrameAllocator::SetExternalFramesResponse(mfxFrameAllocResponse *response)
{
    m_externalFramesResponse = 0;

    if (!response || !response->NumFrameActual)
        return;

    m_externalFramesResponse = response;
}

UMC::Status mfx_UMC_FrameAllocator::Reset()
{
    UMC::AutomaticUMCMutex guard(m_guard);

    m_curIndex = -1;
    mfxStatus sts = MFX_ERR_NONE;

    // unlock internal sufraces
    for (mfxU32 i = 0; i < m_frameData.size(); i++)
    {
        m_frameData[i].first.Data.Locked = 0;  // if app ext allocator then should decrease Locked counter same times as locked by medisSDK
        m_frameData[i].second.Reset();
    }

    // free external sufraces
    for (mfxU32 i = 0; i < m_extSurfaces.size(); i++)
    {
        if (m_extSurfaces[i].isUsed)
        {
            sts = m_pCore->DecreaseReference(&m_extSurfaces[i].FrameSurface->Data);
            if (sts < MFX_ERR_NONE)
                return UMC::UMC_ERR_FAILED;
            m_extSurfaces[i].isUsed = false;
        }

        m_extSurfaces[i].FrameSurface = 0;
    }

    return UMC::UMC_OK;
}

UMC::Status mfx_UMC_FrameAllocator::Alloc(UMC::FrameMemID *pNewMemID, const UMC::VideoDataInfo * info, Ipp32u )
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxStatus sts = MFX_ERR_NONE;
    if (!pNewMemID)
        return UMC::UMC_ERR_NULL_PTR;

    mfxI32 index = FindFreeSurface();
    if (index == -1)
    {
        *pNewMemID = UMC::FRAME_MID_INVALID;
        return UMC::UMC_ERR_ALLOC;
    }

    // DEBUG : need to check that VideoDataInfo info same as allocator have

    //if (MFX_HW_D3D11 == GetVAType())
    //{
    //    mfxMemId mid = m_frameData[index].first.Data.MemId;
    //    mfxHDLPair pair;
    //    // should be external
    //    if (m_IsUseExternalFrames)
    //        sts = m_pCore->GetExternalFrameHDL(mid, (mfxHDL*)&pair);
    //    else
    //        sts = m_pCore->GetFrameHDL(mid, (mfxHDL*)&pair);

    //    if (sts < MFX_ERR_NONE)
    //        return UMC::UMC_ERR_FAILED;

    //    *pNewMemID = static_cast<UMC::FrameMemID>((size_t)pair.second);
    //} 
    //else
    {
        *pNewMemID = (UMC::FrameMemID)index;
    }

    IppiSize allocated = {static_cast<int>(m_info.GetWidth()), static_cast<int>(m_info.GetHeight())};
    IppiSize passed = {static_cast<int>(info->GetWidth()), static_cast<int>(info->GetHeight())};
    UMC::ColorFormat colorFormat = info->GetColorFormat();

    switch(colorFormat)
    {
    case UMC::YUV420:
    case UMC::GRAY:
    case UMC::YV12:
    case UMC::NV12:
    case UMC::IMC3:
    case UMC::RGB32:
        break;
    default:
        return UMC::UMC_ERR_UNSUPPORTED;
    }

    if (passed.width > allocated.width ||
        passed.height > allocated.height)
    {
        return UMC::UMC_ERR_UNSUPPORTED;
    }

    sts = m_pCore->IncreasePureReference(m_frameData[index].first.Data.Locked);
    if (sts < MFX_ERR_NONE)
        return UMC::UMC_ERR_FAILED;

    if (m_IsUseExternalFrames)
    {
        if (m_extSurfaces[index].FrameSurface)
        {
            sts = m_pCore->IncreaseReference(&m_extSurfaces[index].FrameSurface->Data);
            if (sts < MFX_ERR_NONE)
                return UMC::UMC_ERR_FAILED; 

            m_extSurfaces[m_curIndex].isUsed = true;
        }
    }

    m_frameData[index].second.Reset();
    m_curIndex = -1;

    return UMC::UMC_OK;
}

const UMC::FrameData* mfx_UMC_FrameAllocator::Lock(UMC::FrameMemID mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxU32 index = (mfxU32)mid;
    if (index >= m_frameData.size())
        return 0;

    mfxFrameData *data = 0;

    mfxFrameSurface1 check_surface;
    check_surface.Info.FourCC = m_frameData[index].first.Info.FourCC;

    if (m_IsUseExternalFrames)
    {
        if (m_frameData[index].first.Data.MemId != 0)
        {
            data = &m_frameData[index].first.Data;
            mfxStatus sts = m_pCore->LockExternalFrame(m_frameData[index].first.Data.MemId, data);

            if (sts < MFX_ERR_NONE || !data)
                return 0;

            check_surface.Data = *data;
            check_surface.Data.MemId = 0;
            sts = CheckFrameData(&check_surface);
            if (sts < MFX_ERR_NONE)
                return 0;
        }
        else
        {
            data = &m_extSurfaces[index].FrameSurface->Data;
        }
    }
    else
    {
        if (m_frameData[index].first.Data.MemId != 0)
        {
            data = &m_frameData[index].first.Data;
            mfxStatus sts = m_pCore->LockFrame(m_frameData[index].first.Data.MemId, data);

            if (sts < MFX_ERR_NONE || !data)
                return 0;

            check_surface.Data = *data;
            check_surface.Data.MemId = 0;
            sts = CheckFrameData(&check_surface);
            if (sts < MFX_ERR_NONE)
                return 0;
        }
        else // invalid situation, we always allocate internal frames with MemId
            return 0;
    }

    FrameInformation * frameMID = &m_frameData[index].second;
    UMC::FrameData* frameData = &frameMID->m_frame;
    mfxU32 pitch = data->PitchLow + ((mfxU32)data->PitchHigh << 16);

    switch (frameData->GetInfo()->GetColorFormat())
    {
    case UMC::NV12:
        frameData->SetPlanePointer(data->Y, 0, pitch);
        frameData->SetPlanePointer(data->U, 1, pitch);
        break;
    case UMC::YUV420:
        frameData->SetPlanePointer(data->Y, 0, pitch);
        frameData->SetPlanePointer(data->U, 1, pitch >> 1);
        frameData->SetPlanePointer(data->V, 2, pitch >> 1);
        break;
    case UMC::IMC3:
        frameData->SetPlanePointer(data->Y, 0, pitch);
        frameData->SetPlanePointer(data->U, 1, pitch);
        frameData->SetPlanePointer(data->V, 2, pitch);
        break;
    case UMC::RGB32:
        {
            frameData->SetPlanePointer(data->B, 0, pitch);
        }
        break;
    case UMC::YUY2:
        {
            frameData->SetPlanePointer(data->Y, 0, pitch);
        }
        break;
    default:
        if (m_frameData[index].first.Data.MemId)
        {
            if (m_IsUseExternalFrames)
            {
                m_pCore->UnlockExternalFrame(m_extSurfaces[index].FrameSurface->Data.MemId);
            }
            else
            {
                m_pCore->UnlockFrame(m_frameData[index].first.Data.MemId);
            }
        }
        return 0;
    }

    //frameMID->m_locks++;
    return frameData;
}

UMC::Status mfx_UMC_FrameAllocator::Unlock(UMC::FrameMemID mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxU32 index = (mfxU32)mid;
    if (index >= m_frameData.size())
        return UMC::UMC_ERR_FAILED;

    if (m_frameData[index].first.Data.MemId)
    {
        mfxStatus sts;
        if (m_IsUseExternalFrames)
            sts = m_pCore->UnlockExternalFrame(m_extSurfaces[index].FrameSurface->Data.MemId);
        else
            sts = m_pCore->UnlockFrame(m_frameData[index].first.Data.MemId);

        if (sts < MFX_ERR_NONE)
            return UMC::UMC_ERR_FAILED;
    }

    return UMC::UMC_OK;
}

UMC::Status mfx_UMC_FrameAllocator::IncreaseReference(UMC::FrameMemID mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxU32 index = (mfxU32)mid;
    if (index >= m_frameData.size())
        return UMC::UMC_ERR_FAILED;

    FrameInformation * frameMID = &m_frameData[index].second;
    frameMID->m_referenceCounter++;

    return UMC::UMC_OK;
}

UMC::Status mfx_UMC_FrameAllocator::DecreaseReference(UMC::FrameMemID mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxU32 index = (mfxU32)mid;
    if (index >= m_frameData.size())
        return UMC::UMC_ERR_FAILED;

    FrameInformation * frameMID = &m_frameData[index].second;

    frameMID->m_referenceCounter--;
    if (!frameMID->m_referenceCounter)
    {
        if (frameMID->m_locks)
        {
            frameMID->m_referenceCounter++;
            return UMC::UMC_ERR_FAILED;
        }

        return Free(mid);
    }

    return UMC::UMC_OK;
}

UMC::Status mfx_UMC_FrameAllocator::Free(UMC::FrameMemID mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxStatus sts = MFX_ERR_NONE;
    mfxU32 index = (mfxU32)mid;
    if (index >= m_frameData.size())
        return UMC::UMC_ERR_FAILED;

    sts = m_pCore->DecreasePureReference(m_frameData[index].first.Data.Locked);
    if (sts < MFX_ERR_NONE)
        return UMC::UMC_ERR_FAILED;

    if (m_IsUseExternalFrames)
    {
        sts = m_pCore->DecreaseReference(&m_extSurfaces[index].FrameSurface->Data);
        if (sts < MFX_ERR_NONE)
            return UMC::UMC_ERR_FAILED;
        m_extSurfaces[index].isUsed = false;
    }

    return UMC::UMC_OK;
}

mfxStatus mfx_UMC_FrameAllocator::SetCurrentMFXSurface(mfxFrameSurface1 *surf, bool isOpaq)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    if (surf->Data.Locked)
        return MFX_ERR_MORE_SURFACE;

    // device checking
    {
        mfxStatus sts;
        sts = m_pCore->CheckHandle();
        if (sts < MFX_ERR_NONE && MFX_ERR_NOT_FOUND != sts)
            return sts;
    }

    if (m_externalFramesResponse && surf->Data.MemId)
    {
        bool isFound = false;
        for (mfxI32 i = 0; i < m_externalFramesResponse->NumFrameActual; i++)
        {
            if (!isOpaq)
            {
                if (m_pCore->MapIdx(m_externalFramesResponse->mids[i]) == surf->Data.MemId)
                {
                    isFound = true;
                    break;
                }
            }
            else // opaque means that we are working with internal surface as with external
            {
                if (m_externalFramesResponse->mids[i] == surf->Data.MemId)
                {
                    isFound = true;
                    break;
                }
            }
        }

        if (!isFound)
            return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    m_curIndex = -1;
    if (!m_IsUseExternalFrames)
    {
        m_curIndex = FindFreeSurface();
        return MFX_ERR_NONE;
    }

    m_curIndex = FindSurface(surf, isOpaq);

    if (m_curIndex != -1)
    {
        m_extSurfaces[m_curIndex].FrameSurface = surf;
        if (m_frameData[m_curIndex].first.Data.Locked) // surface was locked yet
        {
            m_curIndex = -1;
        }
    }
    else
    {
        m_curIndex = AddSurface(surf);
        if (m_curIndex != -1)
            m_extSurfaces[m_curIndex].FrameSurface = surf;
    }

    return MFX_ERR_NONE;
}

mfxI32 mfx_UMC_FrameAllocator::AddSurface(mfxFrameSurface1 *surface)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxI32 index = -1;

    if (!m_IsUseExternalFrames)
        return -1;

    if (surface->Data.MemId && 
        !m_isSWDecode)
    {
        mfxU32 i;
        for (i = 0; i < m_extSurfaces.size(); i++)
        {
            if (surface->Data.MemId == m_pCore->MapIdx(m_frameData[i].first.Data.MemId))
            {
                m_extSurfaces[i].FrameSurface = surface;
                index = i;
                break;
            }
        }
    }
    else
    {
        m_extSurfaces.push_back(surf_descr(surface,false));
        index = (mfxI32)(m_extSurfaces.size() - 1);
    }

    switch (surface->Info.FourCC)
    {
    case MFX_FOURCC_NV12:
    case MFX_FOURCC_YV12:
    case MFX_FOURCC_YUY2:
    case MFX_FOURCC_RGB4:
    case MFX_FOURCC_P010:
#if defined (MFX_VA_WIN)
    case DXGI_FORMAT_AYUV:
#endif
        break;
    default:
        return -1;
    }

    if (m_IsUseExternalFrames && 
        m_isSWDecode)
    {
        FrameInfo  frameInfo;
        m_frameData.push_back(frameInfo);

        memset(&(m_frameData[index].first), 0, sizeof(m_frameData[index].first));
        m_frameData[index].first.Data.MemId = surface->Data.MemId;
        m_frameData[index].first.Info = surface->Info;

        // fill UMC frameData
        FrameInformation * frameMID = &m_frameData[index].second;
        frameMID->Reset();
        UMC::FrameData* frameData = &frameMID->m_frame;

        // set correct width & height to planes
        frameData->Init(&m_info, (UMC::FrameMemID)index, this);
    }

    return index;
}

mfxI32 mfx_UMC_FrameAllocator::FindSurface(mfxFrameSurface1 *surf, bool isOpaq)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    if (!surf)
        return -1;

    mfxFrameData * data = &surf->Data;

    if (data->MemId && m_IsUseExternalFrames)
    {
        mfxMemId    sMemId;
        for (mfxU32 i = 0; i < m_frameData.size(); i++)
        {
            sMemId = isOpaq == true ? m_frameData[i].first.Data.MemId : m_pCore->MapIdx(m_frameData[i].first.Data.MemId);
            if (sMemId == data->MemId)
            {
                return i;
            }
        }
    }

    for (mfxU32 i = 0; i < m_extSurfaces.size(); i++)
    {
        if (m_extSurfaces[i].FrameSurface == surf)
        {
            return i;
        }
    }

    return -1;
}

mfxI32 mfx_UMC_FrameAllocator::FindFreeSurface()
{
    UMC::AutomaticUMCMutex guard(m_guard);

    if (m_IsUseExternalFrames)
    {
        return m_curIndex;
    }

    if (m_curIndex != -1)
        return m_curIndex;

    for (mfxU32 i = 0; i < m_frameData.size(); i++)
    {
        if (!m_frameData[i].first.Data.Locked)
        {
            return i;
        }
    }

    return -1;
}

mfxFrameSurface1 * mfx_UMC_FrameAllocator::GetInternalSurface(UMC::FrameMemID index)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    if (m_IsUseExternalFrames)
    {
        return 0;
    }

    if (index >= 0)
    {
        if ((Ipp32u)index >= m_frameData.size())
            return 0;
        return &m_frameData[index].first;
    }

    return 0;
}

mfxFrameSurface1 * mfx_UMC_FrameAllocator::GetSurface(UMC::FrameMemID index, mfxFrameSurface1 *surface, const mfxVideoParam * videoPar)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    if (!surface || !videoPar || 0 > index)
        return 0;

    if (m_IsUseExternalFrames)
    {
        if ((Ipp32u)index >= m_frameData.size())
            return 0;
        return m_extSurfaces[index].FrameSurface;
    }
    else
    {
        mfxStatus sts = m_pCore->IncreaseReference(&surface->Data);
        if (sts < MFX_ERR_NONE)
            return 0;

        m_extSurfaces[index].FrameSurface = surface;
    }

    return surface;
}

mfxStatus mfx_UMC_FrameAllocator::PrepareToOutput(mfxFrameSurface1 *surface_work, UMC::FrameMemID index, const mfxVideoParam *, bool isOpaq)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    mfxStatus sts;
    isOpaq;

    UMC::FrameData* frame = &m_frameData[index].second.m_frame;

    if (m_IsUseExternalFrames)
        return MFX_ERR_NONE;

    switch (frame->GetInfo()->GetColorFormat())
    {
    case UMC::NV12:
        m_surface.Data.Y = frame->GetPlaneMemoryInfo(0)->m_planePtr;
        m_surface.Data.UV = frame->GetPlaneMemoryInfo(1)->m_planePtr;
        m_surface.Data.PitchHigh = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch / (1 << 16));
        m_surface.Data.PitchLow  = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch % (1 << 16));
        break;

    case UMC::YUV420:
        m_surface.Data.Y = frame->GetPlaneMemoryInfo(0)->m_planePtr;
        m_surface.Data.U = frame->GetPlaneMemoryInfo(1)->m_planePtr;
        m_surface.Data.V = frame->GetPlaneMemoryInfo(2)->m_planePtr;
        m_surface.Data.PitchHigh = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch / (1 << 16));
        m_surface.Data.PitchLow  = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch % (1 << 16));
        break;

    case UMC::IMC3:
        m_surface.Data.Y = frame->GetPlaneMemoryInfo(0)->m_planePtr;
        m_surface.Data.U = frame->GetPlaneMemoryInfo(1)->m_planePtr;
        m_surface.Data.V = frame->GetPlaneMemoryInfo(2)->m_planePtr;
        m_surface.Data.PitchHigh = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch / (1 << 16));
        m_surface.Data.PitchLow  = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch % (1 << 16));
        break;

    case UMC::RGB32:
        m_surface.Data.B = frame->GetPlaneMemoryInfo(0)->m_planePtr;
        m_surface.Data.G = m_surface.Data.B + 1;
        m_surface.Data.R = m_surface.Data.B + 2;
        m_surface.Data.A = m_surface.Data.B + 3;
        m_surface.Data.PitchHigh = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch / (1 << 16));
        m_surface.Data.PitchLow  = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch % (1 << 16));
        break;
        
    case UMC::YUY2:
        m_surface.Data.Y = frame->GetPlaneMemoryInfo(0)->m_planePtr;
        m_surface.Data.U = m_surface.Data.Y + 1;
        m_surface.Data.V = m_surface.Data.Y + 3;
        m_surface.Data.PitchHigh = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch / (1 << 16));
        m_surface.Data.PitchLow  = (mfxU16)(frame->GetPlaneMemoryInfo(0)->m_pitch % (1 << 16));
        break;

    default:
        return MFX_ERR_UNSUPPORTED;
    }

    MFX_INTERNAL_CPY(&m_surface.Info, &surface_work->Info, sizeof(surface_work->Info));

    mfxFrameSurface1 dstSurface;
    MFX_INTERNAL_CPY(&dstSurface.Info, &surface_work->Info, sizeof(surface_work->Info));
    MFX_INTERNAL_CPY(&dstSurface.Data, &surface_work->Data, sizeof(surface_work->Data));

    sts = m_pCore->DoFastCopyWrapper(surface_work,
                                     MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_DXVA2_DECODER_TARGET,
                                     &m_surface,
                                     MFX_MEMTYPE_INTERNAL_FRAME | MFX_MEMTYPE_SYSTEM_MEMORY);
    MFX_CHECK_STS(sts);

    if (!m_IsUseExternalFrames)
    {
        mfxStatus temp_sts = m_pCore->DecreaseReference(&surface_work->Data);

        if (temp_sts < MFX_ERR_NONE && sts >= MFX_ERR_NONE)
        {
            sts = temp_sts;
        }

        m_extSurfaces[index].FrameSurface = 0;
    }

    return sts;
}
#if !defined( AS_HEVCD_PLUGIN ) || defined (AS_VP8D_PLUGIN) // HEVC decode natively supportes NV12 format - no need to make conversion 
UMC::Status mfx_UMC_FrameAllocator_NV12::InitMfx(UMC::FrameAllocatorParams *pParams,
                                                 VideoCORE* mfxCore,
                                                 const mfxVideoParam * params,
                                                 const mfxFrameAllocRequest *request,
                                                 mfxFrameAllocResponse *response,
                                                 bool isUseExternalFrames,
                                                 bool isSWplatform)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    UMC::Status sts = mfx_UMC_FrameAllocator::InitMfx(pParams, mfxCore, params, request, response, isUseExternalFrames, isSWplatform);
    if (sts != UMC::UMC_OK)
        return sts;

    UMC::Status umcSts = m_info_yuv420.Init(params->mfx.FrameInfo.Width, params->mfx.FrameInfo.Height, UMC::YUV420, 8);
    if (umcSts != UMC::UMC_OK)
        return umcSts;

    if (!m_IsUseExternalFrames)
    {
        m_yv12Frames.resize(response->NumFrameActual);

        UMC::FrameAllocatorParams tmpParams;
        m_defaultUMCAllocator.Init(&tmpParams);

        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            UMC::FrameData * frame = &m_yv12Frames[i];
            size_t pitch;

            UMC::FrameMemID memID;
            UMC::Status umcRes = m_defaultUMCAllocator.Alloc(&memID, &m_info_yuv420, 0);
            if (umcRes != UMC::UMC_OK)
                return umcRes;

            const UMC::FrameData* yuvFrame = m_defaultUMCAllocator.Lock(memID);
            if (!yuvFrame)
                return UMC::UMC_ERR_ALLOC;

            frame->Init(&m_info_yuv420, (UMC::FrameMemID)i, this);

            // set planes' pointers
            pitch = yuvFrame->GetPlaneMemoryInfo(0)->m_pitch;
            frame->SetPlanePointer(yuvFrame->GetPlaneMemoryInfo(0)->m_planePtr, 0, pitch);

            pitch = yuvFrame->GetPlaneMemoryInfo(1)->m_pitch;
            frame->SetPlanePointer(yuvFrame->GetPlaneMemoryInfo(1)->m_planePtr, 1, pitch);

            pitch = yuvFrame->GetPlaneMemoryInfo(2)->m_pitch;
            frame->SetPlanePointer(yuvFrame->GetPlaneMemoryInfo(2)->m_planePtr, 2, pitch);
        }
    }

    return UMC::UMC_OK;
}

const UMC::FrameData* mfx_UMC_FrameAllocator_NV12::Lock(UMC::FrameMemID mid)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    const UMC::FrameData* nv12Frame = mfx_UMC_FrameAllocator::Lock(mid);
    if (!nv12Frame)
        return 0;

    mfxU32 index = (mfxU32)mid;
    if (index >= m_frameData.size())
        return 0;

    const UMC::FrameData* frame = &m_yv12Frames[index];
    return frame;
}

mfxStatus mfx_UMC_FrameAllocator_NV12::ConvertToNV12(const UMC::FrameData * fd, mfxFrameData *data, const mfxVideoParam * videoPar)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    if (!fd || !data || !videoPar)
        return MFX_ERR_NULL_PTR;

    //const UMC::VideoDataInfo * info = fd->GetInfo();
    const UMC::FrameData::PlaneMemoryInfo * planeInfo = fd->GetPlaneMemoryInfo(0);

    if (!planeInfo)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    const mfxFrameInfo & videoInfo = videoPar->mfx.FrameInfo;

    Ipp32s pYVUStep[3] = {(Ipp32s)fd->GetPlaneMemoryInfo(0)->m_pitch, (Ipp32s)fd->GetPlaneMemoryInfo(1)->m_pitch, (Ipp32s)fd->GetPlaneMemoryInfo(2)->m_pitch};
    const Ipp8u *(pYVU[3]) = {fd->GetPlaneMemoryInfo(0)->m_planePtr, fd->GetPlaneMemoryInfo(1)->m_planePtr, fd->GetPlaneMemoryInfo(2)->m_planePtr};

    Ipp8u *(pDst[2]) = {data->Y,
                        data->U};
    Ipp32s pDstStep[2] = {static_cast<Ipp32s>(data->PitchLow + ((mfxU32)data->PitchHigh << 16)),
                          static_cast<Ipp32s>(data->PitchLow + ((mfxU32)data->PitchHigh << 16))};

    IppiSize srcSize = {videoInfo.Width, videoInfo.Height};
    IppStatus sts = ippiYCbCr420_8u_P3P2R(pYVU, pYVUStep, pDst[0], pDstStep[0], pDst[1], pDstStep[1], srcSize);

    if (sts != ippStsNoErr)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    return MFX_ERR_NONE;
}

UMC::Status mfx_UMC_FrameAllocator_NV12::Reset()
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfx_UMC_FrameAllocator::Reset();
    m_defaultUMCAllocator.Reset();
    return UMC::UMC_OK;
}

UMC::Status mfx_UMC_FrameAllocator_NV12::Close()
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfx_UMC_FrameAllocator::Close();
    m_yv12Frames.clear();
    m_defaultUMCAllocator.Close();

    return UMC::UMC_OK;
}

mfxI32 mfx_UMC_FrameAllocator_NV12::AddSurface(mfxFrameSurface1 *surface)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxI32 index = mfx_UMC_FrameAllocator::AddSurface(surface);

    m_yv12Frames.push_back(UMC::FrameData());

    UMC::FrameData * frame = &m_yv12Frames[index];
    size_t pitch;

    UMC::FrameMemID memID;
    UMC::Status umcSts = m_defaultUMCAllocator.Alloc(&memID, &m_info_yuv420, 0);
    if (umcSts != UMC::UMC_OK)
        return umcSts;

    const UMC::FrameData* yuvFrame = m_defaultUMCAllocator.Lock(memID);
    if (!yuvFrame)
        return UMC::UMC_ERR_ALLOC;

    frame->Init(&m_info_yuv420, (UMC::FrameMemID)index, this);

    pitch = yuvFrame->GetPlaneMemoryInfo(0)->m_pitch;
    frame->SetPlanePointer(yuvFrame->GetPlaneMemoryInfo(0)->m_planePtr, 0, pitch);

    pitch = yuvFrame->GetPlaneMemoryInfo(1)->m_pitch;
    frame->SetPlanePointer(yuvFrame->GetPlaneMemoryInfo(1)->m_planePtr, 1, pitch);

    pitch = yuvFrame->GetPlaneMemoryInfo(2)->m_pitch;
    frame->SetPlanePointer(yuvFrame->GetPlaneMemoryInfo(2)->m_planePtr, 2, pitch);

    return index;
}

mfxFrameSurface1 * mfx_UMC_FrameAllocator_NV12::GetSurface(UMC::FrameMemID index, mfxFrameSurface1 *surface, const mfxVideoParam * videoPar)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    if (index >= (Ipp32s)m_frameData.size())
        return 0;

    return mfx_UMC_FrameAllocator::GetSurface(index, surface, videoPar);
}

mfxStatus mfx_UMC_FrameAllocator_NV12::PrepareToOutput(mfxFrameSurface1 *surf_work, UMC::FrameMemID index, const mfxVideoParam * videoPar, bool isOpaq)
{
    isOpaq;
    UMC::AutomaticUMCMutex guard(m_guard);

    if (index < 0 || index >= (Ipp32s)m_frameData.size())
        return MFX_ERR_NOT_FOUND;

    const UMC::FrameData* frame = &m_yv12Frames[index];

    mfxStatus sts;
    if (m_frameData[index].first.Data.MemId)
    {
        sts = ConvertToNV12(frame, &m_frameData[index].first.Data, videoPar);
    }
    else
    {
        sts = ConvertToNV12(frame, &m_extSurfaces[index].FrameSurface->Data, videoPar);
    }

    if (sts < MFX_ERR_NONE)
        return sts;

    if (!m_IsUseExternalFrames)
    {
        sts = mfx_UMC_FrameAllocator::PrepareToOutput(surf_work, index, videoPar, isOpaq);
    }

    return sts;
}
#endif

#if defined (MFX_VA)
// D3D functionality
// we should copy to external SW surface
mfxStatus   mfx_UMC_FrameAllocator_D3D::PrepareToOutput(mfxFrameSurface1 *surface_work, UMC::FrameMemID index, const mfxVideoParam *,bool isOpaq)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxStatus sts = MFX_ERR_NONE;
    mfxMemId memId = isOpaq?(m_frameData[index].first.Data.MemId):(m_pCore->MapIdx(m_frameData[index].first.Data.MemId));

    if ((surface_work->Data.MemId)&&
        (surface_work->Data.MemId == memId))
    {
        // all frames are external. No need to do anything
        return MFX_ERR_NONE;
    }
    else
    {
        UMC::VideoDataInfo VInfo;

        mfxMemId idx = m_frameData[index].first.Data.MemId;

        m_surface.Data.Y = 0;
        m_surface.Data.MemId = idx;
        sts = m_pCore->DoFastCopyWrapper(surface_work,
                                         MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_SYSTEM_MEMORY,
                                         &m_surface,
                                         MFX_MEMTYPE_INTERNAL_FRAME | MFX_MEMTYPE_DXVA2_DECODER_TARGET
                                         );
        MFX_CHECK_STS(sts);

        if (!m_IsUseExternalFrames)
        {
            m_pCore->DecreaseReference(&surface_work->Data);
            m_extSurfaces[index].FrameSurface = 0;
        }

        return sts;
    }
}

#if defined (MFX_ENABLE_MJPEG_VIDEO_DECODE)
mfxStatus DoHwJpegCc(VideoVppJpegD3D9 **pCc, 
                     VideoCORE* mfxCore, 
                     mfxFrameSurface1 *pDst, 
                     mfxFrameSurface1 *pSrc, 
                     const mfxVideoParam * par, 
                     bool isD3DToSys, 
                     mfxU16 *taskId)
{
    mfxStatus sts = MFX_ERR_NONE;

    if (*pCc == NULL)
    {
        *pCc = new VideoVppJpegD3D9(mfxCore, isD3DToSys);

        sts = (*pCc)->Init(par);
        MFX_CHECK_STS( sts );
    }

    sts = (*pCc)->BeginHwJpegProcessing(pSrc, pDst, taskId);

    return sts;
}

mfxStatus DoHwJpegCcFw(VideoVppJpegD3D9 **pCc, 
                       VideoCORE* mfxCore, 
                       mfxFrameSurface1 *pDst, 
                       mfxFrameSurface1 *pSrcTop, 
                       mfxFrameSurface1 *pSrcBottom, 
                       const mfxVideoParam * par, 
                       bool isD3DToSys, 
                       mfxU16 *taskId)
{
    mfxStatus sts = MFX_ERR_NONE;

    if (*pCc == NULL)
    {
        *pCc = new VideoVppJpegD3D9(mfxCore, isD3DToSys);

        sts = (*pCc)->Init(par);
        MFX_CHECK_STS( sts );
    }

    sts = (*pCc)->BeginHwJpegProcessing(pSrcTop, pSrcBottom, pDst, taskId);

    return sts;
}

UMC::Status mfx_UMC_FrameAllocator_D3D_Converter::InitMfx(UMC::FrameAllocatorParams *,
                                                          VideoCORE* mfxCore,
                                                          const mfxVideoParam *params,
                                                          const mfxFrameAllocRequest *request,
                                                          mfxFrameAllocResponse *response,
                                                          bool isUseExternalFrames,
                                                          bool isSWplatform)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    m_isSWDecode = isSWplatform;

    if (!mfxCore || !params)
        return UMC::UMC_ERR_NULL_PTR;

    if (!isUseExternalFrames && (!request || !response))
        return UMC::UMC_ERR_NULL_PTR;

    m_pCore = mfxCore;
    m_IsUseExternalFrames = isUseExternalFrames;

    UMC::ColorFormat color_format;

    switch (params->mfx.FrameInfo.FourCC)
    {
    //case MFX_FOURCC_NV12:
    //    color_format = UMC::NV12;
    //    break;
    //case MFX_FOURCC_RGB4:
    //    color_format = UMC::RGB32;
    //    break;
    //case MFX_FOURCC_YV12:
    //    color_format = UMC::YUV420;
    //    break;
    case MFX_FOURCC_YUV400:
        color_format = UMC::GRAY;
        break;
    case MFX_FOURCC_IMC3:
        color_format = UMC::IMC3;
        break;
    case MFX_FOURCC_YUV422H:
    case MFX_FOURCC_YUV422V:
        color_format = UMC::YUV422;
        break;
    case MFX_FOURCC_YUV444:
        color_format = UMC::YUV444;
        break;
    case MFX_FOURCC_YUV411:
        color_format = UMC::YUV411;
        break;
    case MFX_FOURCC_RGBP:
        color_format = UMC::YUV444;
        break;
    default:
        return UMC::UMC_ERR_UNSUPPORTED;
    }

    UMC::Status umcSts;

    if(MFX_CODEC_JPEG != params->mfx.CodecId || MFX_ROTATION_0 == params->mfx.Rotation || MFX_ROTATION_180 == params->mfx.Rotation)
    {
        umcSts = m_info.Init(params->mfx.FrameInfo.Width, params->mfx.FrameInfo.Height, color_format, 8);

        m_surface.Info.Width = params->mfx.FrameInfo.Width;
        m_surface.Info.Height = params->mfx.FrameInfo.Height;
    }
    else
    {
        umcSts = m_info.Init(params->mfx.FrameInfo.Height, params->mfx.FrameInfo.Width, color_format, 8);

        m_surface.Info.Width = params->mfx.FrameInfo.Height;
        m_surface.Info.Height = params->mfx.FrameInfo.Width;
    }

    if (umcSts != UMC::UMC_OK)
        return umcSts;

    if (!m_IsUseExternalFrames ||
        !m_isSWDecode)
    {
        m_frameData.resize(response->NumFrameActual);
        m_extSurfaces.resize(response->NumFrameActual);

        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            m_frameData[i].first.Data.MemId = response->mids[i];

            MFX_INTERNAL_CPY(&m_frameData[i].first.Info, &request->Info, sizeof(mfxFrameInfo));

            // fill UMC frameData
            FrameInformation * frameMID = &m_frameData[i].second;
            frameMID->Reset();
            UMC::FrameData* frameData = &frameMID->m_frame;

            // set correct width & height to planes
            frameData->Init(&m_info, (UMC::FrameMemID)i, this);
        }
    }

    mfxCore->SetWrapper(this);

    return UMC::UMC_OK;
}

mfxStatus mfx_UMC_FrameAllocator_D3D_Converter::StartPreparingToOutput(mfxFrameSurface1 *surface_work,
                                                                       UMC::FrameData* in,
                                                                       const mfxVideoParam * par,
                                                                       VideoVppJpegD3D9 **pCc,
                                                                       mfxU16 *taskId,
                                                                       bool isOpaq)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxStatus sts = MFX_ERR_NONE;
    bool isD3DToSys = false;

    if(par->mfx.FrameInfo.PicStruct == MFX_PICSTRUCT_PROGRESSIVE)
    {
        UMC::FrameMemID index = in->GetFrameMID();

        mfxMemId memId = isOpaq?(m_frameData[index].first.Data.MemId):(m_pCore->MapIdx(m_frameData[index].first.Data.MemId));
        
        mfxHDLPair pPair;
        sts = m_pCore->GetExternalFrameHDL(surface_work->Data.MemId, (mfxHDL*)&pPair);

        if ((pPair.first)&&
            (pPair.first == memId))
        {
            // all frames are external. No need to do anything
            return MFX_ERR_NONE;
        }
        else
        {
            mfxFrameSurface1 pSrc;
            
            pSrc = m_frameData[index].first;
            if(par->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY)
            {
                isD3DToSys = true;
            }
            else if (par->IOPattern & MFX_IOPATTERN_OUT_OPAQUE_MEMORY)
            {
                mfxExtOpaqueSurfaceAlloc *pOpaqAlloc = (mfxExtOpaqueSurfaceAlloc *)GetExtendedBuffer(par->ExtParam, par->NumExtParam, MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION);
                if (!pOpaqAlloc)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

                isD3DToSys = !((pOpaqAlloc->Out.Type & MFX_MEMTYPE_SYSTEM_MEMORY) == 0);
            }

            sts = DoHwJpegCc(pCc, m_pCore, surface_work, &pSrc, par, isD3DToSys, taskId);
            if (sts < MFX_ERR_NONE)
                return sts;        

            return MFX_ERR_NONE;
        }
    }
    else
    {
        UMC::FrameMemID indexTop = in[0].GetFrameMID();
        UMC::FrameMemID indexBottom = in[1].GetFrameMID();

        // Opaque for interlaced content currently is unsupported
        mfxMemId memId = isOpaq?(m_frameData[indexTop].first.Data.MemId):(m_pCore->MapIdx(m_frameData[indexTop].first.Data.MemId));

        mfxHDLPair pPair;
        sts = m_pCore->GetExternalFrameHDL(surface_work->Data.MemId, (mfxHDL*)&pPair);

        if ((pPair.first)&&
            (pPair.first == memId))
        {
            // all frames are external. No need to do anything
            return MFX_ERR_NONE;
        }
        else
        {
            mfxFrameSurface1 pSrcTop, pSrcBottom;
            
            pSrcTop = m_frameData[indexTop].first;
            pSrcBottom = m_frameData[indexBottom].first;

            if(par->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY)
            {
                isD3DToSys = true;
            }
            else if (par->IOPattern & MFX_IOPATTERN_OUT_OPAQUE_MEMORY)
            {
                mfxExtOpaqueSurfaceAlloc *pOpaqAlloc = (mfxExtOpaqueSurfaceAlloc *)GetExtendedBuffer(par->ExtParam, par->NumExtParam, MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION);
                if (!pOpaqAlloc)
                    return MFX_ERR_INVALID_VIDEO_PARAM;

                isD3DToSys = !((pOpaqAlloc->Out.Type & MFX_MEMTYPE_SYSTEM_MEMORY) == 0);
            }

            sts = DoHwJpegCcFw(pCc, m_pCore, surface_work, &pSrcTop, &pSrcBottom, par, isD3DToSys, taskId);
            if (sts < MFX_ERR_NONE)
                return sts;        

            return MFX_ERR_NONE;
        }
    }
}

mfxStatus mfx_UMC_FrameAllocator_D3D_Converter::CheckPreparingToOutput(mfxFrameSurface1 *surface_work,
                                                                       UMC::FrameData* in,
                                                                       const mfxVideoParam * par,
                                                                       VideoVppJpegD3D9 **pCc,
                                                                       mfxU16 taskId)
{
    UMC::AutomaticUMCMutex guard(m_guard);

    mfxStatus sts = (*pCc)->QueryTaskRoutine(taskId);
    if (sts == MFX_TASK_BUSY)
    {
        return sts;
    }
    if (sts != MFX_TASK_DONE)
        return sts;

    if(par->mfx.FrameInfo.PicStruct == MFX_PICSTRUCT_PROGRESSIVE)
    {
        UMC::FrameMemID index = in->GetFrameMID();

        mfxFrameSurface1 pSrc = m_frameData[index].first;
        sts = (*pCc)->EndHwJpegProcessing(&pSrc, surface_work);
        if (sts < MFX_ERR_NONE)
            return sts;

        if (!m_IsUseExternalFrames)
        {
            m_pCore->DecreaseReference(&surface_work->Data);
            m_extSurfaces[index].FrameSurface = 0;
        }
    }
    else
    {
        UMC::FrameMemID indexTop = in[0].GetFrameMID();
        UMC::FrameMemID indexBottom = in[1].GetFrameMID();

        mfxFrameSurface1 pSrcTop, pSrcBottom;

        pSrcTop = m_frameData[indexTop].first;
        pSrcBottom = m_frameData[indexBottom].first;

        sts = (*pCc)->EndHwJpegProcessing(&pSrcTop, &pSrcBottom, surface_work);
        if (sts < MFX_ERR_NONE)
            return sts;

        if (!m_IsUseExternalFrames)
        {
            m_pCore->DecreaseReference(&surface_work->Data);
            m_extSurfaces[indexTop].FrameSurface = 0;
        }
    }

    return MFX_ERR_NONE;
}

void mfx_UMC_FrameAllocator_D3D_Converter::SetJPEGInfo(mfx_UMC_FrameAllocator_D3D_Converter::JPEG_Info * jpegInfo)
{
    m_jpegInfo = *jpegInfo;
}

#endif // #if defined (MFX_ENABLE_MJPEG_VIDEO_DECODE)
#endif // #if defined (MFX_VA)

