/* ****************************************************************************** *\

Copyright (C) 2006-2014 Intel Corporation.  All rights reserved.

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

File Name: umc_va_linux.cpp

\* ****************************************************************************** */

#include <umc_va_base.h>

#ifdef UMC_VA_LINUX

#include "umc_va_linux.h"
#include "vm_file.h"
#include "mfx_trace.h"

#define UMC_VA_NUM_OF_COMP_BUFFERS 8

//VASurfaceID *va_surface_list = NULL;

UMC::Status va_to_umc_res(VAStatus va_res)
{
    UMC::Status umcRes = UMC::UMC_OK;

    switch (va_res)
    {
    case VA_STATUS_SUCCESS:
        umcRes = UMC::UMC_OK;
        break;
    case VA_STATUS_ERROR_ALLOCATION_FAILED:
        umcRes = UMC::UMC_ERR_ALLOC;
        break;
    case VA_STATUS_ERROR_ATTR_NOT_SUPPORTED:
    case VA_STATUS_ERROR_UNSUPPORTED_PROFILE:
    case VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT:
    case VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT:
    case VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE:
    case VA_STATUS_ERROR_FLAG_NOT_SUPPORTED:
    case VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED:
        umcRes = UMC::UMC_ERR_UNSUPPORTED;
        break;
    case VA_STATUS_ERROR_INVALID_DISPLAY:
    case VA_STATUS_ERROR_INVALID_CONFIG:
    case VA_STATUS_ERROR_INVALID_CONTEXT:
    case VA_STATUS_ERROR_INVALID_SURFACE:
    case VA_STATUS_ERROR_INVALID_BUFFER:
    case VA_STATUS_ERROR_INVALID_IMAGE:
    case VA_STATUS_ERROR_INVALID_SUBPICTURE:
        umcRes = UMC::UMC_ERR_INVALID_PARAMS;
        break;
    case VA_STATUS_ERROR_INVALID_PARAMETER:
        umcRes = UMC::UMC_ERR_INVALID_PARAMS;
    case VA_STATUS_ERROR_DECODING_ERROR:
        umcRes = UMC::UMC_ERR_INVALID_STREAM;
    default:
        umcRes = UMC::UMC_ERR_FAILED;
        break;
    }
    return umcRes;
}

VAEntrypoint umc_to_va_entrypoint(Ipp32u umc_entrypoint)
{
    VAEntrypoint va_entrypoint = (VAEntrypoint)-1;

    switch (umc_entrypoint)
    {
    case UMC::VA_MC:
        va_entrypoint = VAEntrypointMoComp;
        break;
    case UMC::VA_IT:
        va_entrypoint = VAEntrypointIDCT;
        break;
    case UMC::VA_VLD:
        va_entrypoint = VAEntrypointVLD;
        break;
    case UMC::VA_DEBLOCK:
        va_entrypoint = VAEntrypointDeblocking;
        break;
    default:
        va_entrypoint = (VAEntrypoint)-1;
        break;
    }
    return va_entrypoint;
}

// profile priorities for codecs
Ipp32u g_Profiles[] =
{
    UMC::MPEG2_VLD, UMC::MPEG2_IT,
    UMC::H264_VLD,
    UMC::VC1_VLD, UMC::VC1_MC,
    UMC::VP8_VLD
};

// va profile priorities for different codecs
VAProfile g_Mpeg2Profiles[] =
{
    VAProfileMPEG2Main, VAProfileMPEG2Simple
};

VAProfile g_Mpeg4Profiles[] =
{
    VAProfileMPEG4Main, VAProfileMPEG4AdvancedSimple, VAProfileMPEG4Simple
};

VAProfile g_H264Profiles[] =
{
    VAProfileH264High, VAProfileH264Main, VAProfileH264Baseline
};

VAProfile g_VC1Profiles[] =
{
    VAProfileVC1Advanced, VAProfileVC1Main, VAProfileVC1Simple
};

VAProfile g_VP8Profiles[] =
{
    VAProfileVP8Version0_3
};

