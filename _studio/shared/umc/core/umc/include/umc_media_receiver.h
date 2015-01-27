/* ****************************************************************************** *\

Copyright (C) 2003-2008 Intel Corporation.  All rights reserved.

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

File Name: umc_media_receiver.h

\* ****************************************************************************** */

#ifndef __UMC_MEDIA_RECEIVER_H__
#define __UMC_MEDIA_RECEIVER_H__

#include "umc_structures.h"
#include "umc_dynamic_cast.h"
#include "umc_media_data.h"

namespace UMC
{
// base class for parameters of renderers and buffers
class  MediaReceiverParams
{
    DYNAMIC_CAST_DECL_BASE(MediaReceiverParams)

public:
    // Default constructor
    MediaReceiverParams(void){}
    // Destructor
    virtual ~MediaReceiverParams(void){}
};

// Base class for renderers and buffers
class MediaReceiver
{
    DYNAMIC_CAST_DECL_BASE(MediaReceiver)

public:
    // Default constructor
    MediaReceiver(void){}
    // Destructor
    virtual ~MediaReceiver(void){}

    // Initialize media receiver
    virtual Status Init(MediaReceiverParams *init) = 0;

    // Release all receiver resources
    virtual Status Close(void) {return UMC_OK;}

    // Lock input buffer
    virtual Status LockInputBuffer(MediaData *in) = 0;
    // Unlock input buffer
    virtual Status UnLockInputBuffer(MediaData *in, Status StreamStatus = UMC_OK) = 0;

    // Break waiting(s)
    virtual Status Stop(void) = 0;

    // Reset media receiver
    virtual Status Reset(void) {return UMC_OK;}
};

} // end namespace UMC

#endif // __UMC_MEDIA_RECEIVER_H__
