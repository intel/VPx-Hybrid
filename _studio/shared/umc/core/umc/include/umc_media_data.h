/* ****************************************************************************** *\

Copyright (C) 2003-2011 Intel Corporation.  All rights reserved.

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

File Name: umc_media_data.h

\* ****************************************************************************** */

#ifndef __UMC_MEDIA_DATA_H__
#define __UMC_MEDIA_DATA_H__

#include "umc_structures.h"
#include "umc_dynamic_cast.h"

namespace UMC
{

class MediaData
{
    DYNAMIC_CAST_DECL_BASE(MediaData)

public:

    enum MediaDataFlags
    {
        FLAG_VIDEO_DATA_NOT_FULL_FRAME = 1,
        FLAG_VIDEO_DATA_NOT_FULL_UNIT  = 2,
        FLAG_VIDEO_DATA_END_OF_STREAM  = 4
    };

    // Default constructor
    MediaData(size_t length = 0);

    // Copy constructor
    MediaData(const MediaData &another);

    // Destructor
    virtual ~MediaData();

    // Release object
    virtual Status Close(void);

    // Allocate buffer
    virtual Status Alloc(size_t length);

    // Get an address of the beginning of the buffer.
    // This pointer could not be equal to the beginning of valid data.
    virtual void* GetBufferPointer(void) { return m_pBufferPointer; }

    // Get an address of the beginning of valid data.
    // This pointer could not be equal to the beginning of the buffer.
    virtual void* GetDataPointer(void)   { return m_pDataPointer; }

    // Return size of the buffer
    virtual size_t GetBufferSize(void) const   { return m_nBufferSize; }

    // Return size of valid data in the buffer
    virtual size_t GetDataSize(void) const     { return m_nDataSize; }

    // Set the pointer to a buffer allocated by an user.
    // The bytes variable defines the size of the buffer.
    // Size of valid data is set to zero
    virtual Status SetBufferPointer(Ipp8u* ptr, size_t bytes);

    // Set size of valid data in the buffer.
    // Valid data is supposed to be placed from the beginning of the buffer.
    virtual Status SetDataSize(size_t bytes);

    //  Move data pointer inside and decrease or increase data size
    virtual Status MoveDataPointer(Ipp32s bytes);

    // return time stamp of media data
    virtual Ipp64f GetTime(void) const         { return m_pts_start; }

    // return time stamp of media data, start and end
    virtual Status GetTime(Ipp64f& start, Ipp64f& end) const;

    //  Set time stamp of media data block;
    virtual Status SetTime(Ipp64f start, Ipp64f end = 0);

    // Set frame type
    inline Status SetFrameType(FrameType ft);
    // Get frame type
    inline FrameType GetFrameType(void) const;

    // Set Invalid state
    inline void SetInvalid(Ipp32s isInvalid) { m_isInvalid = isInvalid; }
    // Get Invalid state
    inline Ipp32s GetInvalid(void) const           { return m_isInvalid; }

    //  Set data pointer to beginning of buffer and data size to zero
    virtual Status Reset();

    MediaData& operator=(const MediaData&);

    //  Move data to another MediaData
    virtual Status MoveDataTo(MediaData* dst);

    inline void SetFlags(Ipp32u flags);
    inline Ipp32u GetFlags() const;

protected:

    Ipp64f m_pts_start;        // (Ipp64f) start media PTS
    Ipp64f m_pts_end;          // (Ipp64f) finish media PTS
    size_t m_nBufferSize;      // (size_t) size of buffer
    size_t m_nDataSize;        // (size_t) quantity of data in buffer

    Ipp8u* m_pBufferPointer;
    Ipp8u* m_pDataPointer;

    FrameType m_frameType;     // type of the frame
    Ipp32s    m_isInvalid;     // data is invalid when set

    Ipp32u      m_flags;

    // Actually this variable should has type bool.
    // But some compilers generate poor executable code.
    // On count of this, we use type Ipp32u.
    Ipp32u m_bMemoryAllocated; // (Ipp32u) is memory owned by object

};


inline Status MediaData::SetFrameType(FrameType ft)
{
    // check error(s)
    if ((ft < NONE_PICTURE) || (ft > D_PICTURE))
        return UMC_ERR_INVALID_STREAM;

    m_frameType = ft;

    return UMC_OK;
} // VideoData::SetFrameType()


inline FrameType MediaData::GetFrameType(void) const
{
    return m_frameType;
} // VideoData::GetFrameType()


inline void MediaData::SetFlags(Ipp32u flags)
{
    m_flags = flags;
}

inline Ipp32u MediaData::GetFlags() const
{
    return m_flags;
}

} // namespace UMC

#endif /* __UMC_MEDIA_DATA_H__ */
