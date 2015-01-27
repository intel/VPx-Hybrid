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

File Name: mfx_session.h

\* ****************************************************************************** */
#if !defined(_MFX_SESSION_H)
#define _MFX_SESSION_H

#include <memory>

// base mfx headers
#include <mfxdefs.h>
#include <mfxstructures.h>
#include <mfxvideo++int.h>
#include <mfxaudio++int.h>
#include <mfxplugin.h>
#include "mfx_common.h"

// private headers
#include <mfx_interface_scheduler.h>
#include <libmfx_core_operation.h>

// WARNING: please do not change the type of _mfxSession.
// It is declared as 'struct' in the main header.h

template <class T>
class mfx_core_ptr
{
public:

    explicit mfx_core_ptr(T* ptr = 0)
        : m_bIsNew(false)
        , m_ptr(ptr)
                                      
    {
    };
    void reset (T* ptr = 0, bool isNew = true)
    {
        if (m_bIsNew)
            delete m_ptr;
        
        m_ptr = ptr;
        m_bIsNew = isNew;
    };
    T* get() const
    {
        return m_ptr;
    };
    virtual ~mfx_core_ptr()
    {
        if (m_bIsNew)
            delete m_ptr;

        m_ptr = 0;
    };
    T* operator->() const
    {
        return get();
    }

protected:
    mfx_core_ptr<T>& operator=(mfx_core_ptr<T>& ptr);
    bool m_bIsNew;
    T*   m_ptr;
};



struct _mfxSession
{
    // Constructor
    _mfxSession(const mfxU32 adapterNum);
    // Destructor
    ~_mfxSession(void);

    // Clear state
    void Clear(void);

    // Initialize the session
    mfxStatus Init(mfxIMPL implInterface, mfxVersion *ver);

    // Attach to the original scheduler
    mfxStatus RestoreScheduler(void);
    // Release current scheduler
    mfxStatus ReleaseScheduler(void);

    // Declare session's components
    mfx_core_ptr<VideoCORE> m_pCORE;
    std::auto_ptr<AudioCORE> m_pAudioCORE;
    std::auto_ptr<VideoENCODE> m_pENCODE;
    std::auto_ptr<VideoDECODE> m_pDECODE;
    std::auto_ptr<AudioENCODE> m_pAudioENCODE;
    std::auto_ptr<AudioDECODE> m_pAudioDECODE;
    std::auto_ptr<VideoVPP> m_pVPP;
    std::auto_ptr<VideoENC> m_pENC;
    std::auto_ptr<VideoPAK> m_pPAK;
    std::auto_ptr<VideoBRC> m_pBRC;
    
    std::auto_ptr<VideoCodecUSER> m_plgDec;
    std::auto_ptr<VideoCodecUSER> m_plgEnc;
    std::auto_ptr<VideoCodecUSER> m_plgVPP;
    std::auto_ptr<VideoCodecUSER> m_plgGen;

    // Wrapper of interface for core object
    mfxCoreInterface m_coreInt;

    // Current implementation platform ID
    eMFXPlatform m_currentPlatform;
    // Current working HW adapter
    const
    mfxU32 m_adapterNum;
    // Current working interface (D3D9 or so)
    mfxIMPL m_implInterface;

    // Pointer to the scheduler interface being used
    MFXIScheduler *m_pScheduler;
    // Priority of the given session instance
    mfxPriority m_priority;
    // actual version of API
    mfxVersion  m_version;

    MFXIPtr<OperatorCORE> m_pOperatorCore;

    // if there are no Enc HW capabilities but HW library is used
    bool m_bIsHWENCSupport;

    // if there are no Dec HW capabilities but HW library is used
    bool m_bIsHWDECSupport;


    inline
    bool IsParentSession(void)
    {
        // parent session has multiple references to its scheduler.
        // regular session has 2 references to the scheduler.
        // child session has only 1 reference to it.
        if(m_pSchedulerAllocated)
            return (2 < m_pSchedulerAllocated->GetNumRef());
        else
            return false;
    }

    inline
    bool IsChildSession(void)
    {
        // child session has different references to active and allocated
        // scheduler. regular session has 2 references to the scheduler.
        // child session has only 1 reference to it.
        return (NULL == m_pSchedulerAllocated);
    }


protected:
    // Release the object
    void Release(void);

    // this variable is used to deteremine
    // if the object really owns the scheduler.
    MFXIUnknown *m_pSchedulerAllocated;                         // (MFXIUnknown *) pointer to the scheduler allocated

private:
    // Assignment operator is forbidden
    _mfxSession & operator = (const _mfxSession &);
};

//
// DEFINES FOR IMPLICIT FUNCTIONS IMPLEMENTATION
//

