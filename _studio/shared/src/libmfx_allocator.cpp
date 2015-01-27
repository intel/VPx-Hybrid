/* ****************************************************************************** *\

Copyright (C) 2007-2014 Intel Corporation.  All rights reserved.

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

File Name: libmfx_allocator.cpp

\* ****************************************************************************** */
#include "libmfx_allocator.h"

#include "mfxvideo++int.h"

#include "ippcore.h"
#include "ipps.h"

#include "mfx_utils.h"

#define ALIGN32(X) (((mfxU32)((X)+31)) & (~ (mfxU32)31))
#define ID_BUFFER MFX_MAKEFOURCC('B','U','F','F')
#define ID_FRAME  MFX_MAKEFOURCC('F','R','M','E')

#define ERROR_STATUS(sts) ((sts)<MFX_ERR_NONE)

// Implementation of Internal allocators
mfxStatus mfxDefaultAllocator::AllocBuffer(mfxHDL pthis, mfxU32 nbytes, mfxU16 type, mfxHDL *mid)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    mfxU32 header_size = ALIGN32(sizeof(BufferStruct));
    mfxU8 *buffer_ptr=(mfxU8 *)ippMalloc(header_size + nbytes);

    if (!buffer_ptr)
        return MFX_ERR_MEMORY_ALLOC;

    memset(buffer_ptr, 0, header_size + nbytes);

    BufferStruct *bs=(BufferStruct *)buffer_ptr;
    bs->allocator = pthis;
    bs->id = ID_BUFFER;
    bs->type = type;
    bs->nbytes = nbytes;

    // save index
    {
        mfxWideBufferAllocator* pBA = (mfxWideBufferAllocator*)pthis;
        pBA->m_bufHdl.push_back(bs);
        *mid = (mfxHDL) pBA->m_bufHdl.size();
    }

    return MFX_ERR_NONE;

}

inline
size_t midToSizeT(mfxHDL mid)
{
    return ((Ipp8u *) mid - (Ipp8u *) 0);

} // size_t midToSizeT(mfxHDL mid)

