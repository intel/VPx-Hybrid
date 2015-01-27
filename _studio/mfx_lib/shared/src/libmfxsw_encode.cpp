/* ****************************************************************************** *\

Copyright (C) 2008-2014 Intel Corporation.  All rights reserved.

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

File Name: libmfxsw_encode.cpp

\* ****************************************************************************** */

#include <mfxvideo.h>

#include <mfx_session.h>
#include <mfx_tools.h>
#include <mfx_common.h>

// sheduling and threading stuff
#include <mfx_task.h>

// AYA: temp solution DXVA2 Encoder GUID
#include "mfxvideo++int.h"
#if defined (MFX_ENABLE_H264_VIDEO_ENCODE)
#if defined(MFX_VA)
#include "mfx_h264_encode_hw.h"
#endif
#include "mfx_h264_enc_common.h"
#include "mfx_h264_encode.h"
#if defined (MFX_ENABLE_MVC_VIDEO_ENCODE)
#include "mfx_mvc_encode.h"
#endif
#endif

#if defined (MFX_ENABLE_VC1_VIDEO_ENCODE)
#if defined(MFX_VA)
#else
#include "mfx_vc1_enc_encode.h"
#endif
#endif

#if defined (MFX_ENABLE_MPEG2_VIDEO_ENCODE)
#if defined(MFX_VA)
#include "mfx_mpeg2_encode_hw.h"
#include "mfx_mpeg2_encode.h"
#else
#include "mfx_mpeg2_encode.h"
#endif
#endif

#if defined (MFX_ENABLE_VP8_VIDEO_ENCODE)
#include "mfx_vp8_encode.h"
#endif

#if defined (MFX_ENABLE_VP8_VIDEO_ENCODE_HW) && defined (MFX_VA)
#include "mfx_vp8_encode_hw.h"
#endif


#if defined (MFX_ENABLE_MJPEG_VIDEO_ENCODE)
#if defined(MFX_VA_WIN)
#include "mfx_mjpeg_encode_hw.h"
#include "mfx_mjpeg_encode.h"
#else
#include "mfx_mjpeg_encode.h"
#endif
#endif

#if defined (MFX_ENABLE_H265_VIDEO_ENCODE)
#if defined(MFX_VA)
#include "mfx_h265_encode.h"
#else
#include "mfx_h265_encode.h"
#endif
#endif

#if defined (MFX_RT)
#pragma warning(disable:4065)
#endif

// declare static file section
namespace
{

VideoENCODE* CreateUnsupported(VideoCORE *, mfxStatus *res)
{
    *res = MFX_ERR_UNSUPPORTED;
    return 0;
}

} // namespace

