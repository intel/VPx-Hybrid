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

File Name: mfxaudio++int.h

\* ****************************************************************************** */
#ifndef __MFXAUDIOPLUSPLUS_INTERNAL_H
#define __MFXAUDIOPLUSPLUS_INTERNAL_H

#include "mfxaudio.h"
#include <mfx_task_threading_policy.h>
#include <mfx_interface.h>



#if defined(_WIN32) || defined(_WIN64)
#include <guiddef.h>
#else
#ifndef GUID_TYPE_DEFINED

#include <string.h>
// TODO: temporary workaround for linux (aya: see below)
typedef struct {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;

static int operator==(const GUID & guidOne, const GUID & guidOther)
{
    return !memcmp(&guidOne, &guidOne, sizeof(GUID));
}
#define GUID_TYPE_DEFINED
#endif

#endif

#if defined(_WIN32) || defined(_WIN64)
#undef MFX_DEBUG_TOOLS
#define MFX_DEBUG_TOOLS

#if defined(DEBUG) || defined(_DEBUG)
#undef  MFX_DEBUG_TOOLS // to avoid redefinition
#define MFX_DEBUG_TOOLS
#endif
#endif // #if defined(_WIN32) || defined(_WIN64)




// This is the include file for Media SDK component development.
enum eMFXAudioPlatform
{
    MFX_AUDIO_PLATFORM_SOFTWARE      = 0,
    MFX_AUDIO_PLATFORM_HARDWARE      = 1,
};





#ifdef MFX_DEBUG_TOOLS
// commented since running behavior tests w/o catch is very annoying
// #define MFX_CORE_CATCH_TYPE     int**** // to disable catch
#define MFX_CORE_CATCH_TYPE     ...
#else
#define MFX_CORE_CATCH_TYPE     ...
#endif

// Forward declaration of used classes
struct MFX_ENTRY_POINT;
 
// AudioCORE
class AudioCORE {
public:

    virtual ~AudioCORE(void) {};

    // imported to external API
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL *handle) = 0;
    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL handle) = 0;
    virtual mfxStatus SetBufferAllocator(mfxBufferAllocator *allocator) = 0;
    virtual mfxStatus SetFrameAllocator(mfxFrameAllocator *allocator) = 0;

    // Internal interface only
    // Utility functions for memory access
    virtual mfxStatus  AllocBuffer(mfxU32 nbytes, mfxU16 type, mfxMemId *mid) = 0;
    virtual mfxStatus  LockBuffer(mfxMemId mid, mfxU8 **ptr) = 0;
    virtual mfxStatus  UnlockBuffer(mfxMemId mid) = 0;
    virtual mfxStatus  FreeBuffer(mfxMemId mid) = 0;

    
        
    virtual mfxStatus  AllocFrames(mfxFrameAllocRequest *request, 
                                   mfxFrameAllocResponse *response) = 0;

    virtual mfxStatus  AllocFrames(mfxFrameAllocRequest *request, 
                                   mfxFrameAllocResponse *response, 
                                   mfxFrameSurface1 **pOpaqueSurface, 
                                   mfxU32 NumOpaqueSurface) = 0;

    virtual mfxStatus  LockFrame(mfxMemId mid, mfxFrameData *ptr) = 0;
    virtual mfxStatus  UnlockFrame(mfxMemId mid, mfxFrameData *ptr=0) = 0;
    virtual mfxStatus  FreeFrames(mfxFrameAllocResponse *response, bool ExtendedSearch = true) = 0;

    virtual mfxStatus  LockExternalFrame(mfxMemId mid, mfxFrameData *ptr, bool ExtendedSearch = true) = 0;
    virtual mfxStatus  UnlockExternalFrame(mfxMemId mid, mfxFrameData *ptr=0, bool ExtendedSearch = true) = 0;

    virtual mfxMemId MapIdx(mfxMemId mid) = 0;


    // Increment Surface lock
    virtual mfxStatus  IncreaseReference(mfxFrameData *ptr, bool ExtendedSearch = true) = 0;
    // Decrement Surface lock
    virtual mfxStatus  DecreaseReference(mfxFrameData *ptr, bool ExtendedSearch = true) = 0;

    // no care about surface, opaq and all round. Just increasing reference
    virtual mfxStatus IncreasePureReference(mfxU16 &) = 0;
    // no care about surface, opaq and all round. Just decreasing reference
    virtual mfxStatus DecreasePureReference(mfxU16 &) = 0;
    virtual void* QueryCoreInterface(const MFX_GUID &guid) = 0;
    virtual mfxSession GetSession() = 0;
};     


