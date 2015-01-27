/* ****************************************************************************** *\

Copyright (C) 2009-2011 Intel Corporation.  All rights reserved.

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

File Name: vm_interlocked.h

\* ****************************************************************************** */

#ifndef __VM_INTERLOCKED_H__
#define __VM_INTERLOCKED_H__

#include "vm_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Thread-safe 16-bit variable incrementing */
Ipp16u vm_interlocked_inc16(volatile Ipp16u *pVariable);

/* Thread-safe 16-bit variable decrementing */
Ipp16u vm_interlocked_dec16(volatile Ipp16u *pVariable);

/* Thread-safe 32-bit variable incrementing */
Ipp32u vm_interlocked_inc32(volatile Ipp32u *pVariable);

/* Thread-safe 32-bit variable decrementing */
Ipp32u vm_interlocked_dec32(volatile Ipp32u *pVariable);

/* Thread-safe 32-bit variable comparing and storing */
Ipp32u vm_interlocked_cas32(volatile Ipp32u *pVariable, Ipp32u with, Ipp32u cmp);

/* Thread-safe 32-bit variable exchange */
Ipp32u vm_interlocked_xchg32(volatile Ipp32u *pVariable, Ipp32u val);

#ifdef __cplusplus
} // extern "C"
#endif /* __cplusplus */

#endif /* #ifndef __VM_INTERLOCKED_H__ */
