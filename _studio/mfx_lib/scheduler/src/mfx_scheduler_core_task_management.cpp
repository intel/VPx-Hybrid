/* ****************************************************************************** *\

Copyright (C) 2010-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_scheduler_core_task_management.cpp

\* ****************************************************************************** */

#include <mfx_scheduler_core.h>
#include <mfx_scheduler_core_task.h>
#include <mfx_trace.h>

#include <umc_automatic_mutex.h>
#include <vm_time.h>

// declare the static section of the file
namespace
{

const
int TaskPriorityRatio[MFX_PRIORITY_NUMBER] =
{
    // MFX_PRIORITY_LOW tasks utilize 100% of CPU power, which is left
    // unitilized by higher priority tasks.
    100,

    // MFX_PRIORITY_NORMAL tasks utilize 75% of CPU power, which is left
    // unitilized by MFX_PRIORITY_REALTIME priority tasks.
    75,

    // MFX_PRIORITY_HIGH tasks try to utilize 100% of CPU power,
    // if it has enough to do.
    100
};

enum
{
    // waiting tasks are examined every 1 msec
    MFX_WAIT_TIME_MS            = 1
};

} // namespace

int mfxSchedulerCore::GetTaskPriority(mfxTaskHandle task)
{
    const MFX_SCHEDULER_TASK *pTask;
    int taskPriority = -1;

    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    pTask = m_ppTaskLookUpTable[task.taskID];
    if ((pTask) &&
        (pTask->jobID == task.jobID) &&
        (MFX_TASK_WORKING == pTask->curStatus))
    {
        taskPriority = pTask->param.task.priority;
    }

    return taskPriority;

} // int mfxSchedulerCore::GetTaskPriority(mfxTaskHandle task)

void mfxSchedulerCore::GetTimeStat(mfxU64 timeSpent[MFX_PRIORITY_NUMBER],
                                   mfxU64 totalTimeSpent[MFX_PRIORITY_NUMBER])
{
    int priority;

    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    for (priority = MFX_PRIORITY_LOW; priority < MFX_PRIORITY_NUMBER; priority += 1)
    {
        mfxU32 i;

        totalTimeSpent[priority] = 0;
        timeSpent[priority] = 0;
        for (i = 0; i < MFX_TIME_STAT_PARTS; i += 1)
        {
            int other;

            // fill the time spent by threads with given priority
            timeSpent[priority] += m_workingTime[i].time[priority];
            // fill the time spend by threads with given or lowere priority
            for (other = MFX_PRIORITY_LOW; other <= priority; other += 1)
            {
                totalTimeSpent[priority] += m_workingTime[i].time[other];
            }
        }
    }

} // void mfxSchedulerCore::GetTimeStat(mfxU64 totalTimeSpent[MFX_PRIORITY_NUMBER],

enum
{
    // on the first run when the scheduler tries to get a task,
    // it tries to keep tasks prioirties.
    PRIORITY_RUN = 0,
    // on the second run the scheduler tries to get any 'ready' task,
    // ignoring priority.
    REGULAR_RUN = 1,

    NUMBER_OF_RUNS
};

