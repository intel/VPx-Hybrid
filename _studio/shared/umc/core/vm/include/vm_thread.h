/* ****************************************************************************** *\

Copyright (C) 2003-2010 Intel Corporation.  All rights reserved.

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

File Name: vm_thread.h

\* ****************************************************************************** */

#ifndef __VM_THREAD_H__
#define __VM_THREAD_H__

#include "vm_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* IMPORTANT:
 *    The thread return value is always saved for the calling thread to
 *    collect. To avoid memory leak, always vm_thread_wait() for the
 *    child thread to terminate in the calling thread.
 */

#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WCE)
#define VM_THREAD_CALLCONVENTION __stdcall
#else
#define VM_THREAD_CALLCONVENTION
#endif

typedef Ipp32u (VM_THREAD_CALLCONVENTION * vm_thread_callback)(void *);

typedef enum
{
    VM_THREAD_PRIORITY_HIGHEST,
    VM_THREAD_PRIORITY_HIGH,
    VM_THREAD_PRIORITY_NORMAL,
    VM_THREAD_PRIORITY_LOW,
    VM_THREAD_PRIORITY_LOWEST
} vm_thread_priority;

/* Set the thread handler to an invalid value */
void vm_thread_set_invalid(vm_thread *thread);

/* Verify if the thread handler is valid */
Ipp32s vm_thread_is_valid(vm_thread *thread);

/* Create a thread. The thread is set to run at the call.
   Return 1 if successful */
Ipp32s vm_thread_create(vm_thread *thread, vm_thread_callback func, void *arg);

/* Wait until a thread exists */
void vm_thread_wait(vm_thread *thread);

/* Set thread priority. Return 1 if successful */
Ipp32s vm_thread_set_priority(vm_thread *thread, vm_thread_priority priority);

/* Close thread after all */
void vm_thread_close(vm_thread *thread);

/* Get current thread priority */
vm_thread_priority vm_get_current_thread_priority(void);

/* Set current thread priority */
void vm_set_current_thread_priority(vm_thread_priority priority);

/* Set thread's affinity mask */
void vm_set_thread_affinity_mask(vm_thread *thread, unsigned int mask);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_THREAD_H__ */
