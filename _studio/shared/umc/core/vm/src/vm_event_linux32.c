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

File Name: vm_event_linux32.c

\* ****************************************************************************** */

#if defined(LINUX32) || defined(__APPLE__)

#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "vm_event.h"

static void vm_event_set_invalid_internal(vm_event *event)
{
    memset(event, 0, sizeof(vm_event));
    event->state = -1;
}

/* Invalidate an event */
void vm_event_set_invalid(vm_event *event)
{
    /* check error(s) */
    if (NULL == event)
        return;

    vm_event_set_invalid_internal(event);

} /* void vm_event_set_invalid(vm_event *event) */

/* Verify if an event is valid */
Ipp32s vm_event_is_valid(vm_event *event)
{
    /* check error(s) */
    if (NULL == event)
        return 0;

    return event->state >= 0;

} /* Ipp32s vm_event_is_valid(vm_event *event) */

/* Init an event. Event is created unset. return 1 if success */
vm_status vm_event_init(vm_event *event, Ipp32s manual, Ipp32s state)
{
    int res = 0;

    /* check error(s) */
    if (NULL == event)
        return VM_NULL_PTR;

    event->manual = manual;
    event->state = state ? 1 : 0;
    res = pthread_cond_init(&event->cond, 0);
    if (!res)
    {
        res = pthread_mutex_init(&event->mutex, 0);
        if (res)
        {
            pthread_cond_destroy(&event->cond);
            vm_event_set_invalid_internal(event);
        }
    }

    return (res)? VM_OPERATION_FAILED: VM_OK;

} /* vm_status vm_event_init(vm_event *event, Ipp32s manual, Ipp32s state) */

vm_status vm_event_named_init(vm_event *event,
                              Ipp32s manual, Ipp32s state, const char *pcName)
{
    /* linux version of named events is not supported by now */
    return VM_OPERATION_FAILED;

} /* vm_status vm_event_named_init(vm_event *event, */

/* Set the event to either HIGH (1) or LOW (0) state */
vm_status vm_event_signal(vm_event *event)
{
    vm_status umc_status = VM_NOT_INITIALIZED;
    int res = 0;

    /* check error(s) */
    if (NULL == event)
        return VM_NULL_PTR;

    if (0 <= event->state)
    {
        res = pthread_mutex_lock(&event->mutex);
        if (!res)
        {
            umc_status = VM_OK;
            if (0 == event->state)
            {
                event->state = 1;
                if (event->manual)
                {
                    res = pthread_cond_broadcast(&event->cond);
                    if (res)
                    {
                        umc_status = VM_OPERATION_FAILED;
                    }
                }
                else
                {
                    res = pthread_cond_signal(&event->cond);
                    if (res)
                    {
                        umc_status = VM_OPERATION_FAILED;
                    }
                }
            }

            if (0 != pthread_mutex_unlock(&event->mutex))
            {
                umc_status = VM_OPERATION_FAILED;
            }
        }
        else
        {
            umc_status = VM_OPERATION_FAILED;
        }
    }
    return umc_status;

} /* vm_status vm_event_signal(vm_event *event) */

vm_status vm_event_reset(vm_event *event)
{
    vm_status umc_status = VM_NOT_INITIALIZED;
    int res = 0;

    /* check error(s) */
    if (NULL == event)
        return VM_NULL_PTR;

    if (0 <= event->state)
    {
        res = pthread_mutex_lock(&event->mutex);
        if (!res)
        {
            umc_status = VM_OK;
            if (1 == event->state)
                event->state = 0;

            if (0 != pthread_mutex_unlock(&event->mutex))
            {
                umc_status = VM_OPERATION_FAILED;
            }
        }
        else
        {
            umc_status = VM_OPERATION_FAILED;
        }
    }
    return umc_status;

} /* vm_status vm_event_reset(vm_event *event) */

