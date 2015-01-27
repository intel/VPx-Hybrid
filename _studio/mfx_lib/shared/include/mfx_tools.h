/* ****************************************************************************** *\

Copyright (C) 2010-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_tools.h

\* ****************************************************************************** */
#ifndef __MFX_TOOLS_H
#define __MFX_TOOLS_H

#include <mfxvideo++int.h>

// Declare functions for components creation
#if !defined (MFX_RT)
VideoENCODE *CreateENCODESpecificClass(mfxU32 codecId, VideoCORE *pCore, mfxSession session, mfxVideoParam *par);
VideoDECODE *CreateDECODESpecificClass(mfxU32 codecId, VideoCORE *pCore);
VideoVPP *CreateVPPSpecificClass(mfxU32 reserved, VideoCORE *pCore);
VideoENC *CreateENCSpecificClass(mfxU32 codecId, VideoCORE *pCore);
VideoPAK *CreatePAKSpecificClass(mfxU32 codecId, mfxU32 codecProfile, VideoCORE *pCore);
VideoBRC *CreateBRCSpecificClass(mfxU32 codecId, VideoCORE *pCore);

VideoENCODE* CreateMFXHWVideoENCODEH264(VideoCORE *core, mfxStatus *res);
#endif

namespace MFX
{
    unsigned int CreateUniqId();
}

inline
bool IsHWLib(void)
{
#ifdef MFX_VA
    return true;
#else // !MFX_VA
    return false;
#endif // MFX_VA

} // bool IsHWLib(void)

#endif // __MFX_TOOLS_H
