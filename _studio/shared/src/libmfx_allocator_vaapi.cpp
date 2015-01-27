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

File Name: libmfx_allocator_vaapi.cpp

\* ****************************************************************************** */

#include "mfx_common.h"

#if defined (MFX_VA_LINUX)

#include <algorithm>
#include <vector>

#include "ippcore.h"
#include "ipps.h"

#include "libmfx_allocator_vaapi.h"
#include "mfx_utils.h"

#define VA_SAFE_CALL(__CALL)        \
{                                   \
    VAStatus va_sts = __CALL;       \
    if (VA_STATUS_SUCCESS != va_sts) return MFX_ERR_INVALID_HANDLE; \
}

#define VA_TO_MFX_STATUS(_va_res) \
    (VA_STATUS_SUCCESS == (_va_res))? MFX_ERR_NONE: MFX_ERR_DEVICE_FAILED;

enum {
    MFX_FOURCC_VP8_NV12    = MFX_MAKEFOURCC('V','P','8','N'),
    MFX_FOURCC_VP8_MBDATA  = MFX_MAKEFOURCC('V','P','8','M'),
    MFX_FOURCC_VP8_SEGMAP  = MFX_MAKEFOURCC('V','P','8','S'),
};

unsigned int ConvertMfxFourccToVAFormat(mfxU32 fourcc)
{
    switch (fourcc)
    {
    case MFX_FOURCC_NV12:
        return VA_FOURCC_NV12;
    case MFX_FOURCC_YUY2:
        return VA_FOURCC_YUY2;
    case MFX_FOURCC_YV12:
        return VA_FOURCC_YV12;
    case MFX_FOURCC_RGB4:
        return VA_FOURCC_ARGB;
    case MFX_FOURCC_P8:
        return VA_FOURCC_P208;

    default:
        VM_ASSERT(!"unsupported fourcc");
        return 0;
    }
}