/* Pulse the event 0 -> 1 -> 0 */
vm_status vm_event_pulse(vm_event *event)
{
    vm_status umc_status = VM_NOT_INITIALIZED;
    int res = 0;

    /* check error(s) */
    if (NULL == event)
        return VM_NULL_PTR;

    if (0 <= event->state)
    {
        res = pthread_mutex_lock(&event->mutex);
        if (!res)
        {
            umc_status = VM_OK;

            if (event->manual)
            {
                res = pthread_cond_broadcast(&event->cond);
                if (res)
                {
                    umc_status = VM_OPERATION_FAILED;
                }
            }
            else
            {
                res = pthread_cond_signal(&event->cond);
                if (res)
                {
                    umc_status = VM_OPERATION_FAILED;
                }
            }

            event->state = 0;

            if (0 != pthread_mutex_unlock(&event->mutex))
            {
                umc_status = VM_OPERATION_FAILED;
            }
        }
        else
        {
            umc_status = VM_OPERATION_FAILED;
        }
    }
    return umc_status;

} /* vm_status vm_event_pulse(vm_event *event) */

/* Wait for event to be high with blocking */
vm_status vm_event_wait(vm_event *event)
{
    vm_status umc_status = VM_NOT_INITIALIZED;
    int res = 0;

    /* check error(s) */
    if (NULL == event)
        return VM_NULL_PTR;

    if (0 <= event->state)
    {
        res = pthread_mutex_lock(&event->mutex);
        if (!res)
        {
            umc_status = VM_OK;
            if (!event->state)
            {
                res = pthread_cond_wait(&event->cond,&event->mutex);
                if (res)
                {
                    umc_status = VM_OPERATION_FAILED;
                }
            }

            if (!event->manual)
                event->state = 0;

            if (0 != pthread_mutex_unlock(&event->mutex))
            {
                umc_status = VM_OPERATION_FAILED;
            }
        }
        else
        {
            umc_status = VM_OPERATION_FAILED;
        }
    }
    return umc_status;

} /* vm_status vm_event_wait(vm_event *event) */

/* Wait for event to be high without blocking, return 1 if successful */
vm_status vm_event_timed_wait(vm_event *event, Ipp32u msec)
{
    vm_status umc_status = VM_NOT_INITIALIZED;
    int res = 0;

    /* check error(s) */
    if (NULL == event)
        return VM_NULL_PTR;

    if (0 <= event->state)
    {
        res = pthread_mutex_lock(&event->mutex);
        if (!res)
        {
            if (0 == event->state)
            {
                if (0 == msec)
                {
                    umc_status = VM_TIMEOUT;
                }
                else
                {
                    struct timeval tval;
                    struct timespec tspec;
                    Ipp32s i_res;
                    Ipp64u micro_sec;

                    gettimeofday(&tval, NULL);
                    // NOTE: micro_sec _should_ be Ipp64u, not Ipp32u to avoid overflow
                    micro_sec = 1000 * msec + tval.tv_usec;
                    tspec.tv_sec = tval.tv_sec + (Ipp32u)(micro_sec / 1000000);
                    tspec.tv_nsec = (Ipp32u)(micro_sec % 1000000) * 1000;

                    i_res = pthread_cond_timedwait(&event->cond,
                                                &event->mutex,
                                                &tspec);
                    if (0 == i_res)
                        umc_status = VM_OK;
                    else if (ETIMEDOUT == i_res)
                        umc_status = VM_TIMEOUT;
                    else
                        umc_status = VM_OPERATION_FAILED;
                }
            }
            else
                umc_status = VM_OK;

            if (!event->manual)
                event->state = 0;
        }
        else
        {
            umc_status = VM_OPERATION_FAILED;
        }

        if(pthread_mutex_unlock(&event->mutex))
        {
            umc_status = VM_OPERATION_FAILED;
        }
    }
    return umc_status;

} /* vm_status vm_event_timed_wait(vm_event *event, Ipp32u msec) */

/* Destory the event */
void vm_event_destroy(vm_event *event)
{
    /* check error(s) */
    if (NULL == event)
        return;

    if (event->state >= 0)
    {
        pthread_cond_destroy(&event->cond);
        pthread_mutex_destroy(&event->mutex);

        vm_event_set_invalid_internal(event);
    }
} /* void vm_event_destroy(vm_event *event) */
#else
# pragma warning( disable: 4206 )
#endif /* LINUX32 */
