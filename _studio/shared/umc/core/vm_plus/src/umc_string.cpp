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

File Name: umc_string.cpp

\* ****************************************************************************** */
#if defined(_WIN32) || defined(_WIN64)

#include "umc_string.h"

using namespace UMC;


DString::DString()
{
    m_pData = NULL;
    m_iSize = 0;
    m_iLen  = 0;
    Replace((const vm_char*)&(VM_STRING("")), 0); // initialize with zero-length string to avoid typecast memory problems
}

DString::DString(const vm_char *pSrc)
{
    m_pData = NULL;
    m_iSize = 0;
    m_iLen  = 0;
    if(!pSrc)
        Replace((const vm_char*)&(VM_STRING("")), 0);
    else
        Replace(pSrc, (Ipp32u)vm_string_strlen(pSrc));
}

DString::DString(const DString &src)
{
    m_pData = NULL;
    m_iSize = 0;
    m_iLen  = 0;
    Replace(src.m_pData, src.m_iLen);
}

DString::~DString()
{
    if(m_pData)
    {
        delete[] m_pData;
        m_pData = NULL;
    }
    m_iSize = 0;
    m_iLen  = 0;
}

void DString::Clear()
{
    if(m_pData)
    {
        delete[] m_pData;
        m_pData = NULL;
    }
    m_iSize = 0;
    m_iLen  = 0;
    Replace((const vm_char*)&(VM_STRING("")), 0);
}

size_t DString::Replace(const vm_char* pSrc, size_t iSrcSize)
{
    if(pSrc && m_pData != pSrc)
    {
        if(m_iSize < iSrcSize + 1)
        {
            if(m_pData)
                delete[] m_pData;
            m_iSize = iSrcSize + 1;
            m_pData = (vm_char*)new vm_char[m_iSize];
        }
        m_iLen = iSrcSize;
        memcpy_s(m_pData, m_iSize*sizeof(vm_char), pSrc, m_iLen*sizeof(vm_char));
        m_pData[m_iLen] = VM_STRING('\0');

        return iSrcSize;
    }
    return 0;
}

size_t DString::Append(const vm_char* pSrc, size_t iSrcSize)
{
    if(pSrc)
    {
        if(m_iSize < iSrcSize + m_iLen + 1)
        {
            m_iSize = (iSrcSize + m_iLen + 1);
            vm_char *pData = (vm_char*)new vm_char[m_iSize];
            if(m_pData)
            {
                memcpy_s(pData, m_iSize*sizeof(vm_char), m_pData, m_iLen*sizeof(vm_char));
                delete[] m_pData;
            }
            m_pData = pData;
        }
        memcpy_s(&m_pData[m_iLen], m_iLen*sizeof(vm_char), pSrc, iSrcSize*sizeof(vm_char));
        m_iLen += iSrcSize;
        m_pData[m_iLen] = VM_STRING('\0');
        return iSrcSize;
    }
    return 0;
}

Ipp32s DString::Compare(const vm_char *pSrc, bool bCaseSensitive)
{
    if(!pSrc || !m_pData)
        return -1;

    if(bCaseSensitive)
        return vm_string_strcmp(m_pData, pSrc);
    else
        return vm_string_stricmp(m_pData, pSrc);
}

size_t DString::Trim()
{
    return TrimRight() + TrimLeft();
}

size_t DString::TrimLeft()
{
    size_t iSpaces = 0;
    size_t i;

    if(!m_iLen)
        return 0;

    for(i = 0; i < m_iLen; i++)
    {
        if(m_pData[i] != VM_STRING(' '))
            break;

        iSpaces++;
    }

    if(iSpaces)
    {
        for(i = iSpaces; i < m_iLen; i++)
            m_pData[i - iSpaces] = m_pData[i];

        m_iLen -= iSpaces;
        m_pData[m_iLen] = VM_STRING('\0');
    }

    return iSpaces;
}

size_t DString::TrimRight()
{
    size_t iSpaces = 0;

    if(!m_iLen)
        return 0;

    for(size_t i = m_iLen - 1; ; i--)
    {
        if(m_pData[i] != VM_STRING(' '))
            break;

        iSpaces++;
        if(!i)
            break;
    }
    m_iLen = m_iLen - iSpaces;
    m_pData[m_iLen] = VM_STRING('\0');

    return iSpaces;
}

#endif
