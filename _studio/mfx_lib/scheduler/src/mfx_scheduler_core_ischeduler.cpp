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

File Name: mfx_scheduler_core_ischeduler.cpp

\* ****************************************************************************** */

#include <mfx_scheduler_core.h>

#include <mfx_scheduler_core_task.h>
#include <mfx_scheduler_core_handle.h>

#include <vm_time.h>
#include <vm_sys_info.h>
#include <umc_automatic_mutex.h>
#include <mfx_trace.h>

#if defined  (MFX_VA)
#if defined  (MFX_D3D11_ENABLED)
#include "mfx_scheduler_dx11_event.h"
#endif
#endif

#if defined(MFX_SCHEDULER_LOG)
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#elif defined(LINUX32) || defined(LINUX64)
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>
#endif
#include <stdio.h>
#endif // defined(MFX_SCHEDULER_LOG)

enum
{
    MFX_TIME_INFINITE           = 0x7fffffff,
    MFX_TIME_TO_WAIT            = 5
};

mfxStatus mfxSchedulerCore::Initialize(const MFX_SCHEDULER_PARAM *pParam)
{
    UMC::Status umcRes;
    mfxU32 i, iRes;
    mfxU32 numThreads;
    mfxStatus mfxRes;

    // release the object before initialization
    Close();


    // save the parameters
    if (pParam)
    {
        m_param = *pParam;
    }

#if defined(MFX_SCHEDULER_LOG)
    {
        char cPath[256];
#if defined(WIN32) || defined(WIN64)
        SYSTEMTIME time;

        GetLocalTime(&time);
        // create the log path
        sprintf_s(cPath, sizeof(cPath),
                  "c:\\temp\\mfx_scheduler_log(%02u-%02u-%02u %x).txt",
                  time.wHour, time.wMinute, time.wSecond,
                  this);
#elif defined(LINUX32) || defined(LINUX64)
        time_t seconds;
        tm s_time;

        time(&seconds);
        localtime_r(&seconds, &s_time);

        sprintf(cPath, "~/temp/mfx_scheduler_log(%02u-%02u-%02u %x).txt", s_time.tm_hour, s_time.tm_min, s_time.tm_sec);
#endif
        // initialize the log
        m_hLog = mfxLogInit(cPath);
    }
#endif // defined(MFX_SCHEDULER_LOG)

    // clean up the task look up table
    umcRes = m_ppTaskLookUpTable.Alloc(MFX_MAX_NUMBER_TASK);
    if (UMC::UMC_OK != umcRes)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    // allocate the dependency table
    m_pDependencyTable.Alloc(MFX_MAX_NUMBER_TASK * 2);
    if (UMC::UMC_OK != umcRes)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    // allocate the thread assignment object table.
    // its size should be equal to the number of task,
    // larger table is not required.
    m_occupancyTable.Alloc(MFX_MAX_NUMBER_TASK);
    if (UMC::UMC_OK != umcRes)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    // decide the number of threads
    numThreads = vm_sys_info_get_cpu_num();
    if (m_param.numberOfThreads)
    {
        numThreads = UMC::get_min(numThreads, m_param.numberOfThreads);
    }
    //we have to create threads more than cores to support Intel plug-ins threading 
    //m_param.numberOfThreads = numThreads + 1;

#if defined(SYNCHRONIZATION_BY_VA_SYNC_SURFACE)
    m_param.numberOfThreads = UMC::get_max(m_param.numberOfThreads, 2);
#endif

    try
    {
        // allocate 'free task' event
        umcRes = m_freeTasks.Init(MFX_MAX_NUMBER_TASK, MFX_MAX_NUMBER_TASK);
        if (UMC::UMC_OK != umcRes)
        {
            return MFX_ERR_UNKNOWN;
        }

        // initialize the waiting objects.
        m_pTaskAdded = new UMC::Event[m_param.numberOfThreads];
        for (i = 0; i < m_param.numberOfThreads; i += 1)
        {
            umcRes = m_pTaskAdded[i].Init(0, 0);
            if (UMC::UMC_OK != umcRes)
            {
                return MFX_ERR_UNKNOWN;
            }
        }

        // allocate thread contexts
        m_pThreadCtx = new MFX_SCHEDULER_THREAD_CONTEXT[m_param.numberOfThreads];
        memset(m_pThreadCtx, 0, sizeof(MFX_SCHEDULER_THREAD_CONTEXT) * m_param.numberOfThreads);
        // start threads
        for (i = 0; i < m_param.numberOfThreads; i += 1)
        {
            // prepare context
            m_pThreadCtx[i].threadNum = i;
            m_pThreadCtx[i].pSchedulerCore = this;
            // spawn a thread
            vm_thread_set_invalid(&(m_pThreadCtx[i].threadHandle));
            iRes = vm_thread_create(&(m_pThreadCtx[i].threadHandle),
                                    scheduler_thread_proc,
                                    m_pThreadCtx + i);
            if (0 == iRes)
            {
                return MFX_ERR_UNKNOWN;
            }
            // 02.2012 vcherepa: DON'T use high priority for internal thread #0.
            // It is useless when CPU affinity mask is not set.
            //if ((0 == i) && (1 < m_param.numberOfThreads))
            //{
            //    vm_thread_set_priority(&(m_pThreadCtx[i].threadHandle), VM_THREAD_PRIORITY_HIGH);
            //}
        }

        // 02.2012 vcherepa: DON'T set CPU affinity for particular internal
        // threads. it leads to unpredictable stops and hangings up to 60ms.
        // set CPU mask for every thread
        // need to care about internal threading another way
//#if !defined(MFX_VA)
//        SetThreadsAffinityMask();
//#endif // !defined(MFX_VA)
//
    }
    catch(...)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }

    // to run HW listen thread. Will be enabled if tests are OK
    mfxRes = StartWakeUpThread();
    if (MFX_ERR_NONE != mfxRes)
    {
        return mfxRes;
    }

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::Initialize(mfxSchedulerFlags flags, mfxU32 numberOfThreads)

