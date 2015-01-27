/* ****************************************************************************** *\

Copyright (C) 2010-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_task.h

\* ****************************************************************************** */
#ifndef __MFX_TASK_H
#define __MFX_TASK_H

#include <mfxplugin.h>
#include <mfx_thread_task.h>
#include <mfx_task_threading_policy.h>

enum
{
    // Declare the maximum number of source or destination dependencies
    MFX_TASK_NUM_DEPENDENCIES   = 4
};

enum
{
    // Declare overall number of priorities
    MFX_PRIORITY_NUMBER = MFX_PRIORITY_HIGH + 1

};

// Declare implementation types
enum
{
    MFX_TYPE_HARDWARE = 0,
    MFX_TYPE_SOFTWARE = 1,
    MFX_TYPE_NUMBER
};

#define MFX_TASK_NEED_CONTINUE MFX_TASK_WORKING

enum
{
    MFX_TRACE_ID_DECODE  = 0x10000000,
    MFX_TRACE_ID_VPP     = 0x20000000,
    MFX_TRACE_ID_VPP2    = 0x30000000,
    MFX_TRACE_ID_ENCODE  = 0x40000000,
    MFX_TRACE_ID_ENCODE2 = 0x50000000,
    MFX_TRACE_ID_USER    = 0x60000000,
};

inline
bool isFailed(mfxStatus mfxRes)
{
    return (MFX_ERR_NONE > mfxRes);

} // bool isFailed(mfxStatus mfxRes)

/* declare type for MFX task callbacks.
Usually, most thread API involve a single parameter.
MFX callback function require 3 parameters to make developers' life.
pState - is a pointer to the performing object or something like this.
pParam - is an addition parameter to destingush one task from the another inside
    the entry point. It is possible to start different threads with the same
    object (pState), but different task (pParam). Usually, it is a handle or some
    internal pointer.
threadNumber - number of the current additional thread. It can be any number
    not exceeding the maximum required threads for given task. */
typedef mfxStatus (*mfxTaskRoutine) (void *pState, void *pParam, mfxU32 threadNumber, mfxU32 callNumber);
typedef mfxStatus (*mfxTaskCompleteProc) (void *pState, void *pParam, mfxStatus taskRes);
typedef mfxStatus (*mfxGetSubTaskProc) (void *pState, void *pParam, void **ppSubTask);
typedef mfxStatus (*mfxCompleteSubTaskProc) (void *pState, void *pParam, void *pSubTask, mfxStatus taskRes);

typedef
struct MFX_ENTRY_POINT
{
    // pointer to the task processing object
    void *pState;
    // pointer to the task's parameter
    void *pParam;

    // pointer to the task processing routine
    mfxTaskRoutine pRoutine;
    // pointer to the task completing procedure
    mfxTaskCompleteProc pCompleteProc;

    // pointer to get a sub-task from the component (NON-OBLIGATORY)
    mfxGetSubTaskProc pGetSubTaskProc;
    // sub-task is complete. Update the status of it (NON-OBLIGATORY)
    mfxCompleteSubTaskProc pCompleteSubTaskProc;

    // number of simultaneously allowed threads for the task
    mfxU32 requiredNumThreads;

    // name of routine - for debug and tracing purpose
    char *pRoutineName;
} MFX_ENTRY_POINT;

struct MFX_TASK
{
    // Pointer to task owning object
    void *pOwner;

    // Task parameters provided by the component
    MFX_ENTRY_POINT entryPoint;

    // legacy parameters should be eliminated
    bool bObsoleteTask;
    MFX_THREAD_TASK_PARAMETERS obsolete_params;

    // these are not a source/destination parameters,
    // these are only in/out dependencies.

    // Array of source dependencies
    const void *(pSrc[MFX_TASK_NUM_DEPENDENCIES]);
    // Array of destination dependencies
    void *(pDst[MFX_TASK_NUM_DEPENDENCIES]);

    // Task's priority
    mfxPriority priority;
    // how the object processes the tasks
    mfxTaskThreadingPolicy threadingPolicy;

    unsigned int nTaskId;
    unsigned int nParentId;
};

#endif // __MFX_TASK_H
