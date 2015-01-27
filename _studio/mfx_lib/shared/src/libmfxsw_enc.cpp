/* ****************************************************************************** *\

Copyright (C) 2008-2013 Intel Corporation.  All rights reserved.

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

File Name: libmfxsw_enc.cpp

\* ****************************************************************************** */
#include <mfxvideo.h>

#include "mfx_session.h"
#include "mfx_common.h"

// sheduling and threading stuff
#include <mfx_task.h>

#ifdef MFX_VA

#ifdef MFX_ENABLE_H264_VIDEO_ENC_HW
#include "mfx_h264_enc_hw.h"
#endif

#else //MFX_VA

#ifdef MFX_ENABLE_VC1_VIDEO_ENC
#include "mfx_vc1_enc_defs.h"
#include "mfx_vc1_enc_enc.h"
#endif

#ifdef MFX_ENABLE_MPEG2_VIDEO_ENC
#include "mfx_mpeg2_enc.h"
#endif

#ifdef MFX_ENABLE_H264_VIDEO_ENC
#include "mfx_h264_enc_enc.h"
#endif

#endif //MFX_VA

VideoENC *CreateENCSpecificClass(mfxU32 codecId, VideoCORE *pCore)
{
    VideoENC *pENC = (VideoENC *) 0;
    mfxStatus mfxRes = MFX_ERR_MEMORY_ALLOC;

    switch (codecId)
    {
#if defined (MFX_ENABLE_H264_VIDEO_ENC) && !defined (MFX_VA) || defined (MFX_ENABLE_H264_VIDEO_ENC_HW) && defined (MFX_VA)
    case MFX_CODEC_AVC:
#ifdef MFX_VA
        pENC = new MFXHWVideoENCH264(pCore, &mfxRes);
#else //MFX_VA
        pENC = new MFXVideoEncH264(pCore, &mfxRes);
#endif //MFX_VA
        break;
#endif // MFX_ENABLE_H264_VIDEO_ENC || MFX_ENABLE_H264_VIDEO_ENC_H

#ifdef MFX_ENABLE_VC1_VIDEO_ENC
    case MFX_CODEC_VC1:
        pENC = new MFXVideoEncVc1(pCore, &mfxRes);
        break;
#endif // MFX_ENABLE_VC1_VIDEO_ENC

#if defined (MFX_ENABLE_MPEG2_VIDEO_ENC) && !defined(MFX_VA)
    case MFX_CODEC_MPEG2:
        pENC = new MFXVideoENCMPEG2(pCore, &mfxRes);
        break;
#endif // MFX_ENABLE_MPEG2_VIDEO_ENC && !MFX_VA

    case 0: pCore;
    default:
        break;
    }

    // check error(s)
    if (MFX_ERR_NONE != mfxRes)
    {
        delete pENC;
        pENC = (VideoENC *) 0;
    }

    return pENC;

} // VideoENC *CreateENCSpecificClass(mfxU32 codecId, VideoCORE *pCore)

mfxStatus MFXVideoENC_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out)
{
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(out, MFX_ERR_NULL_PTR);

    mfxStatus mfxRes;
    try
    {
        switch (out->mfx.CodecId)
        {
#ifdef MFX_ENABLE_VC1_VIDEO_ENC
        case MFX_CODEC_VC1:
            mfxRes = MFXVideoEncVc1::Query(in, out);
            break;
#endif

#if defined (MFX_ENABLE_H264_VIDEO_ENC) && !defined (MFX_VA) || defined (MFX_ENABLE_H264_VIDEO_ENC_HW) && defined (MFX_VA)
        case MFX_CODEC_AVC:
#ifdef MFX_VA
            mfxRes = MFXHWVideoENCH264::Query(in, out);
#else //MFX_VA
            mfxRes = MFXVideoEncH264::Query(in, out);
#endif //MFX_VA
            break;
#endif // MFX_ENABLE_H264_VIDEO_ENC || MFX_ENABLE_H264_VIDEO_ENC_H

#if defined (MFX_ENABLE_MPEG2_VIDEO_ENC) && !defined (MFX_VA)
        case MFX_CODEC_MPEG2:
            mfxRes = MFXVideoENCMPEG2::Query(in, out);
            break;
#endif // MFX_ENABLE_MPEG2_VIDEO_ENC && !MFX_VA

        case 0: in;
        default:
            mfxRes = MFX_ERR_UNSUPPORTED;
        }
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        mfxRes = MFX_ERR_NULL_PTR;
    }
    return mfxRes;
}

