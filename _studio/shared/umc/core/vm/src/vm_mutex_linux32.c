/* ****************************************************************************** *\

Copyright (C) 2003-2013 Intel Corporation.  All rights reserved.

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

File Name: vm_mutex_linux32.c

\* ****************************************************************************** */

#if defined(LINUX32) || defined(__APPLE__)

#define _GNU_SOURCE /* may need on some OS to support PTHREAD_MUTEX_RECURSIVE */

#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "vm_mutex.h"

static void vm_mutex_set_invalid_internal(vm_mutex *mutex)
{
    memset(mutex, 0, sizeof(vm_mutex));
}

/* Invalidate a mutex */
void vm_mutex_set_invalid(vm_mutex *mutex)
{
    /* check error(s) */
    if (NULL == mutex)
        return;

    vm_mutex_set_invalid_internal(mutex);

} /* void vm_mutex_set_invalid(vm_mutex *mutex) */

/* Verify if a mutex is valid */
Ipp32s vm_mutex_is_valid(vm_mutex *mutex)
{
    /* check error(s) */
    if (NULL == mutex)
        return 0;

    return mutex->is_valid;

} /* Ipp32s vm_mutex_is_valid(vm_mutex *mutex) */

/* Init a mutex, return 1 if success */
vm_status vm_mutex_init(vm_mutex *mutex)
{
    pthread_mutexattr_t mutex_attr;
    int res = 0;

    /* check error(s) */
    if (NULL == mutex)
        return VM_NULL_PTR;

    vm_mutex_destroy(mutex);

    res = pthread_mutexattr_init(&mutex_attr);
    if (!res)
    {
        res = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
        /* in Media SDK we definetely need recursive mutex */
        if (!res)
        {
            res = pthread_mutex_init(&mutex->handle, &mutex_attr);
            mutex->is_valid = !res;
            if (res)
            {
                vm_mutex_set_invalid_internal(mutex);
            }
        }
        pthread_mutexattr_destroy(&mutex_attr);
    }
    return (res)? VM_OPERATION_FAILED: VM_OK;

} /* vm_status vm_mutex_init(vm_mutex *mutex) */

/* Lock the mutex with blocking. */
vm_status vm_mutex_lock(vm_mutex *mutex)
{
    vm_status umc_res = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == mutex)
        return VM_NULL_PTR;

    if (mutex->is_valid)
    {
        if (0 == pthread_mutex_lock(&mutex->handle))
            umc_res = VM_OK;
        else
            umc_res = VM_OPERATION_FAILED;
    }
    return umc_res;

} /* vm_status vm_mutex_lock(vm_mutex *mutex) */

/* Unlock the mutex. */
vm_status vm_mutex_unlock(vm_mutex *mutex)
{
    vm_status umc_res = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == mutex)
        return VM_NULL_PTR;

    if (mutex->is_valid)
    {
        if (0 == pthread_mutex_unlock(&mutex->handle))
            umc_res = VM_OK;
        else
            umc_res = VM_OPERATION_FAILED;
    }
    return umc_res;

} /* vm_status vm_mutex_unlock(vm_mutex *mutex) */

/* Lock the mutex without blocking, return 1 if success */
vm_status vm_mutex_try_lock(vm_mutex *mutex)
{
    vm_status umc_res = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == mutex)
        return VM_NULL_PTR;

    if (mutex->is_valid)
    {
        Ipp32s i_res = pthread_mutex_trylock(&mutex->handle);
        switch (i_res)
        {
        case 0:
            umc_res = VM_OK;
            break;

        case EBUSY:
            umc_res = VM_TIMEOUT;
            break;

        default:
            umc_res = VM_OPERATION_FAILED;
            break;
        }
    }
    return umc_res;

} /* vm_status vm_mutex_try_lock(vm_mutex *mutex) */

/* Destroy the mutex */
void vm_mutex_destroy(vm_mutex *mutex)
{
    /* check error(s) */
    if (NULL == mutex)
        return;

    if (mutex->is_valid)
    {
        pthread_mutex_destroy(&mutex->handle);
        vm_mutex_set_invalid_internal(mutex);
    }
} /* void vm_mutex_destroy(vm_mutex *mutex) */
#else
# pragma warning( disable: 4206 )
#endif /* LINUX32 */
