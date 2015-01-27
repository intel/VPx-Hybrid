/* ****************************************************************************** *\

Copyright (C) 2010-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_user_plugin.h

\* ****************************************************************************** */
#if !defined(__MFX_USER_PLUGIN_H)
#define __MFX_USER_PLUGIN_H

#include <mfxvideo++int.h>

#include <mfxplugin.h>
#include <mfx_task.h>

class VideoUSERPlugin : public VideoCodecUSER
{
public:
    // Default constructor, codec id != for decoder, and encoder plugins
    VideoUSERPlugin();
    // Destructor
    ~VideoUSERPlugin(void);

    // Initialize the user's plugin
    mfxStatus PluginInit(const mfxPlugin *pParam,
                   mfxSession session,
                   mfxU32 type = MFX_PLUGINTYPE_VIDEO_GENERAL);
    // Release the user's plugin
    mfxStatus PluginClose(void);
    // Get the plugin's threading policy
    mfxTaskThreadingPolicy GetThreadingPolicy(void);

    // Check the parameters to start a new tasl
    mfxStatus Check(const mfxHDL *in, mfxU32 in_num,
                    const mfxHDL *out, mfxU32 out_num,
                    MFX_ENTRY_POINT *pEntryPoint);

    mfxStatus QueryIOSurf(VideoCORE *core, mfxVideoParam *par, mfxFrameAllocRequest *in, mfxFrameAllocRequest *out);
    mfxStatus Query(VideoCORE *core, mfxVideoParam *in, mfxVideoParam *out);
    mfxStatus DecodeHeader(VideoCORE *core, mfxBitstream *bs, mfxVideoParam *par);

    mfxStatus Init(mfxVideoParam *par) ;
    mfxStatus Close(void);
    mfxStatus Reset(mfxVideoParam *par) ;
    mfxStatus GetFrameParam(mfxFrameParam *par) ;
    mfxStatus GetVideoParam(mfxVideoParam *par) ;
    mfxStatus GetEncodeStat(mfxEncodeStat *stat) ;
    mfxStatus GetDecodeStat(mfxDecodeStat *stat) ;
    mfxStatus GetVPPStat(mfxVPPStat *stat) ;
    mfxStatus DecodeFrameCheck(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, MFX_ENTRY_POINT * ep) ;
    mfxStatus DecodeFrame(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 *surface_out) ;
    mfxStatus SetSkipMode(mfxSkipMode mode) ;
    mfxStatus GetPayload(mfxU64 *ts, mfxPayload *payload) ;
    
    mfxStatus EncodeFrameCheck(mfxEncodeCtrl *ctrl,
        mfxFrameSurface1 *surface,
        mfxBitstream *bs,
        /*mfxFrameSurface1 **reordered_surface,
        mfxEncodeInternalParams *pInternalParams,*/
        MFX_ENTRY_POINT *pEntryPoint) ;

    mfxStatus VPPFrameCheck(mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, MFX_ENTRY_POINT *ep) ;

    mfxStatus EncodeFrame(mfxEncodeCtrl *ctrl, mfxEncodeInternalParams *pInternalParams, mfxFrameSurface1 *surface, mfxBitstream *bs) ;
    mfxStatus CancelFrame(mfxEncodeCtrl *ctrl, mfxEncodeInternalParams *pInternalParams, mfxFrameSurface1 *surface, mfxBitstream *bs) ;