mfxStatus mfxSchedulerCore::GetTask(MFX_CALL_INFO &callInfo,
                                    mfxTaskHandle previousTask,
                                    const mfxU32 threadNum)
{
    UMC::AutomaticUMCMutex guard(m_guard);
    int prevTaskPriority = -1;
    mfxU32 run;
    mfxU64 totalTimeSpent[MFX_PRIORITY_NUMBER], timeSpent[MFX_PRIORITY_NUMBER];

    // get the current time stamp
    m_currentTimeStamp = GetHighPerformanceCounter();

    // get time spent statistic
    GetTimeStat(timeSpent, totalTimeSpent);

    // get the priority of the previous task
    prevTaskPriority = GetTaskPriority(previousTask);

    // there are three runs over the tasks lists. On the 1st run,
    // the scheduler keeping workload balance, which is described by
    // the TaskPriorityRatio table. On the 2nd run, the scheduler chooses
    // any ready task.
    for (run = 0; run < NUMBER_OF_RUNS; run += 1)
    {
        int priority;

        for (priority = MFX_PRIORITY_HIGH;
             priority >= MFX_PRIORITY_LOW;
             priority -= 1)
        {
            //
            // check the number of assigned task by the particular priority
            //

            // the second run always examines tasks
            if ((PRIORITY_RUN != run) ||
                // during the first run we subscribe tasks only,
                // if the kind of tasks does not utilize its share
                // of CPU performance.
                (TaskPriorityRatio[priority] * totalTimeSpent[priority] >=
                 100 * timeSpent[priority]))
            {
                int type;

                for (type = (threadNum) ? (MFX_TYPE_SOFTWARE) : (MFX_TYPE_HARDWARE);
                     type <= MFX_TYPE_SOFTWARE;
                     type += 1)
                {
                    MFX_SCHEDULER_TASK *pTask = m_pTasks[priority][type];

                    // try to continue the previous task
                    if (prevTaskPriority == priority)
                    {
                        mfxStatus mfxRes;

                        // try get the same task again
                        mfxRes = CanContinuePreviousTask(callInfo,
                                                         previousTask,
                                                         threadNum);
                        if (MFX_ERR_NONE == mfxRes)
                        {
                            return mfxRes;
                        }
                    }

                    // run over the tasks list
                    while (pTask)
                    {
                        mfxStatus mfxRes;

                        // is this task ready?
                        mfxRes = WrapUpTask(callInfo, pTask, threadNum);
                        if (MFX_ERR_NONE == mfxRes)
                        {
                            return mfxRes;
                        }

                        // get the next task
                        pTask = pTask->pNext;
                    }
                }
            }
        }
    }

    // print task parameters for DEBUG purposes
    PrintTaskInfoUnsafe();

    return MFX_ERR_NOT_FOUND;

} // mfxStatus mfxSchedulerCore::GetTask(MFX_CALL_INFO &callInfo,

mfxStatus mfxSchedulerCore::CanContinuePreviousTask(MFX_CALL_INFO &callInfo,
                                                    mfxTaskHandle previousTask,
                                                    const mfxU32 threadNum)
{
    MFX_SCHEDULER_TASK *pTask;

    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    // get the task object
    pTask = m_ppTaskLookUpTable[previousTask.taskID];
    if ((NULL == pTask) ||
        (pTask->jobID != previousTask.jobID))
    {
        return MFX_ERR_NOT_FOUND;
    }

    return WrapUpTask(callInfo, pTask, threadNum);

} // mfxStatus mfxSchedulerCore::CanContinuePreviousTask(MFX_CALL_INFO &callInfo,

// static section of the file
namespace
{

enum
{
    MFX_INVALID_THREAD_NUMBER   = 0x7fffffff
};

inline
mfxU32 GetFreeThreadNumber(MFX_THREAD_ASSIGNMENT &occupancyInfo,
                           MFX_SCHEDULER_TASK *pTask)
{
    mfxU32 mask, numThreads;
    mfxU32 i;

    // get available thread mask and maximum allowed threads number
    if (MFX_TASK_INTER & occupancyInfo.threadingPolicy)
    {
        mask = pTask->param.threadMask;
    }
    else
    {
        mask = occupancyInfo.threadMask;
    }
    numThreads = pTask->param.task.entryPoint.requiredNumThreads;

    for (i = 0; i < numThreads; i += 1)
    {
        if (0 == (mask & (1 << i)))
        {
            return i;
        }
    }

    return MFX_INVALID_THREAD_NUMBER;

} // mfxU32 GetFreeThreadNumber(MFX_THREAD_ASSIGNMENT &occupancyInfo,

} // namespace