VAProfile get_next_va_profile(Ipp32u umc_codec, Ipp32u profile)
{
    VAProfile va_profile = (VAProfile)-1;

    switch (umc_codec)
    {
    case UMC::VA_MPEG2:
        if (profile < UMC_ARRAY_SIZE(g_Mpeg2Profiles)) va_profile = g_Mpeg2Profiles[profile];
        break;
    case UMC::VA_MPEG4:
        if (profile < UMC_ARRAY_SIZE(g_Mpeg4Profiles)) va_profile = g_Mpeg4Profiles[profile];
        break;
    case UMC::VA_H264:
        if (profile < UMC_ARRAY_SIZE(g_H264Profiles)) va_profile = g_H264Profiles[profile];
        break;
    case UMC::VA_VC1:
        if (profile < UMC_ARRAY_SIZE(g_VC1Profiles)) va_profile = g_VC1Profiles[profile];
        break;
    case UMC::VA_VP8:
        if (profile < UMC_ARRAY_SIZE(g_VP8Profiles)) va_profile = g_VP8Profiles[profile];
        break;
    default:
        va_profile = (VAProfile)-1;
        break;
    }
    return va_profile;
}

typedef struct _CodeStringTable
{
    Ipp32s   code;
    const vm_char* string;
} CodeStringTable;

CodeStringTable g_BuffersNames[] =
{
    { VAPictureParameterBufferType,    VM_STRING("VAPictureParameterBufferType") },
    { VAIQMatrixBufferType,            VM_STRING("VAIQMatrixBufferType") },
    { VABitPlaneBufferType,            VM_STRING("VABitPlaneBufferType") },
    { VASliceGroupMapBufferType,       VM_STRING("VASliceGroupMapBufferType") },
    { VASliceParameterBufferType,      VM_STRING("VASliceParameterBufferType") },
    { VASliceDataBufferType,           VM_STRING("VASliceDataBufferType") },
    { VAMacroblockParameterBufferType, VM_STRING("VAMacroblockParameterBufferType") },
    { VAResidualDataBufferType,        VM_STRING("VAResidualDataBufferType") },
    { VADeblockingParameterBufferType, VM_STRING("VADeblockingParameterBufferType") },
    { VAImageBufferType,               VM_STRING("VAImageBufferType") }
};

const vm_char* umcCodeToString(CodeStringTable *table, Ipp32s table_size, Ipp32s code)
{
    Ipp32s i;
    for (i = 0; i < table_size; i++)
    {
        if (table[i].code == code) return table[i].string;
    }
    return VM_STRING("Undefined");
}

#define UMC_VA_CODE_TO_STRING(table, code) \
  umcCodeToString(table, sizeof(table)/sizeof(CodeStringTable), code)

const vm_char* get_va_buffer_type(Ipp32s type)
{
    return UMC_VA_CODE_TO_STRING(g_BuffersNames, type);
}