#if !defined (MFX_RT)
VideoENCODE *CreateENCODESpecificClass(mfxU32 CodecId, VideoCORE *core, mfxSession session, mfxVideoParam *par)
{
    VideoENCODE *pENCODE = (VideoENCODE *) 0;
    mfxStatus mfxRes = MFX_ERR_MEMORY_ALLOC;

    // touch unreferenced parameter
    session = session;
#if !defined(MFX_VA)
    par = par;
#endif // !defined(MFX_VA)

    // create a codec instance
    switch (CodecId)
    {
#if defined(MFX_ENABLE_H264_VIDEO_ENCODE)
    case MFX_CODEC_AVC:
#if defined(MFX_VA) && defined(MFX_ENABLE_H264_VIDEO_ENCODE_HW)
        if (session->m_bIsHWENCSupport)
        {
            pENCODE = CreateMFXHWVideoENCODEH264(core, &mfxRes);
        }
        else
        {
#ifdef MFX_ENABLE_MVC_VIDEO_ENCODE
            if(par && (par->mfx.CodecProfile == MFX_PROFILE_AVC_MULTIVIEW_HIGH || par->mfx.CodecProfile == MFX_PROFILE_AVC_STEREO_HIGH))
                pENCODE = new MFXVideoENCODEMVC(core, &mfxRes);
            else
#endif // MFX_ENABLE_MVC_VIDEO_ENCODE
                pENCODE = new MFXVideoENCODEH264(core, &mfxRes);
        }

#else //MFX_VA

#ifdef MFX_ENABLE_MVC_VIDEO_ENCODE
        if(par && (par->mfx.CodecProfile == MFX_PROFILE_AVC_MULTIVIEW_HIGH || par->mfx.CodecProfile == MFX_PROFILE_AVC_STEREO_HIGH))
            pENCODE = new MFXVideoENCODEMVC(core, &mfxRes);
        else
#endif // MFX_ENABLE_MVC_VIDEO_ENCODE
            pENCODE = new MFXVideoENCODEH264(core, &mfxRes);
#endif //MFX_VA

        break;
#endif // MFX_ENABLE_H264_VIDEO_ENCODE

#if defined(MFX_ENABLE_MPEG2_VIDEO_ENCODE)
    case MFX_CODEC_MPEG2:
#if defined(MFX_VA)
        if (session->m_bIsHWENCSupport)
        {
            pENCODE = new MFXVideoENCODEMPEG2_HW(core, &mfxRes);
        }
        else
        {
            pENCODE = new MFXVideoENCODEMPEG2(core, &mfxRes);
        }
#else //MFX_VA
        pENCODE = new MFXVideoENCODEMPEG2(core, &mfxRes);
#endif //MFX_VA
        break;
#endif // MFX_ENABLE_MPEG2_VIDEO_ENCODE

#if defined(MFX_ENABLE_VC1_VIDEO_ENCODE)
    case MFX_CODEC_VC1:
        pENCODE = new MFXVideoENCODEVC1(core, &mfxRes);
        break;
#endif

#if defined(MFX_ENABLE_VP8_VIDEO_ENCODE)
    case MFX_CODEC_VP8:
#if defined(MFX_VA) && defined(MFX_ENABLE_VP8_VIDEO_ENCODE_HW)
        if (session->m_bIsHWENCSupport)
        {
            pENCODE = new MFXHWVideoENCODEVP8(core, &mfxRes);
        }
        else
#endif // MFX_VA && MFX_ENABLE_VP8_VIDEO_ENCODE_HW
            pENCODE = new MFXVideoENCODEVP8(core, &mfxRes);
        break;

#endif // MFX_ENABLE_VP8_VIDEO_ENCODE

#if defined(MFX_ENABLE_MJPEG_VIDEO_ENCODE)
    case MFX_CODEC_JPEG:
#if defined(MFX_VA_WIN)
        if (session->m_bIsHWENCSupport)
        {
            pENCODE = new MFXVideoENCODEMJPEG_HW(core, &mfxRes);
        }
        else
        {
            pENCODE = new MFXVideoENCODEMJPEG(core, &mfxRes);
        }
        break;
#else  // MFX_VA
        pENCODE = new MFXVideoENCODEMJPEG(core, &mfxRes);
#endif // MFX_VA
        break;
#endif // MFX_ENABLE_MJPEG_VIDEO_ENCODE

#if defined(MFX_ENABLE_H265_VIDEO_ENCODE)
    case MFX_CODEC_HEVC:
        pENCODE = new MFXVideoENCODEH265(core, &mfxRes);
        break;
#endif

    default:
        break;
    }

    // check error(s)
    if (MFX_ERR_NONE != mfxRes)
    {
        delete pENCODE;
        pENCODE = (VideoENCODE *) 0;
    }

    return pENCODE;

} // VideoENCODE *CreateENCODESpecificClass(mfxU32 CodecId, VideoCORE *core)
#endif