mfxStatus mfxDefaultAllocator::LockBuffer(mfxHDL pthis, mfxHDL mid, mfxU8 **ptr)
{
    //BufferStruct *bs=(BufferStruct *)mid;
    BufferStruct *bs;
    try
    {
        size_t index = midToSizeT(mid);
        if (!pthis)
            return MFX_ERR_INVALID_HANDLE;

        mfxWideBufferAllocator* pBA = (mfxWideBufferAllocator*)pthis;
        if ((index > pBA->m_bufHdl.size())||
            (index == 0))
            return MFX_ERR_INVALID_HANDLE;
        bs = pBA->m_bufHdl[index - 1];
    }
    catch (...)
    {
        return MFX_ERR_INVALID_HANDLE;
    }

    if (ptr) *ptr=(mfxU8 *)bs+ALIGN32(sizeof(BufferStruct));
    return MFX_ERR_NONE;
}
mfxStatus mfxDefaultAllocator::UnlockBuffer(mfxHDL pthis, mfxHDL mid)
{
    try
    {
        if (!pthis)
            return MFX_ERR_INVALID_HANDLE;

        BufferStruct *bs;
        size_t index = midToSizeT(mid);
        mfxWideBufferAllocator* pBA = (mfxWideBufferAllocator*)pthis;
        if (index > pBA->m_bufHdl.size())
            return MFX_ERR_INVALID_HANDLE;

        bs = pBA->m_bufHdl[index - 1];
        if (bs->id!=ID_BUFFER)
            return MFX_ERR_INVALID_HANDLE;
    }
    catch (...)
    {
        return MFX_ERR_INVALID_HANDLE;
    }

    return MFX_ERR_NONE;
}
mfxStatus mfxDefaultAllocator::FreeBuffer(mfxHDL pthis, mfxMemId mid)
{
    try
    {
        if (!pthis)
            return MFX_ERR_INVALID_HANDLE;
        BufferStruct *bs;
        size_t index = midToSizeT(mid);
        mfxWideBufferAllocator* pBA = (mfxWideBufferAllocator*)pthis;
        if (index > pBA->m_bufHdl.size())
            return MFX_ERR_INVALID_HANDLE;

        bs = pBA->m_bufHdl[index - 1];
        if (bs->id!=ID_BUFFER)
            return MFX_ERR_INVALID_HANDLE;
        if (bs->id!=ID_BUFFER)
            return MFX_ERR_INVALID_HANDLE;
        ippFree(bs);
        return MFX_ERR_NONE;
    }
    catch (...)
    {
        return MFX_ERR_INVALID_HANDLE;
    }
}
mfxStatus mfxDefaultAllocator::AllocFrames(mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    mfxU32 numAllocated, maxNumFrames;
    mfxWideSWFrameAllocator *pSelf = (mfxWideSWFrameAllocator*)pthis;

    // frames were allocated
    // return existent frames
    if (pSelf->NumFrames)
    {
        if (request->NumFrameSuggested > pSelf->NumFrames)
            return MFX_ERR_MEMORY_ALLOC;
        else
        {
            response->mids = &pSelf->m_frameHandles[0];
            return MFX_ERR_NONE;
        }
    }

    mfxU32 Pitch=ALIGN32(request->Info.Width);
    mfxU32 Height2=ALIGN32(request->Info.Height);
    mfxU32 nbytes;
    // Decoders and Encoders use YV12 and NV12 only
    switch (request->Info.FourCC) {
    case MFX_FOURCC_YV12:
        nbytes=Pitch*Height2 + (Pitch>>1)*(Height2>>1) + (Pitch>>1)*(Height2>>1);
        break;
    case MFX_FOURCC_NV12:
        nbytes=Pitch*Height2 + (Pitch>>1)*(Height2>>1) + (Pitch>>1)*(Height2>>1);
        break;
    case MFX_FOURCC_P010:
        Pitch=ALIGN32(request->Info.Width*2);
        nbytes=Pitch*Height2 + (Pitch>>1)*(Height2>>1) + (Pitch>>1)*(Height2>>1);
        break;
    case MFX_FOURCC_YUY2:
        nbytes=Pitch*Height2 + (Pitch>>1)*(Height2) + (Pitch>>1)*(Height2);
        break;
    case MFX_FOURCC_RGB3:
        if ((request->Type & MFX_MEMTYPE_FROM_VPPIN) ||
            (request->Type & MFX_MEMTYPE_FROM_VPPOUT) )
        {
            nbytes = Pitch*Height2 + Pitch*Height2 + Pitch*Height2;
            break;
        }
        else
            return MFX_ERR_UNSUPPORTED;
    case MFX_FOURCC_RGB4:
        nbytes = Pitch*Height2 + Pitch*Height2 + Pitch*Height2 + Pitch*Height2;
        break;
    case MFX_FOURCC_A2RGB10:
        nbytes = Pitch*Height2 + Pitch*Height2 + Pitch*Height2 + Pitch*Height2;
        break;
    case MFX_FOURCC_IMC3:
        if ((request->Type & MFX_MEMTYPE_FROM_VPPIN) ||
            (request->Type & MFX_MEMTYPE_FROM_VPPOUT) ||
            (request->Type & MFX_MEMTYPE_FROM_DECODE))
        {
            nbytes = Pitch*Height2 + (Pitch)*(Height2>>1) + (Pitch)*(Height2>>1);
            break;
        }
        else
            return MFX_ERR_UNSUPPORTED;
        break;

    case MFX_FOURCC_P8: // MBdata for ENC
        if ( request->Type & MFX_MEMTYPE_FROM_ENCODE )
        {
            nbytes = Pitch*Height2;
            break;
        }
        else
            return MFX_ERR_UNSUPPORTED;

    default:
        return MFX_ERR_UNSUPPORTED;
    }

    // allocate frames in cycle
    maxNumFrames = request->NumFrameSuggested;
    pSelf->m_frameHandles.resize(request->NumFrameSuggested);
    for (numAllocated = 0; numAllocated < maxNumFrames; numAllocated += 1)
    {
        mfxStatus sts = (pSelf->wbufferAllocator.bufferAllocator.Alloc)(pSelf->wbufferAllocator.bufferAllocator.pthis, nbytes + ALIGN32(sizeof(FrameStruct)), request->Type, &pSelf->m_frameHandles[numAllocated]);
        if (ERROR_STATUS(sts)) break;

        FrameStruct *fs;
        sts = (pSelf->wbufferAllocator.bufferAllocator.Lock)(pSelf->wbufferAllocator.bufferAllocator.pthis, pSelf->m_frameHandles[numAllocated], (mfxU8 **)&fs);
        if (ERROR_STATUS(sts)) break;
        fs->id = ID_FRAME;
        fs->info = request->Info;
        (pSelf->wbufferAllocator.bufferAllocator.Unlock)(pSelf->wbufferAllocator.bufferAllocator.pthis, pSelf->m_frameHandles[numAllocated]);
    }
    response->mids = &pSelf->m_frameHandles[0];
    response->NumFrameActual = (mfxU16) numAllocated;

    // check the number of allocated frames
    if (numAllocated < request->NumFrameMin)
    {
        FreeFrames(pSelf->wbufferAllocator.bufferAllocator.pthis, response);
        return MFX_ERR_MEMORY_ALLOC;
    }
    pSelf->NumFrames = maxNumFrames;

    return MFX_ERR_NONE;

}


