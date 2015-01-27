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

File Name: umc_mmap.h

\* ****************************************************************************** */

#ifndef __UMC_MMAP_H__
#define __UMC_MMAP_H__

#include "ippdefs.h"
#include "vm_debug.h"
#include "vm_mmap.h"
#include "umc_structures.h"

namespace UMC
{

class MMap
{
public:
    // Default constructor
    MMap(void);
    // Destructor
    virtual ~MMap(void);

    // Initialize object
    Status Init(vm_char *sz_file);
    // Map memory
    Status Map(Ipp64u st_offset, Ipp64u st_sizet);
    // Get addres of mapping
    void *GetAddr(void);
    // Get offset of mapping
    Ipp64u GetOffset(void);
    // Get size of mapping
    Ipp64u GetSize(void);
    // Get size of mapped file
    Ipp64u GetFileSize(void);

protected:
    vm_mmap m_handle;                                         // (vm_mmap) handle to system mmap object
    void *m_address;                                          // (void *) addres of mapped window
    Ipp64u m_file_size;                                       // (Ipp64u) file size
    Ipp64u m_offset;                                          // (Ipp64u) offset of mapping
    Ipp64u m_sizet;                                           // (Ipp64u) size of window
};

inline
void *MMap::GetAddr(void)
{
    return m_address;

} // void *MMap::GetAddr(void)

inline
Ipp64u MMap::GetOffset(void)
{
    return m_offset;

} // Ipp64u MMap::GetOffset(void)

inline
Ipp64u MMap::GetSize(void)
{
    return m_sizet;

} // Ipp64u MMap::GetSize(void)

inline
Ipp64u MMap::GetFileSize(void)
{
    return m_file_size;

} // Ipp64u MMap::GetFileSize(void)

} // namespace UMC

#endif // __UMC_MMAP_H__
