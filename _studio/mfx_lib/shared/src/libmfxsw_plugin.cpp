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

File Name: libmfxsw_plugin.cpp

\* ****************************************************************************** */

#include <mfxplugin.h>
#include <mfx_session.h>
#include <mfx_task.h>
#include <mfx_user_plugin.h>
#include <mfx_utils.h>

#if defined(MFX_RT)

// static section of the file
namespace
{

    VideoCodecUSER *CreateUSERSpecificClass(mfxU32 type)
    {
        type;
        return new VideoUSERPlugin;

    } // VideoUSER *CreateUSERSpecificClass(mfxU32 type)

    struct SessionPtr
    {
    private:
        mfxSession _session;
        mfxU32 _type;
        std::auto_ptr<VideoCodecUSER> *_ptr;
        bool _isNeedCodec;
        bool _isNeedDeCoder;
        bool _isNeedVPP;
        mutable std::auto_ptr<VideoDECODE> _stubDecode;
        mutable std::auto_ptr<VideoENCODE> _stubEncode;
        mutable std::auto_ptr<VideoVPP> _stubVPP;
    public:
        SessionPtr(mfxSession session, mfxU32 type = MFX_PLUGINTYPE_VIDEO_GENERAL)
            : _session(session)
        {
            switch(type)
            {
            case MFX_PLUGINTYPE_VIDEO_DECODE :
                _ptr = &_session->m_plgDec; 
                _isNeedCodec = false;
                _isNeedDeCoder = true;
                _isNeedVPP = false;
                break;
            case MFX_PLUGINTYPE_VIDEO_ENCODE:
                _ptr =&_session->m_plgEnc; 
                _isNeedCodec = true;
                _isNeedDeCoder = false;
                _isNeedVPP = false;
                break;
            case MFX_PLUGINTYPE_VIDEO_VPP :
                _ptr = &_session->m_plgVPP; 
                _isNeedCodec = false;
                _isNeedDeCoder = false;
                _isNeedVPP = true;
                break;
            case MFX_PLUGINTYPE_VIDEO_GENERAL :
                _ptr = &_session->m_plgGen; 
                _isNeedCodec = false;
                _isNeedDeCoder = false;
                _isNeedVPP = false;
                break;
            case MFX_PLUGINTYPE_VIDEO_ENC :
            default : 
                //unknown plugin type
                throw MFX_ERR_UNDEFINED_BEHAVIOR;
            }
        }
        std::auto_ptr<VideoCodecUSER>& plugin()const
        {
            return *_ptr;
        }

        template <class T>
        std::auto_ptr<T>& codec()const
        {
        }
 
        bool isNeedEncoder()const
        {
            return _isNeedCodec ;
        }
        bool isNeedDecoder()const
        {
            return _isNeedDeCoder;
        }
        bool isNeedVPP()const
        {
            return _isNeedVPP;
        }
    };      



    template <>
    std::auto_ptr<VideoENCODE>& SessionPtr::codec<VideoENCODE>()const
    {
        return _isNeedCodec ? _session->m_pENCODE : _stubEncode;
    }
    template <>
    std::auto_ptr<VideoDECODE>& SessionPtr::codec<VideoDECODE>()const
    {
        return _isNeedDeCoder ? _session->m_pDECODE : _stubDecode;
    }
    template <>
    std::auto_ptr<VideoVPP>& SessionPtr::codec<VideoVPP>()const
    {
        return _isNeedVPP ? _session->m_pVPP : _stubVPP;
    }


} // namespace


mfxStatus MFXVideoUSER_Register(mfxSession session, mfxU32 type,
                                const mfxPlugin *par)
{
    mfxStatus mfxRes;

    // check error(s)
    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    
    try
    {
        SessionPtr sessionPtr(session, type);
        std::auto_ptr<VideoCodecUSER> & pluginPtr = sessionPtr.plugin();
        std::auto_ptr<VideoENCODE> &encPtr = sessionPtr.codec<VideoENCODE>();
        std::auto_ptr<VideoDECODE> &decPtr = sessionPtr.codec<VideoDECODE>();
        std::auto_ptr<VideoVPP> &vppPtr = sessionPtr.codec<VideoVPP>();

        // the plugin with the same type is already exist
        if (pluginPtr.get() || decPtr.get() || encPtr.get())
        {
            return MFX_ERR_UNDEFINED_BEHAVIOR;
        }
        // create a new plugin's instance
        pluginPtr.reset(CreateUSERSpecificClass(type));
        MFX_CHECK(pluginPtr.get(), MFX_ERR_INVALID_VIDEO_PARAM);

        if (sessionPtr.isNeedDecoder()) {
            decPtr.reset(pluginPtr->GetDecodePtr());
        }

        if (sessionPtr.isNeedEncoder()) {
            encPtr.reset(pluginPtr->GetEncodePtr());
        }

        if (sessionPtr.isNeedVPP()) {
            vppPtr.reset(pluginPtr->GetVPPPtr());
        }

        // initialize the plugin
        mfxRes = pluginPtr->PluginInit(par, session, type);
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
       /* else if (0 == registeredPlg || 0 == registeredPlg->get())
        {
            mfxRes = MFX_ERR_INVALID_VIDEO_PARAM;
        }*/
        else if (0 == par)
        {
            mfxRes = MFX_ERR_NULL_PTR;
        }
        else if (type > MFX_PLUGINTYPE_VIDEO_ENCODE)
        {
            mfxRes = MFX_ERR_UNDEFINED_BEHAVIOR;
        }
    }

    return mfxRes;

} // mfxStatus MFXVideoUSER_Register(mfxSession session, mfxU32 type,

