/* ****************************************************************************** *\

Copyright (C) 2009 Intel Corporation.  All rights reserved.

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

File Name: mfx_scheduler_core_thread.h

\* ****************************************************************************** */

#if !defined (__MFX_SCHEDULER_CORE_THREAD_H)
#define __MFX_SCHEDULER_CORE_THREAD_H

#include <mfxdefs.h>

#include <vm_thread.h>

// forward declaration of the owning class
class mfxSchedulerCore;

struct MFX_SCHEDULER_THREAD_CONTEXT
{
    mfxSchedulerCore *pSchedulerCore;                           // (mfxSchedulerCore *) pointer to the owning core
    mfxU32 threadNum;                                           // (mfxU32) number of the thread
    vm_thread threadHandle;                                     // (vm_thread) handle to the thread

    mfxU64 workTime;                                            // (mfxU64) integral working time
    mfxU64 sleepTime;                                           // (mfxU64) integral sleeping time
};

#endif // !defined (__MFX_SCHEDULER_CORE_THREAD_H)
