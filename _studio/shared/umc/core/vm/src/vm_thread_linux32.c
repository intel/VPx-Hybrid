/* ****************************************************************************** *\

Copyright (C) 2003-2014 Intel Corporation.  All rights reserved.

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

File Name: vm_thread_linux32.c

\* ****************************************************************************** */

#if defined(LINUX32) || defined(__APPLE__)

#define _GNU_SOURCE /* required by CPU_ZERO/CPU_SET to be able to link apps */
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include "vm_thread.h"
#include "vm_event.h"
#include "vm_mutex.h"
#ifdef __APPLE__
#include <mach/thread_policy.h>
#include <mach/mach_error.h>
#endif

static
void *vm_thread_proc(void *pv_params)
{
    vm_thread *p_thread = (vm_thread *) pv_params;

    /* check error(s) */
    if (NULL == pv_params)
        return ((void *) -1);

    p_thread->p_thread_func(p_thread->p_arg);
    vm_event_signal(&p_thread->exit_event);

    return ((void *) 1);

} /* void *vm_thread_proc(void *pv_params) */

/* set the thread handler an invalid value */
void vm_thread_set_invalid(vm_thread *thread)
{
    /* check error(s) */
    if (NULL == thread)
        return;

    thread->is_valid = 0;
    thread->i_wait_count = 0;
    vm_event_set_invalid(&thread->exit_event);
    vm_mutex_set_invalid(&thread->access_mut);

} /* void vm_thread_set_invalid(vm_thread *thread) */

/* verify if the thread handler is valid */
Ipp32s vm_thread_is_valid(vm_thread *thread)
{
    /* check error(s) */
    if (NULL == thread)
        return 0;

    if (thread->is_valid)
    {
        vm_mutex_lock(&thread->access_mut);
        if (VM_OK == vm_event_timed_wait(&thread->exit_event, 0))
        {
            vm_mutex_unlock(&thread->access_mut);
            vm_thread_wait(thread);
        }
        else
            vm_mutex_unlock(&thread->access_mut);
    }
    return thread->is_valid;

} /* Ipp32s vm_thread_is_valid(vm_thread *thread) */

/* create a thread. return 1 if success */
Ipp32s vm_thread_create(vm_thread *thread,
                        Ipp32u (*vm_thread_func)(void *),
                        void *arg)
{
    Ipp32s i_res = 1;
    pthread_attr_t attr;

    /* check error(s) */
    if ((NULL == thread) ||
        (NULL == vm_thread_func))
        return 0;

    if (0 != i_res)
    {
        if (VM_OK != vm_event_init(&thread->exit_event, 1, 0))
            i_res = 0;
    }

    if ((0 != i_res) &&
        (VM_OK != vm_mutex_init(&thread->access_mut)))
        i_res = 0;

    if (0 != i_res)
    {
        vm_mutex_lock(&thread->access_mut);
        thread->p_thread_func = vm_thread_func;
        thread->p_arg = arg;
        pthread_attr_init(&attr);
#ifdef ANDROID
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
#else
        // SCHED_RR doesn't work on Android
        pthread_attr_setschedpolicy(&attr, geteuid() ? SCHED_OTHER : SCHED_RR);
#endif

        thread->is_valid =! pthread_create(&thread->handle,
                                           &attr,
                                           vm_thread_proc,
                                           (void*)thread);

        i_res = (thread->is_valid) ? 1 : 0;
        vm_mutex_unlock(&thread->access_mut);
        pthread_attr_destroy(&attr);
    }
    vm_thread_set_priority(thread, VM_THREAD_PRIORITY_LOWEST);
    return i_res;

} /* Ipp32s vm_thread_create(vm_thread *thread, */

