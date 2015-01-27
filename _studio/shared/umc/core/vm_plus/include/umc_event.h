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

File Name: umc_event.h

\* ****************************************************************************** */

#ifndef __UMC_EVENT_H__
#define __UMC_EVENT_H__

#include "vm_debug.h"
#include "vm_event.h"
#include "umc_structures.h"

namespace UMC
{

class Event
{
public:
    // Default constructor
    Event(void);
    // Destructor
    virtual ~Event(void);

    // Initialize event
    Status Init(Ipp32s iManual, Ipp32s iState);
    // Set event to signaled state
    Status Set(void);
    // Set event to non-signaled state
    Status Reset(void);
    // Pulse event (should not be used)
    Status Pulse(void);
    // Wait until event is signaled
    Status Wait(Ipp32u msec);
    // Infinitely wait until event is signaled
    Status Wait(void);

protected:
    vm_event m_handle;                                          // (vm_event) handle to system event
};

inline
Status Event::Set(void)
{
    return vm_event_signal(&m_handle);

} // Status Event::Set(void)

inline
Status Event::Reset(void)
{
    return vm_event_reset(&m_handle);

} // Status Event::Reset(void)

inline
Status Event::Pulse(void)
{
    return vm_event_pulse(&m_handle);

} // Status Event::Pulse(void)

inline
Status Event::Wait(Ipp32u msec)
{
    return vm_event_timed_wait(&m_handle, msec);

} // Status Event::Wait(Ipp32u msec)

inline
Status Event::Wait(void)
{
    return vm_event_wait(&m_handle);

} // Status Event::Wait(void)

} // namespace UMC

#endif // __UMC_EVENT_H__
