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

File Name: umc_video_data.cpp

\* ****************************************************************************** */

#include "umc_video_data.h"
#include "vm_debug.h"

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
    Ipp32s m_iPlanes;        // Number of planes
    Ipp32s m_iMinBitDepth;   // Minimum bitdepth
    Ipp32s m_iMinAlign;      // Minimal required alignment in bytes
    struct {
        Ipp32s m_iWidthDiv;  // Horizontal downsampling factor
        Ipp32s m_iHeightDiv; // Vertical downsampling factor
        Ipp32s m_iChannels;  // Number of merged channels in the plane
        Ipp32s m_iAlignMult; // Alignment value multiplier
    } m_PlaneFormatInfo[MAX_PLANE_NUMBER];
};

// Color format description table
static
const
ColorFormatInfo FormatInfo[] =
{
    {YV12,    3,  8, 1, {{1, 1, 1, 2}, {2, 2, 1, 1}, {2, 2, 1, 1}}},
    {NV12,    2,  8, 2, {{1, 1, 1, 1}, {1, 2, 1, 1}, }},
    {YUY2,    1,  8, 2, {{2, 1, 4, 1}, }},
    {UYVY,    1,  8, 2, {{2, 1, 4, 1}, }},
    {YUV411,  3,  8, 1, {{1, 1, 1, 1}, {4, 1, 1, 1}, {4, 1, 1, 1}}},
    {YUV420,  3,  8, 1, {{1, 1, 1, 1}, {2, 2, 1, 1}, {2, 2, 1, 1}}},
    {YUV422,  3,  8, 1, {{1, 1, 1, 1}, {2, 1, 1, 1}, {2, 1, 1, 1}}},
    {YUV444,  3,  8, 1, {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}}},
    {YUV_VC1, 3,  8, 1, {{1, 1, 1, 1}, {2, 2, 1, 1}, {2, 2, 1, 1}}},
    {Y411,    1,  8, 2, {{4, 1, 6, 1}}},
    {Y41P,    1,  8, 2, {{8, 1, 12, 1}}},
    {RGB32,   1,  8, 1, {{1, 1, 4, 1}}},
    {RGB24,   1,  8, 4, {{1, 1, 3, 1}}},
    {RGB565,  1, 16, 2, {{1, 1, 1, 1}}},
    {RGB555,  1, 16, 2, {{1, 1, 1, 1}}},
    {RGB444,  1, 16, 2, {{1, 1, 1, 1}}},
    {GRAY,    1,  8, 1, {{1, 1, 1, 1}}},
    {GRAYA,   2,  8, 1, {{1, 1, 1, 1}, {1, 1, 1, 1}}},
    {YUV420A, 4,  8, 1, {{1, 1, 1, 1}, {2, 2, 1, 1}, {2, 2, 1, 1}, {1, 1, 1, 1}}},
    {YUV422A, 4,  8, 1, {{1, 1, 1, 1}, {2, 1, 1, 1}, {2, 1, 1, 1}, {1, 1, 1, 1}}},
    {YUV444A, 4,  8, 1, {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}}},
    {YVU9,    3,  8, 1, {{1, 1, 1, 1}, {4, 4, 1, 1}, {4, 4, 1, 1}}},
    {D3D_SURFACE_DEC,   3, 8, 1, {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}}},
    {D3D_SURFACE,       3, 8, 1, {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}}},
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
VideoData::VideoData(void)
{
    m_iAlignment = 1;
    m_pPlaneData = NULL;
    m_iPlanes = 0;
    m_ippSize.width = 0;
    m_ippSize.height = 0;
    m_ColorFormat = NONE;
    m_picStructure = PS_FRAME;

    // set square pixel
    m_iHorzAspect =
    m_iVertAspect = 1;

    m_pbAllocated = NULL;

} // VideoData::VideoData(void)

VideoData::~VideoData(void)
{
    Close();

} // VideoData::~VideoData(void)

// Release all possessed memory
Status VideoData::Close(void)
{
    if (m_pPlaneData)
        delete [] m_pPlaneData;

    m_pPlaneData = NULL;
    m_iPlanes = 0;

    return ReleaseImage();

} // Status VideoData::Close(void)