namespace UMC
{

VACompBuffer::VACompBuffer(void)
{
    m_NumOfItem = 0;
    m_index     = -1;
    m_id        = -1;
    m_bDestroy  = false;
}

VACompBuffer::~VACompBuffer(void)
{
}

Status VACompBuffer::SetBufferInfo(Ipp32s _type, Ipp32s _id, Ipp32s _index)
{
    type  = _type;
    m_id    = _id;
    m_index = _index;
    return UMC_OK;
}

Status VACompBuffer::SetDestroyStatus(bool _destroy)
{
    UNREFERENCED_PARAMETER(_destroy);
    m_bDestroy = true;
    return UMC_OK;
}

LinuxVideoAccelerator::LinuxVideoAccelerator(void)
{
    m_dpy        = NULL;
    m_context    = -1;
    m_config_id  = -1;
    m_surfaces   = NULL;
    m_iIndex     = UMC_VA_LINUX_INDEX_UNDEF;
    m_FrameState = lvaBeforeBegin;

    m_pCompBuffers  = NULL;
    m_NumOfFrameBuffers = 0;
    m_uiCompBuffersNum  = 0;
    m_uiCompBuffersUsed = 0;

    vm_mutex_set_invalid(&m_SyncMutex);

    m_bIsExtSurfaces    = false;
    m_isUseStatuReport  = true;
    m_bH264MVCSupport   = false;
    memset(&m_guidDecoder, 0 , sizeof(GUID));
}

LinuxVideoAccelerator::~LinuxVideoAccelerator(void)
{
    Close();
}

Status LinuxVideoAccelerator::Init(VideoAcceleratorParams* pInfo)
{
    Status         umcRes = UMC_OK;
    VAStatus       va_res = VA_STATUS_SUCCESS;
    VAConfigAttrib va_attributes[2];

    LinuxVideoAcceleratorParams* pParams = DynamicCast<LinuxVideoAcceleratorParams>(pInfo);
    Ipp32s width = 0, height = 0;

    // checking errors in input parameters
    if ((UMC_OK == umcRes) && (NULL == pParams))
        umcRes = UMC_ERR_NULL_PTR;
    if ((UMC_OK == umcRes) && (NULL == pParams->m_pVideoStreamInfo))
        umcRes = UMC_ERR_INVALID_PARAMS;
    if ((UMC_OK == umcRes) && (NULL == pParams->m_Display))
        umcRes = UMC_ERR_INVALID_PARAMS;
    if ((UMC_OK == umcRes) && (pParams->m_iNumberSurfaces < 0))
        umcRes = UMC_ERR_INVALID_PARAMS;
    if (UMC_OK == umcRes)
    {
        vm_mutex_set_invalid(&m_SyncMutex);
        umcRes = (Status)vm_mutex_init(&m_SyncMutex);
    }
    // filling input parameters
    if (UMC_OK == umcRes)
    {
        m_iIndex            = UMC_VA_LINUX_INDEX_UNDEF;
        m_dpy               = pParams->m_Display;
        width               = pParams->m_pVideoStreamInfo->clip_info.width;
        height              = pParams->m_pVideoStreamInfo->clip_info.height;
        m_NumOfFrameBuffers = pParams->m_iNumberSurfaces;
        m_FrameState        = lvaBeforeBegin;

        // profile or stream type should be set
        if (UNKNOWN == (m_Profile & VA_CODEC))
        {
            VideoAccelerationProfile va_codec = VideoType2VAProfile(pParams->m_pVideoStreamInfo->stream_type);
            m_Profile = (VideoAccelerationProfile)(m_Profile | va_codec);
        }
    }
    if ((UMC_OK == umcRes) && (UNKNOWN == m_Profile))
        umcRes = UMC_ERR_INVALID_PARAMS;
    // display initialization
    if((UMC_OK == umcRes) && !pParams->isExt)
    {
        int major_version = 0, minor_version = 0;

        va_res = vaInitialize(m_dpy, &major_version, &minor_version);
        umcRes = va_to_umc_res(va_res);
    }
    if (UMC_OK == umcRes)
    {
        Ipp32s i,j;
        int va_max_num_profiles      = vaMaxNumProfiles   (m_dpy);
        int va_max_num_entrypoints   = vaMaxNumEntrypoints(m_dpy);
        int va_num_profiles          = 0;
        int va_num_entrypoints       = 0;
        VAProfile*    va_profiles    = NULL;
        VAEntrypoint* va_entrypoints = NULL;
        VAProfile     va_profile     = (VAProfile)-1;
        VAEntrypoint  va_entrypoint  = (VAEntrypoint)-1;

        if (UMC_OK == umcRes)
        {
            if ((va_max_num_profiles <= 0) || (va_max_num_entrypoints <= 0))
                umcRes = UMC_ERR_FAILED;
        }
        if (UMC_OK == umcRes)
        {
            va_profiles    = (VAProfile*)   ippsMalloc_8u(va_max_num_profiles   *sizeof(VAProfile));
            va_entrypoints = (VAEntrypoint*)ippsMalloc_8u(va_max_num_entrypoints*sizeof(VAEntrypoint));
            if ((NULL == va_profiles) || (NULL == va_entrypoints))
                umcRes = UMC_ERR_ALLOC;
        }
        if (UMC_OK == umcRes)
        {
            va_res = vaQueryConfigProfiles(m_dpy, va_profiles, &va_num_profiles);
            umcRes = va_to_umc_res(va_res);
        }
        if (UMC_OK == umcRes)
        {
            // checking support of some profile
            for (i = 0; (va_profile = get_next_va_profile(m_Profile & VA_CODEC, i)) != -1; ++i)
            {
                for (j = 0; j < va_num_profiles; ++j) if (va_profile == va_profiles[j]) break;
                if (j < va_num_profiles) break;
                else
                {
                    va_profile = (VAProfile)-1;
                    continue;
                }
            }
            if (va_profile < 0) umcRes = UMC_ERR_FAILED;

            if ((m_Profile & VA_CODEC) == UMC::VA_VC1)
            {
                if ((VC1_VIDEO_RCV == pInfo->m_pVideoStreamInfo->stream_subtype)
                  || (WMV3_VIDEO == pInfo->m_pVideoStreamInfo->stream_subtype))
                    va_profile = VAProfileVC1Main;
                else
                    va_profile = VAProfileVC1Advanced;
            }
        }
        if (UMC_OK == umcRes)
        {
            va_res = vaQueryConfigEntrypoints(m_dpy, va_profile, va_entrypoints, &va_num_entrypoints);
            umcRes = va_to_umc_res(va_res);
        }
        if (UMC_OK == umcRes)
        {
            Ipp32u k = 0, profile = UNKNOWN;

            for (k = 0; k < UMC_ARRAY_SIZE(g_Profiles); ++k)
            {
                if (!(m_Profile & VA_ENTRY_POINT))
                {
                    // entrypoint is not specified, we may chose
                    if (m_Profile != (g_Profiles[k] & VA_CODEC)) continue;
                    profile = g_Profiles[k];
                }
                else
                {
                    // codec and entrypoint are specified, we just need to check validity
                    profile = m_Profile;
                }
                va_entrypoint = umc_to_va_entrypoint(profile & VA_ENTRY_POINT);
                for (i = 0; i < va_num_entrypoints; ++i) if (va_entrypoint == va_entrypoints[i]) break;
                if (i < va_num_entrypoints) break;
                else
                {
                    va_entrypoint = (VAEntrypoint)-1;
                    if (m_Profile == profile) break;
                    else continue;
                }
            }
            m_Profile = (UMC::VideoAccelerationProfile)profile;
            if (va_entrypoint < 0) umcRes = UMC_ERR_FAILED;
        }

        va_attributes[0].type = VAConfigAttribRTFormat;
        va_res = vaGetConfigAttributes(m_dpy, va_profile, va_entrypoint, va_attributes, 1);
        umcRes = va_to_umc_res(va_res);
        if ((UMC_OK == umcRes) && (!(va_attributes[0].value & VA_RT_FORMAT_YUV420)))
           umcRes = UMC_ERR_FAILED;

        if (UMC_OK == umcRes)
        {
            va_res = vaCreateConfig(m_dpy, va_profile, va_entrypoint, va_attributes, 1, &m_config_id);
            umcRes = va_to_umc_res(va_res);
        }
        UMC_FREE(va_profiles);
        UMC_FREE(va_entrypoints);
    }

    // creating surfaces
    if (UMC_OK == umcRes)
    {
        m_surfaces = (VASurfaceID*)ippsMalloc_8u(m_NumOfFrameBuffers*sizeof(VASurfaceID));
        if (NULL == m_surfaces) umcRes = UMC_ERR_ALLOC;
    }
    if (UMC_OK == umcRes)
    {
        if(!pParams->isExt)
        {
            m_bIsExtSurfaces = false;

            va_res = vaCreateSurfaces(m_dpy, va_attributes[0].value, width, height, m_surfaces, m_NumOfFrameBuffers, NULL, 0);
            umcRes = va_to_umc_res(va_res);
        }
        else
        {
#if 1
            VASurfaceID* pSurfaces = (VASurfaceID*)pParams->m_surf;

            m_bIsExtSurfaces = true;
            for(Ipp32s i = 0; i < pParams->m_iNumberSurfaces; i++)
                m_surfaces[i] = pSurfaces[i];
#else
            va_res = vaCreateSurfaces(m_dpy, width, height, va_attributes[0].value, m_NumOfFrameBuffers, m_surfaces);
            umcRes = va_to_umc_res(va_res);
            for(Ipp32s i = 0; i < pParams->m_iNumberSurfaces; i++)
                pParams->m_surf[i] = (void*)m_surfaces[i];
            va_surface_list = m_surfaces;
#endif
        }
    }
    // creating context
    if (UMC_OK == umcRes)
    {
        va_res = vaCreateContext(m_dpy, m_config_id, width, height, VA_PROGRESSIVE, m_surfaces, m_NumOfFrameBuffers, &m_context);
        umcRes = va_to_umc_res(va_res);
    }
    return umcRes;
}

Status LinuxVideoAccelerator::Close(void)
{
    VACompBuffer* pCompBuf = NULL;
    Ipp32u i;

    if (NULL != m_pCompBuffers)
    {
        for (i = 0; i < m_uiCompBuffersUsed; ++i)
        {
            pCompBuf = m_pCompBuffers[i];
            if (pCompBuf->NeedDestroy() && (NULL != m_dpy))
            {
                vaDestroyBuffer(m_dpy, pCompBuf->GetID());
                pCompBuf->SetDestroyStatus(false);
            }
            UMC_DELETE(m_pCompBuffers[i]);
        }
        UMC_FREE(m_pCompBuffers);
    }
    if (NULL != m_dpy)
    {
        if (-1 != m_context)
        {
            vaDestroyContext(m_dpy, m_context);
            m_context = -1;
        }
        if (-1 != m_config_id)
        {
            vaDestroyConfig(m_dpy,m_config_id);
            m_config_id  = -1;
        }
        if (NULL != m_surfaces)
        {
            if (!m_bIsExtSurfaces) // free if self allocated surfaces
            {
                vaDestroySurfaces(m_dpy, m_surfaces, m_NumOfFrameBuffers);
            }
            UMC_FREE(m_surfaces);
        }
        m_dpy = NULL;
    }

    m_iIndex     = UMC_VA_LINUX_INDEX_UNDEF;
    m_FrameState = lvaBeforeBegin;
    m_uiCompBuffersNum  = 0;
    m_uiCompBuffersUsed = 0;
    vm_mutex_unlock (&m_SyncMutex);
    vm_mutex_destroy(&m_SyncMutex);
    return UMC_OK;
}

Status LinuxVideoAccelerator::BeginFrame(Ipp32s FrameBufIndex)
{
    MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "Dec: BeginFrame");
    Status   umcRes = UMC_OK;
    VAStatus va_res = VA_STATUS_SUCCESS;

