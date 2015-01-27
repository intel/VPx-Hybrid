/* ****************************************************************************** *\

Copyright (C) 2003-2009 Intel Corporation.  All rights reserved.

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

File Name: vm_semaphore.h

\* ****************************************************************************** */

#ifndef __VM_SEMAPHORE_H__
#define __VM_SEMAPHORE_H__

#include "vm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Invalidate a semaphore */
void vm_semaphore_set_invalid(vm_semaphore *sem);

/* Verify if a semaphore is valid */
Ipp32s vm_semaphore_is_valid(vm_semaphore *sem);

/* Init a semaphore with value, return VM_OK if successful */
vm_status vm_semaphore_init(vm_semaphore *sem, Ipp32s init_count);
vm_status vm_semaphore_init_max(vm_semaphore *sem, Ipp32s init_count, Ipp32s max_count);

/* Decrease the semaphore value with blocking. */
vm_status vm_semaphore_timedwait(vm_semaphore *sem, Ipp32u msec);

/* Decrease the semaphore value with blocking. */
vm_status vm_semaphore_wait(vm_semaphore *sem);

/* Decrease the semaphore value without blocking, return VM_OK if success */
vm_status vm_semaphore_try_wait(vm_semaphore *sem);

/* Increase the semaphore value */
vm_status vm_semaphore_post(vm_semaphore *sem);

/* Increase the semaphore value */
vm_status vm_semaphore_post_many(vm_semaphore *sem, Ipp32s post_count);

/* Destroy a semaphore */
void vm_semaphore_destroy(vm_semaphore *sem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_SEMAPHORE_H__ */
