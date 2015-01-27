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

File Name: umc_semaphore.h

\* ****************************************************************************** */

#ifndef __UMC_SEMAPHORE_H__
#define __UMC_SEMAPHORE_H__

#include "vm_debug.h"
#include "vm_semaphore.h"
#include "umc_structures.h"

namespace UMC
{

class Semaphore
{
public:
    // Default constructor
    Semaphore(void);
    // Destructor
    virtual ~Semaphore(void);

    // Initialize semaphore
    Status Init(Ipp32s iInitCount);
    Status Init(Ipp32s iInitCount, Ipp32s iMaxCount);
    // Check semaphore state
    bool IsValid(void);
    // Try to obtain semaphore
    Status TryWait(void);
    // Wait until semaphore is signaled
    Status Wait(Ipp32u msec);
    // Infinitely wait until semaphore is signaled
    Status Wait(void);
    // Set semaphore to signaled state
    Status Signal(Ipp32u count = 1);

protected:
    vm_semaphore m_handle;                                      // (vm_semaphore) handle to system semaphore
};

inline
bool Semaphore::IsValid(void)
{
    return vm_semaphore_is_valid(&m_handle) ? true : false;

} // bool Semaphore::IsValid(void)

inline
Status Semaphore::TryWait(void)
{
    return vm_semaphore_try_wait(&m_handle);

} // Status Semaphore::TryWait(void)

inline
Status Semaphore::Wait(Ipp32u msec)
{
    return vm_semaphore_timedwait(&m_handle, msec);

} // Status Semaphore::Wait(Ipp32u msec)

inline
Status Semaphore::Wait(void)
{
    return vm_semaphore_wait(&m_handle);

} // Status Semaphore::Wait(void)

inline
Status Semaphore::Signal(Ipp32u count)
{
    return vm_semaphore_post_many(&m_handle, count);

} // Status Semaphore::Signal(Ipp32u count)

} // namespace UMC

#endif // __UMC_SEMAPHORE_H__
