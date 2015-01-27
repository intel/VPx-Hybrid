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

File Name: vm_semaphore_linux32.c

\* ****************************************************************************** */

#if   defined(LINUX32) || defined(__APPLE__)

#include <sys/time.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "vm_semaphore.h"

static void vm_semaphore_set_invalid_internal(vm_semaphore *sem)
{
    memset(sem, 0, sizeof(vm_semaphore));
    sem->count = -1;
}

/* Invalidate a semaphore */
void vm_semaphore_set_invalid(vm_semaphore *sem)
{
    /* check error(s) */
    if (NULL == sem)
        return;

    vm_semaphore_set_invalid_internal(sem);

} /* void vm_semaphore_set_invalid(vm_semaphore *sem) */

/* Verify if a semaphore is valid */
Ipp32s vm_semaphore_is_valid(vm_semaphore *sem)
{
    /* check error(s) */
    if (NULL == sem)
        return 0;

    return (-1 < sem->count);

} /* Ipp32s vm_semaphore_is_valid(vm_semaphore *sem) */


/* Init a semaphore with value */
vm_status vm_semaphore_init(vm_semaphore *sem, Ipp32s init_count)
{
    int res = 0;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    sem->count = init_count;
    sem->max_count = 1;
    res = pthread_cond_init(&sem->cond, 0);
    if (!res)
    {
        res = pthread_mutex_init(&sem->mutex, 0);
        if (res)
        {
            pthread_cond_destroy(&sem->cond);
            vm_semaphore_set_invalid_internal(sem);
        }
    }

    return (res)? VM_OPERATION_FAILED: VM_OK;

} /* vm_status vm_semaphore_init(vm_semaphore *sem, Ipp32s init_count) */

vm_status vm_semaphore_init_max(vm_semaphore *sem, Ipp32s init_count, Ipp32s max_count)
{
    int res = 0;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    sem->count = init_count;
    sem->max_count = max_count;
    res = pthread_cond_init(&sem->cond, 0);
    if (!res)
    {
        res = pthread_mutex_init(&sem->mutex, 0);
        if (res)
        {
            pthread_cond_destroy(&sem->cond);
            vm_semaphore_set_invalid_internal(sem);
        }
    }

    return (res)? VM_OPERATION_FAILED: VM_OK;

} /* vm_status vm_semaphore_init_max(vm_semaphore *sem, Ipp32s init_count, Ipp32s max_count) */

/* Decrease the semaphore value with blocking. */
vm_status vm_semaphore_timedwait(vm_semaphore *sem, Ipp32u msec)
{
    vm_status umc_status = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (0 <= sem->count)
    {
        umc_status = VM_OK;
        Ipp32s i_res = 0;

        i_res = pthread_mutex_lock(&sem->mutex);
        if (!i_res)
        {
            if (0 == sem->count)
            {
                struct timeval tval;
                struct timespec tspec;

                gettimeofday(&tval, NULL);
                msec = 1000 * msec + tval.tv_usec;
                tspec.tv_sec = tval.tv_sec + msec / 1000000;
                tspec.tv_nsec = (msec % 1000000) * 1000;
                i_res = pthread_cond_timedwait(&sem->cond, &sem->mutex, &tspec);

                if (ETIMEDOUT == i_res)
                    umc_status = VM_TIMEOUT;
                else if (0 != i_res)
                    umc_status = VM_OPERATION_FAILED;
            }

            if (VM_OK == umc_status)
                sem->count--;

            if (pthread_mutex_unlock(&sem->mutex))
            {
                if (VM_OK == umc_status)
                    umc_status = VM_OPERATION_FAILED;
            }
        }
        else
            umc_status = VM_OPERATION_FAILED;
    }
    return umc_status;

} /* vm_status vm_semaphore_timedwait(vm_semaphore *sem, Ipp32u msec) */

