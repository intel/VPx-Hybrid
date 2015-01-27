/* ****************************************************************************** *\

Copyright (C) 2008-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_common.h

\* ****************************************************************************** */
#ifndef _MFX_COMMON_H_
#define _MFX_COMMON_H_

#include "mfxvideo.h"
#include "mfxvideo++int.h"
#include "ippdefs.h"
#include "mfx_utils.h"
#include <stdio.h>
#include <string.h>

#include <string>
#include <stdexcept> /* for std exceptions on Linux/Android */

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <stddef.h>
#endif

#ifdef UMC_VA_LINUX
    #undef  MFX_VA
    #define MFX_VA
#endif // UMC_VA_LINUX

#ifdef MFX_VA
    #if defined(LINUX32) || defined(LINUX64)

        #undef  MFX_VA_LINUX
        #define MFX_VA_LINUX

        #undef  UMC_VA_LINUX
        #define UMC_VA_LINUX

        /* Android and Linux uses one video acceleration library: LibVA, but
         * it is possible that their versions are different (especially during
         * development). To simplify code development MFX_VA_ANDROID macro is introduced.
         */
        #if defined(ANDROID)
            #define MFX_VA_ANDROID
        #endif // #if defined(ANDROID)

    #elif defined(_WIN32) || defined(_WIN64)

        #undef  MFX_VA_WIN
        #define MFX_VA_WIN

        #undef  UMC_VA_DXVA
        #define UMC_VA_DXVA

    #elif defined(__APPLE__)
        #undef  MFX_VA_OSX
        #define MFX_VA_OSX

        #undef  UMC_VA_OSX
        #define UMC_VA_OSX
    #endif // #if defined(LINUX32) || defined(LINUX64)
#endif // MFX_VA

#if defined (MFX_VA_LINUX)
#define SYNCHRONIZATION_BY_VA_SYNC_SURFACE
#endif

#if defined (SYNCHRONIZATION_BY_VA_SYNC_SURFACE)
// if SYNCHRONIZATION_BY_NON_ZERO_THREAD is defined then
// thread #0 submits tasks to the driver
// thread(s) #1, #2, ... make synchronization by vaSyncSurface
//#define SYNCHRONIZATION_BY_NON_ZERO_THREAD
#endif


//#define MFX_ENABLE_C2CPP_DEBUG          // debug the C to C++ layer only.
#ifndef MFX_ENABLE_C2CPP_DEBUG
#include "umc_structures.h"

#if defined(AS_VP8E_PLUGIN)
    #define MFX_ENABLE_VP8_VIDEO_ENCODE_HW
#endif

#define MFX_ENABLE_USER_ENCODE

#define MFX_BIT_IN_KB 8*1000
#endif

class MfxException
{
public:
    MfxException (mfxStatus st, const char *msg = NULL)
        : m_message (msg)
        , m_status (st)
    {
    }

    const char * GetMessage() const
    {
        return m_message;
    }

    mfxStatus GetStatus() const
    {
        return m_status;
    }

protected:
    const char *m_message;
    mfxStatus m_status;
};

#define MFX_AUTO_ASYNC_DEPTH_VALUE  5
#define MFX_MAX_ASYNC_DEPTH_VALUE   15

#endif //_MFX_COMMON_H_
