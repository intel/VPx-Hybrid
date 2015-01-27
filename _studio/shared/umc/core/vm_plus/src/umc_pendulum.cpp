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

File Name: umc_pendulum.cpp

\* ****************************************************************************** */

#include "umc_pendulum.h"

namespace UMC
{

Pendulum::Pendulum(void)
{
    vm_event_set_invalid(&m_hHigh);
    vm_event_set_invalid(&m_hLow);

} // Pendulum::Pendulum(void)

Pendulum::~Pendulum(void)
{
    Release();

} // Pendulum::~Pendulum(void)

void Pendulum::Release(void)
{
    if (vm_event_is_valid(&m_hHigh))
        vm_event_destroy(&m_hHigh);
    if (vm_event_is_valid(&m_hLow))
        vm_event_destroy(&m_hLow);

    vm_event_set_invalid(&m_hHigh);
    vm_event_set_invalid(&m_hLow);

} // void Pendulum::Release(void)

Status Pendulum::Init(bool bSignaled)
{
    vm_status vmRes;

    // Release object before initialization
    Release();

    vmRes = vm_event_init(&m_hHigh, 0, (bSignaled) ? (1) : (0));
    if (VM_OK != vmRes)
        return UMC_ERR_INIT;

    vmRes = vm_event_init(&m_hLow, 0, (bSignaled) ? (0) : (1));
    if (VM_OK != vmRes)
        return UMC_ERR_INIT;

    return UMC_OK;

} // Status Pendulum::Init(bool bSignaled)

Status Pendulum::Reset(bool bSignaled)
{
    if (vm_event_is_valid(&m_hHigh))
    {
        if (bSignaled)
            vm_event_signal(&m_hHigh);
        else
            vm_event_reset(&m_hHigh);
    }

    if (vm_event_is_valid(&m_hLow))
    {
        if (bSignaled)
            vm_event_reset(&m_hLow);
        else
            vm_event_signal(&m_hLow);
    }

    return UMC_OK;

} // Status Pendulum::Reset(bool bSignaled)

Status Pendulum::Set(void)
{
    // wait pendulum is non-signaled
    vm_event_wait(&m_hLow);

    // set pendulum to signaled state
    vm_event_signal(&m_hHigh);

    return UMC_OK;

} // Status Pendulum::Set(void)

Status Pendulum::Wait(void)
{
    // wait pendulum is signaled
    vm_event_wait(&m_hHigh);

    // set pendulum to signaled state
    vm_event_signal(&m_hLow);

    return UMC_OK;

} // Status Pendulum::Wait(void)

} // namespace UMC
