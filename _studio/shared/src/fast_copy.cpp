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

File Name: fast_copy.cpp

\* ****************************************************************************** */

#include "fast_copy.h"
#include "ippi.h"
#include "mfx_trace.h"

IppBool FastCopy::m_bCopyQuit = ippFalse;

FastCopy::FastCopy(void)
{
    m_mode = FAST_COPY_UNSUPPORTED;
    m_pThreads = NULL;
    m_tasks = NULL;

    m_numThreads = 0;

} // FastCopy::FastCopy(void)

FastCopy::~FastCopy(void)
{
    Release();

} // FastCopy::~FastCopy(void)

mfxStatus FastCopy::Initialize(void)
{
    mfxStatus sts = MFX_ERR_NONE;
    mfxU32 i = 0;

    // release object before allocation
    Release();

    if (MFX_ERR_NONE == sts)
    {
        // streaming is on
        m_mode = FAST_COPY_SSE41;
        m_numThreads = 1;
    }
    if (MFX_ERR_NONE == sts)
    {
        m_pThreads = new UMC::Thread[m_numThreads - 1];
        m_tasks = new FC_TASK[m_numThreads - 1];

        if (!m_pThreads || !m_tasks) sts = MFX_ERR_MEMORY_ALLOC;
    }
    // initialize events
    for (i = 0; (MFX_ERR_NONE == sts) && (i < m_numThreads - 1); i += 1)
    {
        if (UMC::UMC_OK != m_tasks[i].EventStart.Init(0, 0))
        {
            sts = MFX_ERR_UNKNOWN;
        }
        if (UMC::UMC_OK != m_tasks[i].EventEnd.Init(0, 0))
        {
            sts = MFX_ERR_UNKNOWN;
        }
    }
    // run threads
    for (i = 0; (MFX_ERR_NONE == sts) && (i < m_numThreads - 1); i += 1)
    {
        if (UMC::UMC_OK != m_pThreads[i].Create(CopyByThread, (void *)(m_tasks + i)))
        {
            sts = MFX_ERR_UNKNOWN;
        }
    }

    return MFX_ERR_NONE;
} // mfxStatus FastCopy::Initialize(void)

mfxStatus FastCopy::Release(void)
{
    m_bCopyQuit = ippTrue;

    if ((m_numThreads > 1) && m_tasks && m_pThreads)
    {
        // set event
        for (mfxU32 i = 0; i < m_numThreads - 1; i += 1)
        {
            m_tasks[i].EventStart.Set();
            m_pThreads[i].Wait();
        }
    }

    m_numThreads = 0;
    m_mode = FAST_COPY_UNSUPPORTED;

    if (m_pThreads)
    {
        delete [] m_pThreads;
        m_pThreads = NULL;
    }

    if (m_tasks)
    {
        delete [] m_tasks;
        m_tasks = NULL;
    }

    m_bCopyQuit = ippFalse;

    return MFX_ERR_NONE;

} // mfxStatus FastCopy::Release(void)

mfxStatus FastCopy::Copy(mfxU8 *pDst, mfxU32 dstPitch, mfxU8 *pSrc, mfxU32 srcPitch, IppiSize roi)
{
    MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_INTERNAL, "ippiCopyManaged");

    if (NULL == pDst || NULL == pSrc)
    {
        return MFX_ERR_NULL_PTR;
    }

    mfxU32 partSize = roi.height / m_numThreads;
    mfxU32 rest = roi.height % m_numThreads;

    roi.height = partSize;

    // distribute tasks
    for (mfxU32 i = 0; i < m_numThreads - 1; i += 1)
    {
        m_tasks[i].pS = pSrc + i * (partSize * srcPitch);
        m_tasks[i].pD = pDst + i * (partSize * dstPitch);
        m_tasks[i].srcPitch = srcPitch;
        m_tasks[i].dstPitch = dstPitch;
        m_tasks[i].roi = roi;

        m_tasks[i].EventStart.Set();
    }

    if (rest != 0)
    {
        roi.height = rest;
    }

    pSrc = pSrc + (m_numThreads - 1) * (partSize * srcPitch);
    pDst = pDst + (m_numThreads - 1) * (partSize * dstPitch);

    ippiCopyManaged_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi, 2);

    Synchronize();

    return MFX_ERR_NONE;

} // mfxStatus FastCopy::Copy(mfxU8 *pDst, mfxU32 dstPitch, mfxU8 *pSrc, mfxU32 srcPitch, IppiSize roi)


eFAST_COPY_MODE FastCopy::GetSupportedMode(void) const
{
    return m_mode;

} // eFAST_COPY_MODE FastCopy::GetSupportedMode(void) const

mfxStatus FastCopy::Synchronize(void)
{
    if (m_mode == FAST_COPY_SSE41)
    {
        for (mfxU32 i = 0; i < m_numThreads - 1; i += 1)
        {
            m_tasks[i].EventEnd.Wait();
        }
    }

    return MFX_ERR_NONE;

} // mfxStatus FastCopy::Synchronize(void)

// thread function
mfxU32 __STDCALL FastCopy::CopyByThread(void *arg)
{
    FC_TASK *task = (FC_TASK *)arg;
    MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_INTERNAL, "ThreadName=FastCopy");

    // wait to event
    task->EventStart.Wait();

    while (!m_bCopyQuit)
    {
        mfxU32 srcPitch = task->srcPitch;
        mfxU32 dstPitch = task->dstPitch;
        IppiSize roi = task->roi;

        Ipp8u *pSrc = task->pS;
        Ipp8u *pDst = task->pD;

        {
            MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_INTERNAL, "ippiCopyManaged");
            ippiCopyManaged_8u_C1R(pSrc, srcPitch, pDst, dstPitch, roi, 2);
        }

        // done copy
        task->EventEnd.Set();

        // wait for the next frame
        task->EventStart.Wait();
    }

    return 0;

} // mfxU32 __stdcall FastCopy::CopyByThread(void *arg)