mfxStatus MFXVideoENCODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out)
{
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(out, MFX_ERR_NULL_PTR);

#if !defined(ANDROID)
    if ((0 != in) && (MFX_HW_VAAPI == session->m_pCORE->GetVAType()))
    {
        // protected content not supported on Linux
        MFX_CHECK(0 == in->Protected, MFX_ERR_UNSUPPORTED);
    }
#endif

    mfxStatus mfxRes;
    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, in);

    bool bIsHWENCSupport = false;

    try
    {
#ifdef MFX_ENABLE_USER_ENCODE
        mfxRes = MFX_ERR_UNSUPPORTED;
        if (session->m_plgEnc.get())
        {
            mfxRes = session->m_plgEnc->Query(session->m_pCORE.get(), in, out);
            if (mfxRes >= MFX_ERR_NONE &&
              mfxRes != MFX_WRN_PARTIAL_ACCELERATION)
              bIsHWENCSupport = true;
        }
        // unsupported reserved to codecid != requested codecid
        if (MFX_ERR_UNSUPPORTED == mfxRes)
#endif
        switch (out->mfx.CodecId)
        {
#ifdef MFX_ENABLE_VC1_VIDEO_ENCODE
        case MFX_CODEC_VC1:
            mfxRes = MFXVideoENCODEVC1::Query(in, out);
            break;
#endif

#ifdef MFX_ENABLE_H264_VIDEO_ENCODE
        case MFX_CODEC_AVC:
#if defined(MFX_VA) && defined (MFX_ENABLE_H264_VIDEO_ENCODE_HW)
            if (!session->m_pENCODE.get())
                mfxRes = MFXHWVideoENCODEH264::Query(session->m_pCORE.get(), in, out);
            else
                mfxRes = MFXHWVideoENCODEH264::Query(session->m_pCORE.get(), in, out, session->m_pENCODE.get());
            if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
            {
#ifdef MFX_ENABLE_MVC_VIDEO_ENCODE
                if(in && (in->mfx.CodecProfile == MFX_PROFILE_AVC_MULTIVIEW_HIGH || in->mfx.CodecProfile == MFX_PROFILE_AVC_STEREO_HIGH))
                    mfxRes = MFXVideoENCODEMVC::Query(in, out);
                else
#endif
                    mfxRes = MFXVideoENCODEH264::Query(in, out);
            }
            else
            {
                bIsHWENCSupport = true;
            }
#else //MFX_VA
#ifdef MFX_ENABLE_MVC_VIDEO_ENCODE
            if(in && (in->mfx.CodecProfile == MFX_PROFILE_AVC_MULTIVIEW_HIGH || in->mfx.CodecProfile == MFX_PROFILE_AVC_STEREO_HIGH))
                mfxRes = MFXVideoENCODEMVC::Query(in, out);
            else
#endif
                mfxRes = MFXVideoENCODEH264::Query(in, out);
#endif //MFX_VA
            break;
#endif

#ifdef MFX_ENABLE_MPEG2_VIDEO_ENCODE
        case MFX_CODEC_MPEG2:
#if defined(MFX_VA)
            mfxRes = MFXVideoENCODEMPEG2_HW::Query(session->m_pCORE.get(), in, out);
            if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
            {
                mfxRes = MFXVideoENCODEMPEG2::Query(in, out);
            }
            else
            {
                bIsHWENCSupport = true;
            }
#else
            mfxRes = MFXVideoENCODEMPEG2::Query(in, out);
#endif
            break;
#endif


#ifdef MFX_ENABLE_VP8_VIDEO_ENCODE
        case MFX_CODEC_VP8:
#if defined(MFX_VA) && defined(MFX_ENABLE_VP8_VIDEO_ENCODE_HW)
            mfxRes = MFXHWVideoENCODEVP8::Query(session->m_pCORE.get(), in, out);

            if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
            {
                mfxRes = MFXVideoENCODEVP8::Query(in, out);
            }
            else
            {
                bIsHWENCSupport = true;
            }
#else // MFX_VA && MFX_ENABLE_VP8_VIDEO_ENCODE_HW
            mfxRes = MFXVideoENCODEVP8::Query(in, out);
#endif // MFX_VA && MFX_ENABLE_VP8_VIDEO_ENCODE_HW
            break;

#endif // MFX_ENABLE_VP8_VIDEO_ENCODE

#if defined(MFX_ENABLE_MJPEG_VIDEO_ENCODE)
        case MFX_CODEC_JPEG:
#if defined(MFX_VA_WIN)
            mfxRes = MFXVideoENCODEMJPEG_HW::Query(session->m_pCORE.get(), in, out);
            if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
            {
                mfxRes = MFXVideoENCODEMJPEG::Query(in, out);
            }
            else
            {
                bIsHWENCSupport = true;
            }
            break;
#else
            mfxRes = MFXVideoENCODEMJPEG::Query(in, out);
#endif
            break;
#endif // MFX_ENABLE_MJPEG_VIDEO_ENCODE

#ifdef MFX_ENABLE_H265_VIDEO_ENCODE
        case MFX_CODEC_HEVC:
            mfxRes = MFXVideoENCODEH265::Query(session->m_pCORE.get(), in, out);
            break;
#endif

        default:
            mfxRes = MFX_ERR_UNSUPPORTED;
        }
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        mfxRes = MFX_ERR_NULL_PTR;
    }
    // SW fallback if EncodeGUID is absence
    if (MFX_PLATFORM_HARDWARE == session->m_currentPlatform &&
        !bIsHWENCSupport &&
        MFX_ERR_NONE <= mfxRes)
    {
        mfxRes = MFX_WRN_PARTIAL_ACCELERATION;
    }
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, out);
    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;
}