// Release image memory if it was owned
Status VideoData::ReleaseImage(void)
{
    Ipp32s i;
    for (i = 0; i < m_iPlanes; i++)
        m_pPlaneData[i].m_pPlane = NULL;

    if (m_pbAllocated)
        delete [] m_pbAllocated;

    m_pbAllocated = NULL;

    return MediaData::Close();

} // Status VideoData::ReleaseImage(void)

// Initializes image dimensions and bitdepth.
// Has to be followed by SetColorFormat call.
// Planes' information is initialized to invalid values
Status VideoData::Init(Ipp32s iWidth,
                       Ipp32s iHeight,
                       Ipp32s iPlanes,
                       Ipp32s iBitDepth)
{
    Ipp32s i;

    // check error(s)
    if ((0 >= iWidth) ||
        (0 >= iHeight) ||
        (0 >= iPlanes) ||
        (8 > iBitDepth))
        return UMC_ERR_INVALID_STREAM;

    // release object before initialization
    Close();

    // allocate plane info
    m_pPlaneData = new PlaneInfo[iPlanes];
    if (NULL == m_pPlaneData)
        return UMC_ERR_ALLOC;

    // fill plane info
    for (i = 0; i < iPlanes; i++)
    {
        m_pPlaneData[i].m_iSamples = 1;
        m_pPlaneData[i].m_iSampleSize = (iBitDepth+7)>>3;
        m_pPlaneData[i].m_iBitDepth = iBitDepth;
        m_pPlaneData[i].m_pPlane = NULL;
        m_pPlaneData[i].m_nMemSize = 0;
        // we can't set pitch without align value
        m_pPlaneData[i].m_nPitch = 0;
        m_pPlaneData[i].m_nOffset = 0;
        // can't assign dimension to unknown planes
        m_pPlaneData[i].m_ippSize.width = 0;
        m_pPlaneData[i].m_ippSize.height = 0;
    }

    m_iPlanes = iPlanes;
    m_ippSize.width = iWidth;
    m_ippSize.height = iHeight;

    return UMC_OK;

} // Status VideoData::Init(Ipp32s iWidth,

// Completely sets image information, without allocation or linking to
// image memory.
Status VideoData::Init(Ipp32s iWidth,
                       Ipp32s iHeight,
                       ColorFormat cFormat,
                       Ipp32s iBitDepth)
{
    Status umcRes;
    const ColorFormatInfo* pFormat;

    // if D3D_SURFACE, data pointer should point to D3DSurface structure
    if (cFormat == D3D_SURFACE)
    {
        m_ColorFormat = cFormat;
        m_ippSize.width = iWidth;
        m_ippSize.height = iHeight;
        umcRes = Alloc();
        memset(m_pBufferPointer, 0, m_nBufferSize);
        return umcRes;
    }

    pFormat = GetColorFormatInfo(cFormat);
    if(NULL == pFormat)
        return UMC_ERR_INVALID_STREAM;

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

} // Status VideoData::Init(Ipp32s iWidth,

