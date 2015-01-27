/* ****************************************************************************** *\

Copyright (C) 2010-2012 Intel Corporation.  All rights reserved.

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

File Name: mfx_search_dll.cpp

\* ****************************************************************************** */

#if defined(_WIN32) || defined(_WIN64)

#include "mfx_search_dll.h"

#include <windows.h>

mfxStatus RemoveSearchPath(char *pDllSearchPath, size_t pathSize)
{
    DWORD dwRes;
    BOOL bRes;

    dwRes = GetDllDirectoryA((DWORD) pathSize, pDllSearchPath);
    if (dwRes >= pathSize)
    {
        // error happened. Terminate the string, do nothing
        pDllSearchPath[0] = 0;
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    // remove the current directory from the default DLL search order
    bRes = SetDllDirectoryA("");
    if (FALSE == bRes)
    {
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    return MFX_ERR_NONE;

} // mfxStatus RemoveSearchPath(char *pDllSearchPath, size_t pathSize)

void RestoreSearchPath(const char *pDllSearchPath)
{
    SetDllDirectoryA(pDllSearchPath);

} // void RestoreSearchPath(const char *pDllSearchPath)

#endif // defined(_WIN32) || defined(_WIN64)
