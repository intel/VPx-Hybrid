/* ****************************************************************************** *\

Copyright (C) 2005-2007 Intel Corporation.  All rights reserved.

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

File Name: umc_malloc.h

\* ****************************************************************************** */

#ifndef __UMC_MALLOC_H__
#define __UMC_MALLOC_H__

//#define VM_MALLOC             // define this macro to use "vm memory allocation" instead of regular
#define VM_MALLOC_STATISTIC   // define this macro to turn on memory tracing into the c:/malloc.csv file

#if defined(VM_MALLOC)

#include <stdlib.h>     // declare native malloc functions to avoid redefinition it by vm_malloc
#include <malloc.h>     // declare native malloc functions to avoid redefinition it by vm_malloc
#include "ipps.h"
#include "vm_types.h"

class VM_MallocItem;
class VM_MallocItemArray
{
    VM_MallocItem** m_array;
    Ipp32u m_count;
    Ipp32u m_allocated;
    Ipp32s m_mem_usage_max;
    Ipp32s m_mem_usage_current;

public:
    VM_MallocItemArray();
    ~VM_MallocItemArray();

    void   AddItem(VM_MallocItem* item);
    void   DeleteItem(void* lpv);

    Ipp32s GetMemUsageMax()     { return m_mem_usage_max;     }
    Ipp32s GetMemUsageCurrent() { return m_mem_usage_current; }

    void   ChangeMemUsage(Ipp32s size);
};

extern VM_MallocItemArray vm_malloc_array;

#define    vm_args_malloc   const char* lpcFileName = 0, Ipp32u iStringNumber = 0

void*      vm_malloc         (Ipp32s size, vm_args_malloc);
void*      vm_calloc         (size_t num, Ipp32s size, vm_args_malloc);
void*      vm_realloc        (void *lpv,  Ipp32s size, vm_args_malloc);
void       vm_free           (void *lpv);

Ipp8u*     vm_ippsMalloc_8u  (Ipp32s size, vm_args_malloc);
Ipp16u*    vm_ippsMalloc_16u (Ipp32s size, vm_args_malloc);
Ipp32u*    vm_ippsMalloc_32u (Ipp32s size, vm_args_malloc);
Ipp8s*     vm_ippsMalloc_8s  (Ipp32s size, vm_args_malloc);
Ipp16s*    vm_ippsMalloc_16s (Ipp32s size, vm_args_malloc);
Ipp32s*    vm_ippsMalloc_32s (Ipp32s size, vm_args_malloc);
Ipp64s*    vm_ippsMalloc_64s (Ipp32s size, vm_args_malloc);
Ipp32f*    vm_ippsMalloc_32f (Ipp32s size, vm_args_malloc);
Ipp64f*    vm_ippsMalloc_64f (Ipp32s size, vm_args_malloc);
Ipp8sc*    vm_ippsMalloc_8sc (Ipp32s size, vm_args_malloc);
Ipp16sc*   vm_ippsMalloc_16sc(Ipp32s size, vm_args_malloc);
Ipp32sc*   vm_ippsMalloc_32sc(Ipp32s size, vm_args_malloc);
Ipp64sc*   vm_ippsMalloc_64sc(Ipp32s size, vm_args_malloc);
Ipp32fc*   vm_ippsMalloc_32fc(Ipp32s size, vm_args_malloc);
Ipp64fc*   vm_ippsMalloc_64fc(Ipp32s size, vm_args_malloc);
void       vm_ippsFree       (void *lpv);

void vm_malloc_measure(Ipp32u start, vm_args_malloc);

#if !defined(VM_MALLOC_OWN)
    void* operator new           (size_t size, const char* lpcFileName, Ipp32u iStringNumber);
    void* operator new[]         (size_t size, const char* lpcFileName, Ipp32u iStringNumber);

    void  operator delete        (void *lpv);
    void  operator delete        (void *lpv, const char* lpcFileName, Ipp32u iStringNumber);
    void  operator delete[]      (void *lpv);
    void  operator delete[]      (void *lpv, const char* lpcFileName, Ipp32u iStringNumber);

    #if defined(VM_MALLOC_STATISTIC)
            #define vm_args_info  __FILE__, __LINE__
    #else
            #define vm_args_info  0, 0
    #endif

    #define new                   new          (           vm_args_info)
    #define delete(lpv)           delete       (lpv,       vm_args_info)
    #define malloc(         size) vm_malloc    (     size, vm_args_info)
    #define calloc(    num, size) vm_calloc    (num, size, vm_args_info)
    #define realloc(   lpv, size) vm_realloc   (lpv, size, vm_args_info)

    #define ippsMalloc_8u(  size) vm_ippsMalloc_8u  (size, vm_args_info)
    #define ippsMalloc_16u( size) vm_ippsMalloc_16u (size, vm_args_info)
    #define ippsMalloc_32u( size) vm_ippsMalloc_32u (size, vm_args_info)
    #define ippsMalloc_8s(  size) vm_ippsMalloc_8s  (size, vm_args_info)
    #define ippsMalloc_16s( size) vm_ippsMalloc_16s (size, vm_args_info)
    #define ippsMalloc_32s( size) vm_ippsMalloc_32s (size, vm_args_info)
    #define ippsMalloc_64s( size) vm_ippsMalloc_64s (size, vm_args_info)
    #define ippsMalloc_32f( size) vm_ippsMalloc_32f (size, vm_args_info)
    #define ippsMalloc_64f( size) vm_ippsMalloc_64f (size, vm_args_info)
    #define ippsMalloc_8sc( size) vm_ippsMalloc_8sc (size, vm_args_info)
    #define ippsMalloc_16sc(size) vm_ippsMalloc_16sc(size, vm_args_info)
    #define ippsMalloc_32sc(size) vm_ippsMalloc_32sc(size, vm_args_info)
    #define ippsMalloc_64sc(size) vm_ippsMalloc_64sc(size, vm_args_info)
    #define ippsMalloc_32fc(size) vm_ippsMalloc_32fc(size, vm_args_info)
    #define ippsMalloc_64fc(size) vm_ippsMalloc_64fc(size, vm_args_info)

    #define free(lpv)     vm_free    (lpv)
    #define ippsFree(lpv) vm_ippsFree(lpv)

    #define vm_malloc_start_measure     vm_malloc_measure(1, vm_args_info);
    #define vm_malloc_finish_measure    vm_malloc_measure(0, vm_args_info);
#endif //VM_MALLOC_OWN

#else //VM_MALLOC

#define vm_malloc_start_measure
#define vm_malloc_finish_measure

#endif //VM_MALLOC

#endif //__UMC_MALLOC_H__
