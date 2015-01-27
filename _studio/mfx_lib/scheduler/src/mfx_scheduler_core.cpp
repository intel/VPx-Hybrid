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

File Name: mfx_scheduler_core.cpp

\* ****************************************************************************** */

#include <mfx_scheduler_core.h>

#include <mfx_scheduler_core_task.h>
#include <mfx_scheduler_core_handle.h>
#include <mfx_trace.h>

#include <umc_automatic_mutex.h>
#include <vm_time.h>
#include <vm_sys_info.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif // defined(_MSC_VER)

#if defined(MFX_SCHEDULER_LOG)
#include <stdio.h>
#endif // defined(MFX_SCHEDULER_LOG)

mfxSchedulerCore::mfxSchedulerCore(void) :
#if defined(_MSC_VER)
    m_timeWaitPeriod(1000000)
#elif defined(LINUX32) && defined(SYNCHRONIZATION_BY_NON_ZERO_THREAD)
    m_timeWaitPeriod(0)
#else // !defined(_MSC_VER)
    m_timeWaitPeriod(vm_time_get_frequency() / 1000)
#endif // defined(_MSC_VER)
{
    memset(&m_param, 0, sizeof(m_param));
    m_refCounter = 1;

    memset(m_workingTime, 0, sizeof(m_workingTime));
    m_timeIdx = 0;

    m_bQuit = false;
    m_pThreadCtx = NULL;
    m_pTaskAdded = NULL;
    vm_event_set_invalid(&m_hwTaskDone);
    vm_thread_set_invalid(&m_hwWakeUpThread);

    // reset task variables
    memset(m_pTasks, 0, sizeof(m_pTasks));
    memset(m_numAssignedTasks, 0, sizeof(m_numAssignedTasks));
    m_pFailedTasks = NULL;
    m_numHwTasks = 0;
    m_numSwTasks = 0;

    m_pFreeTasks = NULL;

    // reset dependency table variables
    m_numDependencies = 0;

    // reset busy objects table
    m_numOccupancies = 0;

    // reset task counters
    m_taskCounter = 0;
    m_jobCounter = 0;

    m_hwEventCounter = 0;

    m_timer_hw_event = MFX_THREAD_TIME_TO_WAIT;

#if defined(LINUX32) && defined(SYNCHRONIZATION_BY_NON_ZERO_THREAD)
    m_zero_thread_wait = MFX_THREAD_TIME_TO_WAIT;
#else
    m_zero_thread_wait = 1;
#endif

#if defined  (MFX_VA)
#if defined  (MFX_D3D11_ENABLED)
    m_pdx11event = 0;
#endif
#endif

#if defined(MFX_SCHEDULER_LOG)
    m_hLog = 0;
#endif // defined(MFX_SCHEDULER_LOG)

} // mfxSchedulerCore::mfxSchedulerCore(void)

mfxSchedulerCore::~mfxSchedulerCore(void)
{
    Close();

} // mfxSchedulerCore::~mfxSchedulerCore(void)

void mfxSchedulerCore::Close(void)
{
    int priority;
    size_t n;

    StopWakeUpThread();

    // stop threads
    if (m_pThreadCtx)
    {
        mfxU32 i;

        // set the 'quit' flag for threads
        m_bQuit = true;

        // set the events to wake up sleeping threads
        WakeUpThreads();

        for (i = 0; i < m_param.numberOfThreads; i += 1)
        {
            // wait for particular thread
            vm_thread_wait(&(m_pThreadCtx[i].threadHandle));
            vm_thread_close(&(m_pThreadCtx[i].threadHandle));
        }

        delete [] m_pThreadCtx;
    }

    // delete the array of waiting objects
    if (m_pTaskAdded)
    {
        delete [] m_pTaskAdded;
    }

    // run over the task lists and abort the existing tasks
    for (priority = MFX_PRIORITY_HIGH;
         priority >= MFX_PRIORITY_LOW;
         priority -= 1)
    {
        int type;

        for (type = MFX_TYPE_HARDWARE; type <= MFX_TYPE_SOFTWARE; type += 1)
        {
            MFX_SCHEDULER_TASK *pTask;

            try
            {
                // get the tasks list
                pTask = m_pTasks[priority][type];
                // and run through it
                while (pTask)
                {
                    MFX_ENTRY_POINT &entryPoint = pTask->param.task.entryPoint;

                    if ((MFX_TASK_WORKING == pTask->curStatus) &&
                        (entryPoint.pCompleteProc))
                    {
                        entryPoint.pCompleteProc(entryPoint.pState,
                                                 entryPoint.pParam,
                                                 MFX_ERR_ABORTED);
                    }

                    // advance the task pointer
                    pTask = pTask->pNext;
                }
            }
            catch(...)
            {
            }
        }
    }

    // delete task objects
    for (n = 0; n < m_ppTaskLookUpTable.Size(); n += 1)
    {
        if (m_ppTaskLookUpTable[n])
        {
            delete m_ppTaskLookUpTable[n];
            m_ppTaskLookUpTable[n] = NULL;
        }
    }

#if defined(MFX_SCHEDULER_LOG)
    mfxLogClose(m_hLog);
    m_hLog = 0;
#endif // defined(MFX_SCHEDULER_LOG)

    memset(&m_param, 0, sizeof(m_param));

    memset(m_workingTime, 0, sizeof(m_workingTime));
    m_timeIdx = 0;

    // reset variables
    m_bQuit = false;
    m_pThreadCtx = NULL;
    m_pTaskAdded = NULL;

    // reset task variables
    memset(m_pTasks, 0, sizeof(m_pTasks));
    memset(m_numAssignedTasks, 0, sizeof(m_numAssignedTasks));
    m_pFailedTasks = NULL;

    m_pFreeTasks = NULL;

    // reset dependency table variables
    m_numDependencies = 0;

    // reset busy objects table
    m_numOccupancies = 0;

    // reset task counters
    m_taskCounter = 0;
    m_jobCounter = 0;

} // void mfxSchedulerCore::Close(void)

