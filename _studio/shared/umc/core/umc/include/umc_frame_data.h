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

File Name: umc_frame_data.h

\* ****************************************************************************** */

#ifndef __UMC_FRAME_DATA_H__
#define __UMC_FRAME_DATA_H__

#include "ippdefs.h"
#include "umc_structures.h"
#include "umc_dynamic_cast.h"

#include "umc_frame_allocator.h"

/*
    USAGE MODEL:
    I. Initialization of VideoData parameters. It has to be done after construction
      or after Close.
      A. Simplest case. No additional planes, planes have equal bitdepth.
        1. Set required alignment for data with SetAlignment. Default is the sample size.
        2. Init(w,h,ColorFormat[,bitdepth]). Default bitdepth is derived from format.
      B. Advanced case
        1. Init(w,h,nplanes[,bitdepth]) and SetAlignment if required before or after.
        2. Modify bitdepth or sample size for planes where necessary
        3. Call SetColorFormat. It is the only moment to call this method.
      This stage fill all internal information about planes, including pitches.
      After this no more changes to parameters shall be introduced.
      Function GetMappingSize can be called now to determine required quantity of memory
      for all planes taking into account current alignment. All other Get methods except
      of GetPlanePointer are possible to use.

    II. Link to memory. These operations assign all plane pointers. After that
      MediaData::GetDataPointer will return aligned beginning of the first plane,
      MediaData::GetDataSize will return value equal to MappingSize,
      MediaData::GetBufferPointer can differ from DataPointer due to aligning
      Two ways:
      A. Allocation using Alloc. BufferSize will be MappingSize + alignment.
      B. Call SetBufferPointer(bufsize). After that BufferSize will be bufsize.
      Method ReleaseImage cancels this operations (unlink from memory).
      These methods only work with continuously located planes.

    III. Operations which don't change plane parameters, like SetFrameType, can be used at
      any moment. Operations SetPlanePointer and SetPlanePitch allow working with separate
      planes or without specified ColorFormat but have to be used with care. Functions like
      GetMappingSize and GetPlaneInfo can provide incorrect results.

    Note:
    parent class methods GetDataPointer, MoveDataPointer operator= shouldn't be used.
*/

namespace UMC
{

class VideoDataInfo
{
    DYNAMIC_CAST_DECL_BASE(VideoDataInfo)

public:

    enum PictureStructure
    {
        PS_TOP_FIELD                = 1,
        PS_BOTTOM_FIELD             = 2,
        PS_FRAME                    = PS_TOP_FIELD | PS_BOTTOM_FIELD,
        PS_TOP_FIELD_FIRST          = PS_FRAME | 4,
        PS_BOTTOM_FIELD_FIRST       = PS_FRAME | 8
    }; // DEBUG : to use types at umc_structure.h

    struct PlaneInfo
    {
        IppiSize m_ippSize;        // width and height of the plane
        Ipp32s   m_iSampleSize;    // sample size (in bytes)
        Ipp32s   m_iSamples;       // number of samples per plane element
        Ipp32s   m_iBitDepth;      // number of significant bits per sample (should be <= 8*m_iSampleSize)
        Ipp32s   m_iWidthScale;    // Horizontal downsampling factor
        Ipp32s   m_iHeightScale;   // Vertical downsampling factor
    };

    VideoDataInfo(void);
    virtual ~VideoDataInfo(void);

    // Initialize. Only remembers image characteristics for future.
    virtual Status Init(Ipp32s iWidth,
                Ipp32s iHeight,
                ColorFormat cFormat,
                Ipp32s iBitDepth = 8);

    void Close();

    inline Ipp32s GetPlaneBitDepth(Ipp32u iPlaneNumber) const;

    Status SetPlaneSampleSize(Ipp32s iSampleSize, Ipp32u iPlaneNumber);
    inline Ipp32u GetPlaneSampleSize(Ipp32u iPlaneNumber) const;

    inline ColorFormat GetColorFormat() const;

    inline Status SetAspectRatio(Ipp32s iHorzAspect, Ipp32s iVertAspect);
    inline Status GetAspectRatio(Ipp32s *piHorzAspect, Ipp32s *piVertAspect) const;

    inline Status SetPictureStructure(PictureStructure picStructure);
    inline PictureStructure GetPictureStructure() const;

    inline Ipp32u GetNumPlanes() const;
    inline Ipp32u GetWidth() const;
    inline Ipp32u GetHeight() const;

    size_t GetSize() const;

    const PlaneInfo* GetPlaneInfo(Ipp32u iPlaneNumber) const;

    void SetPadding();
    void GetPadding() const;

protected:

    enum
    {
        NUM_PLANES = 4
    };

    PlaneInfo        m_pPlaneData[NUM_PLANES]; // pointer to allocated planes info
    Ipp32u           m_iPlanes;       // number of initialized planes