unsigned int ConvertVP8FourccToMfxFourcc(mfxU32 fourcc)
{
    switch (fourcc)
    {
    case MFX_FOURCC_VP8_NV12:
    case MFX_FOURCC_VP8_MBDATA:
        return MFX_FOURCC_NV12;
    case MFX_FOURCC_VP8_SEGMAP:
        return MFX_FOURCC_P8;

    default:
        return fourcc;
    }
}
mfxStatus
mfxDefaultAllocatorVAAPI::AllocFramesHW(
    mfxHDL                  pthis,
    mfxFrameAllocRequest*   request,
    mfxFrameAllocResponse*  response)
{
    mfxStatus mfx_res = MFX_ERR_NONE;
    VAStatus  va_res  = VA_STATUS_SUCCESS;
    unsigned int va_fourcc = 0;
    VASurfaceID* surfaces = NULL;
    VASurfaceAttrib attrib;
    vaapiMemIdInt *vaapi_mids = NULL, *vaapi_mid = NULL;
    mfxU32 fourcc = request->Info.FourCC;
    mfxU16 surfaces_num = request->NumFrameSuggested, numAllocated = 0, i = 0;
    bool bCreateSrfSucceeded = false;

    if (!pthis)
    {
        return MFX_ERR_INVALID_HANDLE;
    }

    memset(response, 0, sizeof(mfxFrameAllocResponse));

    // VP8 hybrid driver has weird requirements for allocation of surfaces/buffers for VP8 encoding
    // to comply with them additional logic is required to support regular and VP8 hybrid allocation pathes
    mfxU32 mfx_fourcc = ConvertVP8FourccToMfxFourcc(fourcc);
    va_fourcc = ConvertMfxFourccToVAFormat(mfx_fourcc);
    if (!va_fourcc || ((VA_FOURCC_NV12 != va_fourcc) &&
                       (VA_FOURCC_YV12 != va_fourcc) &&
                       (VA_FOURCC_YUY2 != va_fourcc) &&
                       (VA_FOURCC_ARGB != va_fourcc) &&
                       (VA_FOURCC_P208 != va_fourcc)))
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
    if (!surfaces_num)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    mfxWideHWFrameAllocator *pSelf = (mfxWideHWFrameAllocator*)pthis;

    // frames were allocated
    // return existent frames
    if (pSelf->NumFrames)
    {
        if (request->NumFrameSuggested > pSelf->NumFrames)
            return MFX_ERR_MEMORY_ALLOC;
        else
        {
            response->mids = &pSelf->m_frameHandles[0];
            return MFX_ERR_NONE;
        }
    }

    // allocate frames in cycle
    if (MFX_ERR_NONE == mfx_res)
    {
        surfaces = (VASurfaceID*)calloc(surfaces_num, sizeof(VASurfaceID));
        vaapi_mids = (vaapiMemIdInt*)calloc(surfaces_num, sizeof(vaapiMemIdInt));
        if ((NULL == surfaces) || (NULL == vaapi_mids))
        {
            mfx_res = MFX_ERR_MEMORY_ALLOC;
        }
    }
    if (MFX_ERR_NONE == mfx_res)
    {
        if( VA_FOURCC_P208 != va_fourcc)
        {
            unsigned int format;
            VASurfaceAttrib *pAttrib = &attrib;

            attrib.type          = VASurfaceAttribPixelFormat;
            attrib.flags         = VA_SURFACE_ATTRIB_SETTABLE;
            attrib.value.type    = VAGenericValueTypeInteger;
            attrib.value.value.i = va_fourcc;
            format               = va_fourcc;

            if (fourcc == MFX_FOURCC_VP8_NV12)
            {
                // special configuration for NV12 surf allocation for VP8 hybrid encoder is required
                attrib.type          = (VASurfaceAttribType)VASurfaceAttribUsageHint;
                attrib.value.value.i = VA_SURFACE_ATTRIB_USAGE_HINT_ENCODER;
            }
            else if (fourcc == MFX_FOURCC_VP8_MBDATA)
            {
                // special configuration for MB data surf allocation for VP8 hybrid encoder is required
                attrib.value.value.i = VA_FOURCC_P208;
                format               = VA_FOURCC_P208;
            }
            else if (va_fourcc == VA_FOURCC_NV12)
            {
                format = VA_RT_FORMAT_YUV420;
            }

            va_res = vaCreateSurfaces(pSelf->pVADisplay,
                                    format,
                                    request->Info.Width, request->Info.Height,
                                    surfaces,
                                    surfaces_num,
                                    pAttrib, pAttrib ? 1 : 0);
            mfx_res = VA_TO_MFX_STATUS(va_res);
            bCreateSrfSucceeded = (MFX_ERR_NONE == mfx_res);
        }
        else
        {
            VAContextID context_id = request->reserved[0];
            int codedbuf_size;
            VABufferType codedbuf_type;
            if (fourcc == MFX_FOURCC_VP8_SEGMAP)
            {
                codedbuf_size = request->Info.Width * request->Info.Height;
                codedbuf_type = (VABufferType)VAEncMacroblockMapBufferType;
            }
            else
            {
#if defined(ANDROID)
                codedbuf_size = static_cast<int>((request->Info.Width * request->Info.Height) * 400 / (16 * 16)); //from libva spec
#else
                int width32 = 32 * ((request->Info.Width + 31) >> 5);
                int height32 = 32 * ((request->Info.Height + 31) >> 5);
                codedbuf_size = static_cast<int>((width32 * height32) * 400LL / (16 * 16)); //from libva spec
                //codedbuf_size = 0x1000 * ((codedbuf_size + 0xfff) >> 12); // align to page size
#endif
                codedbuf_type = VAEncCodedBufferType;
            }

            for (numAllocated = 0; numAllocated < surfaces_num; numAllocated++)
            {
                VABufferID coded_buf;

                va_res = vaCreateBuffer(pSelf->pVADisplay,
                                      context_id,
                                      codedbuf_type,
                                      codedbuf_size,
                                      1,
                                      NULL,
                                      &coded_buf);
                mfx_res = VA_TO_MFX_STATUS(va_res);
                if (MFX_ERR_NONE != mfx_res) break;
                surfaces[numAllocated] = coded_buf;
            }
        }
        
    }
    if (MFX_ERR_NONE == mfx_res)
    {
        for (i = 0; i < surfaces_num; ++i)
        {
            vaapi_mid = &(vaapi_mids[i]);
            vaapi_mid->m_fourcc = fourcc;
            vaapi_mid->m_surface = &(surfaces[i]);

            pSelf->m_frameHandles.push_back(vaapi_mid);
        }
        response->mids = &pSelf->m_frameHandles[0];
        response->NumFrameActual = surfaces_num;
        pSelf->NumFrames = surfaces_num;
    }
    else // i.e. MFX_ERR_NONE != mfx_res
    {
        response->mids = NULL;
        response->NumFrameActual = 0;
        if (VA_FOURCC_P208 != va_fourcc
            || fourcc == MFX_FOURCC_VP8_MBDATA )
        {
            if (bCreateSrfSucceeded) vaDestroySurfaces(pSelf->pVADisplay, surfaces, surfaces_num);
        }
        else
        {
            for (i = 0; i < numAllocated; i++)
                vaDestroyBuffer(pSelf->pVADisplay, surfaces[i]);
        }
        if (vaapi_mids) { free(vaapi_mids); vaapi_mids = NULL; }
        if (surfaces) { free(surfaces); surfaces = NULL; }
    }
    return mfx_res;
}