    if ((UMC_OK == umcRes) && ((FrameBufIndex < 0) || (FrameBufIndex >= m_NumOfFrameBuffers)))
        umcRes = UMC_ERR_INVALID_PARAMS;
    if (UMC_OK == umcRes)
    {
        if (lvaBeforeBegin == m_FrameState)
        {
            {
                MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "vaBeginPicture");
                va_res = vaBeginPicture(m_dpy, m_context, m_surfaces[FrameBufIndex]);
            }
            umcRes = va_to_umc_res(va_res);
            if (UMC_OK == umcRes) m_FrameState = lvaBeforeEnd;
        }
    }
    return umcRes;
}

Status LinuxVideoAccelerator::AllocCompBuffers(void)
{
    Status umcRes = UMC_OK;

    if ((UMC_OK == umcRes) && (m_uiCompBuffersUsed >= m_uiCompBuffersNum))
    {
        if (NULL == m_pCompBuffers)
        {
            m_uiCompBuffersNum = UMC_VA_NUM_OF_COMP_BUFFERS;
            m_pCompBuffers = (VACompBuffer**)ippsMalloc_8u(m_uiCompBuffersNum * sizeof(VACompBuffer*));
            if (NULL == m_pCompBuffers)
            {
                m_uiCompBuffersNum = 0;
                umcRes = UMC_ERR_ALLOC;
            }
        }
        else
        {
            Ipp32u uiNewCompBuffersNum = 0;
            VACompBuffer** pNewCompBuffers = NULL;

            uiNewCompBuffersNum = m_uiCompBuffersNum + UMC_VA_NUM_OF_COMP_BUFFERS;
            pNewCompBuffers = (VACompBuffer**)ippsMalloc_8u(uiNewCompBuffersNum * sizeof(VACompBuffer*));

            if (NULL == pNewCompBuffers)
            {
                umcRes = UMC_ERR_ALLOC;
            }
            else
            {
                ippsCopy_8u((const Ipp8u*)m_pCompBuffers, (Ipp8u*)pNewCompBuffers, m_uiCompBuffersNum*sizeof(VACompBuffer*));
                ippsFree(m_pCompBuffers);
                m_uiCompBuffersNum = uiNewCompBuffersNum;
                m_pCompBuffers = pNewCompBuffers;
            }
        }
    }
    return umcRes;
}

