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

File Name: mfx_scheduler_core_handle.h

\* ****************************************************************************** */

#if !defined(__MFX_SCHEDULER_CORE_HANDLE_H)
#define __MFX_SCHEDULER_CORE_HANDLE_H

#include "vm_types.h"

enum
{
    MFX_BITS_FOR_HANDLE = 32,

    // declare constans for task objects
    MFX_BITS_FOR_TASK_NUM = 10,
    MFX_MAX_NUMBER_TASK = 1 << MFX_BITS_FOR_TASK_NUM,

    // declare constans for job objects
    MFX_BITS_FOR_JOB_NUM = MFX_BITS_FOR_HANDLE - MFX_BITS_FOR_TASK_NUM,
    MFX_MAX_NUMBER_JOB = 1 << MFX_BITS_FOR_JOB_NUM
};

// Type mfxTaskHandle is a composite type,
// which structure is close to a handle structure.
// Few LSBs are used for internal task object indentification.
// Rest bits are used to identify job.
typedef
union mfxTaskHandle
{
    struct
    {
    unsigned int taskID : MFX_BITS_FOR_TASK_NUM;
    unsigned int jobID : MFX_BITS_FOR_JOB_NUM;
    };

    size_t handle;

} mfxTaskHandle;

#endif // !defined(__MFX_SCHEDULER_CORE_HANDLE_H)
