/* ****************************************************************************** *\

Copyright (C) 2003-2010 Intel Corporation.  All rights reserved.

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

File Name: umc_base_codec.h

\* ****************************************************************************** */

#ifndef __UMC_BASE_CODEC_H__
#define __UMC_BASE_CODEC_H__

#include "umc_media_data.h"
#include "umc_memory_allocator.h"

namespace UMC
{

class BaseCodecParams
{
    DYNAMIC_CAST_DECL_BASE(BaseCodecParams)

public:
    // Default constructor
    BaseCodecParams(void);

    // Destructor
    virtual ~BaseCodecParams(void){}

    MediaData *m_pData;
    MemoryAllocator *lpMemoryAllocator; // (MemoryAllocator *) pointer to memory allocator object

    Ipp32u m_SuggestedInputSize;   //max suggested frame size of input stream
    Ipp32u m_SuggestedOutputSize;  //max suggested frame size of output stream

    Ipp32s             numThreads; // maximum number of threads to use

    Ipp32s  profile; // profile
    Ipp32s  level;  // level
};

class BaseCodec
{
    DYNAMIC_CAST_DECL_BASE(BaseCodec)

public:
    // Constructor
    BaseCodec(void);

    // Destructor
    virtual ~BaseCodec(void);

    // Initialize codec with specified parameter(s)
    // Has to be called if MemoryAllocator interface is used
    virtual Status Init(BaseCodecParams *init);

    // Compress (decompress) next frame
    virtual Status GetFrame(MediaData *in, MediaData *out) = 0;

    // Get codec working (initialization) parameter(s)
    virtual Status GetInfo(BaseCodecParams *info) = 0;

    // Close all codec resources
    virtual Status Close(void);

    // Set codec to initial state
    virtual Status Reset(void) = 0;

    // Set new working parameter(s)
    virtual Status SetParams(BaseCodecParams *params)
    {
        if (NULL == params)
            return UMC_ERR_NULL_PTR;

        return UMC_ERR_NOT_IMPLEMENTED;
    }

protected:
    MemoryAllocator *m_pMemoryAllocator; // (MemoryAllocator*) pointer to memory allocator
    bool             m_bOwnAllocator;    // True when default allocator is used
};

} // end namespace UMC

#endif /* __UMC_BASE_CODEC_H__ */
