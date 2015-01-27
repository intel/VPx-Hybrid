/* ****************************************************************************** *\

Copyright (C) 2009-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_umc_alloc_wrapper.h

\* ****************************************************************************** */
#ifndef _MFX_VC1_ALLOCATOR_H_
#define _MFX_VC1_ALLOCATOR_H_

#include <vector>

#include "mfx_common.h"
#include "umc_default_memory_allocator.h"
#include "umc_default_frame_allocator.h"
#include "umc_frame_data.h"

#include "mfxvideo++int.h"

#define MFX_UMC_MAX_ALLOC_SIZE 128

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mfx_UMC_MemAllocator - buffer allocator
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class mfx_UMC_MemAllocator : public UMC::DefaultMemoryAllocator
{
    DYNAMIC_CAST_DECL(mfx_UMC_MemAllocator, DefaultMemoryAllocator)

public:
    mfx_UMC_MemAllocator(void);
    virtual ~mfx_UMC_MemAllocator(void);

    // Initiates object
    virtual UMC::Status InitMem(UMC::MemoryAllocatorParams *pParams, VideoCORE* mfxCore);

    // Closes object and releases all allocated memory
    virtual UMC::Status Close();

    // Allocates or reserves physical memory and return unique ID
    // Sets lock counter to 0
    virtual UMC::Status Alloc(UMC::MemID *pNewMemID, size_t Size, Ipp32u Flags, Ipp32u Align = 16);

    // Lock() provides pointer from ID. If data is not in memory (swapped)
    // prepares (restores) it. Increases lock counter
    virtual void *Lock(UMC::MemID MID);

    // Unlock() decreases lock counter
    virtual UMC::Status Unlock(UMC::MemID MID);

    // Notifies that the data wont be used anymore. Memory can be free
    virtual UMC::Status Free(UMC::MemID MID);

    // Immediately deallocates memory regardless of whether it is in use (locked) or no
    virtual UMC::Status DeallocateMem(UMC::MemID MID);

protected:
    VideoCORE* m_pCore;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mfx_UMC_FrameAllocator
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class mfx_UMC_FrameAllocator : public UMC::FrameAllocator
{
    DYNAMIC_CAST_DECL(mfx_UMC_FrameAllocator, UMC::FrameAllocator)

public:
    mfx_UMC_FrameAllocator(void);
    virtual ~mfx_UMC_FrameAllocator(void);

    // Initiates object
    virtual UMC::Status InitMfx(UMC::FrameAllocatorParams *pParams, 
                                VideoCORE* mfxCore, 
                                const mfxVideoParam *params, 
                                const mfxFrameAllocRequest *request, 
                                mfxFrameAllocResponse *response, 
                                bool isUseExternalFrames,
                                bool isSWplatform);

    // Closes object and releases all allocated memory
    virtual UMC::Status Close();

    // Allocates or reserves physical memory and returns unique ID
    // Sets lock counter to 0
    virtual UMC::Status Alloc(UMC::FrameMemID *pNewMemID, const UMC::VideoDataInfo * info, Ipp32u flags);

    // Lock() provides pointer from ID. If data is not in memory (swapped)
    // prepares (restores) it. Increases lock counter
    virtual const UMC::FrameData* Lock(UMC::FrameMemID mid);

    // Unlock() decreases lock counter
    virtual UMC::Status Unlock(UMC::FrameMemID mid);

    // Notifies that the data won't be used anymore. Memory can be freed.
    virtual UMC::Status IncreaseReference(UMC::FrameMemID mid);

    // Notifies that the data won't be used anymore. Memory can be freed.
    virtual UMC::Status DecreaseReference(UMC::FrameMemID mid);

    virtual UMC::Status Reset();

    virtual mfxStatus SetCurrentMFXSurface(mfxFrameSurface1 *srf, bool isOpaq);

    virtual mfxFrameSurface1*  GetSurface(UMC::FrameMemID index, mfxFrameSurface1 *surface_work, const mfxVideoParam * videoPar);
    virtual mfxStatus          PrepareToOutput(mfxFrameSurface1 *surface_work, UMC::FrameMemID index, const mfxVideoParam * videoPar, bool isOpaq);
    mfxI32 FindSurface(mfxFrameSurface1 *surf, bool isOpaq);
    mfxI32 FindFreeSurface();

    void SetExternalFramesResponse(mfxFrameAllocResponse *response);
    mfxFrameSurface1 * GetInternalSurface(UMC::FrameMemID index);

    mfxMemId  ConvertMemId(UMC::FrameMemID index) {return m_frameData[index].first.Data.MemId;};

protected:
    struct  surf_descr
    {
        surf_descr(mfxFrameSurface1* FrameSurface, bool isUsed):FrameSurface(FrameSurface),
                                                                isUsed(isUsed)
        {
        };
        surf_descr():FrameSurface(0),
                     isUsed(false)
        {
        };
        mfxFrameSurface1* FrameSurface;
        bool              isUsed;
    };

    virtual UMC::Status Free(UMC::FrameMemID mid);

    virtual mfxI32 AddSurface(mfxFrameSurface1 *surface);

    class FrameInformation
    {
    public:
        FrameInformation();
        void Reset();

        mfxU32 m_locks;
        mfxU32 m_referenceCounter;
        UMC::FrameData  m_frame;
    };

    typedef std::pair<mfxFrameSurface1, FrameInformation> FrameInfo;

    std::vector<FrameInfo>  m_frameData;

    std::vector<surf_descr> m_extSurfaces;

    mfxI32        m_curIndex;

    bool m_IsUseExternalFrames;

    mfxFrameSurface1 m_surface;  // for copying

    UMC::VideoDataInfo m_info;

    VideoCORE* m_pCore;

    mfxFrameAllocResponse *m_externalFramesResponse;

    bool       m_isSWDecode;
};

#if !defined( AS_HEVCD_PLUGIN ) || defined (AS_VP8D_PLUGIN) // HEVC decode natively supportes NV12 format - no need to make conversion 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mfx_UMC_FrameAllocator_NV12
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class mfx_UMC_FrameAllocator_NV12 : public mfx_UMC_FrameAllocator
{
public:

    virtual UMC::Status InitMfx(UMC::FrameAllocatorParams *pParams, 
                                VideoCORE* mfxCore, 
                                const mfxVideoParam *params, 
                                const mfxFrameAllocRequest *request, 
                                mfxFrameAllocResponse *response, 
                                bool isUseExternalFrames,
                                bool isSWplatform);

    virtual const UMC::FrameData* Lock(UMC::FrameMemID mid);

    virtual mfxFrameSurface1 * GetSurface(UMC::FrameMemID index, mfxFrameSurface1 *surface_work, const mfxVideoParam * videoPar);
    virtual mfxStatus PrepareToOutput(mfxFrameSurface1 *surface_work, UMC::FrameMemID index, const mfxVideoParam * videoPar, bool isOpaq);
    virtual UMC::Status Reset();

    virtual UMC::Status Close();

protected:
    mfxStatus ConvertToNV12(const UMC::FrameData * fd, mfxFrameData *data, const mfxVideoParam * videoPar);
    virtual mfxI32 AddSurface(mfxFrameSurface1 *surface);

    std::vector<UMC::FrameData>   m_yv12Frames;
    UMC::DefaultFrameAllocator    m_defaultUMCAllocator;
    UMC::VideoDataInfo  m_info_yuv420;
};
#endif

#if defined (MFX_VA)
class mfx_UMC_FrameAllocator_D3D : public mfx_UMC_FrameAllocator
{
public:
    virtual mfxStatus PrepareToOutput(mfxFrameSurface1 *surface_work, UMC::FrameMemID index, const mfxVideoParam * videoPar, bool isOpaq);
};

#if defined (MFX_ENABLE_MJPEG_VIDEO_DECODE)
class VideoVppJpegD3D9;

class mfx_UMC_FrameAllocator_D3D_Converter : public mfx_UMC_FrameAllocator_D3D
{
public:
    virtual UMC::Status InitMfx(UMC::FrameAllocatorParams *pParams, 
                                VideoCORE* mfxCore, 
                                const mfxVideoParam *params, 
                                const mfxFrameAllocRequest *request, 
                                mfxFrameAllocResponse *response, 
                                bool isUseExternalFrames,
                                bool isSWplatform);

    typedef struct
    {
        Ipp32s colorFormat;
        size_t UOffset;
        size_t VOffset;
    } JPEG_Info;

    void SetJPEGInfo(JPEG_Info * jpegInfo);

    mfxStatus StartPreparingToOutput(mfxFrameSurface1 *surface_work, UMC::FrameData* in, const mfxVideoParam * par, VideoVppJpegD3D9 **pCc, mfxU16 *taskId, bool isOpaq);
    mfxStatus CheckPreparingToOutput(mfxFrameSurface1 *surface_work, UMC::FrameData* in, const mfxVideoParam * par, VideoVppJpegD3D9 **pCc, mfxU16 taskId);

protected:

    mfxStatus ConvertToNV12(UMC::FrameMemID index, mfxFrameSurface1 *dst);
    JPEG_Info  m_jpegInfo;
};

#endif // #if defined (MFX_ENABLE_MJPEG_VIDEO_DECODE)
#endif // #if defined (MFX_VA)

#endif //_MFX_MEMORY_ALLOCATOR_H_