class AudioENCODE
{
public:
    // Destructor
    virtual
    ~AudioENCODE(void) {}
   
    virtual
    mfxStatus Init(mfxAudioParam *par) = 0;
    virtual
    mfxStatus Reset(mfxAudioParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetAudioParam(mfxAudioParam *par) = 0;
    virtual
    mfxStatus EncodeFrameCheck(mfxAudioFrame *aFrame, mfxBitstream *buffer_out) = 0;
    virtual
    mfxStatus EncodeFrame(mfxAudioFrame *bs, mfxBitstream *buffer_out) = 0;

    virtual mfxStatus EncodeFrameCheck(mfxAudioFrame *aFrame, mfxBitstream *buffer_out,
        MFX_ENTRY_POINT *pEntryPoint)  = 0;
  
};

class AudioDECODE
{
public:
    // Destructor
    virtual
    ~AudioDECODE(void) {}

    virtual
    mfxStatus Init(mfxAudioParam *par) = 0;
    virtual
    mfxStatus Reset(mfxAudioParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetAudioParam(mfxAudioParam *par) = 0;
    virtual
    mfxStatus DecodeFrameCheck(mfxBitstream *bs,  mfxAudioFrame *aFrame,
        MFX_ENTRY_POINT *pEntryPoint)
    {
        pEntryPoint = pEntryPoint;
        return DecodeFrameCheck(bs, aFrame);
    }
    virtual
    mfxStatus DecodeFrameCheck(mfxBitstream *bs, mfxAudioFrame *aFrame) = 0;
    virtual
    mfxStatus DecodeFrame(mfxBitstream *bs, mfxAudioFrame *buffer_out) = 0;
 protected:



};
   /*
class AudioPP
{
public:
    // Destructor
    virtual
    ~AudioPP(void) {}

    virtual
    mfxStatus Init(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Reset(mfxVideoParam *par) = 0;
    virtual
    mfxStatus Close(void) = 0;
    virtual
    mfxTaskThreadingPolicy GetThreadingPolicy(void) {return MFX_TASK_THREADING_DEFAULT;}

    virtual
    mfxStatus GetVideoParam(mfxVideoParam *par) = 0;
    virtual
    mfxStatus GetVPPStat(mfxVPPStat *stat) = 0;
    virtual
    mfxStatus AudioppFrameCheck(mfxFrameSurface1 *in,
                            mfxFrameSurface1 *out,
                            mfxExtVppAuxData *aux,
                            MFX_ENTRY_POINT *pEntryPoint)
    {
        pEntryPoint = pEntryPoint;
        aux = aux;
        return AudioppFrameCheck(in, out);
    }

    virtual
    mfxStatus AudioppFrameCheck(mfxFrameSurface1 *in,
                            mfxFrameSurface1 *out,
                            mfxExtVppAuxData *aux,
                            MFX_ENTRY_POINT pEntryPoints[],
                            mfxU32 &numEntryPoints)
    {
        mfxStatus mfxRes;

        // call the overweighted version
        mfxRes = VppFrameCheck(in, out, aux, pEntryPoints);
        numEntryPoints = 1;

        return mfxRes;
    }

    virtual
    mfxStatus AudioppFrameCheck(mfxFrameSurface1 *in, mfxFrameSurface1 *out) = 0;
    virtual
    mfxStatus RunFrameAudioPP(mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux) = 0;

};

   */

struct ThreadAudioDecodeTaskInfo
{
    mfxAudioFrame         *out;
    mfxU32                 taskID; // for task ordering
};

struct ThreadAudioEncodeTaskInfo
{
    mfxAudioFrame          *in;
    mfxBitstream           *out;
    mfxU32                 taskID; // for task ordering
};

#endif // __MFXAUDIOPLUSPLUS_INTERNAL_H