void mfxSchedulerCore::SetThreadsAffinityMask(void)
{
    mfxU32 numCpu;

    // get the actual number of threads
    numCpu = vm_sys_info_get_cpu_num();

    // simple case,
    // set one CPU per thread
    if (m_param.numberOfThreads == numCpu)
    {
        mfxU32 i;

        for (i = 0; i < m_param.numberOfThreads; i += 1)
        {
            vm_set_thread_affinity_mask(&(m_pThreadCtx[i].threadHandle), 1 << i);
        }
    }
    else
    {
        mfxU32 i;
        mfxF32 cpuPerThread = ((mfxF32) numCpu) / ((mfxF32) m_param.numberOfThreads);

        for (i = 0; i < m_param.numberOfThreads; i += 1)
        {
            vm_set_thread_affinity_mask(&(m_pThreadCtx[i].threadHandle),
                                        1 << (mfxU32) (cpuPerThread * i));
        }
    }

} // void mfxSchedulerCore::SetThreadsAffinityMask(void)

void mfxSchedulerCore::WakeUpThreads(const mfxU32 curThreadNum,
                                     const eWakeUpReason reason)
{

    // it is a common working situation, wake up only threads, which
    // have something to do
    if (false == m_bQuit)
    {
        mfxU32 i;

        // wake up the dedicated thread
        if ((curThreadNum) && (m_numHwTasks | m_numSwTasks))
        {
            m_pTaskAdded[0].Set();
        }

        // wake up other threads
        if ((MFX_SCHEDULER_NEW_TASK == reason) && (m_numSwTasks))
        {
            for (i = 1; i < m_param.numberOfThreads; i += 1)
            {
                if (i != curThreadNum)
                {
                    m_pTaskAdded[i].Set();
                }
            }
        }
    }
    // the scheduler is going to be deleted, wake up all threads
    else
    {
        mfxU32 i;

        for (i = 0; i < m_param.numberOfThreads; i += 1)
        {
            m_pTaskAdded[i].Set();
        }
    }
} // void mfxSchedulerCore::WakeUpThreads(const mfxU32 curThreadNum,

void mfxSchedulerCore::WakeUpNumThreads(mfxU32 numThreadsToWakeUp,
                                        const mfxU32 curThreadNum)
{
#if defined(SYNCHRONIZATION_BY_NON_ZERO_THREAD)
    if (false == m_bQuit)
    {
        mfxU32 i;
        // wake up the dedicated thread if there are 'dedicated' tasks
        if (curThreadNum && m_numHwTasks)
        {
            m_pTaskAdded[0].Set();
        }

        if (numThreadsToWakeUp && m_numSwTasks)
        {
            for (i = 1; 0 < numThreadsToWakeUp && i < m_param.numberOfThreads; i++)
            {
                if (i != curThreadNum)
                {
                    m_pTaskAdded[i].Set();
                    numThreadsToWakeUp--;
                }
            }
        }
    }
    // the scheduler is going to be deleted, wake up all threads
    else
    {
        mfxU32 i;

        for (i = 0; i < m_param.numberOfThreads; i += 1)
        {
            m_pTaskAdded[i].Set();
        }
    }
#else
    numThreadsToWakeUp;
    WakeUpThreads(curThreadNum);
#endif
} // void mfxSchedulerCore::WakeUpNumThreads(mfxU32 numThreadsToWakeUp,

