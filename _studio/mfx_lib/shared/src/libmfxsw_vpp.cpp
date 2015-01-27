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

File Name: libmfxsw_vpp.cpp

\* ****************************************************************************** */

#include <mfxvideo.h>

#include <mfx_session.h>
#include <mfx_tools.h>
#include <mfx_common.h>
#include <mfx_user_plugin.h>

// sheduling and threading stuff
#include <mfx_task.h>

#ifdef MFX_ENABLE_VPP
// VPP include files here
#ifdef MFX_VA
#include "mfx_vpp_main.h"       // this VideoVPP class builds VPP pipeline and run the VPP pipeline
#else
#include "mfx_vpp_main.h"       // this VideoVPP class builds VPP pipeline and run the VPP pipeline
#include "umc_va_base.h"
#endif
#endif

#if !defined (MFX_RT)
VideoVPP *CreateVPPSpecificClass(mfxU32 reserved, VideoCORE *core)
{
    VideoVPP *pVPP = (VideoVPP *) 0;
    mfxStatus mfxRes = MFX_ERR_MEMORY_ALLOC;

    // touch unreferenced parameter
    reserved = reserved;

#ifdef MFX_ENABLE_VPP
    pVPP = new VideoVPPMain(core, &mfxRes);
    if (MFX_ERR_NONE != mfxRes)
    {
        delete pVPP;
        pVPP = (VideoVPP *) 0;
    }
#endif // MFX_ENABLE_VPP

    return pVPP;

} // VideoVPP *CreateVPPSpecificClass(mfxU32 reserved)
#endif

mfxStatus MFXVideoVPP_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out)
{
    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, in);

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(out, MFX_ERR_NULL_PTR);

    if ((0 != in) && (MFX_HW_VAAPI == session->m_pCORE->GetVAType()))
    {
        // protected content not supported on Linux
        MFX_CHECK(0 == in->Protected, MFX_ERR_UNSUPPORTED);
    }

    mfxStatus mfxRes = MFX_ERR_UNSUPPORTED;
    try
    {
#ifdef MFX_ENABLE_USER_VPP
        if (session->m_plgVPP.get())
        {
            mfxRes = session->m_plgVPP->Query(session->m_pCORE.get(), in, out);
        }
        else
        {
#endif

#if !defined (MFX_RT)
#ifdef MFX_ENABLE_VPP
            mfxRes = VideoVPPMain::Query(session->m_pCORE.get(), in, out);
#endif // MFX_ENABLE_VPP
#endif

#ifdef MFX_ENABLE_USER_VPP
        }
#endif
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        mfxRes = MFX_ERR_NULL_PTR;
    }

    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, out);
    MFX_LTRACE_I(MFX_TRACE_LEVEL_PARAMS, mfxRes);
    return mfxRes;
}

mfxStatus MFXVideoVPP_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request)
{
    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, par);

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(par, MFX_ERR_NULL_PTR);
    MFX_CHECK(request, MFX_ERR_NULL_PTR);

    mfxStatus mfxRes = MFX_ERR_UNSUPPORTED;
    try
    {
#ifdef MFX_ENABLE_USER_VPP
        if (session->m_plgVPP.get())
        {
            mfxRes = session->m_plgVPP->QueryIOSurf(session->m_pCORE.get(), par, &(request[0]),  &(request[1]) );
        }
        else
        {
#endif

#if !defined (MFX_RT)
#ifdef MFX_ENABLE_VPP
            mfxRes = VideoVPPMain::QueryIOSurf(session->m_pCORE.get(), par, request/*, session->m_adapterNum*/);
#endif // MFX_ENABLE_VPP
#endif 

#ifdef MFX_ENABLE_USER_VPP
        }
#endif
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        mfxRes = MFX_ERR_NULL_PTR;
    }

    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, request);
    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;
}