mfxStatus mfxDefaultAllocatorVAAPI::FreeFramesHW(
    mfxHDL                  pthis,
    mfxFrameAllocResponse*  response)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    mfxWideHWFrameAllocator *pSelf = (mfxWideHWFrameAllocator*)pthis;

    vaapiMemIdInt *vaapi_mids = NULL;
    VASurfaceID* surfaces = NULL;
    mfxU32 i = 0;
    bool isBufferMemory=false;

    if (!response) return MFX_ERR_NULL_PTR;

    if (response->mids)
    {
        vaapi_mids = (vaapiMemIdInt*)(response->mids[0]);
        mfxU32 mfx_fourcc = ConvertVP8FourccToMfxFourcc(vaapi_mids->m_fourcc);
        isBufferMemory = (MFX_FOURCC_P8 == mfx_fourcc)?true:false;
        surfaces = vaapi_mids->m_surface;
        for (i = 0; i < response->NumFrameActual; ++i)
        {
            if (MFX_FOURCC_P8 == vaapi_mids[i].m_fourcc)
            {
                vaDestroyBuffer(pSelf->pVADisplay, surfaces[i]);
            }
        }
        free(vaapi_mids);
        response->mids = NULL;

        if (!isBufferMemory) vaDestroySurfaces(pSelf->pVADisplay, surfaces, response->NumFrameActual);
        free(surfaces);
    }
    response->NumFrameActual = 0;

    return MFX_ERR_NONE;
}