mfxStatus MFXVideoENCODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request)
{
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(par, MFX_ERR_NULL_PTR);
    MFX_CHECK(request, MFX_ERR_NULL_PTR);

    mfxStatus mfxRes;
    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, par);

    bool bIsHWENCSupport = false;

    try
    {
#ifdef MFX_ENABLE_USER_ENCODE
        mfxRes = MFX_ERR_UNSUPPORTED;
        if (session->m_plgEnc.get())
        {
            mfxRes = session->m_plgEnc->QueryIOSurf(session->m_pCORE.get(), par, request, 0);
            if (mfxRes >= MFX_ERR_NONE &&
              mfxRes != MFX_WRN_PARTIAL_ACCELERATION)
              bIsHWENCSupport = true;
        }
        // unsupported reserved to codecid != requested codecid
        if (MFX_ERR_UNSUPPORTED == mfxRes)
#endif
        switch (par->mfx.CodecId)
        {
#ifdef MFX_ENABLE_VC1_VIDEO_ENC
        case MFX_CODEC_VC1:
            mfxRes = MFXVideoENCODEVC1::QueryIOSurf(par, request);
            break;
#endif // MFX_ENABLE_VC1_VIDEO_ENC

#ifdef MFX_ENABLE_H264_VIDEO_ENCODE
        case MFX_CODEC_AVC:
#if defined(MFX_VA) && defined (MFX_ENABLE_H264_VIDEO_ENCODE_HW)
            mfxRes = MFXHWVideoENCODEH264::QueryIOSurf(session->m_pCORE.get(), par, request);
            if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
            {
#ifdef MFX_ENABLE_MVC_VIDEO_ENCODE
                if (par->mfx.CodecProfile == MFX_PROFILE_AVC_MULTIVIEW_HIGH || par->mfx.CodecProfile == MFX_PROFILE_AVC_STEREO_HIGH)
                    mfxRes = MFXVideoENCODEMVC::QueryIOSurf(par, request);
                else
#endif // MFX_ENABLE_MVC_VIDEO_ENCODE
                    mfxRes = MFXVideoENCODEH264::QueryIOSurf(par, request);
            }
            else
            {
                bIsHWENCSupport = true;
            }
#else //MFX_VA
#ifdef MFX_ENABLE_MVC_VIDEO_ENCODE
            if (par->mfx.CodecProfile == MFX_PROFILE_AVC_MULTIVIEW_HIGH || par->mfx.CodecProfile == MFX_PROFILE_AVC_STEREO_HIGH)
                mfxRes = MFXVideoENCODEMVC::QueryIOSurf(par, request);
            else
#endif // MFX_ENABLE_MVC_VIDEO_ENCODE
                mfxRes = MFXVideoENCODEH264::QueryIOSurf(par, request);
#endif //MFX_VA
            break;
#endif // MFX_ENABLE_H264_VIDEO_ENCODE


#ifdef MFX_ENABLE_MPEG2_VIDEO_ENC
        case MFX_CODEC_MPEG2:
#if defined(MFX_VA)
            mfxRes = MFXVideoENCODEMPEG2_HW::QueryIOSurf(session->m_pCORE.get(), par, request);
            if (MFX_WRN_PARTIAL_ACCELERATION  == mfxRes)
            {
                mfxRes = MFXVideoENCODEMPEG2::QueryIOSurf(par, request);
            }
            else
            {
                bIsHWENCSupport = true;
            }
#else // MFX_VA
            mfxRes = MFXVideoENCODEMPEG2::QueryIOSurf(par, request);
#endif // MFX_VA
            break;
#endif // MFX_ENABLE_MPEG2_VIDEO_ENC


#ifdef MFX_ENABLE_VP8_VIDEO_ENCODE
        case MFX_CODEC_VP8:
#if defined(MFX_VA) && defined(MFX_ENABLE_VP8_VIDEO_ENCODE_HW)
            mfxRes = MFXHWVideoENCODEVP8::QueryIOSurf(session->m_pCORE.get(), par, request);
            if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
            {
                mfxRes = MFXVideoENCODEVP8::QueryIOSurf(par, request);
            }
            else
            {
                bIsHWENCSupport = true;
            }
#else // MFX_VA && MFX_ENABLE_VP8_VIDEO_ENCODE_HW
            mfxRes = MFXVideoENCODEVP8::QueryIOSurf(par, request);
#endif // MFX_VA && MFX_ENABLE_VP8_VIDEO_ENCODE_HW
                        break;
#endif // MFX_ENABLE_VP8_VIDEO_ENCODE


#if defined(MFX_ENABLE_MJPEG_VIDEO_ENCODE)
        case MFX_CODEC_JPEG:
#if defined(MFX_VA_WIN)
            mfxRes = MFXVideoENCODEMJPEG_HW::QueryIOSurf(session->m_pCORE.get(), par, request);
            if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
            {
                mfxRes = MFXVideoENCODEMJPEG::QueryIOSurf(par, request);
            }
            else
            {
                bIsHWENCSupport = true;
            }
            break;
#else // MFX_VA
            mfxRes = MFXVideoENCODEMJPEG::QueryIOSurf(par, request);
#endif // MFX_VA
            break;
#endif // MFX_ENABLE_MJPEG_VIDEO_ENCODE

#ifdef MFX_ENABLE_H265_VIDEO_ENCODE
        case MFX_CODEC_HEVC:
            mfxRes = MFXVideoENCODEH265::QueryIOSurf(session->m_pCORE.get(), par, request);
            break;
#endif // MFX_ENABLE_VC1_VIDEO_ENC

        default:
            mfxRes = MFX_ERR_UNSUPPORTED;
        }
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        mfxRes = MFX_ERR_NULL_PTR;
    }

    // SW fallback if EncodeGUID is absence
    if (MFX_PLATFORM_HARDWARE == session->m_currentPlatform &&
        !bIsHWENCSupport &&
        MFX_ERR_NONE <= mfxRes)
    {
        mfxRes = MFX_WRN_PARTIAL_ACCELERATION;
    }

    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, request);
    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;
}