mfxStatus MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par)
{
    mfxStatus mfxRes = MFX_ERR_UNSUPPORTED;
    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, par);

    try
    {
#ifdef MFX_ENABLE_USER_VPP
        if (session->m_plgVPP.get())
        {
            mfxRes = session->m_plgVPP->Init(par);
        }
        else
        {
#endif

#if !defined (MFX_RT)
#ifdef MFX_ENABLE_VPP
            // close the existing video processor,
            // if it is initialized.
            if (session->m_pVPP.get())
            {
                MFXVideoVPP_Close(session);
            }


            // create a new instance
            session->m_pVPP.reset(CreateVPPSpecificClass(0 ,session->m_pCORE.get()));
            MFX_CHECK(session->m_pVPP.get(), MFX_ERR_INVALID_VIDEO_PARAM);
            mfxRes = session->m_pVPP->Init(par);
#endif // MFX_ENABLE_VPP
#endif 
#ifdef MFX_ENABLE_USER_VPP
        }
#endif
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
        else if (0 == session->m_pVPP.get())
        {
            mfxRes = MFX_ERR_INVALID_VIDEO_PARAM;
        }
        else if (0 == par)
        {
            mfxRes = MFX_ERR_NULL_PTR;
        }
    }

    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;

} // mfxStatus MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par)

mfxStatus MFXVideoVPP_Close(mfxSession session)
{
    mfxStatus mfxRes;

    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);

    try
    {
        if (!session->m_pVPP.get())
        {
            return MFX_ERR_NOT_INITIALIZED;
        }

        // wait until all tasks are processed
        session->m_pScheduler->WaitForTaskCompletion(session->m_pVPP.get());

        mfxRes = session->m_pVPP->Close();
        // delete the codec's instance if not plugin
        if (!session->m_plgVPP.get())
        {
            session->m_pVPP.reset((VideoVPP *) 0);
        }
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

    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;

} // mfxStatus MFXVideoVPP_Close(mfxSession session)

static
mfxStatus MFXVideoVPPLegacyRoutine(void *pState, void *pParam,
                                   mfxU32 threadNumber, mfxU32 callNumber)
{
    MFX_AUTO_LTRACE_WITHID(MFX_TRACE_LEVEL_SCHED, "VPP");
    VideoVPP *pVPP = (VideoVPP *) pState;
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

    // call the obsolete method
    mfxRes = pVPP->RunFrameVPP(pTaskParam->vpp.in,
                               pTaskParam->vpp.out,
                               pTaskParam->vpp.aux);

    return mfxRes;

} // mfxStatus MFXVideoVPPLegacyRoutine(void *pState, void *pParam,

enum
{
    MFX_NUM_ENTRY_POINTS = 2
};

