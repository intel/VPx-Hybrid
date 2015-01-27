/* ****************************************************************************** *\

Copyright (C) 2003-2012 Intel Corporation.  All rights reserved.

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

File Name: vm_event.h

\* ****************************************************************************** */

#ifndef __VM_EVENT_H__
#define __VM_EVENT_H__

#include "vm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Invalidate an event */
void vm_event_set_invalid(vm_event *event);

/* Verify if an event is valid */
Ipp32s vm_event_is_valid(vm_event *event);

/* Init an event. Event is created unset. return 1 if success */
vm_status vm_event_init(vm_event *event, Ipp32s manual, Ipp32s state);
vm_status vm_event_named_init(vm_event *event, Ipp32s manual, Ipp32s state, const char *pcName);

/* Set the event to either HIGH (>0) or LOW (0) state */
vm_status vm_event_signal(vm_event *event);
vm_status vm_event_reset(vm_event *event);

/* Pulse the event 0 -> 1 -> 0 */
vm_status vm_event_pulse(vm_event *event);

/* Wait for event to be high with blocking */
vm_status vm_event_wait(vm_event *event);

/* Wait for event to be high without blocking, return 1 if signaled */
vm_status vm_event_timed_wait(vm_event *event, Ipp32u msec);

/* Destroy the event */
void vm_event_destroy(vm_event *event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_EVENT_H__ */
