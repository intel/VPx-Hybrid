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

File Name: vm_interlocked_unix.c

\* ****************************************************************************** */

#if defined(LINUX32) || defined(__APPLE__)

#include <vm_interlocked.h>

static Ipp16u vm_interlocked_add16(volatile Ipp16u *pVariable, Ipp16u value_to_add)
{
    asm volatile ("lock; xaddw %0,%1"
                  : "=r" (value_to_add), "=m" (*pVariable)
                  : "0" (value_to_add), "m" (*pVariable)
                  : "memory", "cc");
    return value_to_add;
}

static Ipp32u vm_interlocked_add32(volatile Ipp32u *pVariable, Ipp32u value_to_add)
{
    asm volatile ("lock; xaddl %0,%1"
                  : "=r" (value_to_add), "=m" (*pVariable)
                  : "0" (value_to_add), "m" (*pVariable)
                  : "memory", "cc");
    return value_to_add;
}

Ipp16u vm_interlocked_inc16(volatile Ipp16u *pVariable)
{
    return vm_interlocked_add16(pVariable, 1) + 1;
} /* Ipp16u vm_interlocked_inc16(Ipp16u *pVariable) */

Ipp16u vm_interlocked_dec16(volatile Ipp16u *pVariable)
{
    return vm_interlocked_add16(pVariable, (Ipp16u)-1) - 1;
} /* Ipp16u vm_interlocked_dec16(Ipp16u *pVariable) */

Ipp32u vm_interlocked_inc32(volatile Ipp32u *pVariable)
{
    return vm_interlocked_add32(pVariable, 1) + 1;
} /* Ipp32u vm_interlocked_inc32(Ipp32u *pVariable) */

Ipp32u vm_interlocked_dec32(volatile Ipp32u *pVariable)
{
    return vm_interlocked_add32(pVariable, (Ipp32u)-1) - 1;
} /* Ipp32u vm_interlocked_dec32(Ipp32u *pVariable) */

Ipp32u vm_interlocked_cas32(volatile Ipp32u *pVariable, Ipp32u value_to_exchange, Ipp32u value_to_compare)
{
    Ipp32u previous_value;

    asm volatile ("lock; cmpxchgl %1,%2"
                  : "=a" (previous_value)
                  : "r" (value_to_exchange), "m" (*pVariable), "0" (value_to_compare)
                  : "memory", "cc");
    return previous_value;
} /* Ipp32u vm_interlocked_cas32(volatile Ipp32u *pVariable, Ipp32u value_to_exchange, Ipp32u value_to_compare) */

Ipp32u vm_interlocked_xchg32(volatile Ipp32u *pVariable, Ipp32u value)
{
    Ipp32u previous_value = value;

    asm volatile ("lock; xchgl %0,%1"
                  : "=r" (previous_value), "+m" (*pVariable)
                  : "0" (previous_value));
    return previous_value;
} /* Ipp32u vm_interlocked_xchg32(volatile Ipp32u *pVariable, Ipp32u value); */

#else
# pragma warning( disable: 4206 )
#endif /* #if defined(LINUX32) || defined(__APPLE__) */
