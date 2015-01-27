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

File Name: umc_dynamic_cast.h

\* ****************************************************************************** */

#ifndef __UMC_DYNAMIC_CAST_H__
#define __UMC_DYNAMIC_CAST_H__

#include <stdlib.h> //  for size_t under Linux
#include <string.h>
#include "vm_types.h"
#include "vm_strings.h"

//  There are functional for dynamic cast (linear time casting).
//  Multiple inheritance is no acceptable.
//  Weak conversion is used to casting objects,
//  which created in different places & heaps (ex. EXE & DLL).

//  Declare "dynamic cast" type
typedef const char *(*pDynamicCastFunction)(void);

//  This define must be placed in first line of base class declaration (.H file)
#define DYNAMIC_CAST_DECL_BASE(class_name) \
    public: \
    /* declare function to obtain string with class name */ \
    static const char *__GetClassName(void) {return #class_name;} \
    /* strong casting - compare function adresses of classes */ \
    virtual bool TryStrongCasting(pDynamicCastFunction pCandidateFunction) const \
    { \
        return (pCandidateFunction == &class_name::__GetClassName); \
    } \
    /* weak casting - compare names of classes */ \
    virtual bool TryWeakCasting(pDynamicCastFunction pCandidateFunction) const \
    { \
        return (0 == strcmp(#class_name, pCandidateFunction())); \
    }

//  This define must be placed in first line of descendant class declaration (.H file)
#define DYNAMIC_CAST_DECL(class_name, parent_class) \
    public: \
    /* declare function to obtain string with class name */ \
    static const char *__GetClassName(void) {return #class_name;} \
    /* strong casting - compare function adresses of classes */ \
    virtual bool TryStrongCasting(pDynamicCastFunction pCandidateFunction) const \
    { \
        if (pCandidateFunction == &class_name::__GetClassName) \
            return true; \
        return parent_class::TryStrongCasting(pCandidateFunction); \
    } \
    /* weak casting - compare names of classes */ \
    virtual bool TryWeakCasting(pDynamicCastFunction pCandidateFunction) const \
    { \
        if (0 == strcmp(#class_name, pCandidateFunction())) \
            return true; \
        return parent_class::TryWeakCasting(pCandidateFunction); \
    }

// Function for dynamic cast from one class to another one (forward declaration)
template <class To, class From> To *DynamicCast(From *pFrom);
// Function for dynamic cast from one class to another one (inline implementation)
template <class To, class From> To *DynamicCast(From *pFrom)
{
    // some compiler complain to compare pointer and zero.
    // we use real zero pointer instead const zero.
    void *NullPointer = 0;

    // we don't need do casting
    if (NullPointer == pFrom)
        return reinterpret_cast<To *> (NullPointer);

    // try strong casting
    if (pFrom->TryStrongCasting(&To::__GetClassName))
        return reinterpret_cast<To *> (pFrom);

    // there are no strong conversion, try weak casting
    if (pFrom->TryWeakCasting(&To::__GetClassName))
        return reinterpret_cast<To *> (pFrom);

    // there are no any conversion
    return reinterpret_cast<To *> (NullPointer);

} // template <class To, class From> lpTo *DynamicCast(From *pFrom)

#endif /* __UMC_DYNAMIC_CAST_H__ */