void* LinuxVideoAccelerator::GetCompBuffer(Ipp32s buffer_type, UMCVACompBuffer **buf, Ipp32s size, Ipp32s index)
{
    Ipp32u i;
    VACompBuffer* pCompBuf = NULL;
    void* pBufferPointer = NULL;

    if (NULL != buf) *buf = NULL;

    vm_mutex_lock(&m_SyncMutex);
    for (i = 0; i < m_uiCompBuffersUsed; ++i)
    {
        pCompBuf = m_pCompBuffers[i];
        if ((pCompBuf->GetType() == buffer_type) && (pCompBuf->GetIndex() == index)) break;
    }
    if (i >= m_uiCompBuffersUsed)
    {
        AllocCompBuffers();
        pCompBuf = GetCompBufferHW(buffer_type, size, index);
        if (NULL != pCompBuf)
        {
            m_pCompBuffers[m_uiCompBuffersUsed] = pCompBuf;
            ++m_uiCompBuffersUsed;
        }
    }
    if (NULL != pCompBuf)
    {
        if (NULL != buf) *buf = pCompBuf;
        pBufferPointer = pCompBuf->GetPtr();
    }
    vm_mutex_unlock(&m_SyncMutex);
    return pBufferPointer;
}

VACompBuffer* LinuxVideoAccelerator::GetCompBufferHW(Ipp32s type, Ipp32s size, Ipp32s index)
{
    VAStatus   va_res = VA_STATUS_SUCCESS;
    VABufferID id;
    Ipp8u*      buffer = NULL;
    Ipp32u     buffer_size = 0;
    VACompBuffer* pCompBuffer = NULL;

    if (VA_STATUS_SUCCESS == va_res)
    {
        VABufferType va_type         = (VABufferType)type;
        unsigned int va_size         = 0;
        unsigned int va_num_elements = 0;

        if (VASliceParameterBufferType == va_type)
        {
            switch (m_Profile & VA_CODEC)
            {
            case UMC::VA_MPEG2:
                va_size         = sizeof(VASliceParameterBufferMPEG2);
                va_num_elements = size/sizeof(VASliceParameterBufferMPEG2);
                break;
            case UMC::VA_MPEG4:
                va_size         = sizeof(VASliceParameterBufferMPEG4);
                va_num_elements = size/sizeof(VASliceParameterBufferMPEG4);
                break;
            case UMC::VA_H264:
                if (m_bH264ShortSlice)
                {
                    va_size         = 0;
                    va_num_elements = 0;
                }
                else
                {
                    va_size         = sizeof(VASliceParameterBufferH264);
                    va_num_elements = size/sizeof(VASliceParameterBufferH264);
                }
                break;
            case UMC::VA_VC1:
                va_size         = sizeof(VASliceParameterBufferVC1);
                va_num_elements = size/sizeof(VASliceParameterBufferVC1);
                break;
            case UMC::VA_VP8:
                va_size         = sizeof(VASliceParameterBufferVP8);
                va_num_elements = size/sizeof(VASliceParameterBufferVP8);
                break;
            default:
                va_size         = 0;
                va_num_elements = 0;
                break;
            }
        }
        else
        {
            va_size         = size;
            va_num_elements = 1;
        }
        buffer_size = va_size * va_num_elements;

        va_res = vaCreateBuffer(m_dpy, m_context, va_type, va_size, va_num_elements, NULL, &id);
    }
    if (VA_STATUS_SUCCESS == va_res)
    {
        va_res = vaMapBuffer(m_dpy, id, (void**)&buffer);
    }
    if (VA_STATUS_SUCCESS == va_res)
    {
        pCompBuffer = new VACompBuffer();
        if (NULL != pCompBuffer)
        {
            pCompBuffer->SetBufferPointer(buffer, buffer_size);
            pCompBuffer->SetDataSize(0);
            pCompBuffer->SetBufferInfo(type, id, index);
            pCompBuffer->SetDestroyStatus(true);
        }
    }
    return pCompBuffer;
}