    //expose new encoder/decoder view
    VideoENCODE* GetEncodePtr();
    VideoDECODE* GetDecodePtr();
    VideoVPP* GetVPPPtr();
protected: 
    class VideoENCDECImpl : public VideoENCODE, public VideoDECODE, public VideoVPP
    {
        VideoUSERPlugin *m_plg;
    public:
        VideoENCDECImpl(VideoUSERPlugin * plg)
            : m_plg (plg) {
        }
        mfxStatus Init(mfxVideoParam *par) {return m_plg->Init(par);}
        mfxStatus Reset(mfxVideoParam *par) {return m_plg->Reset(par);}
        mfxStatus Close() {  return m_plg->Close(); }
        mfxTaskThreadingPolicy GetThreadingPolicy(void) {
            return m_plg->GetThreadingPolicy();
        }
        mfxStatus GetVideoParam(mfxVideoParam *par) {
            return m_plg->GetVideoParam(par);
        }
        //encode
        mfxStatus GetFrameParam(mfxFrameParam *par) {
            return m_plg->GetFrameParam(par);
        }
        mfxStatus GetEncodeStat(mfxEncodeStat *stat) {
            return m_plg->GetEncodeStat(stat);
        }
        virtual
            mfxStatus EncodeFrameCheck(mfxEncodeCtrl *ctrl,
            mfxFrameSurface1 *surface,
            mfxBitstream *bs,
            mfxFrameSurface1 **reordered_surface,
            mfxEncodeInternalParams *pInternalParams,
            MFX_ENTRY_POINT *pEntryPoint)
        {
            reordered_surface;
            pInternalParams;
            return m_plg->EncodeFrameCheck(ctrl, surface, bs, pEntryPoint);
        }
        virtual mfxStatus EncodeFrameCheck(mfxEncodeCtrl * /*ctrl*/
            , mfxFrameSurface1 * /*surface*/
            , mfxBitstream * /*bs*/
            , mfxFrameSurface1 ** /*reordered_surface*/
            , mfxEncodeInternalParams * /*pInternalParams*/) {
            return MFX_ERR_NONE;
        }

        mfxStatus EncodeFrame(mfxEncodeCtrl *ctrl, mfxEncodeInternalParams *pInternalParams, mfxFrameSurface1 *surface, mfxBitstream *bs) {
            return m_plg->EncodeFrame(ctrl, pInternalParams, surface, bs);
        }
        mfxStatus CancelFrame(mfxEncodeCtrl *ctrl, mfxEncodeInternalParams *pInternalParams, mfxFrameSurface1 *surface, mfxBitstream *bs) {
            return m_plg->CancelFrame(ctrl, pInternalParams, surface, bs);
        }
        //decode
        mfxStatus GetDecodeStat(mfxDecodeStat *stat) {return m_plg->GetDecodeStat(stat);}
        mfxStatus DecodeFrameCheck(mfxBitstream *bs,
            mfxFrameSurface1 *surface_work,
            mfxFrameSurface1 **surface_out,
            MFX_ENTRY_POINT *pEntryPoint) {
            return m_plg->DecodeFrameCheck(bs, surface_work, surface_out, pEntryPoint);
        }
        
        mfxStatus DecodeFrameCheck(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out) {
            bs;
            surface_work;
            surface_out;
            return MFX_ERR_NONE;
        }
        mfxStatus DecodeFrame(mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 *surface_out) {
            return m_plg->DecodeFrame(bs, surface_work, surface_out);
        }
        mfxStatus SetSkipMode(mfxSkipMode mode) {return m_plg->SetSkipMode(mode);} 
        mfxStatus GetPayload(mfxU64 *ts, mfxPayload *payload) {return m_plg->GetPayload(ts, payload);}
        //vpp
        mfxStatus GetVPPStat(mfxVPPStat *stat) {
            return m_plg->GetVPPStat(stat);
        }
        virtual
            mfxStatus VPPFrameCheck(mfxFrameSurface1 *in,
            mfxFrameSurface1 *out,
            mfxExtVppAuxData *aux,
            MFX_ENTRY_POINT *ep)
        {
            return m_plg->VPPFrameCheck(in, out, aux, ep);
        }
        virtual 
            mfxStatus VppFrameCheck(
            mfxFrameSurface1 *, 
            mfxFrameSurface1 *)
        {
            return MFX_ERR_UNSUPPORTED;
        }
        virtual 
            mfxStatus RunFrameVPP(
            mfxFrameSurface1 *, 
            mfxFrameSurface1 *,
            mfxExtVppAuxData *)
        {
            return MFX_ERR_UNSUPPORTED;
        }
    };

    void Release(void);

    // User's plugin parameters
    mfxPluginParam m_param;
    mfxPlugin m_plugin;

    // Default entry point structure
    MFX_ENTRY_POINT m_entryPoint;
};

#endif // __MFX_USER_PLUGIN_H
