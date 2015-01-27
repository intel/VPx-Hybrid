/* ****************************************************************************** *\

Copyright (C) 2002-2013 Intel Corporation.  All rights reserved.

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

File Name: umc_string.h

\* ****************************************************************************** */

#ifndef __UMC_STRING_H__
#define __UMC_STRING_H__
#if defined(_WIN32) || defined(_WIN64)

#include "vm_strings.h"
#include "ippdefs.h"

namespace UMC
{

// vm_char based string class with dynamic memory allocations
class DString
{
public:
    DString();
    DString(const vm_char *pSrc);
    DString(const DString &src);

    ~DString();

    void Clear(); // reset string buffer

    // replace current string with a new string
    size_t Replace(const vm_char* pSrc, size_t iSrcSize);

    // add new string to the end of the current string
    size_t Append(const vm_char* pSrc, size_t iSrcSize);

    Ipp32s Compare(const vm_char *pSrc, bool bCaseSensitive = true);

    size_t Trim();
    size_t TrimLeft();
    size_t TrimRight();

    size_t                  Size() const { return m_iLen; }
    operator       vm_char*     ()       { return m_pData; }
    operator const vm_char*     () const { return m_pData; }

    DString& operator=(const vm_char* pSrc)
    {
        Replace(pSrc, (Ipp32u)vm_string_strlen(pSrc));
        return *this;
    }

    DString& operator=(const DString &str)
    {
        Replace(str.m_pData, str.m_iLen);
        return *this;
    }

    DString operator + (const DString &right)
    {
        Append(right.m_pData, right.m_iLen);
        return *this;
    }
    DString operator + (const vm_char *right)
    {
        Append(right, (Ipp32u)vm_string_strlen(right));
        return *this;
    }
    DString operator + (const vm_char right)
    {
        Append(&right, 1);
        return *this;
    }

    void operator += (const DString &right)
    {
        Append(right.m_pData, right.m_iLen);
    }
    void operator += (const vm_char *right)
    {
        Append(right, (Ipp32u)vm_string_strlen(right));
    }
    void operator += (const vm_char right)
    {
        Append(&right, 1);
    }

    bool operator == (const vm_char *right)
    {
        if(!Compare(right))
            return true;
        return false;
    }

    bool operator == (const DString &right)
    {
        if(!Compare(right.m_pData))
            return true;
        return false;
    }

    bool operator != (const vm_char *right)
    {
        if(Compare(right))
            return true;
        return false;
    }

    bool operator != (const DString &right)
    {
        if(Compare(right.m_pData))
            return true;
        return false;
    }

protected:
    vm_char *m_pData; // string buffer
    size_t   m_iSize; // buffer size
    size_t   m_iLen;  // string length
};

}

#endif
#endif