mfxStatus MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp)
{
    mfxStatus mfxRes;

    MFX_AUTO_LTRACE_WITHID(MFX_TRACE_LEVEL_API, "MFX_RunFrameVPPAsync");
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, aux);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, in);

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pVPP.get(), MFX_ERR_NOT_INITIALIZED);
    MFX_CHECK(syncp, MFX_ERR_NULL_PTR);

    try
    {
#ifdef MFX_ENABLE_USER_VPP
      if (session->m_plgVPP.get())
      {
          MFX_TASK task;
          mfxSyncPoint syncPoint = NULL;
          *syncp = NULL;
          memset(&task, 0, sizeof(MFX_TASK));

          mfxRes = session->m_plgVPP->VPPFrameCheck(in, out, aux, &task.entryPoint);

          if (task.entryPoint.pRoutine)
          {
              mfxStatus mfxAddRes;
  
              task.pOwner = session->m_plgVPP.get();
              task.priority = session->m_priority;
              task.threadingPolicy = session->m_plgVPP->GetThreadingPolicy();
              // fill dependencies
              task.pSrc[0] = in;
              task.pDst[0] = out;
              if (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes)
                task.pDst[0] = NULL;
  
              #ifdef MFX_TRACE_ENABLE
              task.nParentId = MFX_AUTO_TRACE_GETID();
              task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_VPP;
              #endif
  
              // register input and call the task
              mfxAddRes = session->m_pScheduler->AddTask(task, &syncPoint);
              if (MFX_ERR_NONE != mfxAddRes)
              {
                  return mfxAddRes;
              }
              *syncp = syncPoint;
          }
      }
      else
      {
#endif
        mfxSyncPoint syncPoint = NULL;
        MFX_ENTRY_POINT entryPoints[MFX_NUM_ENTRY_POINTS];
        mfxU32 numEntryPoints = MFX_NUM_ENTRY_POINTS;

        memset(&entryPoints, 0, sizeof(entryPoints));
        mfxRes = session->m_pVPP->VppFrameCheck(in, out, aux, entryPoints, numEntryPoints);
        // source data is OK, go forward
        if ((MFX_ERR_NONE == mfxRes) ||
            (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes) ||
            (MFX_ERR_MORE_SURFACE == mfxRes) ||
            (MFX_WRN_INCOMPATIBLE_VIDEO_PARAM == mfxRes))
        {
            // prepare the absolete kind of task.
            // it is absolete and must be removed.
            if (NULL == entryPoints[0].pRoutine)
            {
                MFX_TASK task;

                memset(&task, 0, sizeof(task));
                // BEGIN OF OBSOLETE PART
                task.bObsoleteTask = true;
                // fill task info
                task.pOwner = session->m_pVPP.get();
                task.entryPoint.pRoutine = &MFXVideoVPPLegacyRoutine;
                task.entryPoint.pState = session->m_pVPP.get();
                task.entryPoint.requiredNumThreads = 1;

                // fill legacy parameters
                task.obsolete_params.vpp.in = in;
                task.obsolete_params.vpp.out = out;
                task.obsolete_params.vpp.aux = aux;
                // END OF OBSOLETE PART

                task.priority = session->m_priority;
                task.threadingPolicy = session->m_pVPP->GetThreadingPolicy();
                // fill dependencies
                task.pSrc[0] = in;
                task.pDst[0] = out;
                
                if (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes)
                    task.pDst[0] = NULL;

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_VPP;
#endif // MFX_TRACE_ENABLE

                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));
            }
            else if (1 == numEntryPoints)
            {
                MFX_TASK task;

                memset(&task, 0, sizeof(task));
                task.pOwner = session->m_pVPP.get();
                task.entryPoint = entryPoints[0];
                task.priority = session->m_priority;
                task.threadingPolicy = session->m_pVPP->GetThreadingPolicy();
                // fill dependencies
                task.pSrc[0] = in;
                task.pDst[0] = out;
                if (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes)
                    task.pDst[0] = NULL;
                

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_VPP;
#endif
                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));
            }
            else
            {
                MFX_TASK task;

                memset(&task, 0, sizeof(task));
                task.pOwner = session->m_pVPP.get();
                task.entryPoint = entryPoints[0];
                task.priority = session->m_priority;
                task.threadingPolicy = MFX_TASK_THREADING_DEDICATED;
                // fill dependencies
                task.pSrc[0] = in;
                task.pDst[0] = entryPoints[0].pParam;
               

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_VPP;
#endif
                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));

                memset(&task, 0, sizeof(task));
                task.pOwner = session->m_pVPP.get();
                task.entryPoint = entryPoints[1];
                task.priority = session->m_priority;
#if defined(SYNCHRONIZATION_BY_VA_SYNC_SURFACE)
                if (MFX_HW_VAAPI == session->m_pCORE->GetVAType())
                    task.threadingPolicy = MFX_TASK_THREADING_WAIT;
                else
#endif
                    task.threadingPolicy = MFX_TASK_THREADING_DEDICATED_WAIT;
                
                // fill dependencies
                task.pSrc[0] = entryPoints[0].pParam;
                task.pDst[0] = out;
                task.pDst[1] = aux;
                if (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes)
                {
                    task.pDst[0] = NULL;
                    task.pDst[1] = NULL;
                }

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_VPP2;
#endif
                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));
            }

            if (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes)
            {
                mfxRes = MFX_ERR_MORE_DATA;
                syncPoint = NULL;
            }
        }

        // return pointer to synchronization point
        *syncp = syncPoint;
#ifdef MFX_ENABLE_USER_VPP
      }
#endif
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
        else if (0 == session->m_pVPP.get())
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
        else if (0 == syncp)
        {
            return MFX_ERR_NULL_PTR;
        }
    }

    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, out);
    if (mfxRes == MFX_ERR_NONE && syncp)
    {
        MFX_LTRACE_P(MFX_TRACE_LEVEL_API, *syncp);
    }
    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;

} // mfxStatus MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp)

//
// THE OTHER VPP FUNCTIONS HAVE IMPLICIT IMPLEMENTATION
//

FUNCTION_RESET_IMPL(VPP, Reset, (mfxSession session, mfxVideoParam *par), (par))

FUNCTION_IMPL(VPP, GetVideoParam, (mfxSession session, mfxVideoParam *par), (par))
FUNCTION_IMPL(VPP, GetVPPStat, (mfxSession session, mfxVPPStat *stat), (stat))