void mfxSchedulerCore::Wait(const mfxU32 curThreadNum)
{
    // to avoid thread #0 to sleep too much
    if (0 == curThreadNum)
    {
        //MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "Wait(1)");
        m_pTaskAdded[curThreadNum].Wait(m_zero_thread_wait);
    }
    else
    {
        //MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "Wait(1000)");
        m_pTaskAdded[curThreadNum].Wait(MFX_THREAD_TIME_TO_WAIT);
    }

} // void mfxSchedulerCore::Wait(const mfxU32 curThreadNum)

mfxU64 mfxSchedulerCore::GetHighPerformanceCounter(void)
{
#if defined(_MSC_VER)
    return (mfxU64) __rdtsc();
#else // !defined(_MSC_VER)
    return (mfxU64) vm_time_get_tick();
#endif // defined(_MSC_VER)

} // mfxU64 mfxSchedulerCore::GetHighPerformanceCounter(void)

mfxU32 mfxSchedulerCore::GetLowResCurrentTime(void)
{
    return vm_time_get_current_time();

} // mfxU32 mfxSchedulerCore::GetCurrentTime(void)
mfxStatus mfxSchedulerCore::AllocateEmptyTask(void)
{
    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    // Clean up task queues
    ScrubCompletedTasks();

    // allocate one new task
    if (NULL == m_pFreeTasks)
    {

#if defined(MFX_SCHEDULER_LOG)

        mfxLogWriteA(m_hLog, "[    ] CAN'T find a free task\n");

#endif // defined(MFX_SCHEDULER_LOG)

        // the maximum allowed number of tasks is reached
        if (MFX_MAX_NUMBER_TASK <= m_taskCounter)
        {
            return MFX_WRN_DEVICE_BUSY;
        }

        // allocate one more task
        try
        {
            UMC::Status umcRes;

            m_pFreeTasks = new MFX_SCHEDULER_TASK(m_taskCounter++, this);
            umcRes = m_pFreeTasks->done.Init(1, 1);
            if (UMC::UMC_OK != umcRes)
            {
                m_taskCounter--;
                delete m_pFreeTasks;
                m_pFreeTasks = NULL;
                throw (int) 0;
            }
        }
        catch(...)
        {
            return MFX_WRN_DEVICE_BUSY;
        }
        // register the task in the look up table
        m_ppTaskLookUpTable[m_pFreeTasks->taskID] = m_pFreeTasks;
    }
    memset(&(m_pFreeTasks->param), 0, sizeof(m_pFreeTasks->param));
    // increment job number. This number must grow evenly.
    // make job number 0 an invalid value to avoid problem with
    // task number 0 with job number 0, which are NULL when being combined.
    m_jobCounter += 1;
    if (MFX_MAX_NUMBER_JOB <= m_jobCounter)
    {
        m_jobCounter = 1;
    }
    m_pFreeTasks->jobID = m_jobCounter;

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::AllocateEmptyTask(void)

mfxStatus mfxSchedulerCore::GetOccupancyTableIndex(mfxU32 &idx,
                                                   const MFX_TASK *pTask)
{
    mfxU32 i = 0;
    MFX_THREAD_ASSIGNMENT *pAssignment = NULL;

    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    // check the table, decrement the number of used entries
    while ((m_numOccupancies) &&
           (0 == m_occupancyTable[m_numOccupancies - 1].m_numRefs))
    {
        m_numOccupancies -= 1;
    }

    // find the existing element with the given pState and pRoutine
    for (i = 0; i < m_numOccupancies; i += 1)
    {
        if ((m_occupancyTable[i].pState == pTask->entryPoint.pState) &&
            (m_occupancyTable[i].pRoutine == pTask->entryPoint.pRoutine))
        {
            // check the type of other tasks using this table entry
            if (m_occupancyTable[i].threadingPolicy != pTask->threadingPolicy)
            {
                return MFX_ERR_INVALID_VIDEO_PARAM;
            }

            pAssignment = &(m_occupancyTable[i]);
            break;
        }
    }

    // if the element exist, check the parameters for compatibility
    if (pAssignment)
    {
        // actually, there is nothing to do
    }
    // allocate one more element in the array
    else
    {
        for (i = 0; i < m_numOccupancies; i += 1)
        {
            if (0 == m_occupancyTable[i].m_numRefs)
            {
                break;
            }
        }
        // we can't reallocate the table
        if (m_occupancyTable.Size() == i)
        {
            return MFX_WRN_DEVICE_BUSY;
        }

        pAssignment = &(m_occupancyTable[i]);

        // fill the parameters
        memset(pAssignment, 0, sizeof(MFX_THREAD_ASSIGNMENT));
        pAssignment->pState = pTask->entryPoint.pState;
        pAssignment->pRoutine = pTask->entryPoint.pRoutine;
        pAssignment->threadingPolicy = pTask->threadingPolicy;
    }

    // update the number of allocated objects
    m_numOccupancies = UMC::get_max(m_numOccupancies, i + 1);

    // save the index to return
    idx = i;

    return MFX_ERR_NONE;

} // mfxStatus mfxSchedulerCore::GetOccupancyTableIndex(mfxU32 &idx,

void mfxSchedulerCore::ScrubCompletedTasks(bool bComprehensive)
{
    int priority;

    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    for (priority = MFX_PRIORITY_HIGH;
         priority >= MFX_PRIORITY_LOW;
         priority -= 1)
    {
        int type;

        for (type = MFX_TYPE_HARDWARE; type <= MFX_TYPE_SOFTWARE; type += 1)
        {
            MFX_SCHEDULER_TASK **ppCur;

            // if there is an empty task, immediately return
            if ((false == bComprehensive) &&
                (m_pFreeTasks))
            {
                return;
            }

            ppCur = m_pTasks[priority] + type;
            while (*ppCur)
            {
                // move task completed to the 'free' queue.
                if (MFX_ERR_NONE == (*ppCur)->opRes)
                {
                    MFX_SCHEDULER_TASK *pTemp;

                    // cut the task from the queue
                    pTemp = *ppCur;
                    *ppCur = pTemp->pNext;
                    // add it to the 'free' queue
                    pTemp->pNext = m_pFreeTasks;
                    m_pFreeTasks = pTemp;
                }
                // move task failed to the 'failed' queue.
                else if ((MFX_ERR_NONE != (*ppCur)->opRes) &&
                         (MFX_WRN_IN_EXECUTION != (*ppCur)->opRes))
                {
                    MFX_SCHEDULER_TASK *pTemp;

                    // cut the task from the queue
                    pTemp = *ppCur;
                    *ppCur = pTemp->pNext;
                    // add it to the 'failed' queue
                    pTemp->pNext = m_pFailedTasks;
                    m_pFailedTasks = pTemp;
                }
                else
                {
                    // set the next task
                    ppCur = &((*ppCur)->pNext);
                }
            }
        }
    }

} // void mfxSchedulerCore::ScrubCompletedTasks(bool bComprehensive)

void mfxSchedulerCore::RegisterTaskDependencies(MFX_SCHEDULER_TASK  *pTask)
{
    mfxU32 i, tableIdx, remainInputs;
    const void *(pSrcCopy[MFX_TASK_NUM_DEPENDENCIES]);
    mfxStatus taskRes = MFX_WRN_IN_EXECUTION;

    //
    // THE EXECUTION IS ALREADY IN SECURE SECTION.
    // Just do what need to do.
    //

    // check if the table have empty position(s),
    // If so decrement the index of the last table entry.
    i = m_numDependencies;
    while ((0 < i) &&
           (NULL == m_pDependencyTable[i - 1].p))
    {
        i -= 1;
    }
    m_numDependencies = i;

    // get the number of source dependencies
    remainInputs = 0;
    for (i = 0; i < MFX_TASK_NUM_DEPENDENCIES; i += 1)
    {
        // make a copy of source dependencies.
        // source dependencies have to be swept, because of duplication in
        // the dependency table. task will sync on the first matching entry.
        pSrcCopy[i] = pTask->param.task.pSrc[i];
        if (pSrcCopy[i])
        {
            remainInputs += 1;
        }
    }

    // run over the table and save the handles of incomplete inputs
    for (tableIdx = 0; tableIdx < m_numDependencies; tableIdx += 1)
    {
        // compare only filled table entries
        if (m_pDependencyTable[tableIdx].p)
        {
            for (i = 0; i < MFX_TASK_NUM_DEPENDENCIES; i += 1)
            {
                // we found the source dependency,
                // save the handle
                if (m_pDependencyTable[tableIdx].p == pSrcCopy[i])
                {
                    // dependency is fail. The dependency resolved, but failed.
                    if (MFX_WRN_IN_EXECUTION != m_pDependencyTable[tableIdx].mfxRes)
                    {
                        // waiting task inherits status from the parent task
                        if (MFX_TASK_WAIT & pTask->param.task.threadingPolicy)
                        {
                            taskRes = m_pDependencyTable[tableIdx].mfxRes;
                        }
                        // all other tasks are aborted
                        else
                        {
                            taskRes = MFX_ERR_ABORTED;
                        }
                    }
                    // link dependency
                    else
                    {
                        m_pDependencyTable[tableIdx].pTask->SetDependentItem(pTask, i);
                    }
                    // sweep already used dependency
                    pSrcCopy[i] = NULL;
                    remainInputs -= 1;
                    break;
                }
            }

            // is there more source dependencies?
            if (0 == remainInputs)
            {
                break;
            }
        }
    }

    // run over the table and register generated outputs
    tableIdx = 0;
    for (i = 0; i < MFX_TASK_NUM_DEPENDENCIES; i += 1)
    {
        if (pTask->param.task.pDst[i])
        {
            // find empty table entry
            while (m_pDependencyTable[tableIdx].p)
            {
                tableIdx += 1;
            }

            // save the generated dependency
            m_pDependencyTable[tableIdx].p = pTask->param.task.pDst[i];
            m_pDependencyTable[tableIdx].mfxRes = taskRes;
            m_pDependencyTable[tableIdx].pTask = pTask;

            // save the index of the output
            pTask->param.dependencies.dstIdx[i] = tableIdx;
            tableIdx += 1;
        }
    }

    // update the dependency table max index
    if (tableIdx >= m_numDependencies)
    {
        m_numDependencies = tableIdx;
    }

    // if dependency were failed,
    // set the task into the 'aborted' state
    if (MFX_WRN_IN_EXECUTION != taskRes)
    {
        // save the status
        m_pFreeTasks->curStatus = taskRes;
        m_pFreeTasks->opRes = taskRes;
        m_pFreeTasks->done.Set();
    }

} // void mfxSchedulerCore::RegisterTaskDependencies(MFX_SCHEDULER_TASK  *pTask)



//#define ENABLE_TASK_DEBUG

#if defined(ENABLE_TASK_DEBUG)
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define sizeLeft (sizeof(cStr) - (pStr - cStr))
#endif // defined(ENABLE_TASK_DEBUG)

void mfxSchedulerCore::PrintTaskInfo(void)
{
#if defined(ENABLE_TASK_DEBUG)
    UMC::AutomaticUMCMutex guard(m_guard);

    PrintTaskInfoUnsafe();
#endif // defined(ENABLE_TASK_DEBUG)

} // void mfxSchedulerCore::PrintTaskInfo(void)

void mfxSchedulerCore::PrintTaskInfoUnsafe(void)
{
#if defined(ENABLE_TASK_DEBUG)
    mfxU32 priority;
    char cStr[4096];
    char *pStr = cStr;
    int written;
    mfxU32 numTasks = 0;

    written = sprintf_s(pStr, sizeLeft, "[% 4u] TASK DEBUG. Current time %u:\n", GetCurrentThreadId(), (mfxU32) m_currentTimeStamp);
    if (0 < written) pStr += written;

    // run over priority queues
    for (priority = 0; priority < MFX_PRIORITY_NUMBER; priority += 1)
    {
        MFX_SCHEDULER_TASK *pTask = m_pTasks[priority];

        while (pTask)
        {
            if (MFX_TASK_WORKING == pTask->curStatus)
            {
                written = sprintf_s(pStr, sizeLeft, "    task - % 2u, job - % 4u, %s, occupancy %u, last enter %u (%u us ago)\n",
                                    pTask->taskID, pTask->jobID,
                                    pTask->param.bWaiting ? ("waiting") : ("non-waiting"),
                                    pTask->param.occupancy,
                                    (mfxU32) pTask->param.timing.timeLastEnter,
                                    (mfxU32) ((m_currentTimeStamp - pTask->param.timing.timeLastEnter) * 1000000 / m_timeFrequency));
                if (0 < written) pStr += written;

                // increment the number of available tasks
                numTasks += 1;
            }

            // advance the task pointer
            pTask = pTask->pNext;
        }
    }

    written = sprintf_s(pStr, sizeLeft, "    %u task total\n", numTasks);
    if (0 < written) pStr += written;
    OutputDebugStringA(cStr);
#endif // defined(ENABLE_TASK_DEBUG)

} // void mfxSchedulerCore::PrintTaskInfoUnsafe(void)
