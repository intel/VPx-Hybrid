/* ****************************************************************************** *\

Copyright (C) 2007-2010 Intel Corporation.  All rights reserved.

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

File Name: mfxvideopro++.h

\* ****************************************************************************** */

#ifndef __MFXVIDEOPROPLUSPLUS_H
#define __MFXVIDEOPROPLUSPLUS_H

#include "mfxvideopro.h"

class MFXVideoPAK
{
public:

    MFXVideoPAK(mfxSession session) { m_session = session; }
    virtual ~MFXVideoPAK(void) { Close(); }

    mfxStatus Query(mfxVideoParam *in, mfxVideoParam *out) { return MFXVideoPAK_Query(m_session, in, out); }
    mfxStatus QueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest *request) { return MFXVideoPAK_QueryIOSurf(m_session, par, request); }
    mfxStatus Init(mfxVideoParam *par) { return MFXVideoPAK_Init(m_session, par); }
    mfxStatus Reset(mfxVideoParam *par) { return MFXVideoPAK_Reset(m_session, par); }
    mfxStatus Close(void) { return MFXVideoPAK_Close(m_session); }

    mfxStatus GetVideoParam(mfxVideoParam *par) { return MFXVideoPAK_GetVideoParam(m_session, par); }
    mfxStatus GetFrameParam(mfxFrameParam *par) { return MFXVideoPAK_GetFrameParam(m_session, par); }
    mfxStatus RunSeqHeader(mfxFrameCUC *cuc) { return MFXVideoPAK_RunSeqHeader(m_session, cuc); }
    mfxStatus RunFramePAKAsync(mfxFrameCUC *cuc, mfxSyncPoint *syncp) { return MFXVideoPAK_RunFramePAKAsync(m_session, cuc, syncp); }

protected:

    mfxSession m_session;                                       // (mfxSession) handle to the owning session
};

class MFXVideoBRC
{
public:

    MFXVideoBRC(mfxSession session) { m_session = session; }
    virtual ~MFXVideoBRC(void) { Close(); }

    mfxStatus Query(mfxVideoParam *in, mfxVideoParam *out) { return MFXVideoBRC_Query(m_session, in, out); }
    mfxStatus Init(mfxVideoParam *par) { return MFXVideoBRC_Init(m_session, par); }
    mfxStatus Reset(mfxVideoParam *par) { return MFXVideoBRC_Reset(m_session, par); }
    mfxStatus Close(void) { return MFXVideoBRC_Close(m_session); }

    mfxStatus FrameENCUpdate(mfxFrameCUC *cuc) { return MFXVideoBRC_FrameENCUpdate(m_session, cuc); }
    mfxStatus FramePAKRefine(mfxFrameCUC *cuc) { return MFXVideoBRC_FramePAKRefine(m_session, cuc); }
    mfxStatus FramePAKRecord(mfxFrameCUC *cuc) { return MFXVideoBRC_FramePAKRecord(m_session, cuc); }

protected:

    mfxSession m_session;                                       // (mfxSession) handle to the owning session
};

#endif // __MFXVIDEOPLUSPLUS_H