mfxStatus
mfxDefaultAllocatorVAAPI::LockFrameHW(
    mfxHDL         pthis,
    mfxMemId       mid,
    mfxFrameData*  ptr)
{
    MFX_CHECK(pthis, MFX_ERR_INVALID_HANDLE);
    mfxWideHWFrameAllocator *pSelf = (mfxWideHWFrameAllocator*)pthis;

    mfxStatus mfx_res = MFX_ERR_NONE;
    VAStatus  va_res  = VA_STATUS_SUCCESS;
    vaapiMemIdInt* vaapi_mids = (vaapiMemIdInt*)mid;
    mfxU8* pBuffer = 0;

    if (!vaapi_mids || !(vaapi_mids->m_surface)) return MFX_ERR_INVALID_HANDLE;

    mfxU32 mfx_fourcc = ConvertVP8FourccToMfxFourcc(vaapi_mids->m_fourcc);

    if (MFX_FOURCC_P8 == mfx_fourcc)   // bitstream processing
    {
        VACodedBufferSegment *coded_buffer_segment;
        if (vaapi_mids->m_fourcc == MFX_FOURCC_VP8_SEGMAP)
            va_res =  vaMapBuffer(pSelf->pVADisplay, *(vaapi_mids->m_surface), (void **)(&pBuffer));
        else
            va_res =  vaMapBuffer(pSelf->pVADisplay, *(vaapi_mids->m_surface), (void **)(&coded_buffer_segment));
        mfx_res = VA_TO_MFX_STATUS(va_res);
        if (MFX_ERR_NONE == mfx_res)
        {
            if (vaapi_mids->m_fourcc == MFX_FOURCC_VP8_SEGMAP)
                ptr->Y = pBuffer;
            else
                ptr->Y = (mfxU8*)coded_buffer_segment->buf;
            
        }
    }
    else
    {
        va_res = vaDeriveImage(pSelf->pVADisplay, *(vaapi_mids->m_surface), &(vaapi_mids->m_image));
        mfx_res = VA_TO_MFX_STATUS(va_res);

        if (MFX_ERR_NONE == mfx_res)
        {
            va_res = vaMapBuffer(pSelf->pVADisplay, vaapi_mids->m_image.buf, (void **) &pBuffer);
            mfx_res = VA_TO_MFX_STATUS(va_res);
        }
        if (MFX_ERR_NONE == mfx_res)
        {
            switch (vaapi_mids->m_image.format.fourcc)
            {
            case VA_FOURCC_NV12:
                if (mfx_fourcc == MFX_FOURCC_NV12)
                {
                    ptr->Pitch = (mfxU16)vaapi_mids->m_image.pitches[0];
                    ptr->Y = pBuffer + vaapi_mids->m_image.offsets[0];
                    ptr->U = pBuffer + vaapi_mids->m_image.offsets[1];
                    ptr->V = ptr->U + 1;
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            case VA_FOURCC_YV12:
                if (mfx_fourcc == MFX_FOURCC_YV12)
                {
                    ptr->Pitch = (mfxU16)vaapi_mids->m_image.pitches[0];
                    ptr->Y = pBuffer + vaapi_mids->m_image.offsets[0];
                    ptr->V = pBuffer + vaapi_mids->m_image.offsets[1];
                    ptr->U = pBuffer + vaapi_mids->m_image.offsets[2];
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            case VA_FOURCC_YUY2:
                if (mfx_fourcc == MFX_FOURCC_YUY2)
                {
                    ptr->Pitch = (mfxU16)vaapi_mids->m_image.pitches[0];
                    ptr->Y = pBuffer + vaapi_mids->m_image.offsets[0];
                    ptr->U = ptr->Y + 1;
                    ptr->V = ptr->Y + 3;
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            case VA_FOURCC_ARGB:
                if (mfx_fourcc == MFX_FOURCC_RGB4)
                {
                    ptr->Pitch = (mfxU16)vaapi_mids->m_image.pitches[0];
                    ptr->B = pBuffer + vaapi_mids->m_image.offsets[0];
                    ptr->G = ptr->B + 1;
                    ptr->R = ptr->B + 2;
                    ptr->A = ptr->B + 3;
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
        case VA_FOURCC_P208:
                if (mfx_fourcc == MFX_FOURCC_NV12)
                {
                    ptr->Pitch = (mfxU16)vaapi_mids->m_image.pitches[0];
                    ptr->Y = pBuffer + vaapi_mids->m_image.offsets[0];
                }
                else mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            default:
                mfx_res = MFX_ERR_LOCK_MEMORY;
                break;
            }
        }
    }
    return mfx_res;
}

mfxStatus mfxDefaultAllocatorVAAPI::UnlockFrameHW(
    mfxHDL         pthis,
    mfxMemId       mid,
    mfxFrameData*  ptr)
{
    // TBD
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    mfxWideHWFrameAllocator *pSelf = (mfxWideHWFrameAllocator*)pthis;

    vaapiMemIdInt* vaapi_mids = (vaapiMemIdInt*)mid;

    if (!vaapi_mids || !(vaapi_mids->m_surface)) return MFX_ERR_INVALID_HANDLE;

    mfxU32 mfx_fourcc = ConvertVP8FourccToMfxFourcc(vaapi_mids->m_fourcc);

    if (MFX_FOURCC_P8 == mfx_fourcc)   // bitstream processing
    {
        vaUnmapBuffer(pSelf->pVADisplay, *(vaapi_mids->m_surface));
    }
    else  // Image processing
    {
        vaUnmapBuffer(pSelf->pVADisplay, vaapi_mids->m_image.buf);
        vaDestroyImage(pSelf->pVADisplay, vaapi_mids->m_image.image_id);

        if (NULL != ptr)
        {
            ptr->Pitch = 0;
            ptr->Y     = NULL;
            ptr->U     = NULL;
            ptr->V     = NULL;
            ptr->A     = NULL;
        }
    }
    return MFX_ERR_NONE;
}

mfxStatus
mfxDefaultAllocatorVAAPI::GetHDLHW(
    mfxHDL    pthis,
    mfxMemId  mid,
    mfxHDL*   handle)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    //mfxWideHWFrameAllocator *pSelf = (mfxWideHWFrameAllocator*)pthis;
    if (0 == mid)
        return MFX_ERR_INVALID_HANDLE;

    vaapiMemIdInt* vaapi_mids = (vaapiMemIdInt*)mid;

    if (!handle || !vaapi_mids || !(vaapi_mids->m_surface)) return MFX_ERR_INVALID_HANDLE;

    *handle = vaapi_mids->m_surface; //VASurfaceID* <-> mfxHDL
    return MFX_ERR_NONE;
}

mfxDefaultAllocatorVAAPI::mfxWideHWFrameAllocator::mfxWideHWFrameAllocator(
    mfxU16  type,
    mfxHDL  handle)
    :
    mfxBaseWideFrameAllocator(type)
{
    frameAllocator.Alloc = &mfxDefaultAllocatorVAAPI::AllocFramesHW;
    frameAllocator.Lock = &mfxDefaultAllocatorVAAPI::LockFrameHW;
    frameAllocator.GetHDL = &mfxDefaultAllocatorVAAPI::GetHDLHW;
    frameAllocator.Unlock = &mfxDefaultAllocatorVAAPI::UnlockFrameHW;
    frameAllocator.Free = &mfxDefaultAllocatorVAAPI::FreeFramesHW;
    pVADisplay = (VADisplay *)handle;
}
#endif // (MFX_VA_LINUX)
/* EOF */
