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

File Name: vm_types.h

\* ****************************************************************************** */

#ifndef __VM_TYPES_H__
#define __VM_TYPES_H__


#if defined(LINUX32) || defined(__APPLE__)
# include "sys/vm_types_linux32.h"
#else /* LINUX32 */
//# include <io.h>
# include "sys/vm_types_win32.h"
#endif /* LINUX32 */

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define VM_ALIGN16_DECL(X) VM_ALIGN_DECL(16,X)
#define VM_ALIGN32_DECL(X) VM_ALIGN_DECL(32,X)

#define _MAX_LEN 256

typedef enum e_vm_Status
{
    VM_OK                           = 0,    /* no error */
    VM_OPERATION_FAILED             =-999,
    VM_NOT_INITIALIZED              =-998,
    VM_TIMEOUT                      =-987,
    VM_NOT_ENOUGH_DATA              =-996,  /* not enough input data */

    VM_NULL_PTR                     =-995,
    VM_SO_CANT_LOAD                 =-994,
    VM_SO_INVALID_HANDLE            =-993,
    VM_SO_CANT_GET_ADDR             =-992

} vm_status;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_TYPES_H__ */
