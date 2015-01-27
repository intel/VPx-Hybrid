/* ****************************************************************************** *\

Copyright (C) 2011-2014 Intel Corporation.  All rights reserved.

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

File Name: libmfx_core_factory.cpp

\* ****************************************************************************** */

#include "mfx_common.h"
#include <libmfx_core_factory.h>
#include <libmfx_core.h>

#if defined (MFX_VA_WIN)
#include <libmfx_core_d3d9.h>
    #if defined (MFX_D3D11_ENABLED)
    #include "umc_va_dxva2_protected.h"
    #include <libmfx_core_d3d11.h>
    #endif
#elif defined(MFX_VA_LINUX)
#include <libmfx_core_vaapi.h>
#elif defined(MFX_VA_OSX)
#include <libmfx_core_vdaapi.h>
#endif

#if defined(MFX_RT)


VideoCORE* FactoryCORE::CreateCORE(eMFXVAType va_type,
                                   mfxU32 adapterNum,
                                   mfxU32 numThreadsAvailable,
                                   mfxSession session)
{
    adapterNum;
    switch(va_type)
    {
    case MFX_HW_NO:
        return new CommonCORE(numThreadsAvailable, session);
#if defined (MFX_VA_WIN)
    case MFX_HW_D3D9:
        return new D3D9VideoCORE(adapterNum, numThreadsAvailable, session);
#if defined (MFX_D3D11_ENABLED)
    case MFX_HW_D3D11:
        return new D3D11VideoCORE(adapterNum, numThreadsAvailable, session);
#endif
#elif defined(MFX_VA_LINUX)
    case MFX_HW_VAAPI:
        return new VAAPIVideoCORE(adapterNum, numThreadsAvailable, session);
#elif defined(MFX_VA_OSX)
    case MFX_HW_VDAAPI:
        return new VDAAPIVideoCORE(adapterNum, numThreadsAvailable, session);
#endif
    default:
        return NULL;
    }

} // VideoCORE* FactoryCORE::CreateCORE(eMFXVAType va_type)

#endif // MFX_RT