mfxStatus mfxSchedulerCore::WrapUpTask(MFX_CALL_INFO &callInfo,
                                       MFX_SCHEDULER_TASK *pTask,
                                       const mfxU32 threadNum)
{
    MFX_THREAD_ASSIGNMENT &occupancyInfo = *(pTask->param.pThreadAssignment);

    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    //
    // check basic conditions
    //

        // if task is done
    if ((MFX_TASK_NEED_CONTINUE != pTask->curStatus) ||
        // or dependencies are not resolved
        (false == pTask->IsDependenciesResolved()))
    {
        return MFX_ERR_NOT_FOUND;
    }
        // or non-zero thread tries to get a dedicated (hardware) task
    if ((MFX_TASK_DEDICATED & occupancyInfo.threadingPolicy) &&
        (threadNum))
    {
        return MFX_ERR_NOT_FOUND;
    }
#if defined(SYNCHRONIZATION_BY_NON_ZERO_THREAD)
        // or the dedicated thread tries to get a non-dedicated 'wait' task
    if ((0 == threadNum) &&
        !(MFX_TASK_DEDICATED & occupancyInfo.threadingPolicy) &&
        (MFX_TASK_WAIT & occupancyInfo.threadingPolicy))
    {
        return MFX_ERR_NOT_FOUND;
    }
#endif
    callInfo.threadNum = GetFreeThreadNumber(occupancyInfo, pTask);
        // or there is no proper thread number
    if (MFX_INVALID_THREAD_NUMBER == callInfo.threadNum)
    {
        return MFX_ERR_NOT_FOUND;
    }

    callInfo.callNum = pTask->param.numberOfCalls;

    // is the task waiting for something ?
    if (pTask->param.bWaiting)
    {
        // prevent entering more than 1 thread in 'waiting' task,
        // let the thread inspect other tasks.
        if (pTask->param.occupancy)
        {
            return MFX_ERR_NOT_FOUND;
        }
        // check the 'waiting' time period
        else
        {
            mfxU64 time;

            // calculate the period elapsed since the last 'need-waiting' call
            time = m_currentTimeStamp - pTask->param.timing.timeLastEnter;
            if (m_timeWaitPeriod > time)
            {
                {
                    const mfxU64 hwCounter = GetHWEventCounter();

                    if (hwCounter == pTask->param.timing.hwCounterLastEnter)
                    {
                        return MFX_ERR_NOT_FOUND;
                    }
                }
            }
        }
    }

    //
    // everything is OK.
    // Update the task and return the parameters
    //

    // update the scheduler
    m_numAssignedTasks[pTask->param.task.priority] += 1;

    // update the number of assigned threads
    occupancyInfo.taskOccupancy += (0 == pTask->param.occupancy) ? (1) : (0);
    if (0 == (MFX_TASK_INTER & occupancyInfo.threadingPolicy))
    {
        occupancyInfo.occupancy += 1;
        occupancyInfo.threadMask |= (1 << callInfo.threadNum);
    }
    pTask->param.occupancy += 1;
    pTask->param.threadMask |= (1 << callInfo.threadNum);

    pTask->param.numberOfCalls += 1;

    // update the task's timing
    pTask->param.timing.timeLastEnter = m_currentTimeStamp;
    pTask->param.timing.timeLastCallIssued = m_currentTimeStamp;
    pTask->param.timing.hwCounterLastEnter = GetHWEventCounter();
    // create handle
    callInfo.taskHandle.taskID = pTask->taskID;
    callInfo.taskHandle.jobID = pTask->jobID;
    // set the task pointer
    callInfo.pTask = &pTask->param.task;
    // set the time stamp of task release
    callInfo.timeStamp = m_currentTimeStamp;

#if defined(MFX_SCHEDULER_LOG)
        mfxLogWriteA(m_hLog,
                     "[% 4u]  get %s task - %u, job - %u, occupancy - %u, priority - %s\n",
                     threadNum,
                     taskType[pTask->param.task.threadingPolicy],
                     pTask->taskID, pTask->jobID,
                     pTask->param.occupancy,
                     (MFX_PRIORITY_HIGH == pTask->param.task.priority) ? ("high") :
                     ((MFX_PRIORITY_NORMAL == pTask->param.task.priority) ? ("normal") : ("low")));
#endif // defined(MFX_SCHEDULER_LOG)

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::WrapUpTask(MFX_CALL_INFO &callInfo,

void mfxSchedulerCore::ResetWaitingTasks(const void *pOwner)
{
    mfxU32 priority;

    for (priority = 0; priority < MFX_PRIORITY_NUMBER; priority += 1)
    {
        int type;

        for (type = MFX_TYPE_HARDWARE; type <= MFX_TYPE_SOFTWARE; type += 1)
        {
            MFX_SCHEDULER_TASK *pTask = m_pTasks[priority][type];

            // run over the tasks with particular priority
            while (pTask)
            {
                // reset the 'waiting' flag
                if ((pTask->param.task.pOwner == pOwner) &&
                    (MFX_TASK_NEED_CONTINUE == pTask->curStatus))
                {
                    // resetting 'waiting' flag should help waking up permanent tasks
                    pTask->param.bWaiting = false;

                    // set new time of the last call processed to avoid overwriting
                    // 'waiting' status flag.
                    pTask->param.timing.timeLastCallProcessed = pTask->param.timing.timeLastCallIssued + 1;
                }

                // advance the task pointer
                pTask = pTask->pNext;
            }
        }
    }

} // void mfxSchedulerCore::ResetWaitingTasks(const void *pOwner)

void mfxSchedulerCore::MarkTaskCompleted(const MFX_CALL_INFO *pCallInfo,
                                         const mfxU32 threadNum)
{
    bool wakeUpThreads = false;
    mfxU32 requiredNumThreads;
    bool taskReleased = false;
    mfxU32 nTraceTaskId = 0;

    // enter the protected code section
    {
        UMC::AutomaticUMCMutex guard(m_guard);
        MFX_SCHEDULER_TASK *pTask = m_ppTaskLookUpTable[pCallInfo->taskHandle.taskID];
        mfxU32 curTime;

        // check error(s)
        if (NULL == pTask)
        {
            return;
        }

        MFX_THREAD_ASSIGNMENT &occupancyInfo = *(pTask->param.pThreadAssignment);

        // update working time
        curTime = GetLowResCurrentTime();
        if (m_workingTime[m_timeIdx].startTime + MFX_TIME_STAT_PERIOD / MFX_TIME_STAT_PARTS <
            curTime)
        {
            // advance the working time index. The current entry is out of time.
            m_timeIdx = (m_timeIdx + 1) % MFX_TIME_STAT_PARTS;
            memset(m_workingTime + m_timeIdx, 0, sizeof(m_workingTime[m_timeIdx]));
            m_workingTime[m_timeIdx].startTime = curTime;
        }
        m_workingTime[m_timeIdx].time[pTask->param.task.priority] += pCallInfo->timeSpend;

        // update the scheduler
        m_numAssignedTasks[pTask->param.task.priority] -= 1;

        // clean up the task object
        pTask->param.occupancy -= 1;
        pTask->param.threadMask &= ~(1 << pCallInfo->threadNum);
        if (0 == (MFX_TASK_INTER & occupancyInfo.threadingPolicy))
        {
            occupancyInfo.occupancy -= 1;
            occupancyInfo.threadMask &= ~(1 << pCallInfo->threadNum);
        }
        occupancyInfo.taskOccupancy -= (0 == pTask->param.occupancy) ? (1) : (0);

        // update the number of available tasks
        if ((MFX_TASK_NEED_CONTINUE == pTask->curStatus) &&
            ((isFailed(pCallInfo->res) || MFX_TASK_DONE == pCallInfo->res)))
        {
            (MFX_TASK_DEDICATED & pTask->param.task.threadingPolicy) ? (m_numHwTasks -= 1) : (m_numSwTasks -= 1);
        }

        // try to not overwrite the newest status from other thread
        if (pTask->param.timing.timeLastCallProcessed < pCallInfo->timeStamp)
        {
            pTask->param.timing.timeLastCallProcessed = pCallInfo->timeStamp;
        }
        // update the status of the current job
        if (isFailed(pCallInfo->res))
        {
            pTask->curStatus = pCallInfo->res;
        }
        // do not overwrite the failed status with successful one
        else if ((MFX_TASK_DONE == pCallInfo->res) &&
                 (MFX_TASK_NEED_CONTINUE == pTask->curStatus))
        {
            wakeUpThreads = true;
            pTask->curStatus = pCallInfo->res;

            // reset all waiting tasks with the given working object
            ResetWaitingTasks(pCallInfo->pTask->pOwner);
        }
        // update the task timing
        else if (MFX_TASK_BUSY == pCallInfo->res)
        {
            // try to not overwrite the newest status from other thread
            if (pTask->param.timing.timeLastCallProcessed <= pCallInfo->timeStamp)
            {
                pTask->param.bWaiting = true;
            }
            pTask->param.timing.timeOverhead += pCallInfo->timeSpend;
        }
        else
        {
            wakeUpThreads = true;

            // reset all waiting tasks with the given working object
            ResetWaitingTasks(pCallInfo->pTask->pOwner);
        }
        pTask->param.timing.timeSpent += pCallInfo->timeSpend;

        //
        // update task status and tasks dependencies
        //

        if (0 == pTask->param.occupancy)
        {
            // complete the task if it is done
            if ((isFailed(pTask->curStatus)) ||
                (MFX_TASK_DONE == pTask->curStatus))
            {
                // store TaskId for tracing event
                nTraceTaskId = pCallInfo->pTask->nTaskId;

                // get entry point parameters to call FreeResources
                MFX_ENTRY_POINT &entryPoint = pTask->param.task.entryPoint;

                if (entryPoint.pCompleteProc)
                {
                    mfxStatus mfxRes;

                    // temporarily leave the protected code section
                    guard.Unlock();
                    // be aware of external call
                    try
                    {
                        // release the component's resources
                        mfxRes = entryPoint.pCompleteProc(entryPoint.pState,
                                                          entryPoint.pParam,
                                                          pTask->curStatus);
                        // update status
                        if ((isFailed(mfxRes)) &&
                            (MFX_ERR_NONE == pTask->curStatus))
                        {
                            pTask->curStatus = mfxRes;
                        }
                    }
                    catch(...)
                    {
                    }
                    // enter the protected code section
                    guard.Lock();
                }
            }

            // update the failed task status
            if (isFailed(pTask->curStatus))
            {
                //mfxU32 i;

                // save the status
                pTask->opRes = pTask->curStatus;
                pTask->done.Set();

                // update dependencies produced from the dependency table
                //for (i = 0; i < MFX_TASK_NUM_DEPENDENCIES; i += 1)
                //{
                //    if (pTask->param.task.pDst[i])
                //    {
                //        mfxU32 idx = pTask->param.dependencies.dstIdx[i];

                //        m_pDependencyTable[idx].mfxRes = pTask->curStatus;
                //    }
                //}
                ResolveDependencyTable(pTask);

                // mark all dependent task as 'failed'
                pTask->ResolveDependencies(pTask->curStatus);
                // release all allocated resources
                pTask->ReleaseResources();
            }
            // process task completed
            else if (MFX_TASK_DONE == pTask->curStatus)
            {
                mfxU32 i;

                // reset jobID to avoid false waiting on complete tasks, which were reused
                pTask->jobID = 0;
                // save the status
                pTask->opRes = MFX_ERR_NONE;
                pTask->done.Set();

                // remove dependencies produced from the dependency table
                for (i = 0; i < MFX_TASK_NUM_DEPENDENCIES; i += 1)
                {
                    if (pTask->param.task.pDst[i])
                    {
                        mfxU32 idx = pTask->param.dependencies.dstIdx[i];

                        m_pDependencyTable[idx].p = NULL;
                    }
                }

                // mark all dependent task as 'ready'
                pTask->ResolveDependencies(MFX_ERR_NONE);
                // release all allocated resources
                pTask->ReleaseResources();

                // task is done. Wake up all threads.
                wakeUpThreads = true;
                // task object becomes free
                taskReleased = true;
            }
        }

#if defined(MFX_SCHEDULER_LOG)
        if (MFX_TASK_DONE == pCallInfo->res)
        {
            mfxLogWriteA(m_hLog,
                         "[% 4u] DONE %s task - %u, job - %u, res - %d, occupancy - %u, time - %u\n",
                         threadNum,
                         taskType[pTask->param.task.threadingPolicy],
                         pTask->taskID, pTask->jobID,
                         pCallInfo->res, pTask->param.occupancy,
                         pCallInfo->timeSpend);
            TASK_STAT stat;
            GetNumTasks(m_pTasks, stat);
            mfxLogWriteA(m_hLog,
                         "[    ] Tasks: total - %u, ready - %u, waiting - %u, done - %u, failed - %u\n",
                         stat.total,
                         stat.ready,
                         stat.waiting,
                         stat.done,
                         stat.failed);
        }
        else
        {
            mfxLogWriteA(m_hLog,
                         "[% 4u]     %s task - %u, job - %u, res - %d, occupancy - %u, time - %u\n",
                         threadNum,
                         taskType[pTask->param.task.threadingPolicy],
                         pTask->taskID, pTask->jobID,
                         pCallInfo->res, pTask->param.occupancy,
                         pCallInfo->timeSpend);
        }
#endif // defined(MFX_SCHEDULER_LOG)

        requiredNumThreads = GetNumResolvedSwTasks();
    }
    // leave the protected code section

    // wake up additional threads for this task and tasks dependent
    if (wakeUpThreads)
    {
        WakeUpNumThreads(requiredNumThreads, threadNum);
    }

    // wake up external threads waiting for a free task object
    if (taskReleased)
    {
        m_freeTasks.Signal(1);
    }

    // send tracing event
    if (nTraceTaskId)
    {
        MFX_LTRACE_1(MFX_TRACE_LEVEL_SCHED, "^Completed^", "%d", nTraceTaskId);
    }

} // void mfxSchedulerCore::MarkTaskCompleted(mfxTaskHandle handle,
                // update dependencies produced from the dependency table
void mfxSchedulerCore::ResolveDependencyTable(MFX_SCHEDULER_TASK *pTask)
{
    mfxU32 i;
    for (i = 0; i < MFX_TASK_NUM_DEPENDENCIES; i += 1)
    {
        if (pTask->param.task.pDst[i])
        {
            mfxU32 idx = pTask->param.dependencies.dstIdx[i];

            m_pDependencyTable[idx].mfxRes = pTask->curStatus;
        }
    }
}

mfxU32 mfxSchedulerCore::GetNumResolvedSwTasks(void) const
{
    mfxU32 numResolvedTasks = 0;

    mfxU32 priority;
    for (priority = 0; priority < MFX_PRIORITY_NUMBER; priority += 1)
    {
        MFX_SCHEDULER_TASK *pTask = m_pTasks[priority][MFX_TYPE_SOFTWARE];

        // run over the tasks with particular priority
        while (pTask)
        {
            // if a task has not been done yet
            if ((MFX_TASK_NEED_CONTINUE == pTask->curStatus) &&
            // and dependencies are resolved
                (true == pTask->IsDependenciesResolved()))
            {
                numResolvedTasks++;
            }

            pTask = pTask->pNext;
        }
    }

    return numResolvedTasks;
}
