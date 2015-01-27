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

File Name: umc_va_linux.h

\* ****************************************************************************** */

#ifndef __UMC_VA_LINUX_H__
#define __UMC_VA_LINUX_H__

#include "umc_va_base.h"

#ifdef UMC_VA_LINUX

#include "umc_mutex.h"
#include "umc_event.h"

namespace UMC
{

#define UMC_VA_LINUX_INDEX_UNDEF -1

/* VACompBuffer --------------------------------------------------------------*/

class VACompBuffer : public UMCVACompBuffer
{
public:
    // constructor
    VACompBuffer(void);
    // destructor
    virtual ~VACompBuffer(void);

    // UMCVACompBuffer methods
    virtual void SetNumOfItem(Ipp32s num) { m_NumOfItem = num; };

    // VACompBuffer methods
    virtual Status SetBufferInfo   (Ipp32s _type, Ipp32s _id, Ipp32s _index = -1);
    virtual Status SetDestroyStatus(bool _destroy);

    virtual Ipp32s GetIndex(void)    { return m_index; }
    virtual Ipp32s GetID(void)       { return m_id; }
    virtual Ipp32s GetNumOfItem(void){ return m_NumOfItem; }
    virtual bool   NeedDestroy(void) { return m_bDestroy; }

protected:
    Ipp32s m_NumOfItem; //number of items in buffer
    Ipp32s m_index;
    Ipp32s m_id;
    bool   m_bDestroy;
    // debug bufferization
    Ipp8u  m_CompareByte;
};

/* LinuxVideoAcceleratorParams -----------------------------------------------*/

class LinuxVideoAcceleratorParams : public VideoAcceleratorParams
{
    DYNAMIC_CAST_DECL(LinuxVideoAcceleratorParams, VideoAcceleratorParams);
public:
    LinuxVideoAcceleratorParams(void)
    {
        m_Display             = NULL;
        m_bComputeVAFncsInfo  = false;
    }
    VADisplay m_Display;
    bool      m_bComputeVAFncsInfo;
};

/* LinuxVideoAccelerator -----------------------------------------------------*/

enum lvaFrameState
{
    lvaBeforeBegin = 0,
    lvaBeforeEnd   = 1,
    lvaNeedUnmap   = 2
};

class LinuxVideoAccelerator : public VideoAccelerator
{
    DYNAMIC_CAST_DECL(LinuxVideoAccelerator, VideoAccelerator);
public:
    // constructor
    LinuxVideoAccelerator (void);
    // destructor
    virtual ~LinuxVideoAccelerator(void);

    // VideoAccelerator methods
    virtual Status Init         (VideoAcceleratorParams* pInfo);
    virtual Status Close        (void);
    virtual Status BeginFrame   (Ipp32s FrameBufIndex);
    // gets buffer from cache if it exists or HW otherwise, buffers will be released in EndFrame
    virtual void* GetCompBuffer(Ipp32s buffer_type, UMCVACompBuffer **buf, Ipp32s size, Ipp32s index);
    virtual Status Execute      (void);
    virtual Status EndFrame     (void*);
    virtual Status DisplayFrame (Ipp32s index, VideoData *pOutputVideoData);
    virtual Ipp32s GetSurfaceID (Ipp32s idx);

    // NOT implemented functions:
    virtual Status ReleaseBuffer(Ipp32s /*type*/)
    { return UMC_OK; };

    // LinuxVideoAccelerator methods
    virtual Ipp32s GetIndex (void);

    // Following functions are absent in menlow!!!!!!!!!!!!!!!!!!!!!!
    virtual Status FindConfiguration(UMC::VideoStreamInfo* /*x*/) { return UMC_ERR_UNSUPPORTED;}
    virtual Status ExecuteExtensionBuffer(void* /*x*/) { return UMC_ERR_UNSUPPORTED;}
    virtual Status ExecuteStatusReportBuffer(void* /*x*/, Ipp32s /*y*/)  { return UMC_ERR_UNSUPPORTED;}
    virtual Status SyncTask(Ipp32s index);
    virtual Status QueryTaskStatus(Ipp32s index, void * status, void * error);
    virtual bool IsIntelCustomGUID() const { return false;}
    virtual GUID GetDecoderGuid(){return m_guidDecoder;};
    virtual void GetVideoDecoder(void** /*handle*/) {};

protected:
    // VideoAcceleratorExt methods
    virtual Status AllocCompBuffers(void);
    virtual VACompBuffer* GetCompBufferHW(Ipp32s type, Ipp32s size, Ipp32s index = -1);

protected:
    VADisplay     m_dpy;
    VAConfigID    m_config_id;
    VASurfaceID*  m_surfaces;
    VAContextID   m_context;
    Ipp32s        m_iIndex;
    lvaFrameState m_FrameState;

    Ipp32s   m_NumOfFrameBuffers;
    Ipp32u   m_uiCompBuffersNum;
    Ipp32u   m_uiCompBuffersUsed;
    vm_mutex m_SyncMutex;
    VACompBuffer** m_pCompBuffers;

    // introduced for MediaSDK
    bool    m_bIsExtSurfaces;

    GUID m_guidDecoder;
};

}; // namespace UMC

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    extern UMC::Status va_to_umc_res(VAStatus va_res);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // #ifdef UMC_VA_LINUX

#endif // #ifndef __UMC_VA_LINUX_H__