mfxStatus mfxSchedulerCore::AddTask(const MFX_TASK &task, mfxSyncPoint *pSyncPoint)
{
    return AddTask(task, pSyncPoint, NULL, 0);

} // mfxStatus mfxSchedulerCore::AddTask(const MFX_TASK &task, mfxSyncPoint *pSyncPoint)

mfxStatus mfxSchedulerCore::Synchronize(mfxSyncPoint syncPoint, mfxU32 timeToWait)
{
    mfxTaskHandle handle;
    mfxStatus mfxRes;

    // check error(s)
    if (NULL == syncPoint)
    {
        return MFX_ERR_NULL_PTR;
    }

    // cast the pointer to handle and make syncing
    handle.handle = (size_t) syncPoint;

#if defined(MFX_SCHEDULER_LOG)
    Ipp32u start = vm_time_get_current_time();
#endif // defined(MFX_SCHEDULER_LOG)

    // waiting on task's handle
    mfxRes = Synchronize(handle, timeToWait);

#if defined(MFX_SCHEDULER_LOG)

    Ipp32u stop = vm_time_get_current_time();

#if defined(WIN32) || defined(WIN64)
    mfxLogWriteA(m_hLog,
                 "[% u4] WAIT task - %u, job - %u took %u msec (priority - %d)\n",
                 GetCurrentThreadId(),
                 handle.taskID,
                 handle.jobID,
                 stop - start,
                 GetThreadPriority(GetCurrentThread()));
#elif defined(LINUX32) || defined(LINUX64)
    mfxLogWriteA(m_hLog,
                 "[% u4] WAIT task - %u, job - %u took %u msec (priority - %d)\n",
                 syscall(SYS_gettid),
                 handle.taskID,
                 handle.jobID,
                 stop - start,
                 syscall(SYS_getpriority));
#endif

#endif // defined(MFX_SCHEDULER_LOG)

    return mfxRes;

} // mfxStatus mfxSchedulerCore::Synchronize(mfxSyncPoint syncPoint, mfxU32 timeToWait)

