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

File Name: umc_semaphore.cpp

\* ****************************************************************************** */

#include "umc_semaphore.h"

namespace UMC
{

Semaphore::Semaphore(void)
{
    vm_semaphore_set_invalid(&m_handle);

} // Semaphore::Semaphore(void)

Semaphore::~Semaphore(void)
{
    if (vm_semaphore_is_valid(&m_handle))
    {
        vm_semaphore_post(&m_handle);
        vm_semaphore_destroy(&m_handle);
    }

} // Semaphore::~Semaphore(void)

Status Semaphore::Init(Ipp32s iInitCount)
{
    if (vm_semaphore_is_valid(&m_handle))
    {
        vm_semaphore_post(&m_handle);
        vm_semaphore_destroy(&m_handle);
    }
    return vm_semaphore_init(&m_handle, iInitCount);

} // Status Semaphore::Init(Ipp32s iInitCount)

Status Semaphore::Init(Ipp32s iInitCount, Ipp32s iMaxCount)
{
    if (vm_semaphore_is_valid(&m_handle))
    {
        vm_semaphore_post(&m_handle);
        vm_semaphore_destroy(&m_handle);
    }
    return vm_semaphore_init_max(&m_handle, iInitCount, iMaxCount);

} // Status Semaphore::Init(Ipp32s iInitCount, Ipp32s iMaxCount)

} // namespace UMC
