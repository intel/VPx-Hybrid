/* ****************************************************************************** *\

Copyright (C) 2003-2009 Intel Corporation.  All rights reserved.

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

File Name: vm_mmap.h

\* ****************************************************************************** */

#ifndef __VM_MMAP_H__
#define __VM_MMAP_H__

#include "ippdefs.h"
#include "vm_types.h"
#include "vm_strings.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* flags */
enum
{
    FLAG_ATTRIBUTE_READ         = 0x00000001,
    FLAG_ATTRIBUTE_WRITE        = 0x00000002
};

/* Set the mmap handle to be invalid */
void vm_mmap_set_invalid(vm_mmap *handle);

/* Verify if the mmap handle is valid */
Ipp32s vm_mmap_is_valid(vm_mmap *handle);

/* Map a file into system memory, return size of the mapped file */
Ipp64u vm_mmap_create(vm_mmap *handle, vm_char *file, Ipp32s fileAttr);

/* Obtain a view of the mapped file, return the page aligned offset & size */
void *vm_mmap_set_view(vm_mmap *handle, Ipp64u *offset, Ipp64u *size);

/* Delete the mmap handle */
void vm_mmap_close(vm_mmap *handle);

/*  Return page size*/
Ipp32u vm_mmap_get_page_size(void);

/*  Return allocation granularity*/
Ipp32u vm_mmap_get_alloc_granularity(void);

/* Unmap the mmap handle */
void vm_mmap_unmap(vm_mmap *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_MMAP_H__ */
