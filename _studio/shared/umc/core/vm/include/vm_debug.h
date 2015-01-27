/* ****************************************************************************** *\

Copyright (C) 2003-2011 Intel Corporation.  All rights reserved.

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

File Name: vm_debug.h

\* ****************************************************************************** */

#ifndef __VM_DEBUG_H__
#define __VM_DEBUG_H__

#include "vm_types.h"
#include "vm_strings.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
/*
#undef  VM_DEBUG      // uncomment to disable debug trace
#define VM_DEBUG (-1) // uncomment to enable full debug trace
*/
/* ============================================================================
// Define ASSERT and VERIFY for debugging purposes
*/
#ifdef VM_DEBUG
#include <assert.h>
#define VM_ASSERT(f) assert((f))
#else /* VM_DEBUG */
#define VM_ASSERT(f) ((void) 0)
#endif /* VM_DEBUG */

#if defined(_WIN32_WCE)
#if (_WIN32_WCE == 0x0300)
#define VM_ASSERT(exp) ((void)0)
#endif /* (_WIN32_WCE == 0x0300) */
#endif /* defined(_WIN32_WCE) */

typedef enum vm_debug_level {  /* debug level */
    VM_DEBUG_NONE           = 0x0000, /* none of the debug levels */
    VM_DEBUG_ERROR          = 0x0001, /* critical errors */
    VM_DEBUG_WARNING        = 0x0002, /* warning/non-critical errors */
    VM_DEBUG_INFO           = 0x0004, /* general attribute information */
    VM_DEBUG_MEMORY         = 0x0008, /* new/delete and alloc/free information */
    VM_DEBUG_PROGRESS       = 0x0010, /* general progressive information */
    VM_DEBUG_CALL           = 0x0020, /* information about calls */
    VM_DEBUG_VERBOSE        = 0x0040, /* other information */
    VM_DEBUG_MACROS         = 0x0080, /* used in macros in umc_structures.h */
    VM_DEBUG_LOG_ALL        = 0x00FF, /* all information */

    VM_DEBUG_SHOW_FILELINE  = 0x0100, /* show filename & line info in the logging message */
    VM_DEBUG_SHOW_DIRECTORY = 0x0200, /* show filename with directory */
    VM_DEBUG_SHOW_TIME      = 0x0400, /* show time info in the logging message */
    VM_DEBUG_SHOW_PID       = 0x0800, /* show thread PID in the logging message */
    VM_DEBUG_SHOW_FUNCTION  = 0x1000, /* show function name */
    VM_DEBUG_SHOW_LEVEL     = 0x2000, /* show level in the logging message */
    VM_DEBUG_SHOW_THIS      = 0x4000, /* show 'this' pointer */
    VM_DEBUG_SHOW_ALL       = 0xFF00, /* show all */

    VM_DEBUG_ALL            = 0xFFFF  /* all above */

} vm_debug_level;

typedef enum vm_debug_output {  /* debug output */
    VM_DEBUG_AUTO       = 0,        /* automatic (stdio for console app, OutputDebugString for win32 app) */
    VM_DEBUG_CONSOLE    = 0x10000,  /* stdio or stderr */
    VM_DEBUG_SYSLOG     = 0x20000,  /* OutputDebugString or syslog */
    VM_DEBUG_FILE       = 0x40000   /* append to a debug file */

} vm_debug_output;

/* ///////////////////// */

vm_debug_level vm_debug_setlevel(vm_debug_level level);
vm_debug_output vm_debug_setoutput(vm_debug_output output);
void vm_debug_setfile(vm_char *file, Ipp32s truncate);

void vm_debug_message(const vm_char *format, ...);

/* ///////////////////// */

#if defined(UNICODE) || defined(_UNICODE)
#define VM_STRING_FORMAT VM_STRING("%S")
#else
#define VM_STRING_FORMAT VM_STRING("%s")
#endif

/* ///////////////////// */

#ifdef VM_DEBUG

#ifdef _MSC_VER
#define VM_DEBUG_FUNC_NAME  VM_STRING(__FUNCTION__) /* Microsoft-specific */
#else
#define VM_DEBUG_FUNC_NAME  NULL /* name of function, redefine in code */
#endif
/*
#define PTR_THIS this */
#define PTR_THIS NULL

void vm_debug_trace_ex(Ipp32s level,
                       const void *ptr_this,
                       const vm_char *func_name,
                       const vm_char *file_name,
                       Ipp32s num_line,
                       const vm_char *format,
                       ...);

#define vm_debug_trace(level, format) \
  vm_debug_trace_ex(level, PTR_THIS, VM_DEBUG_FUNC_NAME, VM_STRING(__FILE__), __LINE__, format)

#define vm_debug_trace1(level, format, a1) \
  vm_debug_trace_ex(level, PTR_THIS, VM_DEBUG_FUNC_NAME, VM_STRING(__FILE__), __LINE__, format, a1)

