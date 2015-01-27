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

File Name: vm_time.h

\* ****************************************************************************** */

#ifndef __VM_TIME_H__
#define __VM_TIME_H__

#include "vm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef Ipp64s vm_tick;
typedef Ipp32s vm_time_handle;

/* Yield the execution of current thread for msec miliseconds */
void vm_time_sleep(Ipp32u msec);

/* Obtain the time since the machine started in milliseconds on Windows
 * and time elapsed since the Epoch on Linux
 */
Ipp32u vm_time_get_current_time(void);

/* Obtain the clock tick of an uninterrupted master clock */
vm_tick vm_time_get_tick(void);

/* Obtain the clock resolution */
vm_tick vm_time_get_frequency(void);

/* Create the object of time measure */
vm_status vm_time_open(vm_time_handle *handle);

/* Initialize the object of time measure */
vm_status vm_time_init(vm_time *m);

/* Start the process of time measure */
vm_status vm_time_start(vm_time_handle handle, vm_time *m);

/* Stop the process of time measure and obtain the sampling time in seconds */
Ipp64f vm_time_stop(vm_time_handle handle, vm_time *m);

/* Close the object of time measure */
vm_status vm_time_close(vm_time_handle *handle);

/* Unix style time related functions */
/* TZP must be NULL or VM_NOT_INITIALIZED status will returned */
/* !!! Time zone not supported, !!! vm_status instead of int will returned */
vm_status vm_time_gettimeofday( struct vm_timeval *TP, struct vm_timezone *TZP );

void vm_time_timeradd(struct vm_timeval* destination,  struct vm_timeval* src1, struct vm_timeval* src2);
void vm_time_timersub(struct vm_timeval* destination,  struct vm_timeval* src1, struct vm_timeval* src2);
int vm_time_timercmp(struct vm_timeval* src1, struct vm_timeval* src2, struct vm_timeval* threshold);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_TIME_H__ */
