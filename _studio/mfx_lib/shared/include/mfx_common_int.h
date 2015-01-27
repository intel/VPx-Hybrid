/* ****************************************************************************** *\

Copyright (C) 2008 - 2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_common_int.h

\* ****************************************************************************** */
#ifndef __MFX_COMMON_INT_H__
#define __MFX_COMMON_INT_H__

#include <vector>
#include <memory>
#include <errno.h>
#include "mfx_common.h"
#include "mfxaudio.h"

#if defined(ANDROID)
typedef int error_t;
#endif

mfxStatus CheckFrameInfoCommon(mfxFrameInfo  *info, mfxU32 codecId);
mfxStatus CheckFrameInfoEncoders(mfxFrameInfo  *info);
mfxStatus CheckFrameInfoCodecs(mfxFrameInfo  *info, mfxU32 codecId = MFX_CODEC_AVC);

mfxStatus CheckVideoParamEncoders(mfxVideoParam *in, bool IsExternalFrameAllocator, eMFXHWType type);
mfxStatus CheckVideoParamDecoders(mfxVideoParam *in, bool IsExternalFrameAllocator, eMFXHWType type);

mfxStatus CheckAudioParamEncoders(mfxAudioParam *in);
mfxStatus CheckAudioParamCommon(mfxAudioParam *in);
mfxStatus CheckAudioParamDecoders(mfxAudioParam *in);

mfxStatus CheckBitstream(const mfxBitstream *bs);
mfxStatus CheckAudioFrame(const mfxAudioFrame *aFrame);
mfxStatus CheckFrameData(const mfxFrameSurface1 *surface);

mfxStatus CheckDecodersExtendedBuffers(mfxVideoParam* par);

mfxExtBuffer* GetExtendedBuffer(mfxExtBuffer** extBuf, mfxU32 numExtBuf, mfxU32 id);
mfxExtBuffer* GetExtendedBufferInternal(mfxExtBuffer** extBuf, mfxU32 numExtBuf, mfxU32 id);

class ExtendedBuffer
{
public:

    ExtendedBuffer();
    virtual ~ExtendedBuffer();

    template<typename T> void AddTypedBuffer(mfxU32 id)
    {
        if (GetBufferByIdInternal(id))
            return;

        mfxExtBuffer * buffer = (mfxExtBuffer *)(new mfxU8[sizeof(T)]);
        memset(buffer, 0, sizeof(T));
        buffer->BufferSz = sizeof(T);
        buffer->BufferId = id;
        AddBufferInternal(buffer);
    }

    void AddBuffer(mfxExtBuffer * buffer);

    size_t GetCount() const;

    template<typename T> T * GetBufferById(mfxU32 id)
    {
        return (T*)GetBufferByIdInternal(id);
    }

    template<typename T> T * GetBufferByPosition(mfxU32 pos)
    {
        return (T*)GetBufferByPositionInternal(pos);
    }

    mfxExtBuffer ** GetBuffers();

private:

    void AddBufferInternal(mfxExtBuffer * buffer);

    mfxExtBuffer * GetBufferByIdInternal(mfxU32 id);

    mfxExtBuffer * GetBufferByPositionInternal(mfxU32 pos);

    void Release();

    typedef std::vector<mfxExtBuffer *>  BuffersList;
    BuffersList m_buffers;
};

class mfxVideoParamWrapper : public mfxVideoParam
{
public:

    mfxVideoParamWrapper();

    mfxVideoParamWrapper(const mfxVideoParam & par);

    virtual ~mfxVideoParamWrapper();

    mfxVideoParamWrapper & operator = (const mfxVideoParam & par);

    mfxVideoParamWrapper & operator = (const mfxVideoParamWrapper & par);

    bool CreateExtendedBuffer(mfxU32 bufferId);

    template<typename T> T * GetExtendedBuffer(mfxU32 id)
    {
        T * extBuf = m_buffers.GetBufferById<T>(id);

        if (!extBuf)
        {
            m_buffers.AddTypedBuffer<T>(id);
            extBuf = m_buffers.GetBufferById<T>(id);
            if (!extBuf)
                throw 1;
        }

        return extBuf;
    }

private:

    ExtendedBuffer m_buffers;
    mfxU8* m_mvcSequenceBuffer;

    void CopyVideoParam(const mfxVideoParam & par);

    // Deny copy constructor
    mfxVideoParamWrapper(const mfxVideoParamWrapper &);
};

/*
#pragma warning(disable: 4127)
template<typename T1, typename T2>
#if defined(__GNUC__)
__attribute__((always_inline))
#else
inline
#endif
mfxU32 memcpy_s(T1* pDst, T2* pSrc)
{
    if (sizeof(T1) != sizeof(T2))
        return 0;

    ippsCopy_8u((Ipp8u*)pSrc, (Ipp8u*)pDst, sizeof(T1));
    return sizeof(T1);
}
*/

#endif