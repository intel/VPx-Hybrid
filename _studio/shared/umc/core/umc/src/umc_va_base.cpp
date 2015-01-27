/* ****************************************************************************** *\

Copyright (C) 2006-2013 Intel Corporation.  All rights reserved.

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

File Name: umc_va_base.cpp

\* ****************************************************************************** */

#ifdef _DEBUG
#ifndef VM_DEBUG
#define VM_DEBUG
#endif
#endif

#include "umc_va_base.h"
#include "umc_defs.h"

#ifndef UMC_RESTRICTED_CODE_VA

using namespace UMC;

Status VideoAccelerator::Close(void)
{
    return UMC_OK;
}

Status VideoAccelerator::Reset(void)
{
    return UMC_OK;
}

namespace UMC
{
VideoAccelerationProfile VideoType2VAProfile(VideoStreamType video_type)
{
    switch (video_type)
    {
        case MPEG2_VIDEO: return VA_MPEG2;
        case H261_VIDEO:  return VA_MPEG4;
        case H263_VIDEO:  return VA_MPEG4;
        case MPEG4_VIDEO: return VA_MPEG4;
        case H264_VIDEO:  return VA_H264;
        case VC1_VIDEO:   return VA_VC1;
        case WMV_VIDEO:   return VA_VC1;
        default:          return UNKNOWN;
    }
    return UNKNOWN;
}
}

void UMCVACompBuffer::SetDataSize(Ipp32s size)
{
    DataSize = size;
    if (DataSize > BufferSize)
    //if (DataSize)
    {
        vm_trace_i(type);
        vm_trace_i(DataSize);
        vm_trace_i(BufferSize);
        //printf("SetDataSize!!!!!!!!!! %d %d\n", DataSize, BufferSize);
    }
    VM_ASSERT(DataSize <= BufferSize);
#if 0
    if (DataSize > BufferSize)
    {
        //exit(1);
        throw /*SimError(UMC_ERR_FAILED, */"Buffer is too small in UMCVACompBuffer::SetDataSize";
    }
#endif
}

void UMCVACompBuffer::SetNumOfItem(Ipp32s )
{
}

Status UMCVACompBuffer::SetPVPState(void *buf, Ipp32u size)
{
    if (16 < size)
        return UMC_ERR_ALLOC;
    if (NULL != buf)
    {
        if (0 == size)
            return UMC_ERR_ALLOC;
        PVPState = PVPStateBuf;
        MFX_INTERNAL_CPY(PVPState, buf, size);
    }
    else
        PVPState = NULL;

    return UMC_OK;
}

#endif // UMC_RESTRICTED_CODE_VA
