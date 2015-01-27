/* ****************************************************************************** *\

Copyright (C) 2014 Intel Corporation.  All rights reserved.

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

File Name: ippdefs.h

\* ****************************************************************************** */

#ifndef __IPPDEFS_H__
#define __IPPDEFS_H__

#include <stdlib.h>
#include <malloc.h>
#include <mfxdefs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define __STDCALL  MFX_STDCALL

typedef mfxU8 Ipp8u;
typedef mfxI16 Ipp16s;
typedef mfxU16 Ipp16u;
typedef mfxI32 Ipp32s;
typedef mfxU32 Ipp32u;
typedef mfxI64 Ipp64s;
typedef mfxU64 Ipp64u;

typedef mfxF32 Ipp32f;
typedef mfxF64 Ipp64f;

typedef struct {
    int width;
    int height;
} IppiSize;

typedef enum { ippFalse = 0, ippTrue = 1 } IppBool;

typedef enum {
  ippStsErr = -2,   /* Generic error. */
  ippStsNoErr = 0,  /* No errors. */
} IppStatus;

#define ippInit()

#define ippsCopy_8u(_src, _dst, _count) memcpy(_dst, _src, _count)
#define ippsZero_8u(_buf, _count) memset(_buf, 0, _count)

#define ippsMalloc_8u(_count) memalign(32, _count)
#define ippsFree(_buf) free(_buf)

#define ippMalloc(_count) memalign(64, _count)
#define ippFree(_buf) free(_buf)

IppStatus ippiCopy_8u_C1R(
  const Ipp8u* pSrc, int srcStep,
  Ipp8u* pDst, int dstStep,IppiSize roiSize);

IppStatus ippiCopyManaged_8u_C1R(
  const Ipp8u* pSrc, int srcStep,
  Ipp8u* pDst, int dstStep,
  IppiSize roiSize, int flags);

IppStatus ippiYCbCr420_8u_P3P2R(
  const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
  int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize);

#define IPP_MINABS_64F ( 2.2250738585072014e-308 )
#define IPP_MAXABS_64F ( 1.7976931348623158e+308 )

#define IPP_MAX( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define IPP_MIN( a, b ) ( ((a) < (b)) ? (a) : (b) )

// TODO: useless functions:
//  - ippGetCpuFreqMhz: called in vm_time_linux32.c, result used, vm function itself not called
//  - ippGetCpuFeatures: called in fast_copy.cpp, result is not used

IppStatus ippGetCpuFreqMhz(int* pMhz);
#define ippGetCpuFeatures(_one, _two)

#ifdef __cplusplus
} // extern "C"
#endif /* __cplusplus */

#endif // #ifndef __IPPDEFS_H__