#undef FUNCTION_IMPL
#define FUNCTION_IMPL(component, func_name, formal_param_list, actual_param_list) \
mfxStatus MFXVideo##component##_##func_name formal_param_list \
{ \
    mfxStatus mfxRes; \
    try \
    { \
        if (0 == session) \
        { \
            mfxRes = MFX_ERR_INVALID_HANDLE; \
        } \
        /* the absent components caused many issues in application. \
        check the pointer to avoid extra exceptions */ \
        else if (0 == session->m_p##component.get()) \
        { \
            mfxRes = MFX_ERR_NOT_INITIALIZED; \
        } \
        else \
        { \
            /* call the codec's method */ \
            mfxRes = session->m_p##component->func_name actual_param_list; \
        } \
    } \
    /* handle error(s) */ \
    catch(MFX_CORE_CATCH_TYPE) \
    { \
        /* set the default error value */ \
        mfxRes = MFX_ERR_NULL_PTR; \
        if (0 == session) \
        { \
            mfxRes = MFX_ERR_INVALID_HANDLE; \
        } \
        else if (0 == session->m_p##component.get()) \
        { \
            mfxRes = MFX_ERR_NOT_INITIALIZED; \
        } \
    } \
    return mfxRes; \
}

#undef FUNCTION_AUDIO_IMPL
#define FUNCTION_AUDIO_IMPL(component, func_name, formal_param_list, actual_param_list) \
    mfxStatus MFXAudio##component##_##func_name formal_param_list \
{ \
    mfxStatus mfxRes; \
    try \
    { \
    /* the absent components caused many issues in application. \
check the pointer to avoid extra exceptions */ \
    if (0 == session->m_pAudio##component.get()) \
        { \
        mfxRes = MFX_ERR_NOT_INITIALIZED; \
} \
        else \
        { \
        /* call the codec's method */ \
        mfxRes = session->m_pAudio##component->func_name actual_param_list; \
} \
} \
    /* handle error(s) */ \
    catch(MFX_CORE_CATCH_TYPE) \
    { \
    /* set the default error value */ \
    mfxRes = MFX_ERR_NULL_PTR; \
    if (0 == session) \
        { \
        mfxRes = MFX_ERR_INVALID_HANDLE; \
} \
        else if (0 == session->m_pAudio##component.get()) \
        { \
        mfxRes = MFX_ERR_NOT_INITIALIZED; \
} \
} \
    return mfxRes; \
}


#undef FUNCTION_RESET_IMPL
#define FUNCTION_RESET_IMPL(component, func_name, formal_param_list, actual_param_list) \
mfxStatus MFXVideo##component##_##func_name formal_param_list \
{ \
    mfxStatus mfxRes; \
    try \
    { \
        if (0 == session) \
        { \
            mfxRes = MFX_ERR_INVALID_HANDLE; \
        } \
        /* the absent components caused many issues in application. \
        check the pointer to avoid extra exceptions */ \
        else if (0 == session->m_p##component.get()) \
        { \
            mfxRes = MFX_ERR_NOT_INITIALIZED; \
        } \
        else \
        { \
            /* wait until all tasks are processed */ \
            session->m_pScheduler->WaitForTaskCompletion(session->m_p##component.get()); \
            /* call the codec's method */ \
            mfxRes = session->m_p##component->func_name actual_param_list; \
        } \
    } \
    /* handle error(s) */ \
    catch(MFX_CORE_CATCH_TYPE) \
    { \
        /* set the default error value */ \
        mfxRes = MFX_ERR_NULL_PTR; \
        if (0 == session) \
        { \
            mfxRes = MFX_ERR_INVALID_HANDLE; \
        } \
        else if (0 == session->m_p##component.get()) \
        { \
            mfxRes = MFX_ERR_NOT_INITIALIZED; \
        } \
    } \
    return mfxRes; \
}

#undef FUNCTION_AUDIO_RESET_IMPL
#define FUNCTION_AUDIO_RESET_IMPL(component, func_name, formal_param_list, actual_param_list) \
    mfxStatus MFXAudio##component##_##func_name formal_param_list \
{ \
    mfxStatus mfxRes; \
    try \
    { \
    /* the absent components caused many issues in application. \
check the pointer to avoid extra exceptions */ \
    if (0 == session->m_pAudio##component.get()) \
        { \
        mfxRes = MFX_ERR_NOT_INITIALIZED; \
} \
        else \
        { \
        /* wait until all tasks are processed */ \
        session->m_pScheduler->WaitForTaskCompletion(session->m_pAudio##component.get()); \
        /* call the codec's method */ \
        mfxRes = session->m_pAudio##component->func_name actual_param_list; \
} \
} \
    /* handle error(s) */ \
    catch(MFX_CORE_CATCH_TYPE) \
    { \
    /* set the default error value */ \
    mfxRes = MFX_ERR_NULL_PTR; \
    if (0 == session) \
        { \
        mfxRes = MFX_ERR_INVALID_HANDLE; \
} \
        else if (0 == session->m_pAudio##component.get()) \
        { \
        mfxRes = MFX_ERR_NOT_INITIALIZED; \
} \
} \
    return mfxRes; \
}

mfxStatus MFXInternalPseudoJoinSession(mfxSession session, mfxSession child_session);
mfxStatus MFXInternalPseudoDisjoinSession(mfxSession session);

#endif // _MFX_SESSION_H