#define vm_debug_trace2(level, format, a1, a2) \
  vm_debug_trace_ex(level, PTR_THIS, VM_DEBUG_FUNC_NAME, VM_STRING(__FILE__), __LINE__, format, a1, a2)

#define vm_debug_trace3(level, format, a1, a2, a3) \
  vm_debug_trace_ex(level, PTR_THIS, VM_DEBUG_FUNC_NAME, VM_STRING(__FILE__), __LINE__, format, a1, a2, a3)

#define vm_debug_trace4(level, format, a1, a2, a3, a4) \
  vm_debug_trace_ex(level, PTR_THIS, VM_DEBUG_FUNC_NAME, VM_STRING(__FILE__), __LINE__, format, a1, a2, a3, a4)

#define vm_debug_trace_withfunc(level, func, format) \
  vm_debug_trace_ex(level, PTR_THIS, func, VM_STRING(__FILE__), __LINE__, format)

#define vm_trace_GUID(guid) \
{ \
    vm_trace_fourcc(guid.Data1); \
    vm_trace_GUID_func((const UMC_GUID*)&guid, VM_STRING(#guid)); \
}

#else /* VM_DEBUG */

#define vm_debug_trace(level, format)
#define vm_debug_trace1(level, format, a1)
#define vm_debug_trace2(level, format, a1, a2)
#define vm_debug_trace3(level, format, a1, a2, a3)
#define vm_debug_trace4(level, format, a1, a2, a3, a4)
#define vm_debug_trace_withfunc(level, func, format)

#define vm_trace_GUID(guid)

#endif /* VM_DEBUG */

#define vm_trace_hresult(hr, mess, pthis) \
    vm_trace_hresult_func(hr, mess, pthis, (vm_char*)VM_STRING(__FUNCTION__), VM_STRING(__FILE__), __LINE__)

/* ////////////////// */

#define vm_debug_trace_i(level, a1)    \
  vm_debug_trace1(level, VM_STRING(#a1) VM_STRING(" = %d"), a1)

#define vm_debug_trace_x(level, a1)    \
  vm_debug_trace2(level, VM_STRING(#a1) VM_STRING(" = 0x%x = %d"), a1, a1)

#define vm_debug_trace_f(level, a1)    \
  vm_debug_trace1(level, VM_STRING(#a1) VM_STRING(" = %.4f"), a1)

#define vm_debug_trace_s(level, a1)    \
  vm_debug_trace1(level, VM_STRING("#####") VM_STRING(" %s"), VM_STRING(a1))

#define vm_debug_trace_ss(level, a1)    \
  vm_debug_trace1(level, VM_STRING_FORMAT, a1)

/* ///////////////////// */

#define vm_trace_i(a1)    \
  vm_debug_trace_i(VM_DEBUG_VERBOSE, a1)

#define vm_trace_x(a1)    \
  vm_debug_trace_x(VM_DEBUG_VERBOSE, a1)

#define vm_trace_f(a1)    \
  vm_debug_trace_f(VM_DEBUG_VERBOSE, a1)

#define vm_trace_s(a1)    \
  vm_debug_trace_s(VM_DEBUG_VERBOSE, a1)

#define vm_trace_ss(a1)    \
  vm_debug_trace_ss(VM_DEBUG_VERBOSE, a1)

#define TRACE_FUNC  vm_trace_s("")

#undef  SAFE_RELEASE
#define SAFE_RELEASE(PTR) \
if (PTR) \
{ \
    vm_debug_trace1(VM_DEBUG_MEMORY, VM_STRING("[RELEASE] %s"), VM_STRING(#PTR)); \
    PTR->Release(); \
    PTR = NULL; \
}

#define _PRINTABLE(_x)  ((_x < ' ') ? ' ' : _x)

#define vm_trace_fourcc(x) \
    vm_debug_trace4(VM_DEBUG_VERBOSE, \
        VM_STRING(#x) VM_STRING(" = %c%c%c%c"), \
        _PRINTABLE(((char*)&(x))[0]),  \
        _PRINTABLE(((char*)&(x))[1]),  \
        _PRINTABLE(((char*)&(x))[2]),  \
        _PRINTABLE(((char*)&(x))[3]))

typedef struct {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4_0;
    unsigned char  Data4_1;
    unsigned char  Data4_2;
    unsigned char  Data4_3;
    unsigned char  Data4_4;
    unsigned char  Data4_5;
    unsigned char  Data4_6;
    unsigned char  Data4_7;
} UMC_GUID;

void vm_trace_GUID_func(const UMC_GUID *pGUID, vm_char *descr);
Ipp32s vm_trace_hresult_func(Ipp32s hr, vm_char *mess, void *pthis, vm_char *func, vm_char *file, Ipp32u line);

/* ///////////////////// */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_DEBUG_H__ */
