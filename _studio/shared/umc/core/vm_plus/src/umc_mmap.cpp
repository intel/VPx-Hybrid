/* ****************************************************************************** *\

Copyright (C) 2003-2007 Intel Corporation.  All rights reserved.

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

File Name: umc_mmap.cpp

\* ****************************************************************************** */

#include "umc_mmap.h"

namespace UMC
{

MMap::MMap(void) :
    m_address(NULL),
    m_file_size(0),
    m_offset(0),
    m_sizet(0)
{
    vm_mmap_set_invalid(&m_handle);

} // MMap::MMap(void) :

MMap::~MMap(void)
{
    vm_mmap_close(&m_handle);

} // MMap::~MMap(void)

Status MMap::Init(vm_char *sz_file)
{
    vm_mmap_close(&m_handle);

    m_offset = 0;
    m_sizet = 0;
    m_address = NULL;

    m_file_size = vm_mmap_create(&m_handle, sz_file, FLAG_ATTRIBUTE_READ);

    if (0 == m_file_size)
        return UMC_ERR_FAILED;

    return UMC_OK;

} // Status MMap::Init(vm_char *sz_file)

Status MMap::Map(Ipp64u st_offset, Ipp64u st_sizet)
{
    void *pv_addr;
    Ipp64u st_align = st_offset % vm_mmap_get_alloc_granularity();

    // check error(s)
    if (!vm_mmap_is_valid(&m_handle))
        return UMC_ERR_NOT_INITIALIZED;
    if (st_offset + st_sizet > m_file_size)
        return UMC_ERR_NOT_ENOUGH_DATA;

    st_sizet += st_align;
    st_offset -= st_align;

    // set new window
    pv_addr = vm_mmap_set_view(&m_handle, &st_offset, &st_sizet);
    if (NULL == pv_addr)
    {
        m_offset = 0;
        m_sizet = 0;
        return UMC_ERR_FAILED;
    }

    // save setting(s)
    m_address = pv_addr;
    m_offset = st_offset;
    m_sizet = st_sizet;

    return UMC_OK;

} // Status MMap::Map(Ipp64u st_offset, Ipp64u st_sizet)

}   //  namespace UMC
