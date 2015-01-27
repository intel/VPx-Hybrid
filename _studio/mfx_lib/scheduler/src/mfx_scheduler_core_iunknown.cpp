/* ****************************************************************************** *\

Copyright (C) 2009-2011 Intel Corporation.  All rights reserved.

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

File Name: mfx_scheduler_core_iunknown.cpp

\* ****************************************************************************** */

#include <mfx_scheduler_core.h>

#include <vm_interlocked.h>

void *mfxSchedulerCore::QueryInterface(const MFX_GUID &guid)
{
    // Specific interface is required
    if (MFXIScheduler_GUID == guid)
    {
        // increment reference counter
        vm_interlocked_inc32(&m_refCounter);

        return (MFXIScheduler *) this;
    }

    // it is unsupported interface
    return NULL;

} // void *mfxSchedulerCore::QueryInterface(const MFX_GUID &guid)

void mfxSchedulerCore::AddRef(void)
{
    // increment reference counter
    vm_interlocked_inc32(&m_refCounter);

} // void mfxSchedulerCore::AddRef(void)

void mfxSchedulerCore::Release(void)
{
    // decrement reference counter
    vm_interlocked_dec32(&m_refCounter);

    if (0 == m_refCounter)
    {
        delete this;
    }

} // void mfxSchedulerCore::Release(void)

mfxU32 mfxSchedulerCore::GetNumRef(void) const
{
    return m_refCounter;

} // mfxU32 mfxSchedulerCore::GetNumRef(void) const


//explicit specification of interface creation
template<> MFXIScheduler*  CreateInterfaceInstance<MFXIScheduler>(const MFX_GUID &guid)
{
    if (MFXIScheduler_GUID == guid)
        return (MFXIScheduler*) (new mfxSchedulerCore);

    return NULL;

} //template<> MFXIScheduler*  CreateInterfaceInstance<MFXIScheduler>()