// Sets or change Color format information for image, only when it has
// specified size, number of planes and bitdepth. Number of planes in cFormat must
// be not greater than specified in image.
Status VideoData::SetColorFormat(ColorFormat cFormat)
{
    Ipp32s i;
    const ColorFormatInfo *pFormat;

    // check error(s)
    pFormat = GetColorFormatInfo(cFormat);
    if (NULL == pFormat)
        return UMC_ERR_INVALID_STREAM;
    if (m_iPlanes < pFormat->m_iPlanes)
        return UMC_ERR_INVALID_STREAM;

    m_ColorFormat = cFormat;

    m_pPlaneData[0].m_nOffset = 0;

    // set correct width & height to planes
    for (i = 0; i < m_iPlanes; i += 1)
    {
        Ipp32s align, bpp;
        if(i>0) {
            m_pPlaneData[i].m_nOffset =
                m_pPlaneData[i-1].m_nOffset + m_pPlaneData[i-1].m_nMemSize;
        }
        if (i < pFormat->m_iPlanes)
        {
            m_pPlaneData[i].m_iWidthDiv = pFormat->m_PlaneFormatInfo[i].m_iWidthDiv;
            m_pPlaneData[i].m_iHeightDiv = pFormat->m_PlaneFormatInfo[i].m_iHeightDiv;
            m_pPlaneData[i].m_iSamples = pFormat->m_PlaneFormatInfo[i].m_iChannels;
        } else {
            m_pPlaneData[i].m_iWidthDiv = 1;
            m_pPlaneData[i].m_iHeightDiv = 1;
            m_pPlaneData[i].m_iSamples = 1;
        }
        if (m_pPlaneData[i].m_iWidthDiv != 1) {
            int sz = m_pPlaneData[i].m_iWidthDiv;
            m_pPlaneData[i].m_ippSize.width = (m_ippSize.width + sz - 1) / sz;
        } else {
            m_pPlaneData[i].m_ippSize.width = m_ippSize.width;
        }
        if (m_pPlaneData[i].m_iHeightDiv != 1) {
            int sz = m_pPlaneData[i].m_iHeightDiv;
            m_pPlaneData[i].m_ippSize.height = (m_ippSize.height + sz - 1) / sz;
        } else {
            m_pPlaneData[i].m_ippSize.height = m_ippSize.height;
        }
        bpp = m_pPlaneData[i].m_iSampleSize * m_pPlaneData[i].m_iSamples;
        align = IPP_MAX(m_iAlignment, bpp);
        if (i < pFormat->m_iPlanes) {
            // sometimes dimension of image may be not aligned to native size
            align = IPP_MAX(align, pFormat->m_iMinAlign);
            align *= pFormat->m_PlaneFormatInfo[i].m_iAlignMult;
        }
        m_pPlaneData[i].m_nPitch =
            align_value<size_t>(m_pPlaneData[i].m_ippSize.width * bpp, align);
        m_pPlaneData[i].m_nMemSize = m_pPlaneData[i].m_nPitch * m_pPlaneData[i].m_ippSize.height;
    }
    // special case, can't be completely covered by format table
    //if(cFormat == YV12) { // V than U
    //  size_t tmp = m_pPlaneData[1].m_nOffset;
    //  m_pPlaneData[1].m_nOffset = m_pPlaneData[2].m_nOffset;
    //  m_pPlaneData[2].m_nOffset = tmp;
    //}

    // for complexer cases, such as IMC2 or IMC4, need more manual changes here

    return UMC_OK;

} // Status VideoData::SetColorFormat(ColorFormat cFormat)


// Set common Alignment
Status VideoData::SetAlignment(Ipp32s iAlignment)
{
    // check alignment
    Ipp32s i;
    if(iAlignment <= 0)
        return UMC_ERR_INVALID_STREAM;
    for (i = 1; i < (1 << 16); i <<= 1) {
        if (i & iAlignment) {
            m_iAlignment = i;
            break; // stop at last nonzero bit
        }
    }

    if (i != iAlignment)
        return UMC_WRN_INVALID_STREAM;

    return UMC_OK;
}

// Allocates memory according to existing information in VideoData and given alignment
// If image memory was already allocated it is released.
// return UMC_OK on success, UMC_ERR_INVALID_STREAM if image is improperly described
Status VideoData::Alloc(size_t requiredSize)
{
    size_t nSize;

    // touch unreferenced parameters
    requiredSize = requiredSize;

    // Release previous buffer
    ReleaseImage();

    // get size of buffer to allocate
    nSize = GetMappingSize();
    if (0 == nSize)
        return UMC_ERR_INVALID_STREAM;

    // allocate buffer
    m_pbAllocated = new Ipp8u[nSize + m_iAlignment - 1];
    if (NULL == m_pbAllocated)
        return UMC_ERR_ALLOC;

    // set pointer to image
    return SetBufferPointer(m_pbAllocated, nSize);

} // Status VideoData::Alloc()

// Links to provided image memory
// Image must be described before
Status VideoData::SetBufferPointer(Ipp8u *pbBuffer, size_t nSize)
{
    Ipp32s i;
    size_t mapsize;
    Ipp8u *ptr = align_pointer<Ipp8u *>(pbBuffer, m_iAlignment);

    // check error(s)
    if (NULL == m_pPlaneData && m_ColorFormat != D3D_SURFACE) {
        SetDataSize(0);
        return UMC_ERR_FAILED;
    }

    mapsize = GetMappingSize();
    if (nSize < mapsize) {
        SetDataSize(0);
        return UMC_ERR_NOT_ENOUGH_BUFFER;
    }

    // set new plane pointers
    if (m_pPlaneData)
    {
        Ipp8u *tmp = ptr;
        for(i=0; i<m_iPlanes; i++) {
            m_pPlaneData[i].m_pPlane = tmp;
            tmp += m_pPlaneData[i].m_nMemSize;
        }
    }

    // call parent class methods
    MediaData::SetBufferPointer(pbBuffer, nSize);
    // set valid data size
    SetDataSize(mapsize + (ptr - pbBuffer));
    MoveDataPointer((Ipp32s)(ptr - pbBuffer));

    return UMC_OK;

} // Status VideoData::SetBufferPointer(Ipp8u *pbBuffer, size_t nSize)