Status
LinuxVideoAccelerator::Execute()
{
    MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "Dec: Execute");
    Status         umcRes = UMC_OK;
    VAStatus       va_res = VA_STATUS_SUCCESS;
    VAStatus       va_sts = VA_STATUS_SUCCESS;
    VABufferID     id;
    Ipp32u         i;
    VACompBuffer*  pCompBuf = NULL;

    vm_mutex_lock(&m_SyncMutex);

    if (UMC_OK == umcRes)
    {
        for (i = 0; i < m_uiCompBuffersUsed; i++)
        {
            pCompBuf = m_pCompBuffers[i];
            id = pCompBuf->GetID();

            if (!m_bH264ShortSlice)
            {
                if (pCompBuf->GetType() == VASliceParameterBufferType)
                {
                    va_sts = vaBufferSetNumElements(m_dpy, id, pCompBuf->GetNumOfItem());
                    if (VA_STATUS_SUCCESS == va_res) va_res = va_sts;
                }
            }

            va_sts = vaUnmapBuffer(m_dpy, id);
            if (VA_STATUS_SUCCESS == va_res) va_res = va_sts;


            {
                MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "vaRenderPicture");
                va_sts = vaRenderPicture(m_dpy, m_context, &id, 1); // TODO: send all at once?
                if (VA_STATUS_SUCCESS == va_res) va_res = va_sts;
            }

            pCompBuf->SetDestroyStatus(false);
        }
