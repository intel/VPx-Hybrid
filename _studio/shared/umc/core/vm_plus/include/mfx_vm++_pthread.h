/* ****************************************************************************** *\

Copyright (C) 2012-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_vm++_pthread.h

\* ****************************************************************************** */

#ifndef __MFX_VMPLUSPLUS_PTHREAD_H__
#define __MFX_VMPLUSPLUS_PTHREAD_H__

#include "mfxdefs.h"

#include <new>

/* Getting platform-specific definitions. */
#include "sys/mfx_vm++_pthread_windows.h"
#include "sys/mfx_vm++_pthread_unix.h"

/*------------------------------------------------------------------------------*/

class MfxMutex
{
public:
    MfxMutex(void);
    virtual ~MfxMutex(void);

    mfxStatus Lock(void);
    mfxStatus Unlock(void);
    bool TryLock(void);
    
private: // variables
    MfxMutexHandle m_handle;

private: // functions
    MfxMutex(const MfxMutex&);
    MfxMutex& operator=(const MfxMutex&);
};

/*------------------------------------------------------------------------------*/

class MfxAutoMutex
{
public:
    MfxAutoMutex(MfxMutex& mutex);
    virtual ~MfxAutoMutex(void);

    mfxStatus Lock(void);
    mfxStatus Unlock(void);

private: // variables
    MfxMutex& m_rMutex;
    bool m_bLocked;

private: // functions
    MfxAutoMutex(const MfxAutoMutex&);
    MfxAutoMutex& operator=(const MfxAutoMutex&);
};

/*------------------------------------------------------------------------------*/

#endif //__MFX_VMPLUSPLUS_PTHREAD_H__