// Returns required image memory size according to alignment and current image description
size_t VideoData::GetMappingSize() const
{
    Ipp32s i;
    size_t size = 0;

    if (m_ColorFormat == D3D_SURFACE)
    {
        return D3D_SURFACE_SIZE;
    }

    UMC_CHECK(m_pPlaneData, 0);

    for (i = 0; i < m_iPlanes; i++) {
      size += m_pPlaneData[i].m_nMemSize;
    }

    return size;

} // size_t VideoData::GetMappingSize(Ipp32s iAlignment)

// Set pointer for specified plane. Should be used for additional planes,
// or when image layout is different.
Status VideoData::SetPlanePointer(void *pDest, Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return UMC_ERR_FAILED;

    m_pPlaneData[iPlaneNumber].m_pPlane = (Ipp8u *) pDest;

    return UMC_OK;

} // Status VideoData::SetPlanePointer(void *pDest, Ipp32s iPlaneNumber)

Status VideoData::SetImageSize(Ipp32s width, Ipp32s height)
{
    // check error(s)
    if (NULL == m_pPlaneData)
        return UMC_ERR_FAILED;

    m_ippSize.width = width;
    m_ippSize.height = height;

    return UMC_OK;
}

// Set pitch for specified plane. Should be used for additional planes,
// or when image layout is different.
Status VideoData::SetPlanePitch(size_t nPitch, Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return UMC_ERR_FAILED;

    m_pPlaneData[iPlaneNumber].m_nPitch = nPitch;
    m_pPlaneData[iPlaneNumber].m_nMemSize = nPitch * m_pPlaneData[iPlaneNumber].m_ippSize.height;

    return UMC_OK;

} // Status VideoData::SetPlanePitch(size_t nPitch, Ipp32s iPlaneNumber)

// Set bitdepth for specified plane, usually additional or when bitdepth differs
// for main planes
Status VideoData::SetPlaneBitDepth(Ipp32s iBitDepth, Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return UMC_ERR_FAILED;

    m_pPlaneData[iPlaneNumber].m_iBitDepth = iBitDepth;
    if(m_pPlaneData[iPlaneNumber].m_iSampleSize*8 < iBitDepth)
        m_pPlaneData[iPlaneNumber].m_iSampleSize = (iBitDepth+7)>>3;

    return UMC_OK;

} // Status VideoData::SetPlaneBitDepth(Ipp32s iBitDepth, Ipp32s iPlaneNumber)

// Set sample size for specified plane, usually additional or when bitdepth differs
// for main planes
Status VideoData::SetPlaneSampleSize(Ipp32s iSampleSize, Ipp32s iPlaneNumber)
{
    // check error(s)
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return UMC_ERR_FAILED;

    m_pPlaneData[iPlaneNumber].m_iSampleSize = iSampleSize;
    if(iSampleSize*8 < m_pPlaneData[iPlaneNumber].m_iBitDepth)
        m_pPlaneData[iPlaneNumber].m_iBitDepth = iSampleSize*8;

    return UMC_OK;

} // Status VideoData::SetPlaneSampleSize(Ipp32s iSampleSize, Ipp32s iPlaneNumber)