#if 0
        /* NOTE:
        *  That code was used once and was invoked by setting lvaNeedUnmap in BeginFrame (see trunk@r36835).
        *
        */
        if ((UMC_OK == umcRes) && (lvaNeedUnmap == m_FrameState))
        {
            for (i = 0; i < m_uiCompBuffersUsed; ++i)
            {
                pCompBuf = m_pCompBuffers[i];

                va_sts = vaUnmapBuffer(m_dpy, pCompBuf->GetID());
                if (VA_STATUS_SUCCESS == va_res) va_res = va_sts;
            }
        }
#endif
    }

    vm_mutex_unlock(&m_SyncMutex);
    if (UMC_OK == umcRes)
    {
        umcRes = va_to_umc_res(va_res);
    }
    return umcRes;
}

Status LinuxVideoAccelerator::EndFrame(void*)
{
    MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "Dec: EndFrame");
    Status   umcRes = UMC_OK;
    VAStatus va_res = VA_STATUS_SUCCESS, va_sts = VA_STATUS_SUCCESS;
    Ipp32u i;
    VACompBuffer* pCompBuf = NULL;

    vm_mutex_lock(&m_SyncMutex);
#if 0
    /* NOTE:
    *  That code was used once and was invoked by setting lvaNeedUnmap in BeginFrame (see trunk@r36835).
    *
    */
    if ((UMC_OK == umcRes) && (lvaNeedUnmap == m_FrameState))
    {
        for (i = 0; i < m_uiCompBuffersUsed; ++i)
        {
            pCompBuf = m_pCompBuffers[i];
            va_sts = vaUnmapBuffer(m_dpy, pCompBuf->GetID());
            if (VA_STATUS_SUCCESS == va_sts) va_res = va_sts;
        }
    }
#endif
    if (UMC_OK == umcRes)
    {
        for (i = 0; i < m_uiCompBuffersUsed; ++i)
        {
            pCompBuf = m_pCompBuffers[i];
            if (pCompBuf->NeedDestroy())
            {
                va_sts = vaDestroyBuffer(m_dpy, pCompBuf->GetID());
                pCompBuf->SetDestroyStatus(false);
                if (VA_STATUS_SUCCESS == va_sts) va_res = va_sts;
            }
        }
    }
    if (UMC_OK == umcRes)
    {
        for (i = 0; i < m_uiCompBuffersUsed; ++i)
        {
            UMC_DELETE(m_pCompBuffers[i]);
        }
        m_uiCompBuffersUsed = 0;
    }
    if (UMC_OK == umcRes)
    {
        {
            MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "vaEndPicture");
            va_sts = vaEndPicture(m_dpy, m_context);
        }
        if (VA_STATUS_SUCCESS == va_sts) va_res = va_sts;
        m_FrameState = lvaBeforeBegin;
    }
    vm_mutex_unlock(&m_SyncMutex);
    if (UMC_OK == umcRes) umcRes = va_to_umc_res(va_res);
    return umcRes;
}