mfxStatus MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par)
{
    mfxStatus mfxRes;

    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, par);

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(par, MFX_ERR_NULL_PTR);

    try
    {

#if !defined (MFX_RT)
        // check existence of component
        if (!session->m_pENCODE.get())
        {
            // create a new instance
            session->m_bIsHWENCSupport = true;
            session->m_pENCODE.reset(CreateENCODESpecificClass(par->mfx.CodecId, session->m_pCORE.get(), session, par));
            MFX_CHECK(session->m_pENCODE.get(), MFX_ERR_INVALID_VIDEO_PARAM);
        }
#endif

        mfxRes = session->m_pENCODE->Init(par);

        if (MFX_WRN_PARTIAL_ACCELERATION == mfxRes)
        {
            session->m_bIsHWENCSupport = false;
#if !defined (MFX_RT)
            session->m_pENCODE.reset(CreateENCODESpecificClass(par->mfx.CodecId, session->m_pCORE.get(), session, par));
            MFX_CHECK(session->m_pENCODE.get(), MFX_ERR_NULL_PTR);
            mfxRes = session->m_pENCODE->Init(par);
#endif
        }
        else if (mfxRes >= MFX_ERR_NONE)
            session->m_bIsHWENCSupport = true;

        // SW fallback if EncodeGUID is absence
        if (MFX_PLATFORM_HARDWARE == session->m_currentPlatform &&
            !session->m_bIsHWENCSupport &&
            MFX_ERR_NONE <= mfxRes)
        {
            mfxRes = MFX_WRN_PARTIAL_ACCELERATION;
        }
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_pENCODE.get())
        {
            mfxRes = MFX_ERR_INVALID_VIDEO_PARAM;
        }
        else if (0 == par)
        {
            mfxRes = MFX_ERR_NULL_PTR;
        }
    }

    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;

} // mfxStatus MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par)

mfxStatus MFXVideoENCODE_Close(mfxSession session)
{
    mfxStatus mfxRes = MFX_ERR_NONE;

    MFX_AUTO_LTRACE_FUNC(MFX_TRACE_LEVEL_API);

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pScheduler, MFX_ERR_NOT_INITIALIZED);

    try
    {
        if (!session->m_pENCODE.get())
        {
            return MFX_ERR_NOT_INITIALIZED;
        }

        // wait until all tasks are processed
        session->m_pScheduler->WaitForTaskCompletion(session->m_pENCODE.get());

        mfxRes = session->m_pENCODE->Close();
        // delete the codec's instance if not plugin
        if (!session->m_plgEnc.get())
        {
            session->m_pENCODE.reset((VideoENCODE *) 0);
        }
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
    }

    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;

} // mfxStatus MFXVideoENCODE_Close(mfxSession session)

