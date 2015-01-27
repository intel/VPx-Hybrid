/* ****************************************************************************** *\

Copyright (C) 2003-2013 Intel Corporation.  All rights reserved.

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

File Name: umc_frame_data.cpp

\* ****************************************************************************** */

#include "umc_defs.h"
#include "umc_frame_data.h"
#include "ipps.h"

namespace UMC
{

// Number of planes in format description table.
// Number of planes in image can be greater. In this case additional
// planes should be supported by the user with functions SetPlanePointer,
// SetPlanePitch.
enum
{
    MAX_PLANE_NUMBER            = 4
};

// Color format description structure
struct ColorFormatInfo
{
    ColorFormat m_cFormat;
    Ipp32u m_iPlanes;        // Number of planes
    Ipp32s m_iMinBitDepth;   // Minimum bitdepth

    struct {
        Ipp32s m_iWidthScale;  // Horizontal downsampling factor
        Ipp32s m_iHeightScale; // Vertical downsampling factor
        Ipp32s m_iChannels;  // Number of merged channels in the plane
    } m_PlaneFormatInfo[MAX_PLANE_NUMBER];
};

// Color format description table
static
const
ColorFormatInfo FormatInfo[] =
{
    {YV12,    3,  8, {{0, 0, 1}, {1, 1, 1}, {1, 1, 1}}},
    {NV12,    2,  8, {{0, 0, 1}, {1, 1, 2}, }},
    {IMC3,    3,  8, {{0, 0, 1}, {1, 1, 1}, {1, 1, 1}}},
    {YUY2,    1,  8, {{2, 1, 4}, }},
    {UYVY,    1,  8, {{2, 1, 4}, }},
    {YUV411,  3,  8, {{1, 1, 1}, {4, 1, 1}, {4, 1, 1}}},
    {YUV420,  3,  8, {{0, 0, 1}, {1, 1, 1}, {1, 1, 1}}},
    {YUV422,  3,  8, {{0, 0, 1}, {1, 0, 1}, {1, 0, 1}}},
    {YUV444,  3,  8, {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}}},
    {YUV_VC1, 3,  8, {{1, 1, 1}, {2, 2, 1}, {2, 2, 1}}},
    {Y411,    1,  8, {{4, 1, 6}}},
    {Y41P,    1,  8, {{8, 1, 12}}},
    {RGB32,   1,  8, {{1, 1, 4}}},
    {RGB24,   1,  8, {{1, 1, 3}}},
    {RGB565,  1, 16, {{1, 1, 1}}},
    {RGB555,  1, 16, {{1, 1, 1}}},
    {RGB444,  1, 16, {{1, 1, 1}}},
    {GRAY,    1,  8, {{0, 0, 1}}},
    {GRAYA,   2,  8, {{0, 0, 1}, {0, 0, 1}}},
    {YUV420A, 4,  8, {{0, 0, 1}, {1, 1, 1}, {1, 1, 1}, {0, 0, 1}}},
    {YUV422A, 4,  8, {{0, 0, 1}, {1, 0, 1}, {1, 0, 1}, {0, 0, 1}}},
    {YUV444A, 4,  8, {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}}},
    {YVU9,    3,  8, {{1, 1, 1}, {4, 4, 1}, {4, 4, 1}}},
    {D3D_SURFACE_DEC,   3, 8, {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}}},
    {D3D_SURFACE,       3, 8, {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}}}
};

#define D3D_SURFACE_SIZE    256

// Number of entries in the FormatInfo table
static
const
Ipp32s iNumberOfFormats = sizeof(FormatInfo) / sizeof(FormatInfo[0]);

// returns pointer to color format description table for cFormat
// or NULL if cFormat is not described (unknown color format).
static
const
ColorFormatInfo *GetColorFormatInfo(ColorFormat cFormat)
{
    const ColorFormatInfo *pReturn = NULL;
    Ipp32s i;

    // find required format
    for (i = 0; i < iNumberOfFormats; i += 1)
    {
        if (FormatInfo[i].m_cFormat == cFormat)
        {
            pReturn = FormatInfo + i;
            break;
        }
    }

    return pReturn;
} // ColorFormatInfo *GetColorFormatInfo(ColorFormat cFormat)

// Initialization to undefined image type
VideoDataInfo::VideoDataInfo(void)
{
    Close();
    memset(m_pPlaneData, 0, sizeof(m_pPlaneData));
} // VideoDataInfo::VideoDataInfo(void)

VideoDataInfo::~VideoDataInfo()
{

} // VideoDataInfo::~VideoDataInfo(void)

