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

File Name: fast_copy.h

\* ****************************************************************************** */

#ifndef __FAST_COPY_H__
#define __FAST_COPY_H__

#include "ippcore.h"
#include "umc_event.h"
#include "umc_thread.h"
#include "libmfx_allocator.h"

struct FC_TASK
{
    // pointers to source and destination
    Ipp8u *pS;
    Ipp8u *pD;

    // size of chunk to copy
    Ipp32u chunkSize;

    // pitches and frame size
    IppiSize roi;
    Ipp32u srcPitch, dstPitch;

    // event handles
    UMC::Event EventStart;
    UMC::Event EventEnd;
};
enum eFAST_COPY_MODE
{
    FAST_COPY_SSE41         =   0x02,
    FAST_COPY_UNSUPPORTED   =   0x03
};

class FastCopy
{
public:

    // constructor
    FastCopy(void);

    // destructor
    virtual ~FastCopy(void);

    // initialize available functionality
    mfxStatus Initialize(void);

    // release object
    mfxStatus Release(void);

    // copy memory by streaming
    mfxStatus Copy(mfxU8 *pDst, mfxU32 dstPitch, mfxU8 *pSrc, mfxU32 srcPitch, IppiSize roi);

    // return supported mode
    virtual eFAST_COPY_MODE GetSupportedMode(void) const;

protected:

   // synchronize threads
    mfxStatus Synchronize(void);

    static mfxU32 __STDCALL CopyByThread(void *object);
    static IppBool m_bCopyQuit;

    // mode
    eFAST_COPY_MODE m_mode;

    // handles
    UMC::Thread *m_pThreads;
    Ipp32u m_numThreads;

    FC_TASK *m_tasks;
};

#endif // __FAST_COPY_H__