mfxStatus mfxDefaultAllocator::LockFrame(mfxHDL pthis, mfxHDL mid, mfxFrameData *ptr)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    mfxWideSWFrameAllocator *pSelf = (mfxWideSWFrameAllocator*)pthis;

    // The default LockFrame is to simulate using LockBuffer.
    FrameStruct *fs;
    mfxStatus sts = (pSelf->wbufferAllocator.bufferAllocator.Lock)(pSelf->wbufferAllocator.bufferAllocator.pthis, mid,(mfxU8 **)&fs);
    if (ERROR_STATUS(sts)) return sts;
    if (fs->id!=ID_FRAME)
    {
        (pSelf->wbufferAllocator.bufferAllocator.Unlock)(pSelf->wbufferAllocator.bufferAllocator.pthis, mid);
        return MFX_ERR_INVALID_HANDLE;
    }

    //ptr->MemId = mid; !!!!!!!!!!!!!!!!!!!!!!!!!
    mfxU32 Height2=ALIGN32(fs->info.Height);
    mfxU8 *sptr = (mfxU8 *)fs+ALIGN32(sizeof(FrameStruct));
    switch (fs->info.FourCC) {
    case MFX_FOURCC_NV12:
        ptr->PitchHigh=0;
        ptr->PitchLow=(mfxU16)ALIGN32(fs->info.Width);
        ptr->Y = sptr;
        ptr->U = ptr->Y + ptr->Pitch*Height2;
        ptr->V = ptr->U + 1;
        break;
    case MFX_FOURCC_P010:
        ptr->PitchHigh=0;
        ptr->PitchLow=(mfxU16)ALIGN32(fs->info.Width*2);
        ptr->Y = sptr;
        ptr->U = ptr->Y + ptr->Pitch*Height2;
        ptr->V = ptr->U + 1;
        break;
    case MFX_FOURCC_YV12:
        ptr->PitchHigh=0;
        ptr->PitchLow=(mfxU16)ALIGN32(fs->info.Width);
        ptr->Y = sptr;
        ptr->V = ptr->Y + ptr->Pitch*Height2;
        ptr->U = ptr->V + (ptr->Pitch>>1)*(Height2>>1);
        break;
    case MFX_FOURCC_YUY2:
        ptr->Y = sptr;
        ptr->U = ptr->Y + 1;
        ptr->V = ptr->Y + 3;
        ptr->PitchHigh = (mfxU16)((2*ALIGN32(fs->info.Width)) / (1 << 16));
        ptr->PitchLow  = (mfxU16)((2*ALIGN32(fs->info.Width)) % (1 << 16));
        break;
    case MFX_FOURCC_RGB3:
        ptr->B = sptr;
        ptr->G = ptr->B + 1;
        ptr->R = ptr->B + 2;
        ptr->PitchHigh = (mfxU16)((3*ALIGN32(fs->info.Width)) / (1 << 16));
        ptr->PitchLow  = (mfxU16)((3*ALIGN32(fs->info.Width)) % (1 << 16));
        break;
    case MFX_FOURCC_RGB4:
    case MFX_FOURCC_A2RGB10:
        ptr->B = sptr;
        ptr->G = ptr->B + 1;
        ptr->R = ptr->B + 2;
        ptr->A = ptr->B + 3;
        ptr->PitchHigh = (mfxU16)((4*ALIGN32(fs->info.Width)) / (1 << 16));
        ptr->PitchLow  = (mfxU16)((4*ALIGN32(fs->info.Width)) % (1 << 16));
        break;
    /*case MFX_FOURCC_IMC3:
        ptr->Pitch = (mfxU16)ALIGN32(fs->info.Width);
        ptr->Y = sptr;
        ptr->U = ptr->Y +  ptr->Pitch*Height2;
        ptr->V = ptr->U + (ptr->Pitch)*(Height2>>1);
        break;*/
    case MFX_FOURCC_P8:
        ptr->PitchHigh=0;
        ptr->PitchLow=(mfxU16)ALIGN32(fs->info.Width);
        ptr->Y = sptr;
        ptr->U = 0;
        ptr->V = 0;
        break;
    default:
        return MFX_ERR_UNSUPPORTED;
    }
    return sts;

}
mfxStatus mfxDefaultAllocator::GetHDL(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    *handle = mid;
    return MFX_ERR_NONE;
}

