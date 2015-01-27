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

File Name: mfx_msg.h

\* ****************************************************************************** */
#ifndef __MFX_MSG_H
#define __MFX_MSG_H

#include <ippdefs.h>

#include <stdio.h>
#include <windows.h>

extern "C"
Ipp64u freq;

inline
void Msg(Ipp32u threadNum, const char *pMsg, Ipp64u time, Ipp64u lasting)
{
    char cStr[256];
    Ipp32s timeSec, timeMSec, us;

    timeSec = (Ipp32u) ((time / freq) % 60);
    timeMSec = (Ipp32u) (((time * 1000) / freq) % 1000);
    us = (Ipp32u) ((lasting * 1000000) / freq);
    sprintf_s(cStr, sizeof(cStr),
              "[% 4u] %3u.%03u % 30s % 6u us\n",
              threadNum, timeSec, timeMSec, pMsg, us);
    OutputDebugStringA(cStr);

} // void Msg(Ipp32u threadNum, const char *pMsg, Ipp64u time)

#endif // __MFX_MSG_H