    IppiSize         m_ippSize;       // dimension of the image

    ColorFormat      m_ColorFormat;   // color format of image
    PictureStructure m_picStructure;  // variants: progressive frame, top first, bottom first, only top, only bottom

    Ipp32s           m_iHorzAspect;   // aspect ratio: pixel width/height proportion
    Ipp32s           m_iVertAspect;   // default 1,1 - square pixels

    // Set color format and planes' information
    Status SetColorFormat(ColorFormat cFormat);

    Status Init(Ipp32s iWidth,
                       Ipp32s iHeight,
                       Ipp32s iPlanes,
                       Ipp32s iBitDepth);

};

inline Ipp32s VideoDataInfo::GetPlaneBitDepth(Ipp32u iPlaneNumber) const
{
    // check error(s)
    if (NUM_PLANES <= iPlaneNumber)
        return 0;

  return m_pPlaneData[iPlaneNumber].m_iBitDepth;

}

inline Ipp32u VideoDataInfo::GetPlaneSampleSize(Ipp32u iPlaneNumber) const
{
    // check error(s)
    if (NUM_PLANES <= iPlaneNumber)
        return 0;

  return m_pPlaneData[iPlaneNumber].m_iSampleSize;

}

inline ColorFormat VideoDataInfo::GetColorFormat(void) const
{
    return m_ColorFormat;
}

inline Status VideoDataInfo::SetAspectRatio(Ipp32s iHorzAspect, Ipp32s iVertAspect)
{
    if ((1 > iHorzAspect) || (1 > iVertAspect))
        return UMC_ERR_INVALID_STREAM;

    m_iHorzAspect = iHorzAspect;
    m_iVertAspect = iVertAspect;

    return UMC_OK;
}

inline Status VideoDataInfo::GetAspectRatio(Ipp32s *piHorzAspect, Ipp32s *piVertAspect) const
{
    if ((NULL == piHorzAspect) ||
        (NULL == piVertAspect))
        return UMC_ERR_NULL_PTR;

    *piHorzAspect = m_iHorzAspect;
    *piVertAspect = m_iVertAspect;

    return UMC_OK;
}

inline Status VideoDataInfo::SetPictureStructure(PictureStructure picStructure)
{
    if ((PS_TOP_FIELD != picStructure) &&
        (PS_BOTTOM_FIELD != picStructure) &&
        (PS_FRAME != picStructure) &&
        (PS_TOP_FIELD_FIRST != picStructure) &&
        (PS_BOTTOM_FIELD_FIRST != picStructure))
        return UMC_ERR_INVALID_STREAM;

    m_picStructure = picStructure;

    return UMC_OK;
}

inline VideoDataInfo::PictureStructure VideoDataInfo::GetPictureStructure() const
{
    return m_picStructure;
}

inline Ipp32u VideoDataInfo::GetNumPlanes() const
{
    return m_iPlanes;
}

inline Ipp32u VideoDataInfo::GetWidth() const
{
    return m_ippSize.width;
}

inline Ipp32u VideoDataInfo::GetHeight() const
{
    return m_ippSize.height;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Time
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FrameTime
{
public:
    FrameTime();
    virtual ~FrameTime() {}

    virtual void Reset();

    // return time stamp of media data
    virtual Ipp64f GetTime(void) const         { return m_pts_start; }

    // return time stamp of media data, start and end
    virtual Status GetTime(Ipp64f& start, Ipp64f& end) const;

    //  Set time stamp of media data block;
    virtual Status SetTime(Ipp64f start, Ipp64f end = 0);

private:
    Ipp64f m_pts_start;        // (Ipp64f) start media PTS
    Ipp64f m_pts_end;          // (Ipp64f) finish media PTS
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FrameData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FrameData : public FrameTime
{
public:

    struct PlaneMemoryInfo
    {
        Ipp8u* m_planePtr;
        size_t m_pitch;
    };

    FrameData();
    FrameData(const FrameData & fd);

    virtual ~FrameData();

    const VideoDataInfo * GetInfo() const;

    const PlaneMemoryInfo * GetPlaneMemoryInfo(Ipp32u plane) const;

    void Init(const VideoDataInfo * info, FrameMemID memID = FRAME_MID_INVALID, FrameAllocator * frameAlloc = 0);

    void Close();

    FrameMemID GetFrameMID() const;

    FrameMemID Release();

    void SetPlanePointer(Ipp8u* planePtr, Ipp32u plane, size_t pitch);

    FrameData& operator=(const FrameData& );

    bool            m_locked;

protected:

    enum
    {
        NUM_PLANES = 4
    };

    VideoDataInfo   m_Info;
    FrameMemID      m_FrameMID;
    FrameAllocator *m_FrameAlloc;

    PlaneMemoryInfo m_PlaneInfo[NUM_PLANES];
};

} // namespace UMC

#endif // __UMC_FRAME_DATA_H__