mfxStatus MFXVideoUSER_Unregister(mfxSession session, mfxU32 type)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    try
    {
        SessionPtr sessionPtr(session, type);
        std::auto_ptr<VideoCodecUSER> & registeredPlg = sessionPtr.plugin();
        if (NULL == registeredPlg.get())
            return MFX_ERR_NOT_INITIALIZED;

        // wait until all tasks are processed
        session->m_pScheduler->WaitForTaskCompletion(registeredPlg.get());

        // deinitialize the plugin
        mfxRes = registeredPlg->PluginClose();
        
        // delete the plugin's instance
        registeredPlg.reset();
        //delete corresponding codec instance
        if (sessionPtr.isNeedDecoder()) {
            sessionPtr.codec<VideoDECODE>().reset();
        }
        if (sessionPtr.isNeedEncoder()) {
            sessionPtr.codec<VideoENCODE>().reset();
        }
        if (sessionPtr.isNeedVPP()) {
            sessionPtr.codec<VideoVPP>().reset();
        }
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            mfxRes = MFX_ERR_INVALID_HANDLE;
        }
        else if (type > MFX_PLUGINTYPE_VIDEO_ENCODE)
        {
            mfxRes = MFX_ERR_UNDEFINED_BEHAVIOR;
        }
    }

    return mfxRes;

} // mfxStatus MFXVideoUSER_Unregister(mfxSession session, mfxU32 type)

mfxStatus MFXVideoUSER_ProcessFrameAsync(mfxSession session,
                                         const mfxHDL *in, mfxU32 in_num,
                                         const mfxHDL *out, mfxU32 out_num,
                                         mfxSyncPoint *syncp)
{
    mfxStatus mfxRes;

    MFX_CHECK(session, MFX_ERR_INVALID_HANDLE);
    //MFX_CHECK(session->m_pUSER.get(), MFX_ERR_NOT_INITIALIZED);
    MFX_CHECK(syncp, MFX_ERR_NULL_PTR);
    try
    {
        //generic plugin function
        std::auto_ptr<VideoCodecUSER> & registeredPlg = SessionPtr(session).plugin();

        mfxSyncPoint syncPoint = NULL;
        MFX_TASK task;

        memset(&task, 0, sizeof(MFX_TASK));
        mfxRes = registeredPlg->Check(in, in_num, out, out_num, &task.entryPoint);
        // source data is OK, go forward
        if (MFX_ERR_NONE == mfxRes)
        {
            mfxU32 i;

            task.pOwner = registeredPlg.get();
            task.priority = session->m_priority;
            task.threadingPolicy = registeredPlg->GetThreadingPolicy();
            // fill dependencies
            for (i = 0; i < in_num; i += 1)
            {
                task.pSrc[i] = in[i];
            }
            for (i = 0; i < out_num; i += 1)
            {
                task.pDst[i] = out[i];
            }

            // register input and call the task
            mfxRes = session->m_pScheduler->AddTask(task, &syncPoint);
        }

        // return pointer to synchronization point
        *syncp = syncPoint;
    }
    catch(MFX_CORE_CATCH_TYPE)
    {
        // set the default error value
        mfxRes = MFX_ERR_UNKNOWN;
        if (0 == session)
        {
            return MFX_ERR_INVALID_HANDLE;
        }
        else if (0 == session->m_plgGen.get())
        {
            return MFX_ERR_NOT_INITIALIZED;
        }
        else if (0 == syncp)
        {
            return MFX_ERR_NULL_PTR;
        }
    }

    return mfxRes;

} // mfxStatus MFXVideoUSER_ProcessFrameAsync(mfxSession session,

#endif // MFX_RT
