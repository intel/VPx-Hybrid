/* ****************************************************************************** *\

Copyright (C) 2003-2007 Intel Corporation.  All rights reserved.

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

File Name: vm_mutex.h

\* ****************************************************************************** */

#ifndef __VM_MUTEX_H__
#define __VM_MUTEX_H__

#include "vm_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Invalidate a mutex */
void vm_mutex_set_invalid(vm_mutex *mutex);

/* Verify if a mutex is valid */
Ipp32s  vm_mutex_is_valid(vm_mutex *mutex);

/* Init a mutex, return VM_OK if success */
vm_status vm_mutex_init(vm_mutex *mutex);

/* Lock the mutex with blocking. */
vm_status vm_mutex_lock(vm_mutex *mutex);

/* Unlock the mutex. */
vm_status vm_mutex_unlock(vm_mutex *mutex);

/* Lock the mutex without blocking, return VM_OK if success */
vm_status vm_mutex_try_lock(vm_mutex *mutex);

/* Destroy a mutex */
void vm_mutex_destroy(vm_mutex *mutex);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_MUTEX_H__ */