void VideoDataInfo::Close()
{
    m_iPlanes = 0;
    m_ippSize.width = 0;
    m_ippSize.height = 0;
    m_ColorFormat = NONE;
    m_picStructure = PS_FRAME;

    // set square pixel
    m_iHorzAspect =
    m_iVertAspect = 1;
}

// Initializes image dimensions and bitdepth.
// Has to be followed by SetColorFormat call.
// Planes' information is initialized to invalid values
Status VideoDataInfo::Init(Ipp32s iWidth,
                       Ipp32s iHeight,
                       Ipp32s iPlanes,
                       Ipp32s iBitDepth)
{
    Ipp32s i;

    if ((0 >= iWidth) ||
        (0 >= iHeight) ||
        (0 >= iPlanes) ||
        (8 > iBitDepth))
        return UMC_ERR_INVALID_PARAMS;

    for (i = 0; i < iPlanes; i++)
    {
        m_pPlaneData[i].m_iSamples = 1;
        m_pPlaneData[i].m_iSampleSize = (iBitDepth+7)>>3;
        m_pPlaneData[i].m_iBitDepth = iBitDepth;
        // can't assign dimension to unknown planes
        m_pPlaneData[i].m_ippSize.width = 0;
        m_pPlaneData[i].m_ippSize.height = 0;
    }

    m_iPlanes = iPlanes;
    m_ippSize.width = iWidth;
    m_ippSize.height = iHeight;

    return UMC_OK;

} // Status VideoDataInfo::Init(Ipp32s iWidth,

// Completely sets image information, without allocation or linking to
// image memory.
Status VideoDataInfo::Init(Ipp32s iWidth,
                       Ipp32s iHeight,
                       ColorFormat cFormat,
                       Ipp32s iBitDepth)
{
    Status umcRes;
    const ColorFormatInfo* pFormat;

    pFormat = GetColorFormatInfo(cFormat);
    if(NULL == pFormat)
        return UMC_ERR_INVALID_PARAMS;

    // allocate planes
    if(iBitDepth == 0)
      iBitDepth = pFormat->m_iMinBitDepth;
    umcRes = Init(iWidth, iHeight, pFormat->m_iPlanes, iBitDepth);
    if (UMC_OK != umcRes)
        return umcRes;

    // set color format and
    // correct width & height for planes
    umcRes = SetColorFormat(cFormat);
    if (UMC_OK != umcRes)
        return umcRes;

    return UMC_OK;

} // Status VideoDataInfo::Init(Ipp32s iWidth,

// Sets or change Color format information for image, only when it has
// specified size, number of planes and bitdepth. Number of planes in cFormat must
// be not greater than specified in image.
Status VideoDataInfo::SetColorFormat(ColorFormat cFormat)
{
    const ColorFormatInfo *pFormat;

    // check error(s)
    pFormat = GetColorFormatInfo(cFormat);
    if (NULL == pFormat)
        return UMC_ERR_INVALID_STREAM;

    if (m_iPlanes < pFormat->m_iPlanes)
        return UMC_ERR_INVALID_STREAM;

    m_ColorFormat = cFormat;

    // set correct width & height to planes
    for (Ipp32u i = 0; i < m_iPlanes; i += 1)
    {
        if (i < pFormat->m_iPlanes)
        {
            m_pPlaneData[i].m_iWidthScale = pFormat->m_PlaneFormatInfo[i].m_iWidthScale;
            m_pPlaneData[i].m_iHeightScale = pFormat->m_PlaneFormatInfo[i].m_iHeightScale;
            m_pPlaneData[i].m_iSamples = pFormat->m_PlaneFormatInfo[i].m_iChannels;
        } else {
            m_pPlaneData[i].m_iWidthScale = 0;
            m_pPlaneData[i].m_iHeightScale = 0;
            m_pPlaneData[i].m_iSamples = 1;
        }

        m_pPlaneData[i].m_ippSize.width = m_ippSize.width >> m_pPlaneData[i].m_iWidthScale;
        m_pPlaneData[i].m_ippSize.height = m_ippSize.height >> m_pPlaneData[i].m_iHeightScale;
    }

    return UMC_OK;
} // Status VideoDataInfo::SetColorFormat(ColorFormat cFormat)


// Set sample size for specified plane, usually additional or when bitdepth differs
// for main planes
Status VideoDataInfo::SetPlaneSampleSize(Ipp32s iSampleSize, Ipp32u iPlaneNumber)
{
    // check error(s)
    if (m_iPlanes <= iPlaneNumber)
        return UMC_ERR_FAILED;

    m_pPlaneData[iPlaneNumber].m_iSampleSize = iSampleSize;
    if(iSampleSize*8 < m_pPlaneData[iPlaneNumber].m_iBitDepth)
        m_pPlaneData[iPlaneNumber].m_iBitDepth = iSampleSize*8;

    return UMC_OK;
}

