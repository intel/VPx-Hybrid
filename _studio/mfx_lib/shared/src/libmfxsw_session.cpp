/* ****************************************************************************** *\

Copyright (C) 2007-2013 Intel Corporation.  All rights reserved.

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

File Name: libmfxsw_session.cpp

\* ****************************************************************************** */

#include <mfxvideo.h>

#include <mfx_session.h>
#include <mfx_utils.h>

mfxStatus MFXJoinSession(mfxSession session, mfxSession child_session)
{
    mfxStatus mfxRes;
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    //MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);
    MFX_CHECK(child_session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(child_session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);

    try
    {
        MFXIUnknown* pInt = session->m_pScheduler;
        // check if the child session has its own children
        if (child_session->IsParentSession())
        {
            return MFX_ERR_UNSUPPORTED;
        }

        // release the child scheduler
        mfxRes = child_session->ReleaseScheduler();
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }

        // join the parent scheduler
        child_session->m_pScheduler = QueryInterface<MFXIScheduler> (pInt,
            MFXIScheduler_GUID);
        if (NULL == child_session->m_pScheduler)
        {
            session->RestoreScheduler();
            return MFX_ERR_INVALID_HANDLE;
        }
        session->m_pOperatorCore->AddCore(child_session->m_pCORE.get());
        child_session->m_pOperatorCore = session->m_pOperatorCore;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // check errors()
        if ((NULL == session) ||
            (NULL == child_session))
        {
            return MFX_ERR_INVALID_HANDLE;
        }

        if ((NULL == session->m_pScheduler) ||
            (NULL == child_session->m_pScheduler))
        {
            return MFX_ERR_NOT_INITIALIZED;
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus MFXJoinSession(mfxSession session, mfxSession child_session)

mfxStatus MFXDisjoinSession(mfxSession session)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);

    try
    {
        // check if the session has its own children.
        // only true child session can be disjoined.
        if (session->IsParentSession())
        {
            return MFX_ERR_UNDEFINED_BEHAVIOR;
        }

        // detach all tasks from the scheduler
        session->m_pScheduler->WaitForTaskCompletion(session->m_pENCODE.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pDECODE.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pVPP.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pENC.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pPAK.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_plgGen.get());

        // remove child core from parent core operator
        session->m_pOperatorCore->RemoveCore(session->m_pCORE.get());

        // create new self core operator
        session->m_pOperatorCore = new OperatorCORE(session->m_pCORE.get());



        // leave the scheduler
        session->m_pScheduler->Release();
        session->m_pScheduler = NULL;


        // join the original scheduler
        mfxRes = session->RestoreScheduler();
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // check errors()
        if (NULL == session)
        {
            return MFX_ERR_INVALID_HANDLE;
        }

        if (NULL == session->m_pScheduler)
        {
            return MFX_ERR_NOT_INITIALIZED;
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus MFXDisjoinSession(mfxSession session)

mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone)
{
    // touch unreferenced parameters
    session = session;
    clone = clone;

    // this function is implemented at the dispatcher's level.
    // there is nothing to do inside the llibrary.
    return MFX_ERR_UNSUPPORTED;

} // mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone)

enum
{
    MFX_PRIORITY_STOP_HW_LISTENING = 0x100,
    MFX_PRIORITY_START_HW_LISTENING = 0x101
};

mfxStatus MFXSetPriority(mfxSession session, mfxPriority priority)
{
    // check error(s)
    if (((MFX_PRIORITY_LOW > priority) || (MFX_PRIORITY_HIGH < priority)) &&
        (MFX_PRIORITY_STOP_HW_LISTENING != priority) &&
        (MFX_PRIORITY_START_HW_LISTENING != priority))
    {
        return MFX_ERR_UNSUPPORTED;
    }

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);

    try
    {
        // set the new priority value
        if ((MFX_PRIORITY_LOW <= priority) && (MFX_PRIORITY_HIGH >= priority))
        {
            session->m_priority = priority;
        }
        // adjust scheduler performance
        else
        {
            switch ((int) priority)
            {
            case MFX_PRIORITY_STOP_HW_LISTENING:
                session->m_pScheduler->AdjustPerformance(MFX_SCHEDULER_STOP_HW_LISTENING);
                break;

            case MFX_PRIORITY_START_HW_LISTENING:
                session->m_pScheduler->AdjustPerformance(MFX_SCHEDULER_START_HW_LISTENING);
                break;

            default:
                break;
            }
        }
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // check errors()
        if (NULL == session)
        {
            return MFX_ERR_INVALID_HANDLE;
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus MFXSetPriority(mfxSession session, mfxPriority priority)

mfxStatus MFXGetPriority(mfxSession session, mfxPriority *priority)
{
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(priority, MFX_ERR_NULL_PTR);

    try
    {
        // set the new priority value
        *priority = session->m_priority;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // check errors()
        if (NULL == session)
        {
            return MFX_ERR_INVALID_HANDLE;
        }
        if (NULL == priority)
        {
            return MFX_ERR_NULL_PTR;
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus MFXGetPriority(mfxSession session, mfxPriority *priority)

#if 0
mfxStatus MFXGetLogMessage(mfxSession session, char *msg, mfxU32 size)
{
    session; msg; size;
    return MFX_ERR_UNSUPPORTED;
} // mfxStatus MFXGetLogMessage(mfxSession session, char *msg, mfxU32 size)
#endif

mfxStatus MFXInternalPseudoJoinSession(mfxSession session, mfxSession child_session)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    //MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);
    MFX_CHECK(child_session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(child_session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);

    try
    {
 

        //  release  the child scheduler
        mfxRes = child_session->ReleaseScheduler();
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }

        child_session->m_pScheduler = session->m_pScheduler;


        child_session->m_pCORE.reset(session->m_pCORE.get(), false);
        child_session->m_pOperatorCore = session->m_pOperatorCore;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // check errors()
        if ((NULL == session) ||
            (NULL == child_session))
        {
            return MFX_ERR_INVALID_HANDLE;
        }

        if ((NULL == session->m_pScheduler) ||
            (NULL == child_session->m_pScheduler))
        {
            return MFX_ERR_NOT_INITIALIZED;
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus MFXJoinSession(mfxSession session, mfxSession child_session)
mfxStatus MFXInternalPseudoDisjoinSession(mfxSession session)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);

    try
    {
        // detach all tasks from the scheduler
        session->m_pScheduler->WaitForTaskCompletion(session->m_pENCODE.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pDECODE.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pVPP.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pENC.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_pPAK.get());
        session->m_pScheduler->WaitForTaskCompletion(session->m_plgGen.get());


        // just zeroing pointer to external scheduler (it will be released in external session close)
        session->m_pScheduler = NULL;

        mfxRes = session->RestoreScheduler();
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }
          
        // core will released automatically 


    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // check errors()
        if (NULL == session)
        {
            return MFX_ERR_INVALID_HANDLE;
        }

        if (NULL == session->m_pScheduler)
        {
            return MFX_ERR_NOT_INITIALIZED;
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus MFXDisjoinSession(mfxSession session)