mfxStatus mfxSchedulerCore::Synchronize(mfxTaskHandle handle, mfxU32 timeToWait)
{
    MFX_SCHEDULER_TASK *pTask;

    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }

    // look up the task
    pTask = m_ppTaskLookUpTable[handle.taskID];
    if (NULL == pTask)
    {
        return MFX_ERR_NULL_PTR;
    }

    //
    // inspect the task
    //

    // the handle is outdated,
    // the previous task job is over and completed with successful status
    // NOTE: it make sense to read task result and job ID the following order.
    if ((MFX_ERR_NONE == pTask->opRes) ||
        (pTask->jobID != handle.jobID))
    {
        return MFX_ERR_NONE;
    }

    // wait the result on the event
    if (MFX_WRN_IN_EXECUTION == pTask->opRes)
    {
        UMC::Status res;

        MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_PRIVATE, "Scheduler::Wait");
        MFX_LTRACE_1(MFX_TRACE_LEVEL_SCHED, "^Depends^on", "%d", pTask->param.task.nParentId);
        MFX_LTRACE_I(MFX_TRACE_LEVEL_SCHED, timeToWait);

        res = pTask->done.Wait(timeToWait);
        if (UMC::UMC_ERR_TIMEOUT == res)
        {
            return MFX_WRN_IN_EXECUTION;
        }
    }

    // check error status
    if ((MFX_ERR_NONE != pTask->opRes) &&
        (pTask->jobID == handle.jobID))
    {
        return pTask->opRes;
    }

    // in all other cases task is complete
    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::Synchronize(mfxTaskHandle handle, mfxU32 timeToWait)

mfxStatus mfxSchedulerCore::WaitForDependencyResolved(const void *pDependency)
{
    mfxTaskHandle waitHandle = {};
    bool bFind = false;

    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }
    if (NULL == pDependency)
    {
        return MFX_ERR_NONE;
    }

    // find a handle to wait
    {
        UMC::AutomaticUMCMutex guard(m_guard);
        mfxU32 curIdx;

        for (curIdx = 0; curIdx < m_numDependencies; curIdx += 1)
        {
            if (m_pDependencyTable[curIdx].p == pDependency)
            {
                // get the handle before leaving protected section
                waitHandle.taskID = m_pDependencyTable[curIdx].pTask->taskID;
                waitHandle.jobID = m_pDependencyTable[curIdx].pTask->jobID;

                // handle is found, go to wait
                bFind = true;
                break;
            }
        }
        // leave the protected section
    }

    // the dependency is still in the table,
    // wait until it leaves the table.
    if (bFind)
    {
        return Synchronize(waitHandle, MFX_TIME_INFINITE);
    }

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::WaitForDependencyResolved(const void *pDependency)

mfxStatus mfxSchedulerCore::WaitForTaskCompletion(const void *pOwner)
{
    mfxTaskHandle waitHandle;

    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }
    if (NULL == pOwner)
    {
        return MFX_ERR_NULL_PTR;
    }

    // make sure that threads are running
    {
        UMC::AutomaticUMCMutex guard(m_guard);

        ResetWaitingTasks(pOwner);
    }
    WakeUpThreads();

    do
    {
        // reset waiting handle
        waitHandle.handle = 0;

        // enter protected section.
        // make searching in a separate code block,
        // to avoid dead-locks with other threads.
        {
            UMC::AutomaticUMCMutex guard(m_guard);
            int priority;

            priority = MFX_PRIORITY_HIGH;
            do
            {
                int type;

                for (type = MFX_TYPE_HARDWARE; type <= MFX_TYPE_SOFTWARE; type += 1)
                {
                    const MFX_SCHEDULER_TASK *pTask = m_pTasks[priority][type];

                    // look through tasks list. Find the specified object.
                    while (pTask)
                    {
                        // the task with equal ID has been found.
                        if ((pTask->param.task.pOwner == pOwner) &&
                            (MFX_WRN_IN_EXECUTION == pTask->opRes))
                        {
                            // create wait handle to make waiting more precise
                            waitHandle.taskID = pTask->taskID;
                            waitHandle.jobID = pTask->jobID;
                            break;
                        }

                        // get the next task
                        pTask = pTask->pNext;
                    }
                }

            } while ((0 == waitHandle.handle) && (--priority >= MFX_PRIORITY_LOW));

            // leave the protected section
        }

        // wait for a while :-)
        // varistar - for a while and infinite - diffrent things )
        if (waitHandle.handle)
        {
            Synchronize(waitHandle, MFX_TIME_TO_WAIT);
        }

    } while (waitHandle.handle);

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::WaitForTaskCompletion(const void *pOwner)