/* Decrease the semaphore value with blocking. */
vm_status vm_semaphore_wait(vm_semaphore *sem)
{
    vm_status umc_status = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (0 <= sem->count)
    {
        umc_status = VM_OK;
        if (0 == pthread_mutex_lock(&sem->mutex))
        {
            if (0 == sem->count && 0 != pthread_cond_wait(&sem->cond, &sem->mutex))
                umc_status = VM_OPERATION_FAILED;

            if (VM_OK == umc_status)
                sem->count--;

            if (pthread_mutex_unlock(&sem->mutex))
                umc_status = VM_OPERATION_FAILED;
        }
        else
            umc_status = VM_OPERATION_FAILED;
    }
    return umc_status;

} /* vm_status vm_semaphore_wait(vm_semaphore *sem) */


/* Decrease the semaphore value without blocking, return 1 if success */
vm_status vm_semaphore_try_wait(vm_semaphore *sem)
{
    vm_status umc_status = VM_NOT_INITIALIZED;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (0 <= sem->count)
    {
        if (0 == pthread_mutex_lock(&sem->mutex))
        {
            if (0 == sem->count)
                umc_status = VM_TIMEOUT;
            else
            {
                sem->count--;
                umc_status = VM_OK;
            }
            if (pthread_mutex_unlock(&sem->mutex))
            {
                if (VM_OK == umc_status)
                    umc_status = VM_OPERATION_FAILED;
            }
        }
        else
            umc_status = VM_OPERATION_FAILED;
    }
    return umc_status;

} /* vm_status vm_semaphore_try_wait(vm_semaphore *sem) */

/* Increase the semaphore value */
vm_status vm_semaphore_post(vm_semaphore *sem)
{
    vm_status umc_status = VM_NOT_INITIALIZED;
    int res = 0;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (0 <= sem->count)
    {
        if (0 == pthread_mutex_lock(&sem->mutex))
        {
            if (0 == sem->count++)
                res = pthread_cond_signal(&sem->cond);

            umc_status = (res)? VM_OPERATION_FAILED: VM_OK;

            if (pthread_mutex_unlock(&sem->mutex))
            {
                umc_status = VM_OPERATION_FAILED;
            }
        }
        else
            umc_status = VM_OPERATION_FAILED;
    }
    return umc_status;

} /* vm_status vm_semaphore_post(vm_semaphore *sem) */

/* Increase the semaphore value */
vm_status vm_semaphore_post_many(vm_semaphore *sem, Ipp32s post_count)
{
    vm_status umc_status = VM_NOT_INITIALIZED;
    int res = 0, sts = 0;

    /* check error(s) */
    if (NULL == sem)
        return VM_NULL_PTR;

    if (post_count > sem->max_count)
        return VM_OPERATION_FAILED;

    if (0 <= sem->count)
    {
        Ipp32s i;
        for (i = 0; i < post_count; i++)
        {
            res = pthread_mutex_lock(&sem->mutex);
            if (0 == res)
            {
                if (i == sem->count++)
                {
                    sts = pthread_cond_signal(&sem->cond);
                    if (!res) res = sts;
                }

                sts = pthread_mutex_unlock(&sem->mutex);
                if (!res) res = sts;
            }

            if(res)
            {
                break;
            }
        }
        umc_status = (res)? VM_OPERATION_FAILED: VM_OK;
    }
    return umc_status;

} /* vm_status vm_semaphore_post_many(vm_semaphore *sem, Ipp32s post_count) */


/* Destory a semaphore */
void vm_semaphore_destroy(vm_semaphore *sem)
{
    /* check error(s) */
    if (NULL == sem)
        return;

    if (0 <= sem->count)
    {
        pthread_cond_destroy(&sem->cond);
        pthread_mutex_destroy(&sem->mutex);

        vm_semaphore_set_invalid_internal(sem);
    }
} /* void vm_semaphore_destroy(vm_semaphore *sem) */
#else
# pragma warning( disable: 4206 )
#endif /* LINUX32 */