mfxStatus MFXVideoENC_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request)
{
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(par, MFX_ERR_NULL_PTR);
    MFX_CHECK(request, MFX_ERR_NULL_PTR);

    mfxStatus mfxRes;
    try
    {
        switch (par->mfx.CodecId)
        {
#ifdef MFX_ENABLE_VC1_VIDEO_ENC
        case MFX_CODEC_VC1:
            mfxRes = MFXVideoEncVc1::QueryIOSurf(par, request);
            break;
#endif

#if defined (MFX_ENABLE_H264_VIDEO_ENC) && !defined (MFX_VA) || defined (MFX_ENABLE_H264_VIDEO_ENC_HW) && defined (MFX_VA)
        case MFX_CODEC_AVC:
#ifdef MFX_VA
            mfxRes = MFXHWVideoENCH264::QueryIOSurf(par, request);
#else //MFX_VA
            mfxRes = MFXVideoEncH264::QueryIOSurf(par, request);
#endif //MFX_VA
            break;
#endif // MFX_ENABLE_H264_VIDEO_ENC || MFX_ENABLE_H264_VIDEO_ENC_H

#if defined (MFX_ENABLE_MPEG2_VIDEO_ENC) && !defined (MFX_VA)
        case MFX_CODEC_MPEG2:
            mfxRes = MFXVideoENCMPEG2::QueryIOSurf(par, request);
            break;
#endif // MFX_ENABLE_MPEG2_VIDEO_ENC && !MFX_VA

        case 0:
        default:
            mfxRes = MFX_ERR_UNSUPPORTED;
        }
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        mfxRes = MFX_ERR_NULL_PTR;
    }
    return mfxRes;
}

mfxStatus MFXVideoENC_Init(mfxSession session, mfxVideoParam *par)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(par, MFX_ERR_NULL_PTR);
    try
    {
        // close the existing encoder,
        // if it is initialized.
        if (session->m_pENC.get())
        {
            MFXVideoENC_Close(session);
        }

        // create a new instance
        session->m_pENC.reset(CreateENCSpecificClass(par->mfx.CodecId, session->m_pCORE.get()));
        MFX_CHECK(session->m_pENC.get(), MFX_ERR_INVALID_VIDEO_PARAM);
        mfxRes = session->m_pENC->Init(par);
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
        else if (0 == session->m_pENC.get())
        {
            mfxRes = MFX_ERR_INVALID_VIDEO_PARAM;
        }
        else if (0 == par)
        {
            mfxRes = MFX_ERR_NULL_PTR;
        }
    }

    return mfxRes;

} // mfxStatus MFXVideoENC_Init(mfxSession session, mfxVideoParam *par)

mfxStatus MFXVideoENC_Close(mfxSession session)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);

    try
    {
        if (!session->m_pENC.get())
        {
            return MFX_ERR_NOT_INITIALIZED;
        }

        // wait until all tasks are processed
        session->m_pScheduler->WaitForTaskCompletion(session->m_pENC.get());

        mfxRes = session->m_pENC->Close();
        // delete the codec's instance
        session->m_pENC.reset((VideoENC *) 0);
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
    }

    return mfxRes;

} // mfxStatus MFXVideoENC_Close(mfxSession session)

