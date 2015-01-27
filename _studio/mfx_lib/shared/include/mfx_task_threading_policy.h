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

File Name: mfx_task_threading_policy.h

\* ****************************************************************************** */
#ifndef __MFX_TASK_THREADING_POLICY_H
#define __MFX_TASK_THREADING_POLICY_H

// Declare task execution flag's
enum
{
    // Task can't share processing unit with other tasks
    MFX_TASK_INTRA = 1,
    // Task can share processing unit with other tasks
    MFX_TASK_INTER = 2,
    // Task can be managed by thread 0 only
    MFX_TASK_DEDICATED = 4,
    // Task share executing threads with other tasks
    MFX_TASK_SHARED = 8,
    // Task is waiting for result of another task.
    // it doesn't have its own return value.
    MFX_TASK_WAIT = 16

};

typedef
enum mfxTaskThreadingPolicy
{
    // The plugin doesn't support parallel task execution.
    // Tasks need to be processed one by one.
    MFX_TASK_THREADING_INTRA = MFX_TASK_INTRA,

    // The plugin supports parallel task execution.
    // Tasks can be processed independently.
    MFX_TASK_THREADING_INTER = MFX_TASK_INTER,

    // All task marked 'dedicated' is executed by thread #0 only.
    MFX_TASK_THREADING_DEDICATED = MFX_TASK_DEDICATED | MFX_TASK_INTRA,

    // As inter, but the plugin has limited processing resources.
    // The total number of threads is limited.
    MFX_TASK_THREADING_SHARED = MFX_TASK_SHARED,

    // Tasks of this type are 'waiting' tasks
    MFX_TASK_THREADING_DEDICATED_WAIT = MFX_TASK_WAIT | MFX_TASK_DEDICATED | MFX_TASK_INTRA,

    // Tasks of this type can't be executed by thread #0.
    MFX_TASK_THREADING_WAIT = MFX_TASK_WAIT | MFX_TASK_INTRA,

#ifdef MFX_VA
    MFX_TASK_THREADING_DEFAULT = MFX_TASK_THREADING_DEDICATED
#else // !MFX_VA
    MFX_TASK_THREADING_DEFAULT = MFX_TASK_THREADING_INTRA
#endif // MFX_VA

} mfxTaskThreadingPolicy;

#endif // __MFX_TASK_THREADING_POLICY_H
