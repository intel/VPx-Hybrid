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

File Name: umc_thread.h

\* ****************************************************************************** */

#ifndef __UMC_THREAD_H__
#define __UMC_THREAD_H__

#include "vm_debug.h"
#include "vm_thread.h"
#include "umc_structures.h"

namespace UMC
{
class Thread
{
public:
    // Default constructor
    Thread(void);
    virtual ~Thread(void);

    // Check thread status
    bool IsValid(void);
    // Create new thread
    Status Create(vm_thread_callback func, void *arg);
    // Wait until thread does exit
    void Wait(void);
    // Set thread priority
    Status SetPriority(vm_thread_priority priority);
    // Close thread object
    void Close(void);

#if defined(_WIN32) || defined(_WIN64) || defined(_WIN32_WCE)
    // Set reaction on exception, if exception is caught(VM_THREADCATCHCRASH define)
    Status SetExceptionReaction(vm_thread_callback func);
#endif

protected:
    vm_thread m_Thread;                                         // (vm_thread) handle to system thread
};

inline
bool Thread::IsValid(void)
{
    return vm_thread_is_valid(&m_Thread) ? true : false;

} // bool Thread::IsValid(void)

inline
void Thread::Wait(void)
{
    vm_thread_wait(&m_Thread);

} // void Thread::Wait(void)

inline
void Thread::Close(void)
{
    vm_thread_close(&m_Thread);

} // void Thread::Close(void)

} // namespace UMC

#endif // __UMC_THREAD_H__