static
mfxStatus MFXVideoENCODELegacyRoutine(void *pState, void *pParam,
                                      mfxU32 threadNumber, mfxU32 callNumber)
{
    MFX_AUTO_LTRACE(MFX_TRACE_LEVEL_SCHED, "EncodeFrame");
    VideoENCODE *pENCODE = (VideoENCODE *) pState;
    MFX_THREAD_TASK_PARAMETERS *pTaskParam = (MFX_THREAD_TASK_PARAMETERS *) pParam;
    mfxStatus mfxRes;

    // touch unreferenced parameter(s)
    callNumber = callNumber;

    // check error(s)
    if ((NULL == pState) ||
        (NULL == pParam) ||
        (0 != threadNumber))
    {
        return MFX_ERR_NULL_PTR;
    }

    // call the obsolete method
    mfxRes = pENCODE->EncodeFrame(pTaskParam->encode.ctrl,
                                  &pTaskParam->encode.internal_params,
                                  pTaskParam->encode.surface,
                                  pTaskParam->encode.bs);

    return mfxRes;

} // mfxStatus MFXVideoENCODELegacyRoutine(void *pState, void *pParam,

enum
{
    MFX_NUM_ENTRY_POINTS = 2
};

mfxStatus MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp)
{
    mfxStatus mfxRes;

    MFX_AUTO_LTRACE_WITHID(MFX_TRACE_LEVEL_API, "MFX_EncodeFrameAsync");
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, ctrl);
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, surface);

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    MFX_CHECK(session->m_pENCODE.get(), MFX_ERR_NOT_INITIALIZED);
    MFX_CHECK(syncp, MFX_ERR_NULL_PTR);

    try
    {
        mfxSyncPoint syncPoint = NULL;
        mfxFrameSurface1 *reordered_surface = NULL;
        mfxEncodeInternalParams internal_params;
        MFX_ENTRY_POINT entryPoints[MFX_NUM_ENTRY_POINTS];
        mfxU32 numEntryPoints = MFX_NUM_ENTRY_POINTS;
        mfxExtVppAuxData *aux;

        memset(&entryPoints, 0, sizeof(entryPoints));
        mfxRes = session->m_pENCODE->EncodeFrameCheck(ctrl,
                                                      surface,
                                                      bs,
                                                      &reordered_surface,
                                                      &internal_params,
                                                      entryPoints,
                                                      numEntryPoints,
                                                      aux);
        // source data is OK, go forward
        if ((MFX_ERR_NONE == mfxRes) ||
            (MFX_WRN_INCOMPATIBLE_VIDEO_PARAM == mfxRes) ||
            (MFX_WRN_OUT_OF_RANGE == mfxRes) ||
            // WHAT IS IT??? IT SHOULD BE REMOVED
            (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes) ||
            (MFX_ERR_MORE_BITSTREAM == mfxRes))
        {
            // prepare the obsolete kind of task.
            // it is obsolete and must be removed.
            if (NULL == entryPoints[0].pRoutine)
            {
                MFX_TASK task;

                memset(&task, 0, sizeof(task));
                // BEGIN OF OBSOLETE PART
                task.bObsoleteTask = true;
                task.obsolete_params.encode.internal_params = internal_params;
                // fill task info
                task.pOwner = session->m_pENCODE.get();
                task.entryPoint.pRoutine = &MFXVideoENCODELegacyRoutine;
                task.entryPoint.pState = session->m_pENCODE.get();
                task.entryPoint.requiredNumThreads = 1;

                // fill legacy parameters
                task.obsolete_params.encode.ctrl = ctrl;
                task.obsolete_params.encode.surface = reordered_surface;
                task.obsolete_params.encode.bs = bs;
                // END OF OBSOLETE PART

                task.priority = session->m_priority;
                task.threadingPolicy = session->m_pENCODE->GetThreadingPolicy();
                // fill dependencies
                task.pSrc[0] = reordered_surface;
                task.pDst[0] = bs;

// specific plug-in case to run additional task after main task 
#if !defined(AS_HEVCE_PLUGIN) 
                task.pSrc[1] =  bs;
#endif

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_ENCODE;
#endif // MFX_TRACE_ENABLE

                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));
            }
            else if (1 == numEntryPoints)
            {
                MFX_TASK task;

                memset(&task, 0, sizeof(task));
                task.pOwner = session->m_pENCODE.get();
                task.entryPoint = entryPoints[0];
                task.priority = session->m_priority;
                task.threadingPolicy = session->m_pENCODE->GetThreadingPolicy();
                // fill dependencies
                task.pSrc[0] = reordered_surface;
                task.pSrc[1] = aux;
                task.pDst[0] = bs;

                // specific plug-in case to run additional task after main task 
#if !defined(AS_HEVCE_PLUGIN) 
                task.pSrc[1] =  bs;
#endif

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_ENCODE;
#endif
                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));
            }
            else
            {
                MFX_TASK task;

                memset(&task, 0, sizeof(task));
                task.pOwner = session->m_pENCODE.get();
                task.entryPoint = entryPoints[0];
                task.priority = session->m_priority;
                task.threadingPolicy = MFX_TASK_THREADING_DEDICATED;
                // fill dependencies
                task.pSrc[0] = reordered_surface;
                task.pSrc[1] = aux;
                task.pDst[0] = entryPoints[0].pParam;

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_ENCODE;
#endif
                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));

                memset(&task, 0, sizeof(task));
                task.pOwner = session->m_pENCODE.get();
                task.entryPoint = entryPoints[1];
                task.priority = session->m_priority;
