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

File Name: umc_event.cpp

\* ****************************************************************************** */

#include "umc_event.h"

namespace UMC
{

Event::Event(void)
{
    vm_event_set_invalid(&m_handle);

} // Event::Event(void)

Event::~Event(void)
{
    if (vm_event_is_valid(&m_handle))
    {
        vm_event_signal(&m_handle);
        vm_event_destroy(&m_handle);
    }
} // Event::~Event(void)

Status Event::Init(Ipp32s iManual, Ipp32s iState)
{
    Status umcRes = UMC_OK;

    if (vm_event_is_valid(&m_handle))
    {
        umcRes = vm_event_signal(&m_handle);
        vm_event_destroy(&m_handle);
    }

    if (UMC_OK == umcRes)
        umcRes = vm_event_init(&m_handle, iManual, iState);

    return umcRes;

} // Status Event::Init(Ipp32s iManual, Ipp32s iState)

} // namespace UMC