/* set thread priority, return 1 if successful */
Ipp32s vm_thread_set_priority(vm_thread *thread, vm_thread_priority priority)
{
    Ipp32s i_res = 1;
    Ipp32s policy, pmin, pmax, pmean;
    struct sched_param param;

    /* check error(s) */
    if (NULL == thread)
        return 0;

    if (thread->is_valid)
    {
#if !defined(OS_CHROMEOS)
        // NOTE: one of the below operations is prohibited by Chrome Sandbox
        vm_mutex_lock(&thread->access_mut);
        pthread_getschedparam(thread->handle,&policy,&param);

        pmin = sched_get_priority_min(policy);
        pmax = sched_get_priority_max(policy);
        pmean = (pmin + pmax) / 2;

        switch (priority)
        {
        case VM_THREAD_PRIORITY_HIGHEST:
            param.sched_priority = pmax;
            break;

        case VM_THREAD_PRIORITY_LOWEST:
            param.sched_priority = pmin;
            break;

        case VM_THREAD_PRIORITY_NORMAL:
            param.sched_priority = pmean;
            break;

        case VM_THREAD_PRIORITY_HIGH:
            param.sched_priority = (pmax + pmean) / 2;
            break;

        case VM_THREAD_PRIORITY_LOW:
            param.sched_priority = (pmin + pmean) / 2;
            break;

        default:
            i_res = 0;
            break;
        }

        if (i_res)
            i_res = !pthread_setschedparam(thread->handle, policy, &param);
        vm_mutex_unlock(&thread->access_mut);
#endif
    }
    return i_res;

} /* Ipp32s vm_thread_set_priority(vm_thread *thread, vm_thread_priority priority) */

/* wait until a thread exists */
void vm_thread_wait(vm_thread *thread)
{
    /* check error(s) */
    if (NULL == thread)
        return;

    if (thread->is_valid)
    {
        vm_mutex_lock(&thread->access_mut);
        thread->i_wait_count++;
        vm_mutex_unlock(&thread->access_mut);

        vm_event_wait(&thread->exit_event);

        vm_mutex_lock(&thread->access_mut);
        thread->i_wait_count--;
        if (0 == thread->i_wait_count)
        {
            pthread_join(thread->handle, NULL);
            thread->is_valid = 0;
        }
        vm_mutex_unlock(&thread->access_mut);
    }
} /* void vm_thread_wait(vm_thread *thread) */

/* close thread after all */
void vm_thread_close(vm_thread *thread)
{
    /* check error(s) */
    if (NULL == thread)
        return;

    vm_thread_wait(thread);
    vm_event_destroy(&thread->exit_event);
    vm_mutex_destroy(&thread->access_mut);

} /* void vm_thread_close(vm_thread *thread) */

vm_thread_priority vm_get_current_thread_priority()
{
    return VM_THREAD_PRIORITY_NORMAL;
}

void vm_set_current_thread_priority(vm_thread_priority priority)
{
    priority = priority;
}

void vm_set_thread_affinity_mask(vm_thread *thread, unsigned int mask)
{
#ifdef __APPLE__
    int cpu = -1;

    /* check error(s) */
    if (NULL == thread)
        return;

    do { mask >>= 1; ++cpu; } while(mask);

    //Get the mach port associated with this pthread
    //thread_t is a mach_port_t
    thread_t machPort;
    machPort = pthread_mach_thread_np(thread->handle);

    //Setup the affinity policy using thread_policy_set
    kern_return_t returnValue;
    thread_affinity_policy_data_t affinityPolicy;
    affinityPolicy.affinity_tag = cpu;

    if (KERN_SUCCESS != (returnValue = thread_policy_set(machPort,
                                                         THREAD_AFFINITY_POLICY,
                                                         &affinityPolicy,
                                                         THREAD_AFFINITY_POLICY_COUNT)))
    {
        //Log the error (TODO)
        //DebugMessage("ERROR: vm_set_thread_affinity_mask, thread_policy_set of THREAD_AFFINITY_POLICY returned error %s\n", mach_error_string(returnValue));
        return;
    }
#elif !defined(__ANDROID__)
    int cpu = -1;
    cpu_set_t cpuset;

    /* check error(s) */
    if (NULL == thread)
        return;

    CPU_ZERO(&cpuset);
    do { mask >>= 1; ++cpu; } while(mask);
    CPU_SET(cpu, &cpuset);
    pthread_setaffinity_np(thread->handle, sizeof(cpu_set_t), &cpuset);
#endif
}

#else
# pragma warning( disable: 4206 )
#endif /* LINUX32 */
