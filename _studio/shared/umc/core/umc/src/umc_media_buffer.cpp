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

File Name: umc_media_buffer.cpp

\* ****************************************************************************** */

#include "umc_media_buffer.h"
#include "umc_default_memory_allocator.h"

namespace UMC
{

MediaBufferParams::MediaBufferParams(void)
{
    m_prefInputBufferSize = 0;
    m_numberOfFrames = 0;
    m_prefOutputBufferSize = 0;

    m_pMemoryAllocator = 0;

} // MediaBufferParams::MediaBufferParams(void)

MediaBuffer::MediaBuffer(void)
{
    m_pMemoryAllocator = 0;
    m_pAllocated = 0;

} // MediaBuffer::MediaBuffer(void)

MediaBuffer::~MediaBuffer(void)
{
    Close();

} // MediaBuffer::~MediaBuffer(void)

Status MediaBuffer::Init(MediaReceiverParams *pInit)
{
    MediaBufferParams *pParams = DynamicCast<MediaBufferParams> (pInit);

    // check error(s)
    if (NULL == pParams)
        return UMC_ERR_NULL_PTR;

    // release the object before initialization
    MediaBuffer::Close();

    // use the external memory allocator
    if (pParams->m_pMemoryAllocator)
    {
        m_pMemoryAllocator = pParams->m_pMemoryAllocator;
    }
    // allocate default memory allocator
    else
    {
        m_pAllocated = new DefaultMemoryAllocator();
        if (NULL == m_pAllocated)
            return UMC_ERR_ALLOC;

        m_pMemoryAllocator = m_pAllocated;
    }

    return UMC_OK;

} // Status MediaBuffer::Init(MediaReceiverParams *pInit)

Status MediaBuffer::Close(void)
{
    if (m_pAllocated)
        delete m_pAllocated;

    m_pMemoryAllocator = NULL;
    m_pAllocated = NULL;

    return UMC_OK;

} // Status MediaBuffer::Close(void)

} // namespace UMC
