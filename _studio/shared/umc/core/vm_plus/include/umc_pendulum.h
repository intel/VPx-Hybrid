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

File Name: umc_pendulum.h

\* ****************************************************************************** */

#ifndef __UMC_PENDULUM_H__
#define __UMC_PENDULUM_H__

#include "umc_structures.h"
#include "vm_event.h"

namespace UMC
{

class Pendulum
{
public:
    // Default constructor
    Pendulum(void);
    // Destructor
    virtual
    ~Pendulum(void);

    // Initialize
    Status Init(bool bSignaled = true);

    // Set pendulum to specific state
    Status Reset(bool bSignaled);

    // Set pendulum to signaled state (synchronized)
    Status Set(void);
    // Infinitely wait until event is signaled
    Status Wait(void);

protected:
    // Release object
    void Release(void);

    vm_event m_hHigh;                                           // (vm_event) event to waiting high state
    vm_event m_hLow;                                            // (vm_event) event to waiting low state
};

} // namespace UMC

#endif // __UMC_PENDULUM_H__
