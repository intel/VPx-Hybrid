/* ****************************************************************************** *\

Copyright (C) 2006-2013 Intel Corporation.  All rights reserved.

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

File Name: umc_va.cpp

\* ****************************************************************************** */

#include "umc_va_base.h"

#if 0  //absent in menlow project
#ifdef UMC_VA_LINUX

#include "umc_va.h"

namespace UMC
{

UMCVAFrameBuffer* VideoAcceleratorExt::GetFrameBuffer(int index) // to get pointer to uncompressed buffer.
{
    if(index<m_NumOfFrameBuffers)
        return &m_FrameBuffers[index];
    else
        return NULL;
}

void* VideoAcceleratorExt::GetCompBuffer(Ipp32s buffer_type, UMCVACompBuffer **buf, Ipp32s size, Ipp32s index)
{
    /*try to find cached buffer*/
    list<VACompBuffer>::iterator i= find( m_CachedCompBuffers.begin(), m_CachedCompBuffers.end(), VACompBuffer(buffer_type, -1, index));
    if(i == m_CachedCompBuffers.end())
    {
        /*get buffer from HW*/
        AutomaticMutex guard(m_mGuard);
        i= find( m_CachedCompBuffers.begin(), m_CachedCompBuffers.end(), VACompBuffer(buffer_type, -1, index));
        if(i == m_CachedCompBuffers.end())
        {
            m_CachedCompBuffers.push_back( GetCompBufferHW(buffer_type, size, index) );
            i = m_CachedCompBuffers.end();
            i--;
            i->SetDataSize(0);
        }
    }

    if(buf != NULL)
        *buf = &(*i);
    return (void*)i->GetPtr();
}

Status VideoAcceleratorExt::EndFrame()
{
    for(list<VACompBuffer>::iterator i = m_CachedCompBuffers.begin(); i != m_CachedCompBuffers.end(); ++i)
        ReleaseBuffer(i->GetType());
    m_CachedCompBuffers.clear();
    return UMC_OK;
}

} //  namespace UMC

#endif // #ifdef UMC_VA_LINUX
#endif