// Links plane pointers to surface using provided pitch.
// All pitches and plane info are updated according to current
// color format.
// Supposes no gaps between planes.
Status VideoData::SetSurface(void* ptr, size_t nPitch)
{
    Status ret;
    size_t size = 0;
    int i;

    // check error(s)
    UMC_CHECK(ptr, UMC_ERR_NULL_PTR);
    UMC_CHECK(m_pPlaneData, UMC_ERR_NOT_INITIALIZED);

    if(nPitch == 0) // use current
      nPitch = m_pPlaneData[0].m_nPitch;

    m_pPlaneData[0].m_nOffset = 0;

    for (i = 0; i < m_iPlanes; i++) {
      m_pPlaneData[i].m_nPitch = nPitch;
      if (i > 0) {
        m_pPlaneData[i].m_nPitch *= m_pPlaneData[i].m_iSamples*m_pPlaneData[0].m_iWidthDiv;
        m_pPlaneData[i].m_nPitch /= m_pPlaneData[i].m_iWidthDiv*m_pPlaneData[0].m_iSamples;
        m_pPlaneData[i].m_nOffset = m_pPlaneData[i - 1].m_nOffset + m_pPlaneData[i - 1].m_nMemSize;
      }
      m_pPlaneData[i].m_pPlane = (Ipp8u*)ptr + m_pPlaneData[i].m_nOffset;
      m_pPlaneData[i].m_nMemSize = m_pPlaneData[i].m_nPitch * m_pPlaneData[i].m_ippSize.height;
      size += m_pPlaneData[i].m_nMemSize;
    }

    ret = MediaData::SetBufferPointer((Ipp8u*)ptr, size);
    ret = MediaData::SetDataSize(size);

    return ret;
}

#define PITCH_PREC  8

// Calculate pitch from mapping size
size_t VideoData::GetPitchFromMappingSize(size_t mappingSize) const
{
    size_t size = 0;
    int i;

    // check error(s)
    UMC_CHECK(m_pPlaneData, 0);

    // calculate mapping size for pitch equal to (1 << PITCH_PREC)
    size = m_pPlaneData[0].m_ippSize.height << PITCH_PREC;
    for (i = 1; i < m_iPlanes; i++) {
      size_t plane_size = m_pPlaneData[i].m_ippSize.height << PITCH_PREC;
      plane_size *= m_pPlaneData[i].m_iSamples*m_pPlaneData[0].m_iWidthDiv;
      plane_size /= m_pPlaneData[i].m_iWidthDiv*m_pPlaneData[0].m_iSamples;
      size += plane_size;
    }

    UMC_CHECK(size, 0);

    // calulate real pitch
    return ((mappingSize << PITCH_PREC)/size);
}

Status VideoData::ConvertPictureStructure(PictureStructure newPicStructure)
{
  PictureStructure curr = (PictureStructure)(m_picStructure & PS_FRAME);
  int k;

  vm_debug_trace2(VM_DEBUG_VERBOSE, VM_STRING("VideoData::ConvertPictureStructure %d->%d\n"), curr, newPicStructure);

  if (curr == PS_FRAME && newPicStructure == PS_TOP_FIELD) {
    m_ippSize.height >>= 1;
    for (k = 0; k < m_iPlanes; k++) {
      m_pPlaneData[k].m_ippSize.height >>= 1;
      m_pPlaneData[k].m_nPitch <<= 1;
    }
    curr = PS_TOP_FIELD;
  }

  if (curr == PS_TOP_FIELD && newPicStructure == PS_BOTTOM_FIELD) {
    for (k = 0; k < m_iPlanes; k++) {
      m_pPlaneData[k].m_pPlane += (m_pPlaneData[k].m_nPitch >> 1);
    }
    curr = PS_BOTTOM_FIELD;
  }

  if (curr == PS_BOTTOM_FIELD && (newPicStructure == PS_TOP_FIELD || newPicStructure == PS_FRAME)) {
    for (k = 0; k < m_iPlanes; k++) {
      m_pPlaneData[k].m_pPlane -= (m_pPlaneData[k].m_nPitch >> 1);
    }
    curr = PS_TOP_FIELD;
  }

  if (curr == PS_TOP_FIELD && newPicStructure == PS_FRAME) {
    m_ippSize.height <<= 1;
    for (k = 0; k < m_iPlanes; k++) {
      m_pPlaneData[k].m_ippSize.height <<= 1;
      m_pPlaneData[k].m_nPitch >>= 1;
    }
    curr = PS_FRAME;
  }

  return SetPictureStructure(curr /*| (m_picStructure &~ PS_FRAME)*/);
}

