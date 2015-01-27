/* ****************************************************************************** *\

Copyright (C) 2009-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_common_int.cpp

\* ****************************************************************************** */

#include "mfx_common_int.h"
#include <stdexcept>
#include <string>

#if defined(_WIN32) || defined(_WIN64) 
#include <DXGI.h>
#endif

mfxStatus CheckVideoParamCommon(mfxVideoParam *in, eMFXHWType type);

mfxExtBuffer* GetExtendedBuffer(mfxExtBuffer** extBuf, mfxU32 numExtBuf, mfxU32 id)
{
    if (extBuf != 0)
    {
        for (mfxU16 i = 0; i < numExtBuf; i++)
        {
            if (extBuf[i] != 0 && extBuf[i]->BufferId == id) // assuming aligned buffers
                return (extBuf[i]);
        }
    }

    return 0;
}

mfxExtBuffer* GetExtendedBufferInternal(mfxExtBuffer** extBuf, mfxU32 numExtBuf, mfxU32 id)
{
    mfxExtBuffer* result = GetExtendedBuffer(extBuf, numExtBuf, id);

    if (!result) throw std::logic_error(": no external buffer found");
    return result;
}

mfxStatus CheckFrameInfoCommon(mfxFrameInfo  *info, mfxU32 /* codecId */)
{
    if (!info)
        return MFX_ERR_NULL_PTR;

    if ((info->Width % 16) || !info->Width)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if ((info->Height % 16) || !info->Height)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    switch (info->FourCC)
    {
    case MFX_FOURCC_NV12:
    case MFX_FOURCC_YV12:
    case MFX_FOURCC_YUY2:
    case MFX_FOURCC_RGB3:
    case MFX_FOURCC_RGB4:
    case MFX_FOURCC_P010:
#if defined(_WIN32) || defined(_WIN64) 
    case DXGI_FORMAT_AYUV:
#endif
        break;
    default:
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (info->ChromaFormat > MFX_CHROMAFORMAT_YUV444)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

mfxStatus CheckFrameInfoEncoders(mfxFrameInfo  *info)
{
    if (info->CropX > info->Width)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (info->CropY > info->Height)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (info->CropX + info->CropW > info->Width)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (info->CropY + info->CropH > info->Height)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    switch (info->PicStruct)
    {
    case MFX_PICSTRUCT_UNKNOWN:
    case MFX_PICSTRUCT_PROGRESSIVE:
    case MFX_PICSTRUCT_FIELD_TFF:
    case MFX_PICSTRUCT_FIELD_BFF:
        break;

    default:
    case MFX_PICSTRUCT_FIELD_REPEATED:
    case MFX_PICSTRUCT_FRAME_DOUBLING:
    case MFX_PICSTRUCT_FRAME_TRIPLING:
        return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    // both zero or not zero
    if ((info->AspectRatioW || info->AspectRatioH) && !(info->AspectRatioW && info->AspectRatioH))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if ((info->FrameRateExtN || info->FrameRateExtD) && !(info->FrameRateExtN && info->FrameRateExtD))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

mfxStatus CheckFrameInfoCodecs(mfxFrameInfo  *info, mfxU32 codecId)
{
    mfxStatus sts = CheckFrameInfoCommon(info, codecId);
    if (sts != MFX_ERR_NONE)
        return sts;

    switch (codecId)
    {
    case MFX_CODEC_JPEG:
#if defined(_WIN32) || defined(_WIN64)
        if (info->FourCC != MFX_FOURCC_NV12 && info->FourCC != DXGI_FORMAT_AYUV && info->FourCC != MFX_FOURCC_RGB4 && info->FourCC != MFX_FOURCC_YUY2)
            return MFX_ERR_INVALID_VIDEO_PARAM;
#else
        if (info->FourCC != MFX_FOURCC_NV12 && info->FourCC != MFX_FOURCC_RGB4 && info->FourCC != MFX_FOURCC_YUY2)
            return MFX_ERR_INVALID_VIDEO_PARAM;
#endif
        break;
    case MFX_CODEC_VP8:
        if (info->FourCC != MFX_FOURCC_NV12 && info->FourCC != MFX_FOURCC_YV12)
            return MFX_ERR_INVALID_VIDEO_PARAM;
            break;
    case MFX_CODEC_HEVC:
        if (info->FourCC != MFX_FOURCC_NV12 && info->FourCC != MFX_FOURCC_P010)
            return MFX_ERR_INVALID_VIDEO_PARAM;
        break;
    default:
        if (info->FourCC != MFX_FOURCC_NV12)
            return MFX_ERR_INVALID_VIDEO_PARAM;
        break;
    }

    if (codecId == MFX_CODEC_JPEG)
    {
        if (info->ChromaFormat != MFX_CHROMAFORMAT_YUV420 && info->ChromaFormat != MFX_CHROMAFORMAT_YUV444 && 
            info->ChromaFormat != MFX_CHROMAFORMAT_YUV400 && info->ChromaFormat != MFX_CHROMAFORMAT_YUV422H)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    else 
    {
        if (info->ChromaFormat != MFX_CHROMAFORMAT_YUV420 && info->ChromaFormat != MFX_CHROMAFORMAT_YUV400)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    return MFX_ERR_NONE;
}

mfxStatus CheckAudioParamCommon(mfxAudioParam *in)
{
//    mfxStatus sts;

    switch (in->mfx.CodecId)
    {
        case MFX_CODEC_AAC:
        case MFX_CODEC_MP3:
            break;
        default:
            return MFX_ERR_INVALID_AUDIO_PARAM;
    }

    return MFX_ERR_NONE;
}





mfxStatus CheckAudioParamDecoders(mfxAudioParam *in)
{
    mfxStatus sts = CheckAudioParamCommon(in);
    if (sts < MFX_ERR_NONE)
        return sts;

    return MFX_ERR_NONE;
}

mfxStatus CheckAudioParamEncoders(mfxAudioParam *in)
{
    mfxStatus sts = CheckAudioParamCommon(in);
    if (sts < MFX_ERR_NONE)
        return sts;

    return MFX_ERR_NONE;
}

  
mfxStatus CheckVideoParamCommon(mfxVideoParam *in, eMFXHWType type)
{
    if (!in)
        return MFX_ERR_NULL_PTR;

    mfxStatus sts = CheckFrameInfoCodecs(&in->mfx.FrameInfo, in->mfx.CodecId);
    if (sts != MFX_ERR_NONE)
        return sts;

    // need to change on SNB type. Only SNB supports protected currently

    switch (in->mfx.CodecId)
    {
        case MFX_CODEC_AVC:
        case MFX_CODEC_HEVC:
        case MFX_CODEC_MPEG2:
        case MFX_CODEC_VC1:
        case MFX_CODEC_JPEG:
        case MFX_CODEC_VP8:
            break;
        default:
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }

    if (!in->IOPattern)
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

mfxStatus CheckVideoParamDecoders(mfxVideoParam *in, bool IsExternalFrameAllocator, eMFXHWType type)
{
    mfxStatus sts = CheckVideoParamCommon(in, type);
    if (sts < MFX_ERR_NONE)
        return sts;
    
    if (!(in->IOPattern & MFX_IOPATTERN_OUT_VIDEO_MEMORY) && !(in->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY) && !(in->IOPattern & MFX_IOPATTERN_OUT_OPAQUE_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if ((in->IOPattern & MFX_IOPATTERN_OUT_VIDEO_MEMORY) && (in->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if ((in->IOPattern & MFX_IOPATTERN_OUT_OPAQUE_MEMORY) && (in->IOPattern & MFX_IOPATTERN_OUT_SYSTEM_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if ((in->IOPattern & MFX_IOPATTERN_OUT_OPAQUE_MEMORY) && (in->IOPattern & MFX_IOPATTERN_OUT_VIDEO_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (in->mfx.DecodedOrder && in->mfx.CodecId != MFX_CODEC_JPEG)
        return MFX_ERR_UNSUPPORTED;

    if (!IsExternalFrameAllocator && (in->IOPattern & MFX_IOPATTERN_OUT_VIDEO_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    if (in->mfx.CodecId == MFX_CODEC_AVC || in->mfx.CodecId == MFX_CODEC_JPEG)
    {
        sts = CheckDecodersExtendedBuffers(in);
        if (sts < MFX_ERR_NONE)
            return sts;
    }
    else if (!(in->IOPattern & MFX_IOPATTERN_OUT_OPAQUE_MEMORY))
    {
        if (in->NumExtParam)
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
    return MFX_ERR_NONE;
}

mfxStatus CheckVideoParamEncoders(mfxVideoParam *in, bool IsExternalFrameAllocator, eMFXHWType type)
{
    mfxStatus sts = CheckFrameInfoEncoders(&in->mfx.FrameInfo);
    if (sts < MFX_ERR_NONE)
        return sts;

    sts = CheckVideoParamCommon(in, type);
    if (sts < MFX_ERR_NONE)
        return sts;

    if (!IsExternalFrameAllocator && (in->IOPattern & MFX_IOPATTERN_IN_VIDEO_MEMORY))
        return MFX_ERR_INVALID_VIDEO_PARAM;

    return MFX_ERR_NONE;
}

mfxStatus CheckAudioFrame(const mfxAudioFrame *aFrame)
{
    if (!aFrame || !aFrame->Data)
        return MFX_ERR_NULL_PTR;

    if (aFrame->DataLength > aFrame->MaxLength)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    return MFX_ERR_NONE;
}


mfxStatus CheckBitstream(const mfxBitstream *bs)
{
    if (!bs || !bs->Data)
        return MFX_ERR_NULL_PTR;

    if (bs->DataOffset + bs->DataLength > bs->MaxLength)
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    return MFX_ERR_NONE;
}

mfxStatus CheckFrameData(const mfxFrameSurface1 *surface)
{
    if (!surface)
        return MFX_ERR_NULL_PTR;

    if (!surface->Data.MemId)
    {
        mfxU32 pitch = surface->Data.PitchLow + ((mfxU32)surface->Data.PitchHigh << 16);

        switch (surface->Info.FourCC)
        {
        case MFX_FOURCC_NV12:
            if (!surface->Data.Y || !surface->Data.UV)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            if (pitch > 0xFFFF)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            break;
        case MFX_FOURCC_YV12:
            if (!surface->Data.Y || !surface->Data.U || !surface->Data.V)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            if (pitch > 0xFFFF)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            break;
        case MFX_FOURCC_YUY2:
            if (!surface->Data.Y || !surface->Data.U || !surface->Data.V)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            if (pitch > 0x1FFFF)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            break;
        case MFX_FOURCC_RGB3:
            if (!surface->Data.R || !surface->Data.G || !surface->Data.B)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            if (pitch > 0x2FFFF)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            break;
        case MFX_FOURCC_RGB4:
            if (!surface->Data.A || !surface->Data.R || !surface->Data.G || !surface->Data.B)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            if (pitch > 0x3FFFF)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            break;
#if defined(_WIN32) || defined(_WIN64) 
        case DXGI_FORMAT_AYUV:
            if (!surface->Data.A || !surface->Data.R || !surface->Data.G || !surface->Data.B)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            if (pitch > 0x3FFFF)
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            break;
#endif
        default:
            break;
        }
    }
    return MFX_ERR_NONE;
}

mfxStatus CheckDecodersExtendedBuffers(mfxVideoParam* par)
{
    static const mfxU32 g_decoderSupportedExtBuffersAVC[] = {MFX_EXTBUFF_MVC_SEQ_DESC,
        MFX_EXTBUFF_MVC_TARGET_VIEWS, MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION, MFX_EXTBUFF_SVC_SEQ_DESC, MFX_EXTBUFF_SVC_TARGET_LAYER};

    static const mfxU32 g_decoderSupportedExtBuffersHEVC[] = {MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION};

    static const mfxU32 g_decoderSupportedExtBuffersVC1[] = {MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION};
    static const mfxU32 g_decoderSupportedExtBuffersMJPEG[] = {MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION, MFX_EXTBUFF_JPEG_HUFFMAN, MFX_EXTBUFF_JPEG_QT};

    const mfxU32 *supported_buffers = 0;
    mfxU32 numberOfSupported = 0;

    if (par->mfx.CodecId == MFX_CODEC_AVC)
    {
        supported_buffers = g_decoderSupportedExtBuffersAVC;
        numberOfSupported = sizeof(g_decoderSupportedExtBuffersAVC) / sizeof(g_decoderSupportedExtBuffersAVC[0]);
    }
    else if (par->mfx.CodecId == MFX_CODEC_VC1 || par->mfx.CodecId == MFX_CODEC_MPEG2)
    {
        supported_buffers = g_decoderSupportedExtBuffersVC1;
        numberOfSupported = sizeof(g_decoderSupportedExtBuffersVC1) / sizeof(g_decoderSupportedExtBuffersVC1[0]);
    }else if (par->mfx.CodecId == MFX_CODEC_HEVC)
    {
        supported_buffers = g_decoderSupportedExtBuffersHEVC;
        numberOfSupported = sizeof(g_decoderSupportedExtBuffersHEVC) / sizeof(g_decoderSupportedExtBuffersHEVC[0]);
    }

    if (par->mfx.CodecId == MFX_CODEC_JPEG)
    {
        supported_buffers = g_decoderSupportedExtBuffersMJPEG;
        numberOfSupported = sizeof(g_decoderSupportedExtBuffersMJPEG) / sizeof(g_decoderSupportedExtBuffersMJPEG[0]);
    }

    if (!supported_buffers)
        return MFX_ERR_NONE;

    for (mfxU32 i = 0; i < par->NumExtParam; i++)
    {
        if (par->ExtParam[i] == NULL)
        {
           return MFX_ERR_NULL_PTR;
        }

        bool is_known = false;
        for (mfxU32 j = 0; j < numberOfSupported; ++j)
        {
            if (par->ExtParam[i]->BufferId == supported_buffers[j])
            {
                if (is_known)
                {
                    return MFX_ERR_UNDEFINED_BEHAVIOR;
                }

                is_known = true;
            }
        }

        if (!is_known)
        {
            return MFX_ERR_UNSUPPORTED;
        }
    }

    return MFX_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Extended Buffer class
//////////////////////////////////////////////////////////////////////////////////////////////////////////
ExtendedBuffer::ExtendedBuffer()
{
}

ExtendedBuffer::~ExtendedBuffer()
{
    Release();
}

void ExtendedBuffer::Release()
{
    if(m_buffers.empty())
        return;

    BuffersList::iterator iter = m_buffers.begin();
    BuffersList::iterator iter_end = m_buffers.end();

    for( ; iter != iter_end; ++iter)
    {
        mfxU8 * buffer = (mfxU8 *)*iter;
        delete[] buffer;
    }

    m_buffers.clear();
}

size_t ExtendedBuffer::GetCount() const
{
    return m_buffers.size();
}

void ExtendedBuffer::AddBuffer(mfxExtBuffer * in)
{
    if (GetBufferByIdInternal(in->BufferId))
        return;

    mfxExtBuffer * buffer = (mfxExtBuffer *)(new mfxU8[in->BufferSz]);
    memset(buffer, 0, in->BufferSz);
    buffer->BufferSz = in->BufferSz;
    buffer->BufferId = in->BufferId;
    AddBufferInternal(buffer);
}

void ExtendedBuffer::AddBufferInternal(mfxExtBuffer * buffer)
{
    m_buffers.push_back(buffer);
}

mfxExtBuffer ** ExtendedBuffer::GetBuffers()
{
    return &m_buffers[0];
}

mfxExtBuffer * ExtendedBuffer::GetBufferByIdInternal(mfxU32 id)
{
    BuffersList::iterator iter = m_buffers.begin();
    BuffersList::iterator iter_end = m_buffers.end();

    for( ; iter != iter_end; ++iter)
    {
        mfxExtBuffer * buffer = *iter;
        if (buffer->BufferId == id)
            return buffer;
    }

    return 0;
}

mfxExtBuffer * ExtendedBuffer::GetBufferByPositionInternal(mfxU32 pos)
{
    if (pos >= m_buffers.size())
        return 0;

    return m_buffers[pos];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  mfxVideoParamWrapper implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////
mfxVideoParamWrapper::mfxVideoParamWrapper()
    : m_mvcSequenceBuffer(0)
{
}

mfxVideoParamWrapper::mfxVideoParamWrapper(const mfxVideoParam & par)
    : m_mvcSequenceBuffer(0)
{
    CopyVideoParam(par);
}

mfxVideoParamWrapper::~mfxVideoParamWrapper()
{
    delete[] m_mvcSequenceBuffer;
}

mfxVideoParamWrapper & mfxVideoParamWrapper::operator = (const mfxVideoParam & par)
{
    CopyVideoParam(par);
    return *this;
}

mfxVideoParamWrapper & mfxVideoParamWrapper::operator = (const mfxVideoParamWrapper & par)
{
    return mfxVideoParamWrapper::operator = ( *((const mfxVideoParam *)&par));
}

/*void mfxVideoParamWrapper::ReserveForSPSPPSBuffers(size_t spsSize, size_t ppsSize)
{
    mfxExtCodingOptionSPSPPS * spsPps = (mfxExtCodingOptionSPSPPS *)GetExtendedBuffer(ExtParam, NumExtParam, MFX_EXTBUFF_CODING_OPTION_SPSPPS);

}*/

bool mfxVideoParamWrapper::CreateExtendedBuffer(mfxU32 bufferId)
{
    if (m_buffers.GetBufferById<void *>(bufferId))
        return true;

    switch(bufferId)
    {
    case MFX_EXTBUFF_VIDEO_SIGNAL_INFO:
        m_buffers.AddTypedBuffer<mfxExtVideoSignalInfo>(bufferId);
        break;

    case MFX_EXTBUFF_CODING_OPTION_SPSPPS:
        m_buffers.AddTypedBuffer<mfxExtCodingOptionSPSPPS>(bufferId);
        break;

    default:
        VM_ASSERT(false);
        return false;
    }

    NumExtParam = (mfxU16)m_buffers.GetCount();
    ExtParam = NumExtParam ? m_buffers.GetBuffers() : 0;

    return true;
}

void mfxVideoParamWrapper::CopyVideoParam(const mfxVideoParam & par)
{
    mfxVideoParam * temp = this;
    *temp = par;

    this->NumExtParam = 0;
    this->ExtParam = 0;

    for (mfxU32 i = 0; i < par.NumExtParam; i++)
    {
        switch(par.ExtParam[i]->BufferId)
        {
        case MFX_EXTBUFF_MVC_TARGET_VIEWS:
        case MFX_EXTBUFF_VIDEO_SIGNAL_INFO:
        case MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION:
        case MFX_EXTBUFF_SVC_SEQ_DESC:
        case MFX_EXTBUFF_SVC_TARGET_LAYER:
        case MFX_EXTBUFF_JPEG_QT:
        case MFX_EXTBUFF_JPEG_HUFFMAN:
            {
                void * in = GetExtendedBufferInternal(par.ExtParam, par.NumExtParam, par.ExtParam[i]->BufferId);
                m_buffers.AddBuffer(par.ExtParam[i]);
                mfxExtBuffer * out = m_buffers.GetBufferById<mfxExtBuffer >(par.ExtParam[i]->BufferId);
                if (NULL == out)
                {
                    VM_ASSERT(false);
                    throw UMC::UMC_ERR_FAILED;
                }
                memcpy_s((void*)out, out->BufferSz, in, par.ExtParam[i]->BufferSz);
            }
            break;
        case MFX_EXTBUFF_MVC_SEQ_DESC:
            {
                mfxExtMVCSeqDesc * mvcPoints = (mfxExtMVCSeqDesc *)GetExtendedBufferInternal(par.ExtParam, par.NumExtParam, MFX_EXTBUFF_MVC_SEQ_DESC);

                m_buffers.AddTypedBuffer<mfxExtMVCSeqDesc>(MFX_EXTBUFF_MVC_SEQ_DESC);
                mfxExtMVCSeqDesc * points = m_buffers.GetBufferById<mfxExtMVCSeqDesc>(MFX_EXTBUFF_MVC_SEQ_DESC);

                size_t size = mvcPoints->NumView * sizeof(mfxMVCViewDependency) + mvcPoints->NumOP * sizeof(mfxMVCOperationPoint) + mvcPoints->NumViewId * sizeof(mfxU16);
                if (!m_mvcSequenceBuffer)
                    m_mvcSequenceBuffer = new mfxU8[size];
                else
                {
                    delete[] m_mvcSequenceBuffer;
                    m_mvcSequenceBuffer = new mfxU8[size];
                }

                mfxU8 * ptr = m_mvcSequenceBuffer;

                if (points)
                {
                    points->NumView = points->NumViewAlloc = mvcPoints->NumView;
                    points->View = (mfxMVCViewDependency * )ptr;
                    memcpy_s(points->View, mvcPoints->NumView * sizeof(mfxMVCViewDependency), mvcPoints->View, mvcPoints->NumView * sizeof(mfxMVCViewDependency));
                    ptr += mvcPoints->NumView * sizeof(mfxMVCViewDependency);

                    points->NumView = points->NumViewAlloc = mvcPoints->NumView;
                    points->ViewId = (mfxU16 *)ptr;
                    memcpy_s(points->ViewId, mvcPoints->NumViewId * sizeof(mfxU16), mvcPoints->ViewId, mvcPoints->NumViewId * sizeof(mfxU16));
                    ptr += mvcPoints->NumViewId * sizeof(mfxU16);

                    points->NumOP = points->NumOPAlloc = mvcPoints->NumOP;
                    points->OP = (mfxMVCOperationPoint *)ptr;
                    memcpy_s(points->OP, mvcPoints->NumOP * sizeof(mfxMVCOperationPoint), mvcPoints->OP, mvcPoints->NumOP * sizeof(mfxMVCOperationPoint));

                    mfxU16 * targetView = points->ViewId;
                    for (mfxU32 i = 0; i < points->NumOP; i++)
                    {
                        points->OP[i].TargetViewId = targetView;
                        targetView += points->OP[i].NumTargetViews;
                    }

                    //points->CompatibilityMode = mvcPoints->CompatibilityMode;
                }
            }
            break;

        case MFX_EXTBUFF_CODING_OPTION_SPSPPS:
            {
            /*mfxExtCodingOptionSPSPPS * spsPpsInternal = (mfxExtCodingOptionSPSPPS *)GetExtendedBuffer(m_vPar.ExtParam, m_vPar.NumExtParam, MFX_EXTBUFF_CODING_OPTION_SPSPPS);

            VM_ASSERT(spsPpsInternal);

            spsPps->SPSId = spsPpsInternal->SPSId;
            spsPps->PPSId = spsPpsInternal->PPSId;

            if (spsPps->SPSBufSize < spsPpsInternal->SPSBufSize ||
                spsPps->PPSBufSize < spsPpsInternal->PPSBufSize)
                return MFX_ERR_NOT_ENOUGH_BUFFER;

            spsPps->SPSBufSize = spsPpsInternal->SPSBufSize;
            spsPps->PPSBufSize = spsPpsInternal->PPSBufSize;

            MFX_INTERNAL_CPY(spsPps->SPSBuffer, spsPpsInternal->SPSBuffer, spsPps->SPSBufSize);
            MFX_INTERNAL_CPY(spsPps->PPSBuffer, spsPpsInternal->PPSBuffer, spsPps->PPSBufSize);*/
            }
            break;

        default:
            {
                VM_ASSERT(false);
                throw UMC::UMC_ERR_FAILED;
            }
            break;
        };
    }

    NumExtParam = (mfxU16)m_buffers.GetCount();
    ExtParam = NumExtParam ? m_buffers.GetBuffers() : 0;
}
