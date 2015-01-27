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

File Name: umc_mutex.h

\* ****************************************************************************** */

#ifndef __UMC_MUTEX_H__
#define __UMC_MUTEX_H__

#include "mfx_vm++_pthread.h"
#include "vm_debug.h"
#include "vm_mutex.h"
#include "umc_structures.h"

namespace UMC
{

class Mutex : public MfxMutex
{
public:
    Mutex(void): MfxMutex() {}
    // Try to get ownership of mutex
    inline Status TryLock(void) { return (MfxMutex::TryLock())? UMC_OK: UMC_ERR_TIMEOUT; }

private: // functions
    Mutex(const Mutex&);
    Mutex& operator=(const Mutex&);
};

class AutomaticUMCMutex : public MfxAutoMutex
{
public:
    // Constructor
    AutomaticUMCMutex(UMC::Mutex &mutex)
        : MfxAutoMutex(mutex)
    {
    }

private: // functions
    AutomaticUMCMutex(const AutomaticUMCMutex&);
    AutomaticUMCMutex& operator=(const AutomaticUMCMutex&);
};

} // namespace UMC

#endif // __UMC_MUTEX_H__