// fills PlaneInfo structure
Status VideoData::GetPlaneInfo(PlaneInfo* pInfo, Ipp32s iPlaneNumber)
{
    // check error(s)
    if (NULL == pInfo)
        return UMC_ERR_NULL_PTR;
    if ((m_iPlanes <= iPlaneNumber) ||
        (0 > iPlaneNumber) ||
        (NULL == m_pPlaneData))
        return UMC_ERR_FAILED;

    *pInfo = m_pPlaneData[iPlaneNumber];
    return UMC_OK;

} // Status VideoData::GetPlaneInfo(PlaneInfo* pInfo, Ipp32s iPlaneNumber)

// converts display aspect ratio to pixel AR
// or vise versa with exchanged width and height
Status DARtoPAR(Ipp32s width, Ipp32s height, Ipp32s dar_h, Ipp32s dar_v,
                Ipp32s *par_h, Ipp32s *par_v)
{
  // (width*par_h) / (height*par_v) == dar_h/dar_v =>
  // par_h / par_v == dar_h * height / (dar_v * width)
  Ipp32s simple_tab[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59};
  Ipp32s i, denom;

  // suppose no overflow of 32s
  Ipp32s h = dar_h * height;
  Ipp32s v = dar_v * width;
  // remove common multipliers
  while( ((h|v)&1) == 0 ) {
    h>>=1;
    v>>=1;
  }

  for(i=0;i<sizeof(simple_tab)/sizeof(simple_tab[0]);i++) {
    denom = simple_tab[i];
    while(h%denom==0 && v%denom==0) {
      v /= denom;
      h /= denom;
    }
    if(v<=denom || h<=denom)
      break;
  }
  *par_h = h;
  *par_v = v;
  // can don't reach minimum, no problem
  if(i<sizeof(simple_tab)/sizeof(simple_tab[0]))
    return UMC_WRN_INVALID_STREAM;
  return UMC_OK;
}

// Crop
Status VideoData::Crop(UMC::sRECT CropArea)
{
  int left = CropArea.left;
  int top = CropArea.top;
  int right = CropArea.right;
  int bottom = CropArea.bottom;
  if (!right) right = m_ippSize.width;
  if (!bottom) bottom = m_ippSize.height;
  int w = right - left;
  int h = bottom - top;
  int k;

  UMC_CHECK(w > 0, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(h > 0, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(left >= 0, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(top >= 0, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(right <= m_ippSize.width, UMC_ERR_INVALID_PARAMS);
  UMC_CHECK(bottom <= m_ippSize.height, UMC_ERR_INVALID_PARAMS);

  for (k = 0; k < m_iPlanes; k++) {
    int wDiv = (m_pPlaneData[k].m_ippSize.width) ? m_ippSize.width/m_pPlaneData[k].m_ippSize.width : 1;
    int hDiv = (m_pPlaneData[k].m_ippSize.height) ? m_ippSize.height/m_pPlaneData[k].m_ippSize.height : 1;
    m_pPlaneData[k].m_pPlane += (top / hDiv) * m_pPlaneData[k].m_nPitch +
      (left / wDiv) * m_pPlaneData[k].m_iSamples * m_pPlaneData[k].m_iSampleSize;
    m_pPlaneData[k].m_ippSize.width = w / wDiv;
    m_pPlaneData[k].m_ippSize.height = h / hDiv;
  }
  m_ippSize.width = w;
  m_ippSize.height = h;

  return UMC_OK;
}

VideoData & VideoData::operator = (const VideoData &par)
{
    // check assignment for self
    if (this == &par)
    {
        return *this;
    }

    PlaneInfo *PlaneData = m_pPlaneData;
    if(m_iPlanes < par.m_iPlanes)
    {
        Close();
        PlaneData = new PlaneInfo[par.m_iPlanes];
    } else {
        ReleaseImage();
    }

    MediaData::operator=(par);

    memcpy_s(PlaneData, par.m_iPlanes*sizeof(PlaneInfo), par.m_pPlaneData, par.m_iPlanes*sizeof(PlaneInfo));

    m_iPlanes      = par.m_iPlanes;
    m_ippSize      = par.m_ippSize;
    m_ColorFormat  = par.m_ColorFormat;
    m_picStructure = par.m_picStructure;
    m_iHorzAspect  = par.m_iHorzAspect;
    m_iVertAspect  = par.m_iVertAspect;
    m_iAlignment   = par.m_iAlignment;

    m_pPlaneData = PlaneData;

    m_pbAllocated = NULL;

    return *this;
}

} // end namespace UMC
