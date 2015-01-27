/* ****************************************************************************** *\

Copyright (C) 2012-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_vm++_pthread_unix.cpp

\* ****************************************************************************** */

#if !defined(_WIN32) && !defined(_WIN64)

#if !defined _GNU_SOURCE
#  define _GNU_SOURCE /* may need on some OS to support PTHREAD_MUTEX_RECURSIVE */
#endif

#include "mfx_vm++_pthread.h"

struct _MfxMutexHandle
{
    pthread_mutex_t m_mutex;
};

MfxMutex::MfxMutex(void)
{
    int res = 0;
    pthread_mutexattr_t mutex_attr;

    res = pthread_mutexattr_init(&mutex_attr);
    if (!res)
    {
        res = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
        if (!res) res = pthread_mutex_init(&m_handle.m_mutex, &mutex_attr);
        pthread_mutexattr_destroy(&mutex_attr);
    }
    if (res) throw std::bad_alloc();
}

MfxMutex::~MfxMutex(void)
{
    pthread_mutex_destroy(&m_handle.m_mutex);
}

mfxStatus MfxMutex::Lock(void)
{
    return (pthread_mutex_lock(&m_handle.m_mutex))? MFX_ERR_UNKNOWN: MFX_ERR_NONE;
}

mfxStatus MfxMutex::Unlock(void)
{
    return (pthread_mutex_unlock(&m_handle.m_mutex))? MFX_ERR_UNKNOWN: MFX_ERR_NONE;
}

bool MfxMutex::TryLock(void)
{
    return (pthread_mutex_trylock(&m_handle.m_mutex))? false: true;
}

#endif // #if !defined(_WIN32) && !defined(_WIN64)
