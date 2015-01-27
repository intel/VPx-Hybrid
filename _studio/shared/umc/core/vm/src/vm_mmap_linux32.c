/* ****************************************************************************** *\

Copyright (C) 2003-2013 Intel Corporation.  All rights reserved.

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

File Name: vm_mmap_linux32.c

\* ****************************************************************************** */

#if defined(LINUX32) || defined(__APPLE__)

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "vm_mmap.h"

/* Set the mmap handle an invalid value */
void vm_mmap_set_invalid(vm_mmap *handle)
{
    /* check error(s) */
    if (NULL == handle)
        return;

    handle->fd= -1;
    handle->address = NULL;
    handle->fAccessAttr = 0;

} /* void vm_mmap_set_invalid(vm_mmap *handle) */

/* Verify if the mmap handle is valid */
Ipp32s vm_mmap_is_valid(vm_mmap *handle)
{
    /* check error(s) */
    if (NULL == handle)
        return 0;

    return (-1 != handle->fd);

} /* Ipp32s vm_mmap_is_valid(vm_mmap *handle) */

/* Map a file into system meory, return size of the mapped file */
Ipp64u vm_mmap_create(vm_mmap *handle, vm_char *file, Ipp32s fileAccessAttr)
{
    size_t sizet;

    /* check error(s) */
    if (NULL == handle)
        return 0;

    handle->address = NULL;
    handle->sizet = 0;

    if(FLAG_ATTRIBUTE_READ & fileAccessAttr)
        handle->fd = open(file, O_RDONLY);
    else
        handle->fd = open(file, O_RDWR | O_CREAT,
                                S_IRUSR | S_IWUSR | // rw permissions for user
                                S_IRGRP | S_IWGRP | // rw permissions for group
                                S_IROTH); // r permissions for others

    if (-1 == handle->fd)
        return 0;

    sizet = lseek(handle->fd, 0, SEEK_END);
    lseek(handle->fd, 0, SEEK_SET);

    return sizet;

} /* Ipp64u vm_mmap_create(vm_mmap *handle, vm_char *file, Ipp32s fileAccessAttr) */

/* Obtain a view of the mapped file, return the adjusted offset & size */
void *vm_mmap_set_view(vm_mmap *handle, Ipp64u *offset, Ipp64u *sizet)
{
    Ipp64u pagesize = getpagesize();
    Ipp64u edge;

    /* check error(s) */
    if (NULL == handle)
        return NULL;

    if (handle->address)
        munmap(handle->address,handle->sizet);

    edge = (*sizet) + (*offset);
    (*offset) = ((Ipp64u)((*offset) / pagesize)) * pagesize;
    handle->sizet = (*sizet) = edge - (*offset);
    handle->address = mmap(0,
                           *sizet,
                           PROT_READ,
                           MAP_SHARED,
                           handle->fd,
                           *offset);

    return (handle->address == (void *)-1) ? NULL : handle[0].address;

} /* void *vm_mmap_set_view(vm_mmap *handle, Ipp64u *offset, Ipp64u *sizet) */

/* Remove the mmap */
void vm_mmap_close(vm_mmap *handle)
{
    /* check error(s) */
    if (NULL == handle)
        return;

    if (handle->address)
    {
        munmap(handle->address, handle->sizet);
        handle->address = NULL;
    }

    if (-1 != handle->fd)
    {
        close(handle->fd);
        handle->fd= -1;
    }
} /* void vm_mmap_close(vm_mmap *handle) */

Ipp32u vm_mmap_get_page_size(void)
{
    return getpagesize();

} /* Ipp32u vm_mmap_get_page_size(void) */

Ipp32u vm_mmap_get_alloc_granularity(void)
{
    return 16 * getpagesize();

} /* Ipp32u vm_mmap_get_alloc_granularity(void) */

void vm_mmap_unmap(vm_mmap *handle)
{
    /* check error(s) */
    if (NULL == handle)
        return;

    if (handle->address)
    {
        munmap(handle->address, handle->sizet);
        handle->address = NULL;
    }
} /* void vm_mmap_unmap(vm_mmap *handle) */
#else
# pragma warning( disable: 4206 )
#endif /* LINUX32 */
