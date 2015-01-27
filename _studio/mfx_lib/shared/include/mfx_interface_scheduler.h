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

File Name: mfx_interface_scheduler.h

\* ****************************************************************************** */
#ifndef __MFX_INTERFACE_SCHEDULER_H
#define __MFX_INTERFACE_SCHEDULER_H

#include <mfxvideo.h>
#include <mfx_interface.h>
#include <mfxvideo++int.h>

#include <memory.h>

// {BE080281-4C93-4D26-B763-ED2AAB5D4BA1}
static const
MFX_GUID MFXIScheduler_GUID =
{ 0xbe080281, 0x4c93, 0x4d26, { 0xb7, 0x63, 0xed, 0x2a, 0xab, 0x5d, 0x4b, 0xa1 } };

enum mfxSchedulerFlags
{
    // default behaviour policy
    MFX_SCHEDULER_DEFAULT = 0

};

enum mfxSchedulerMessage
{
    // Drop any performance adjustments
    MFX_SCHEDULER_RESET_TO_DEFAULTS = 0,
    // Start listening to the HW event from the driver
    MFX_SCHEDULER_START_HW_LISTENING = 1,
    // Stop listening to the HW event from the driver
    MFX_SCHEDULER_STOP_HW_LISTENING = 2
};

#pragma pack(1)

struct MFX_SCHEDULER_PARAM
{
    // Working flags for the scheduler being initialized
    mfxSchedulerFlags flags;
    // Number of working threads
    mfxU32 numberOfThreads;
    // core interface to get access to event handle in case of Metro mode
    VideoCORE  *pCore;
};

#pragma pack()

// Forward declaration of used classes
struct MFX_TASK;
//class VideoCORE;

// MFXIScheduler interface.
// The interface provides task management and execution functionality.

class MFXIScheduler : public MFXIUnknown
{
public:

    virtual ~MFXIScheduler(void){}
    // Initialize the scheduler. Initialize the dependency tables and run threads.
    virtual
    mfxStatus Initialize(const MFX_SCHEDULER_PARAM *pParam = 0) = 0;

    // Add a new task to the scheduler. Threads start processing task immediately.
    virtual
    mfxStatus AddTask(const MFX_TASK &task, mfxSyncPoint *pSyncPoint) = 0;

    // Make synchronization, wait until task is done.
    virtual
    mfxStatus Synchronize(mfxSyncPoint syncPoint, mfxU32 timeToWait) = 0;

    // Wait until specified dependency become resolved
    virtual
    mfxStatus WaitForDependencyResolved(const void *pDependency) = 0;

    // Wait until task(s) of specified owner become complete or unattended
    virtual
    mfxStatus WaitForTaskCompletion(const void *pOwner) = 0;

    // Reset 'waiting' status for tasks of specified owner
    virtual
    mfxStatus ResetWaitingStatus(const void *pOwner) = 0;

    // Check the current status of the scheduler.
    virtual
    mfxStatus GetState(void) = 0;

    // Get the initialization parameters of the scheduler
    virtual
    mfxStatus GetParam(MFX_SCHEDULER_PARAM *pParam) = 0;

    // Recover from the failure.
    virtual
    mfxStatus Reset(void) = 0;

    // Send a performance message to the scheduler
    virtual
    mfxStatus AdjustPerformance(const mfxSchedulerMessage message) = 0;


#if defined(SCHEDULER_DEBUG)

    virtual
    mfxStatus AddTask(const MFX_TASK &task, void **ppSyncPoint, const char *pFileName, int lineNumber) = 0;

#define AddTask(task, ppSyncPoint) \
    AddTask(task, ppSyncPoint, __FILE__, __LINE__)

#endif // defined(SCHEDULER_DEBUG)
};

#endif // __MFX_INTERFACE_SCHEDULER_H