static
mfxStatus MFXVideoENCLegacyRoutine(void *pState, void *pParam,
                                   mfxU32 threadNumber, mfxU32 callNumber)
{
    VideoENC *pENC = (VideoENC *) pState;
    VideoBRC *pBRC;
    MFX_THREAD_TASK_PARAMETERS *pTaskParam = (MFX_THREAD_TASK_PARAMETERS *) pParam;
    mfxStatus mfxRes;

    // touch unreferenced parameter(s)
    callNumber = callNumber;

    // check error(s)
    if ((NULL == pState) ||
        (NULL == pParam) ||
        (0 != threadNumber))
    {
        return MFX_ERR_NULL_PTR;
    }

    // get the BRC pointer
    pBRC = (VideoBRC *) pTaskParam->enc.pBRC;

    // call the obsolete method
    mfxRes = pBRC->FrameENCUpdate(pTaskParam->enc.cuc);
    if (MFX_ERR_NONE == mfxRes)
    {
        mfxRes = pENC->RunFrameVmeENC(pTaskParam->enc.cuc);
    }

    return mfxRes;

} // mfxStatus MFXVideoENCLegacyRoutine(void *pState, void *pParam,

mfxStatus MFXVideoENC_RunFrameVmeENCAsync(mfxSession session, mfxFrameCUC *cuc, mfxSyncPoint *syncp)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pENC.get(), MFX_ERR_NOT_INITIALIZED);
    MFX_CHECK(syncp, MFX_ERR_NULL_PTR);

    try
    {
        mfxSyncPoint syncPoint = NULL;
        MFX_TASK task;

        // unfortunately, we have to check error(s),
        // because several members are not used in the sync part
        if (NULL == session->m_pENC.get())
        {
            return MFX_ERR_NOT_INITIALIZED;
        }

        memset(&task, 0, sizeof(MFX_TASK));
        mfxRes = session->m_pENC->RunFrameVmeENCCheck(cuc, &task.entryPoint);
        // source data is OK, go forward
        if (MFX_ERR_NONE == mfxRes)
        {
            // prepare the absolete kind of task.
            // it is absolete and must be removed.
            if (NULL == task.entryPoint.pRoutine)
            {
                // BEGIN OF OBSOLETE PART
                task.bObsoleteTask = true;
                // fill task info
                task.entryPoint.pRoutine = &MFXVideoENCLegacyRoutine;
                task.entryPoint.pState = session->m_pENC.get();
                task.entryPoint.requiredNumThreads = 1;

                // fill legacy parameters
                task.obsolete_params.enc.cuc = cuc;
                task.obsolete_params.enc.pBRC = session->m_pBRC.get();

            } // END OF OBSOLETE PART

            task.pOwner = session->m_pENC.get();
            task.priority = session->m_priority;
            task.threadingPolicy = session->m_pENC->GetThreadingPolicy();
            // fill dependencies
            task.pSrc[0] = cuc;
            task.pDst[0] = cuc;

            // register input and call the task
            mfxRes = session->m_pScheduler->AddTask(task, &syncPoint);

        }

        // return pointer to synchronization point
        *syncp = syncPoint;
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
        else if (0 == session->m_pENC.get())
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
        else if (0 == syncp)
        {
            return MFX_ERR_NULL_PTR;
        }
    }

    return mfxRes;

} // mfxStatus MFXVideoENC_RunFrameVmeENCAsync(mfxSession session, mfxFrameCUC *cuc, mfxSyncPoint *syncp)

//
// THE OTHER ENC FUNCTIONS HAVE IMPLICIT IMPLEMENTATION
//

FUNCTION_RESET_IMPL(ENC, Reset, (mfxSession session, mfxVideoParam *par), (par))

FUNCTION_IMPL(ENC, GetVideoParam, (mfxSession session, mfxVideoParam *par), (par))
FUNCTION_IMPL(ENC, GetFrameParam, (mfxSession session, mfxFrameParam *par), (par))
