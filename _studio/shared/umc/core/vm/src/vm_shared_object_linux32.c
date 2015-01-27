/* ****************************************************************************** *\

Copyright (C) 2003-2014 Intel Corporation.  All rights reserved.

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

File Name: vm_shared_object_linux32.c

\* ****************************************************************************** */

#if defined(LINUX32) || defined (OSX32)

#include <dlfcn.h>
#include "vm_shared_object.h"

vm_so_handle vm_so_load(const vm_char* so_file_name)
{
    void *handle;

    /* check error(s) */
    if (NULL == so_file_name)
        return NULL;

    handle = dlopen(so_file_name, RTLD_LAZY);

    return (vm_so_handle)handle;

} /* vm_so_handle vm_so_load(vm_char* so_file_name) */

vm_so_func vm_so_get_addr(vm_so_handle so_handle, const char *so_func_name)
{
    vm_so_func addr;

    /* check error(s) */
    if (NULL == so_handle)
        return NULL;

    addr = dlsym(so_handle,so_func_name);

    return addr;

} /* void *vm_so_get_addr(vm_so_handle so_handle, const vm_char *so_func_name) */

void vm_so_free(vm_so_handle so_handle)
{
    /* check error(s) */
    if (NULL == so_handle)
        return;

    dlclose(so_handle);

} /* void vm_so_free(vm_so_handle so_handle) */
#else
# pragma warning( disable: 4206 )
#endif /* LINUX32 || OSX32 */