const VideoDataInfo::PlaneInfo* VideoDataInfo::GetPlaneInfo(Ipp32u plane) const
{
    // check error(s)
    if (m_iPlanes <= plane)
        return 0;

    return &m_pPlaneData[plane];
}

size_t VideoDataInfo::GetSize() const
{
    size_t sz = 0;

    for (Ipp32u i = 0; i < m_iPlanes; i++)
    {
        sz += m_pPlaneData[i].m_ippSize.width * m_pPlaneData[i].m_ippSize.height * m_pPlaneData[i].m_iSampleSize * m_pPlaneData[i].m_iSamples;
    }

    return sz;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Time
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FrameTime::FrameTime()
{
    Reset();
}

void FrameTime::Reset()
{
    m_pts_start = -1;
    m_pts_end = -1;
}

Status FrameTime::GetTime(Ipp64f& start, Ipp64f& end) const
{
    start = m_pts_start;
    end = m_pts_end;
    return UMC_OK;
}

Status FrameTime::SetTime(Ipp64f start, Ipp64f end)
{
    m_pts_start = start;
    m_pts_end = end;
    return UMC_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FrameData::FrameData()
    : m_locked(false)
    , m_FrameMID(FRAME_MID_INVALID)
    , m_FrameAlloc(0)
{
}

FrameData::FrameData(const FrameData & fd)
    : m_locked(false)
    , m_Info(fd.m_Info)
    , m_FrameMID(fd.m_FrameMID)
    , m_FrameAlloc(fd.m_FrameAlloc)
{
    MFX_INTERNAL_CPY(m_PlaneInfo, fd.m_PlaneInfo, sizeof(m_PlaneInfo));
    if (m_FrameAlloc)
    {
        m_FrameAlloc->IncreaseReference(m_FrameMID);
    }
}

FrameData::~FrameData()
{
    Close();
}

FrameData& FrameData::operator=(const FrameData& fd)
{
    // check for assignment for self
    if (this == &fd)
    {
        return *this;
    }

    Close();

    m_FrameAlloc = fd.m_FrameAlloc;
    m_FrameMID = fd.m_FrameMID;
    m_Info = fd.m_Info;
    MFX_INTERNAL_CPY(m_PlaneInfo, fd.m_PlaneInfo, sizeof(m_PlaneInfo));
    m_locked = false;// fd.m_locked;

    if (m_FrameAlloc)
    {
        m_FrameAlloc->IncreaseReference(m_FrameMID);
    }

    /*if (m_locked)
    {
        const FrameData * frm = m_FrameAlloc->Lock(m_FrameMID);
        if (!frm)
            throw VM_STRING("exception1");
    }*/

    return *this;
}

const VideoDataInfo * FrameData::GetInfo() const
{
    return &m_Info;
}

FrameMemID FrameData::GetFrameMID() const
{
    return m_FrameMID;
}

FrameMemID FrameData::Release()
{
    FrameTime::Reset();
    FrameMemID mid = m_FrameMID;
    m_locked = false;
    m_FrameMID = FRAME_MID_INVALID;
    m_FrameAlloc = 0;
    m_Info.Close();
    return mid;
}

void FrameData::Init(const VideoDataInfo * info, FrameMemID memID, FrameAllocator * frameAlloc)
{
    Close();

    m_Info = *info;
    m_FrameMID = memID;
    m_FrameAlloc = frameAlloc;
    if (m_FrameAlloc && m_FrameMID != FRAME_MID_INVALID)
        m_FrameAlloc->IncreaseReference(m_FrameMID);
}

void FrameData::Close()
{
    FrameTime::Reset();

    if (m_FrameAlloc && m_FrameMID != FRAME_MID_INVALID)
    {
        if (m_locked)
        {
            m_FrameAlloc->Unlock(m_FrameMID);
        }

        m_locked = false;

        m_FrameAlloc->DecreaseReference(m_FrameMID);
        m_FrameMID = FRAME_MID_INVALID;
        m_FrameAlloc = 0;
    }

    memset(m_PlaneInfo, 0, sizeof(m_PlaneInfo));
    m_Info.Close();
}

const FrameData::PlaneMemoryInfo * FrameData::GetPlaneMemoryInfo(Ipp32u plane) const
{
    if (plane >= m_Info.GetNumPlanes())
        return 0;

    return &(m_PlaneInfo[plane]);
}

void FrameData::SetPlanePointer(Ipp8u* planePtr, Ipp32u plane, size_t pitch)
{
    if (plane >= m_Info.GetNumPlanes())
        return;

    m_PlaneInfo[plane].m_planePtr = planePtr;
    m_PlaneInfo[plane].m_pitch = pitch;
}

} // end namespace UMC
