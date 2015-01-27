/* ****************************************************************************** *\

Copyright (C) 2009-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_scheduler_core_task.cpp

\* ****************************************************************************** */

#include <mfx_scheduler_core_task.h>
#include <mfx_scheduler_core.h>


#include <memory.h>

MFX_SCHEDULER_TASK::MFX_SCHEDULER_TASK(mfxU32 taskID, mfxSchedulerCore *pSchedulerCore) :
    taskID(taskID),
    jobID(0),
    m_pSchedulerCore(pSchedulerCore)
{
    // reset task parameters
    memset(&param, 0, sizeof(param));

    pNext = NULL;

} // MFX_SCHEDULER_TASK::MFX_SCHEDULER_TASK(void)

mfxStatus MFX_SCHEDULER_TASK::Reset(void)
{
    // reset task parameters
    memset(&param, 0, sizeof(param));

    // reset waiting event and task results
    done.Reset();
    opRes = MFX_WRN_IN_EXECUTION;
    curStatus = MFX_TASK_WORKING;

    return MFX_ERR_NONE;

} // mfxStatus MFX_SCHEDULER_TASK::Reset(void)

void MFX_SCHEDULER_TASK::OnDependencyResolved(mfxStatus result)
{
    if (isFailed(result))
    {
        // need to update dependency table for all tasks dependent from failed 
        m_pSchedulerCore->ResolveDependencyTable(this);
        // waiting task inherits status from the parent task
        if (MFX_TASK_WAIT & param.task.threadingPolicy)
        {
            opRes = result;
            curStatus = result;
        }
        // all other tasks are aborted
        else
        {
            opRes = MFX_ERR_ABORTED;
            curStatus = MFX_ERR_ABORTED;
        }
        done.Set();

        // release the current task resources
        ReleaseResources();

        // be aware of external call
        try
        {
            MFX_ENTRY_POINT &entryPoint = param.task.entryPoint;

            if (entryPoint.pCompleteProc)
            {
                // release the component's resources
                entryPoint.pCompleteProc(entryPoint.pState,
                                         entryPoint.pParam,
                                         MFX_ERR_ABORTED);
            }
        }
        catch(...)
        {
        }
    }

    // call the parent's method
    mfxDependencyItem<MFX_TASK_NUM_DEPENDENCIES>::OnDependencyResolved(result);

} // void MFX_SCHEDULER_TASK::OnDependencyResolved(mfxStatus result)

void MFX_SCHEDULER_TASK::ReleaseResources(void)
{
    if (param.pThreadAssignment)
    {
        param.pThreadAssignment->m_numRefs -= 1;
        if (param.pThreadAssignment->pLastTask == this)
        {
            param.pThreadAssignment->pLastTask = NULL;
        }
    }

    // thread assignment info is not required for the task any more
    param.pThreadAssignment = NULL;

} // void MFX_SCHEDULER_TASK::ReleaseResources(void)
