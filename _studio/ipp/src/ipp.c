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

File Name: ipp.c

\* ****************************************************************************** */


#include <ippdefs.h>
#include <assert.h>

IppStatus ippGetCpuFreqMhz(int* pMhz)
{
  assert(!"not implemented");
  return ippStsNoErr;
}

IppStatus ippiCopy_8u_C1R(
  const Ipp8u* pSrc, int srcStep,
  Ipp8u* pDst, int dstStep,IppiSize roiSize)
{
  assert(!"not implemented");
  return ippStsErr;
}

IppStatus ippiCopyManaged_8u_C1R(
  const Ipp8u* pSrc, int srcStep,
  Ipp8u* pDst, int dstStep,
  IppiSize roiSize, int flags)
{
  assert(!"not implemented");
  return ippStsErr;
}

IppStatus ippiYCbCr420_8u_P3P2R(
  const Ipp8u* pSrc[3], int srcStep[3], Ipp8u* pDstY,
  int dstYStep,Ipp8u* pDstCbCr, int dstCbCrStep, IppiSize roiSize)
{
  assert(!"not implemented");
  return ippStsErr;
}
