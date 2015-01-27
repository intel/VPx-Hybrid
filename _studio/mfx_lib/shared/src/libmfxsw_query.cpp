/* ****************************************************************************** *\

Copyright (C) 2007-2013 Intel Corporation.  All rights reserved.

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

File Name: libmfxsw_query.cpp

\* ****************************************************************************** */

#include <mfxvideo.h>
#include <mfx_session.h>

mfxStatus MFXQueryIMPL(mfxSession session, mfxIMPL *impl)
{
    mfxIMPL currentImpl;

    // check error(s)
    if (0 == session)
    {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == impl)
    {
        return MFX_ERR_NULL_PTR;
    }

    // set the library's type
#ifdef MFX_VA
#ifdef LINUX
    if ((*impl == MFX_IMPL_VIA_D3D9) ||
        (*impl == MFX_IMPL_VIA_D3D11) )
        return MFX_ERR_UNSUPPORTED;
#endif
    if (0 == session->m_adapterNum)
    {
        currentImpl = MFX_IMPL_HARDWARE;
    }
    else
    {
        currentImpl = (mfxIMPL) (MFX_IMPL_HARDWARE2 + (session->m_adapterNum - 1));
    }
    currentImpl |= session->m_implInterface;
#else
    currentImpl = MFX_IMPL_SOFTWARE;
#endif

    // save the current implementation type
    *impl = currentImpl;

    return MFX_ERR_NONE;

} // mfxStatus MFXQueryIMPL(mfxSession session, mfxIMPL *impl)

mfxStatus MFXQueryVersion(mfxSession session, mfxVersion *pVersion)
{
    if (0 == session)
    {
        return MFX_ERR_INVALID_HANDLE;
    }
    if (0 == pVersion)
    {
        return MFX_ERR_NULL_PTR;
    }

    // set the library's version
    pVersion->Major = MFX_VERSION_MAJOR;
    pVersion->Minor = MFX_VERSION_MINOR;

    return MFX_ERR_NONE;

} // mfxStatus MFXQueryVersion(mfxSession session, mfxVersion *pVersion)
