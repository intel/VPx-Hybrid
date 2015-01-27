/* ****************************************************************************** *\

Copyright (C) 2010-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_platform_headers.h

\* ****************************************************************************** */
#ifndef __MFX_PLATFORM_HEADERS_H__
#define __MFX_PLATFORM_HEADERS_H__

#include <mfxvideo++int.h>

#if (defined(_WIN32) || defined(_WIN64)) 
#include <windows.h>
#include <d3d9.h>
#include <dxva.h>
#include <dxva2api.h>
#include <ddraw.h>

//#if defined(MFX_D3D11_ENABLED)
#include <d3d11.h>
//#endif

#ifndef D3DDDIFORMAT
#define D3DDDIFORMAT        D3DFORMAT
#endif

typedef IDirectXVideoDecoderService*    _mfxPlatformAccelerationService;
typedef IDirect3DSurface9*              _mfxPlatformVideoSurface;

#else // #if (defined(_WIN32) || defined(_WIN64)) 

#if (defined(LINUX32) || defined(LINUX64) )
    #if defined(MFX_VA)
        /* That's tricky: if LibVA will not be installed on the machine, you should be able
         * to build SW Media SDK and some apps in SW mode. Thus, va.h should not be visible.
         * Since we develop on machines where LibVA is installed, we forget about LibVA-free
         * build sometimes. So, that's the reminder why MFX_VA protection was added here.
         */
        #include <va/va.h>
        typedef VADisplay                       _mfxPlatformAccelerationService;
        typedef VASurfaceID                     _mfxPlatformVideoSurface;
    #endif // #if defined(MFX_VA)
#endif // #if (defined(LINUX32) || defined(LINUX64) )

#ifndef D3DDDIFORMAT
#define D3DDDIFORMAT        D3DFORMAT
#endif

typedef int             BOOL;
typedef char            CHAR;
typedef unsigned char   BYTE;
typedef short           SHORT;
typedef int             INT;


typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned int   UINT;

#if defined(_WIN32) || defined(_WIN64)
typedef long            LONG;
typedef unsigned long   ULONG;
typedef unsigned long   DWORD;
#else
typedef int            LONG;  // should be 32 bit to align with Windows
typedef unsigned int   ULONG;
typedef unsigned int   DWORD;
#endif

typedef unsigned long long UINT64;

#define FALSE               0
#define TRUE                1

// DXVA2 types
struct IDirect3DSurface9;
struct IDirect3DDeviceManager9;
struct ID3D11Texture2D;
struct ID3D11Device;

typedef int D3DFORMAT;

typedef struct _DXVA2_Frequency
{
    UINT Numerator;
    UINT Denominator;
}     DXVA2_Frequency;

typedef struct _DXVA2_ExtendedFormat
{
    union 
    {
        struct 
        {
            UINT SampleFormat    : 8;
            UINT VideoChromaSubsampling    : 4;
            UINT NominalRange    : 3;
            UINT VideoTransferMatrix    : 3;
            UINT VideoLighting    : 4;
            UINT VideoPrimaries    : 5;
            UINT VideoTransferFunction    : 5;
        }     ;
        UINT value;
    }     ;
}     DXVA2_ExtendedFormat;

typedef struct _DXVA2_VideoDesc
{
    UINT SampleWidth;
    UINT SampleHeight;
    DXVA2_ExtendedFormat SampleFormat;
    D3DFORMAT Format;
    DXVA2_Frequency InputSampleFreq;
    DXVA2_Frequency OutputFrameFreq;
    UINT UABProtectionLevel;
    UINT Reserved;
}     DXVA2_VideoDesc;


typedef struct _D3DAES_CTR_IV
{
    UINT64   IV;         // Big-Endian IV
    UINT64   Count;      // Big-Endian Block Count
} D3DAES_CTR_IV;

#undef  LOWORD
#define LOWORD(_dw)     ((_dw) & 0xffff)
#undef  HIWORD
#define HIWORD(_dw)     (((_dw) >> 16) & 0xffff)


//typedef int GUID;

//static const GUID DXVA2_Intel_Encode_AVC = 0;
//static const GUID DXVA2_Intel_Encode_MPEG2 = 1;

#endif // #if (defined(_WIN32) || defined(_WIN64)) 

#endif // __MFX_PLATFORM_HEADERS_H__