mfxStatus mfxSchedulerCore::ResetWaitingStatus(const void *pOwner)
{
    // reset 'waiting' tasks belong to the given state
    ResetWaitingTasks(pOwner);

    // wake up sleeping threads
    WakeUpThreads();

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::ResetWaitingStatus(const void *pOwner)

mfxStatus mfxSchedulerCore::GetState(void)
{
    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::GetState(void)

mfxStatus mfxSchedulerCore::GetParam(MFX_SCHEDULER_PARAM *pParam)
{
    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }
    if (NULL == pParam)
    {
        return MFX_ERR_NULL_PTR;
    }

    // copy the parameters
    *pParam = m_param;

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::GetParam(MFX_SCHEDULER_PARAM *pParam)

mfxStatus mfxSchedulerCore::Reset(void)
{
    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }

    if (NULL == m_pFailedTasks)
    {
        return MFX_ERR_NONE;
    }

    // enter guarded section
    {
        UMC::AutomaticUMCMutex guard(m_guard);

        // clean up the working queue
        ScrubCompletedTasks(true);
    }

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::Reset(void)

mfxStatus mfxSchedulerCore::AdjustPerformance(const mfxSchedulerMessage message)
{
    mfxStatus mfxRes = MFX_ERR_NONE;

    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }

    switch(message)
    {
        // reset the scheduler to the performance default state
    case MFX_SCHEDULER_RESET_TO_DEFAULTS:
        break;

    case MFX_SCHEDULER_START_HW_LISTENING:
        if (0 == vm_thread_is_valid(&m_hwWakeUpThread))
        {
            mfxRes = StartWakeUpThread();
        }
        break;

    case MFX_SCHEDULER_STOP_HW_LISTENING:
        if (vm_thread_is_valid(&m_hwWakeUpThread))
        {
            mfxRes = StopWakeUpThread();
        }
        break;

        // unknown message
    default:
        mfxRes = MFX_ERR_UNKNOWN;
        break;
    }

    return mfxRes;

} // mfxStatus mfxSchedulerCore::AdjustPerformance(const mfxSchedulerMessage message)

#if defined(MFX_SCHEDULER_LOG)

const
char *taskType[] =
{
    "UNKNOWN (0)",
    "MFX_TASK_THREADING_INTRA",
    "MFX_TASK_THREADING_INTER",
    "UNKNOWN (3)",
    "UNKNOWN (4)",
    "MFX_TASK_THREADING_DEDICATED",
    "UNKNOWN (6)",
    "UNKNOWN (7)",
    "MFX_TASK_THREADING_SHARED",
    "UNKNOWN (9)",
    "UNKNOWN (10)",
    "UNKNOWN (11)",
    "UNKNOWN (12)",
    "UNKNOWN (13)",
    "UNKNOWN (14)",
    "UNKNOWN (15)",
    "UNKNOWN (16)",
    "UNKNOWN (17)",
    "UNKNOWN (18)",
    "UNKNOWN (19)",
    "UNKNOWN (20)",
    "UNKNOWN (21)",
    "MFX_TASK_THREADING_DEDICATED_WAIT"
};

void GetNumTasks(MFX_SCHEDULER_TASK * const (ppTasks[MFX_PRIORITY_NUMBER][MFX_TYPE_NUMBER]), TASK_STAT &stat)
{
    mfxU32 priority;

    memset(&stat, 0, sizeof(stat));

    for (priority = 0; priority < MFX_PRIORITY_NUMBER; priority += 1)
    {
        int type;

        for (type = MFX_TYPE_HARDWARE; type <= MFX_TYPE_SOFTWARE; type += 1)
        {
            const MFX_SCHEDULER_TASK *pTask = ppTasks[priority][type];

            // run over the tasks with particular priority
            while (pTask)
            {
                stat.total += 1;
                if ((MFX_WRN_IN_EXECUTION == pTask->curStatus) ||
                    (MFX_TASK_WORKING == pTask->curStatus) ||
                    (MFX_TASK_BUSY == pTask->curStatus))
                {
                    if (pTask->IsDependenciesResolved())
                    {
                        stat.ready += 1;
                    }
                    else
                    {
                        stat.waiting += 1;
                    }
                }
                else if (isFailed(pTask->curStatus))
                {
                    stat.failed += 1;
                }
                else
                {
                    stat.done += 1;
                }

                // advance the task pointer
                pTask = pTask->pNext;
            }
        }
    }

} // void GetNumTasks(MFX_SCHEDULER_TASK * const (ppTasks[MFX_PRIORITY_NUMBER][MFX_TYPE_NUMBER]), TASK_STAT &stat)

#endif // defined(MFX_SCHEDULER_LOG)

mfxStatus mfxSchedulerCore::AddTask(const MFX_TASK &task, mfxSyncPoint *pSyncPoint,
                                    const char *pFileName, int lineNumber)
{
#ifdef MFX_TRACE_ENABLE
    MFX_LTRACE_1(MFX_TRACE_LEVEL_SCHED, "^Enqueue^", "%d", task.nTaskId);
#endif
    Ipp32u numThreads;
    mfxU32 requiredNumThreads;

#if defined  (MFX_VA)
#if defined  (MFX_D3D11_ENABLED)
    if (!m_pdx11event && !m_hwTaskDone.handle)
    {
        m_pdx11event = new DX11GlobalEvent(m_param.pCore);
        m_hwTaskDone.handle = m_pdx11event->CreateBatchBufferEvent();

        //back original sleep
        if (m_hwTaskDone.handle)
            m_zero_thread_wait = 15;

        StartWakeUpThread();
    }
#endif
#endif

    // check error(s)
    if (0 == m_param.numberOfThreads)
    {
        return MFX_ERR_NOT_INITIALIZED;
    }
    if ((NULL == task.entryPoint.pRoutine) ||
        (NULL == pSyncPoint))
    {
        return MFX_ERR_NULL_PTR;
    }

    // make sure that there is enough free task objects
    m_freeTasks.Wait();

    // enter protected section
    {
        UMC::AutomaticUMCMutex guard(m_guard);
        mfxStatus mfxRes;
        MFX_SCHEDULER_TASK *pTask, **ppTemp;
        mfxTaskHandle handle;
        MFX_THREAD_ASSIGNMENT *pAssignment = NULL;
        mfxU32 occupancyIdx;
        int type;

        // Make sure that there is an empty task object

        mfxRes = AllocateEmptyTask();
        if (MFX_ERR_NONE != mfxRes)
        {
            // better to return error instead of WRN  (two-tasks per component scheme)
            return MFX_ERR_MEMORY_ALLOC;
        }

        // initialize the task
        m_pFreeTasks->ResetDependency();
        mfxRes = m_pFreeTasks->Reset();
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }
        m_pFreeTasks->param.task = task;
        mfxRes = GetOccupancyTableIndex(occupancyIdx, &task);
        if (MFX_ERR_NONE != mfxRes)
        {
            return mfxRes;
        }
        pAssignment = &(m_occupancyTable[occupancyIdx]);

        // update the thread assignment parameters
        if (MFX_TASK_INTRA & task.threadingPolicy)
        {
            // last entries in the dependency arrays must be empty
            if ((m_pFreeTasks->param.task.pSrc[MFX_TASK_NUM_DEPENDENCIES - 1]) ||
                (m_pFreeTasks->param.task.pDst[MFX_TASK_NUM_DEPENDENCIES - 1]))
            {
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            // fill INTRA task dependencies
            m_pFreeTasks->param.task.pSrc[MFX_TASK_NUM_DEPENDENCIES - 1] = pAssignment->pLastTask;
            m_pFreeTasks->param.task.pDst[MFX_TASK_NUM_DEPENDENCIES - 1] = m_pFreeTasks;
            // update the last intra task pointer
            pAssignment->pLastTask = m_pFreeTasks;
        }
        // do not save the pointer to thread assigment instance
        // until all checking have been done
        m_pFreeTasks->param.pThreadAssignment = pAssignment;
        pAssignment->m_numRefs += 1;

        // saturate the number of available threads
        numThreads = m_pFreeTasks->param.task.entryPoint.requiredNumThreads;
        numThreads = (0 == numThreads) ? (m_param.numberOfThreads) : (numThreads);
        numThreads = UMC::get_min(m_param.numberOfThreads, numThreads);
        numThreads = (Ipp32u) UMC::get_min(sizeof(pAssignment->threadMask) * 8, numThreads);
        m_pFreeTasks->param.task.entryPoint.requiredNumThreads = numThreads;

        // set the advanced task's info
        m_pFreeTasks->param.sourceInfo.pFileName = pFileName;
        m_pFreeTasks->param.sourceInfo.lineNumber = lineNumber;
        // set the sync point for the task
        handle.taskID = m_pFreeTasks->taskID;
        handle.jobID = m_pFreeTasks->jobID;
        *pSyncPoint = (mfxSyncPoint) handle.handle;

        // Register task dependencies
        RegisterTaskDependencies(m_pFreeTasks);

#if defined(MFX_SCHEDULER_LOG)
#if defined(WIN32) || defined(WIN64)
        mfxLogWriteA(m_hLog,
                     "[% 4u]  ADD %s task - %u, job - %u (priority - %d)\n",
                     GetCurrentThreadId(),
                     taskType[m_pFreeTasks->param.task.threadingPolicy],
                     m_pFreeTasks->taskID, m_pFreeTasks->jobID,
                     GetThreadPriority(GetCurrentThread()));
#elif defined(LINUX32) || defined(LINUX64)
        mfxLogWriteA(m_hLog,
                     "[% 4u]  ADD %s task - %u, job - %u (priority - %d)\n",
                     syscall(SYS_gettid),
                     taskType[m_pFreeTasks->param.task.threadingPolicy],
                     m_pFreeTasks->taskID, m_pFreeTasks->jobID,
                     syscall(SYS_getpriority));
#endif
        TASK_STAT stat;

        GetNumTasks(m_pTasks, stat);
        stat.total += 1;
        if (m_pFreeTasks->IsDependenciesResolved())
        {
            stat.ready += 1;
        }
        else
        {
            stat.waiting += 1;
        }
        mfxLogWriteA(m_hLog,
                     "[    ] Tasks: total - %u, ready - %u, waiting - %u, done - %u, failed - %u\n",
                     stat.total,
                     stat.ready,
                     stat.waiting,
                     stat.done,
                     stat.failed);
#endif // defined(MFX_SCHEDULER_LOG)

        //
        // move task to the corresponding task
        //

        // remove the task from the 'free' queue
        pTask = m_pFreeTasks;
        m_pFreeTasks = m_pFreeTasks->pNext;
        pTask->pNext = NULL;

        // find the end of the corresponding queue
        type = (task.threadingPolicy & MFX_TASK_DEDICATED) ? (MFX_TYPE_HARDWARE) : (MFX_TYPE_SOFTWARE);
        ppTemp = m_pTasks[task.priority] + type;
        while (*ppTemp)
        {
            ppTemp = &((*ppTemp)->pNext);
        }

        // add the task to the end of the corresponding queue
        *ppTemp = pTask;

        // reset all 'waiting' tasks to prevent freezing
        // so called 'permanent' tasks.
        ResetWaitingTasks(pTask->param.task.pOwner);

        // increment the number of available tasks
        (MFX_TASK_DEDICATED & task.threadingPolicy) ? (m_numHwTasks += 1) : (m_numSwTasks += 1);

        requiredNumThreads = GetNumResolvedSwTasks();
        // leave the protected section
    }

    // if there are tasks for waiting threads, then wake up these threads
    WakeUpNumThreads(requiredNumThreads, (mfxU32)MFX_INVALID_THREAD_ID);

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::AddTask(const MFX_TASK &task, mfxSyncPoint *pSyncPoint,
