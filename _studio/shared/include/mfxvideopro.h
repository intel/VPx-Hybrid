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

File Name: mfxvideopro.h

\* ****************************************************************************** */
#ifndef __MFXVIDEOPRO_H__
#define __MFXVIDEOPRO_H__
#include "mfxstructurespro.h"
#include "mfxvideo.h"

/* This is the external include file for the Media SDK product */

#ifdef __cplusplus
extern "C"
{
#endif

/* VideoCORE: */
mfxStatus MFXVideoCORE_SyncOperation(mfxSession session, mfxSyncPoint syncp, mfxU32 wait);

/* VideoENC: */
mfxStatus MFXVideoENC_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFXVideoENC_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFXVideoENC_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoENC_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoENC_Close(mfxSession session);

mfxStatus MFXVideoENC_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoENC_GetFrameParam(mfxSession session, mfxFrameParam *par);
mfxStatus MFXVideoENC_RunFrameVmeENCAsync(mfxSession session, mfxFrameCUC *cuc, mfxSyncPoint *syncp);

/* VideoPAK: */
mfxStatus MFXVideoPAK_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFXVideoPAK_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFXVideoPAK_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoPAK_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoPAK_Close(mfxSession session);

mfxStatus MFXVideoPAK_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoPAK_GetFrameParam(mfxSession session, mfxFrameParam *par);
mfxStatus MFXVideoPAK_RunSeqHeader(mfxSession session, mfxFrameCUC *cuc);
mfxStatus MFXVideoPAK_RunFramePAKAsync(mfxSession session, mfxFrameCUC *cuc, mfxSyncPoint *syncp);

/* VideoBRC */
mfxStatus MFXVideoBRC_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFXVideoBRC_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoBRC_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoBRC_Close(mfxSession session);

mfxStatus MFXVideoBRC_FrameENCUpdate(mfxSession session, mfxFrameCUC *cuc);
mfxStatus MFXVideoBRC_FramePAKRefine(mfxSession session, mfxFrameCUC *cuc);
mfxStatus MFXVideoBRC_FramePAKRecord(mfxSession session, mfxFrameCUC *cuc);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