#if defined(SYNCHRONIZATION_BY_NON_ZERO_THREAD)
                if (MFX_HW_VAAPI == session->m_pCORE->GetVAType())
                    task.threadingPolicy = MFX_TASK_THREADING_WAIT;
                else
#endif
                    task.threadingPolicy = MFX_TASK_THREADING_DEDICATED_WAIT;
                // fill dependencies
                task.pSrc[0] = entryPoints[0].pParam;
                task.pDst[0] = bs;

#ifdef MFX_TRACE_ENABLE
                task.nParentId = MFX_AUTO_TRACE_GETID();
                task.nTaskId = MFX::CreateUniqId() + MFX_TRACE_ID_ENCODE2;
#endif
                // register input and call the task
                MFX_CHECK_STS(session->m_pScheduler->AddTask(task, &syncPoint));
            }

            // IT SHOULD BE REMOVED
            if (MFX_ERR_MORE_DATA_RUN_TASK == mfxRes)
            {
                mfxRes = MFX_ERR_MORE_DATA;
                syncPoint = NULL;
            }
        }

        // return pointer to synchronization point
        *syncp = syncPoint;
    }
    // handle error(s)
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_pENCODE.get())
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
        else if (0 == syncp)
        {
            return MFX_ERR_NULL_PTR;
        }
    }

    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL_API, bs);
    if (mfxRes == MFX_ERR_NONE && syncp)
    {
        MFX_LTRACE_P(MFX_TRACE_LEVEL_API, *syncp);
    }
    MFX_LTRACE_I(MFX_TRACE_LEVEL_API, mfxRes);
    return mfxRes;

} // mfxStatus MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp)

//
// THE OTHER ENCODE FUNCTIONS HAVE IMPLICIT IMPLEMENTATION
//

mfxStatus MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par)
{
    mfxStatus mfxRes;
    try
    {
        /* the absent components caused many issues in application.
        check the pointer to avoid extra exceptions */
        if (0 == session->m_pENCODE.get())
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
        else
        {
            /* wait until all tasks are processed */
            session->m_pScheduler->WaitForTaskCompletion(session->m_pENCODE.get());
            /* call the codec's method */
            mfxRes = session->m_pENCODE->Reset(par);

            // SW fallback if EncodeGUID is absence
            if (MFX_PLATFORM_HARDWARE == session->m_currentPlatform &&
                !session->m_bIsHWENCSupport &&
                MFX_ERR_NONE <= mfxRes)
            {
                mfxRes = MFX_WRN_PARTIAL_ACCELERATION;
            }
        }
    }
    /* handle error(s) */
    catch(MFX_CORE_CATCH_TYPE)
    {
        /* set the default error value */
        mfxRes = MFX_ERR_NULL_PTR;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_pENCODE.get())
        {
            mfxRes = MFX_ERR_NOT_INITIALIZED;
        }
    }
    return mfxRes;

} // mfxStatus MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par)

FUNCTION_IMPL(ENCODE, GetVideoParam, (mfxSession session, mfxVideoParam *par), (par))
FUNCTION_IMPL(ENCODE, GetEncodeStat, (mfxSession session, mfxEncodeStat *stat), (stat))