/* TODO: need to rewrite return value type (possible problems with signed/unsigned) */
Ipp32s LinuxVideoAccelerator::GetSurfaceID(Ipp32s idx)
{
    Ipp32s surface = VA_INVALID_SURFACE;

    if ((idx >= 0) && (idx < m_NumOfFrameBuffers)) surface = m_surfaces[idx];
    return surface;
}

Status LinuxVideoAccelerator::DisplayFrame(Ipp32s index, VideoData *pOutputVideoData)
{
    m_iIndex = index;
    return UMC_OK;
}

Ipp32s LinuxVideoAccelerator::GetIndex(void)
{
    return m_iIndex;
}

Status LinuxVideoAccelerator::QueryTaskStatus(Ipp32s FrameBufIndex, void * status, void * error)
{
    if ((FrameBufIndex < 0) || (FrameBufIndex >= m_NumOfFrameBuffers))
        return UMC_ERR_INVALID_PARAMS;

    VASurfaceStatus surface_status;
    VAStatus va_status = vaQuerySurfaceStatus(m_dpy, m_surfaces[FrameBufIndex], &surface_status);

    if ((VA_STATUS_SUCCESS == va_status) && (VASurfaceReady == surface_status))
    {
        // handle decoding errors
        VAStatus va_sts = vaSyncSurface(m_dpy, m_surfaces[FrameBufIndex]);
        if (VA_STATUS_ERROR_DECODING_ERROR == va_sts)
        {
            // TODO: to reduce number of checks we can cache all used render targets
            // during vaBeginPicture() call. For now check all render targets binded to context
            for(int cnt = 0; cnt < m_NumOfFrameBuffers; ++cnt)
            {
                VASurfaceDecodeMBErrors* pVaDecErr = NULL;
                va_sts = vaQuerySurfaceError(m_dpy, m_surfaces[cnt], VA_STATUS_ERROR_DECODING_ERROR, (void**)&pVaDecErr);

                if ((VA_STATUS_SUCCESS == va_sts) && (NULL != error))
                {
                    if (NULL != pVaDecErr)
                    {
                        for (int i = 0; pVaDecErr[i].status != -1; ++i)
                        {
                            if (VADecodeMBError == pVaDecErr[i].decode_error_type)
                            {
                                *(VAStatus*)error = VA_STATUS_ERROR_DECODING_ERROR;
                            }
                        }
                    }
                    else
                    {
                        *(VAStatus*)error = VA_STATUS_ERROR_DECODING_ERROR;
                    }
                }
            }
        }
    }

    if (NULL != status)
    {
        *(VASurfaceStatus*)status = surface_status; // done or not
    }

    Status umcRes = va_to_umc_res(va_status); // OK or not
    return umcRes;
}

Status LinuxVideoAccelerator::SyncTask(Ipp32s FrameBufIndex)
{
    Status umcRes = UMC_OK;
    VAStatus va_res = VA_STATUS_SUCCESS;

    if ((UMC_OK == umcRes) && ((FrameBufIndex < 0) || (FrameBufIndex >= m_NumOfFrameBuffers)))
        umcRes = UMC_ERR_INVALID_PARAMS;

    if (UMC_OK == umcRes)
    {
        va_res = vaSyncSurface(m_dpy, m_surfaces[FrameBufIndex]);
        if (VA_STATUS_ERROR_DECODING_ERROR == va_res)
            va_res = VA_STATUS_SUCCESS;

        umcRes = va_to_umc_res(va_res);
    }

    return umcRes;
}

}; // namespace UMC

#endif // UMC_VA_LINUX
