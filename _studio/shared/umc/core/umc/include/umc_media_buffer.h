/* ****************************************************************************** *\

Copyright (C) 2003-2007 Intel Corporation.  All rights reserved.

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

File Name: umc_media_buffer.h

\* ****************************************************************************** */

#ifndef __UMC_MEDIA_BUFFER_H__
#define __UMC_MEDIA_BUFFER_H__

#include "umc_structures.h"
#include "umc_dynamic_cast.h"
#include "umc_media_receiver.h"
#include "umc_memory_allocator.h"

namespace UMC
{

class MediaBufferParams : public MediaReceiverParams
{
    DYNAMIC_CAST_DECL(MediaBufferParams, MediaReceiverParams)

public:
    // Default constructor
    MediaBufferParams(void);

    size_t m_prefInputBufferSize;                               // (size_t) preferable size of input potion(s)
    Ipp32u m_numberOfFrames;                                    // (Ipp32u) minimum number of data potion in buffer
    size_t m_prefOutputBufferSize;                              // (size_t) preferable size of output potion(s)

    MemoryAllocator *m_pMemoryAllocator;                        // (MemoryAllocator *) pointer to memory allocator object
};

class MediaBuffer : public MediaReceiver
{
    DYNAMIC_CAST_DECL(MediaBuffer, MediaReceiver)

public:

    // Constructor
    MediaBuffer(void);
    // Destructor
    virtual
    ~MediaBuffer(void);

    // Lock output buffer
    virtual
    Status LockOutputBuffer(MediaData *out) = 0;
    // Unlock output buffer
    virtual
    Status UnLockOutputBuffer(MediaData *out) = 0;

    // Initialize the buffer
    virtual
    Status Init(MediaReceiverParams *pInit);

    // Release object
    virtual
    Status Close(void);

protected:

    MemoryAllocator *m_pMemoryAllocator;                        // (MemoryAllocator *) pointer to memory allocator object
    MemoryAllocator *m_pAllocated;                              // (MemoryAllocator *) owned pointer to memory allocator object
};

} // end namespace UMC

#endif // __UMC_MEDIA_BUFFER_H__