mfxStatus mfxDefaultAllocator::UnlockFrame(mfxHDL pthis, mfxHDL mid, mfxFrameData *ptr)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    mfxWideSWFrameAllocator *pSelf = (mfxWideSWFrameAllocator*)pthis;

    // The default UnlockFrame behavior is to simulate using UnlockBuffer
    mfxStatus sts = (pSelf->wbufferAllocator.bufferAllocator.Unlock)(pSelf->wbufferAllocator.bufferAllocator.pthis, mid);
    if (ERROR_STATUS(sts)) return sts;
    if (ptr) {
        ptr->PitchHigh=0;
        ptr->PitchLow=0;
        ptr->U=ptr->V=ptr->Y=0;
        ptr->A=ptr->R=ptr->G=ptr->B=0;
    }
    return sts;

} // mfxStatus SWVideoCORE::UnlockFrame(mfxHDL mid, mfxFrameData *ptr)
mfxStatus mfxDefaultAllocator::FreeFrames(mfxHDL pthis, mfxFrameAllocResponse *response)
{
    if (!pthis)
        return MFX_ERR_INVALID_HANDLE;

    mfxWideSWFrameAllocator *pSelf = (mfxWideSWFrameAllocator*)pthis;
    mfxU32 i;
    // free all allocated frames in cycle
    for (i = 0; i < response->NumFrameActual; i += 1)
    {
        if (response->mids[i])
        {
            (pSelf->wbufferAllocator.bufferAllocator.Free)(pSelf->wbufferAllocator.bufferAllocator.pthis, response->mids[i]);
        }
    }

    pSelf->m_frameHandles.clear();

    return MFX_ERR_NONE;

} // mfxStatus SWVideoCORE::FreeFrames(void)

mfxWideBufferAllocator::mfxWideBufferAllocator()
{
    memset(bufferAllocator.reserved, 0, sizeof(bufferAllocator.reserved));

    bufferAllocator.Alloc = &mfxDefaultAllocator::AllocBuffer;
    bufferAllocator.Lock =  &mfxDefaultAllocator::LockBuffer;
    bufferAllocator.Unlock = &mfxDefaultAllocator::UnlockBuffer;
    bufferAllocator.Free = &mfxDefaultAllocator::FreeBuffer;

    bufferAllocator.pthis = 0;
}

mfxWideBufferAllocator::~mfxWideBufferAllocator()
{
    memset((void*)&bufferAllocator, 0, sizeof(mfxBufferAllocator));
}
mfxBaseWideFrameAllocator::mfxBaseWideFrameAllocator(mfxU16 type)
    : NumFrames(0)
    , type(type)
{
    memset((void*)&frameAllocator, 0, sizeof(frameAllocator));

}
mfxBaseWideFrameAllocator::~mfxBaseWideFrameAllocator()
{
    memset((void*)&frameAllocator, 0, sizeof(mfxFrameAllocator));
}
mfxWideSWFrameAllocator::mfxWideSWFrameAllocator(mfxU16 type):mfxBaseWideFrameAllocator(type)
{
    frameAllocator.Alloc = &mfxDefaultAllocator::AllocFrames;
    frameAllocator.Lock = &mfxDefaultAllocator::LockFrame;
    frameAllocator.GetHDL = &mfxDefaultAllocator::GetHDL;
    frameAllocator.Unlock = &mfxDefaultAllocator::UnlockFrame;
    frameAllocator.Free = &mfxDefaultAllocator::FreeFrames;
}




