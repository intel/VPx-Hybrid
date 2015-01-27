/* ****************************************************************************** *\

Copyright (C) 2010-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_trace.h

\* ****************************************************************************** */

#ifndef __MFX_TRACE_H__
#define __MFX_TRACE_H__

#ifndef MFX_TRACE_DISABLE
// Uncomment one or several lines below to enable tracing
#if defined(_WIN32) || defined(_WIN64)
#define MFX_TRACE_ENABLE_ETW
#endif // #if defined(_WIN32) || defined(_WIN64)

//#define MFX_TRACE_ENABLE_ITT
//#define MFX_TRACE_ENABLE_TEXTLOG
//#define MFX_TRACE_ENABLE_STAT
//#define MFX_TRACE_ENABLE_TAL

#if defined(MFX_TRACE_ENABLE_TEXTLOG) || defined(MFX_TRACE_ENABLE_STAT) || defined(MFX_TRACE_ENABLE_ETW) || defined(MFX_TRACE_ENABLE_ITT)
#define MFX_TRACE_ENABLE
#endif
#endif // #ifndef MFX_TRACE_DISABLE

#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <tchar.h>

    #define __INT64   __int64
    #define __UINT64  unsigned __int64

    typedef TCHAR mfxTraceChar;

    #define MFX_TRACE_STRING(x) _T(x)
#else
    #define MAX_PATH 260

    #define __INT64   long long
    #define __UINT64  unsigned long long

    typedef char mfxTraceChar;

    #define MFX_TRACE_STRING(x) x
#endif // #if defined(_WIN32) || defined(_WIN64)

typedef unsigned int mfxTraceU32;
typedef __UINT64 mfxTraceU64;

/*------------------------------------------------------------------------------*/
// C section

#ifdef __cplusplus
extern "C"
{
#endif

// list of output modes
enum
{
    MFX_TRACE_OUTPUT_TRASH  = 0x00,
    MFX_TRACE_OUTPUT_TEXTLOG = 0x01,
    MFX_TRACE_OUTPUT_STAT   = 0x02,
    MFX_TRACE_OUTPUT_ETW    = 0x04,
    MFX_TRACE_OUTPUT_TAL    = 0x08,
    MFX_TRACE_OUTPUT_ITT    = 0x10,
    // special keys
    MFX_TRACE_OUTPUT_ALL     = 0xFFFFFFFF,
    MFX_TRACE_OUTPUT_REG     = MFX_TRACE_OUTPUT_ALL // output mode should be read from registry
};

// enumeration of the trace levels inside any category
typedef enum
{
    MFX_TRACE_LEVEL_0 = 0,
    MFX_TRACE_LEVEL_1 = 1,
    MFX_TRACE_LEVEL_2 = 2,
    MFX_TRACE_LEVEL_3 = 3,
    MFX_TRACE_LEVEL_4 = 4,
    MFX_TRACE_LEVEL_5 = 5,
    MFX_TRACE_LEVEL_6 = 6,
    MFX_TRACE_LEVEL_7 = 7,
    MFX_TRACE_LEVEL_8 = 8,
    MFX_TRACE_LEVEL_9 = 9,
    MFX_TRACE_LEVEL_10 = 10,
    MFX_TRACE_LEVEL_11 = 11,
    MFX_TRACE_LEVEL_12 = 12,
    MFX_TRACE_LEVEL_13 = 13,
    MFX_TRACE_LEVEL_14 = 14,
    MFX_TRACE_LEVEL_15 = 15,
    MFX_TRACE_LEVEL_16 = 16,

    MFX_TRACE_LEVEL_MAX = 0xFF
} mfxTraceLevel;

#define MFX_TRACE_LEVEL_API         MFX_TRACE_LEVEL_1
#define MFX_TRACE_LEVEL_SCHED       MFX_TRACE_LEVEL_2
#define MFX_TRACE_LEVEL_SCHED2      MFX_TRACE_LEVEL_3
#define MFX_TRACE_LEVEL_INTERNAL    MFX_TRACE_LEVEL_4
#define MFX_TRACE_LEVEL_DXVA        MFX_TRACE_LEVEL_5
#define MFX_TRACE_LEVEL_PARAMS      MFX_TRACE_LEVEL_8
#define MFX_TRACE_LEVEL_PRIVATE     MFX_TRACE_LEVEL_16

// defines default trace category
#define MFX_TRACE_CATEGORY_DEFAULT  NULL

// defines category for the current module
#ifndef MFX_TRACE_CATEGORY
    #define MFX_TRACE_CATEGORY      MFX_TRACE_CATEGORY_DEFAULT
#endif

// defines default trace level
#define MFX_TRACE_LEVEL_DEFAULT     MFX_TRACE_LEVEL_MAX

// defines default trace level for the current module
#ifndef MFX_TRACE_LEVEL
    #define MFX_TRACE_LEVEL         MFX_TRACE_LEVEL_DEFAULT
#endif

/*------------------------------------------------------------------------------*/

#ifdef MFX_TRACE_ENABLE

typedef union
{
    unsigned int  uint32;
    __UINT64      uint64;
    __INT64       tick;
    char*         str;
    void*         ptr;
    mfxTraceChar* category;
    mfxTraceLevel level;
} mfxTraceHandle;

typedef struct
{
    mfxTraceChar* category;
    mfxTraceLevel level;
    // reserved for stat dump:
    mfxTraceHandle sd1;
    mfxTraceHandle sd2;
    mfxTraceHandle sd3;
    mfxTraceHandle sd4;
    mfxTraceHandle sd5;
    mfxTraceHandle sd6;
    mfxTraceHandle sd7;
} mfxTraceStaticHandle;

typedef struct
{
    // reserved for file dump:
    mfxTraceHandle fd1;
    mfxTraceHandle fd2;
    mfxTraceHandle fd3;
    mfxTraceHandle fd4;
    // reserved for stat dump:
    mfxTraceHandle sd1;
    // reserved for TAL:
    mfxTraceHandle tal1;
    // reserved for ETW:
    mfxTraceHandle etw1;
    mfxTraceHandle etw2;
} mfxTraceTaskHandle;

/*------------------------------------------------------------------------------*/
// basic trace functions (macroses are recommended to use instead)

mfxTraceU32 MFXTrace_Init(const mfxTraceChar *filename, mfxTraceU32 output_mode);

mfxTraceU32 MFXTrace_Close(void);

mfxTraceU32 MFXTrace_SetLevel(mfxTraceChar* category,
                              mfxTraceLevel level);

mfxTraceU32 MFXTrace_DebugMessage(mfxTraceStaticHandle *static_handle,
                             const char *file_name, mfxTraceU32 line_num,
                             const char *function_name,
                             mfxTraceChar* category, mfxTraceLevel level,
                             const char *message,
                             const char *format, ...);

mfxTraceU32 MFXTrace_vDebugMessage(mfxTraceStaticHandle *static_handle,
                              const char *file_name, mfxTraceU32 line_num,
                              const char *function_name,
                              mfxTraceChar* category, mfxTraceLevel level,
                              const char *message,
                              const char *format, va_list args);

mfxTraceU32 MFXTrace_BeginTask(mfxTraceStaticHandle *static_handle,
                          const char *file_name, mfxTraceU32 line_num,
                          const char *function_name,
                          mfxTraceChar* category, mfxTraceLevel level,
                          const char *task_name, mfxTraceTaskHandle *task_handle,
                          const void *task_params);

mfxTraceU32 MFXTrace_EndTask(mfxTraceStaticHandle *static_handle,
                             mfxTraceTaskHandle *task_handle);

/*------------------------------------------------------------------------------*/
// basic macroses

#define MFX_TRACE_PARAMS \
    &_trace_static_handle, __FILE__, __LINE__, __FUNCTION__, MFX_TRACE_CATEGORY

#define MFX_TRACE_INIT(_filename, _output_mode) \
    MFXTrace_Init(_filename, _output_mode);

#define MFX_TRACE_INIT_RES(_res, _filename, _output_mode) \
    _res = MFXTrace_Init(_filename, _output_mode);

#define MFX_TRACE_CLOSE() \
    MFXTrace_Close();

#define MFX_TRACE_CLOSE_RES(_res, _filename, _output_mode) \
    _res = MFXTrace_Close();

#define MFX_LTRACE(_trace_all_params)                       \
{                                                           \
    static mfxTraceStaticHandle _trace_static_handle = {0}; \
    MFXTrace_DebugMessage _trace_all_params;                \
}
#else
#define MFX_TRACE_INIT(_filename, _output_mode)
#define MFX_TRACE_INIT_RES(res, _filename, _output_mode)
#define MFX_TRACE_CLOSE()
#define MFX_TRACE_CLOSE_RES(res, _filename, _output_mode)
#define MFX_LTRACE(_trace_all_params)
#endif

/*------------------------------------------------------------------------------*/
// standard formats

#define MFX_TRACE_FORMAT_S    "%s"
#define MFX_TRACE_FORMAT_WS   "%S"
#define MFX_TRACE_FORMAT_P    "%p"
#define MFX_TRACE_FORMAT_I    "%d"
#define MFX_TRACE_FORMAT_X    "%x"
#define MFX_TRACE_FORMAT_D    "%d (0x%x)"
#define MFX_TRACE_FORMAT_F    "%g"
#define MFX_TRACE_FORMAT_GUID "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"

/*------------------------------------------------------------------------------*/
// these macroses permit to set trace level

#define MFX_LTRACE_1(_level, _message, _format, _arg1) \
    MFX_LTRACE((MFX_TRACE_PARAMS, _level, _message, _format, _arg1))

#define MFX_LTRACE_2(_level, _message, _format, _arg1, _arg2) \
    MFX_LTRACE((MFX_TRACE_PARAMS, _level, _message, _format, _arg1, _arg2))

#define MFX_LTRACE_MSG(_level, _message) \
    MFX_LTRACE_1(_level, _message, NULL, 0)

#define MFX_LTRACE_S(_level, _string) \
    MFX_LTRACE_1(_level, #_string " = ", MFX_TRACE_FORMAT_S, _string)

#define MFX_LTRACE_WS(_level, _string) \
    MFX_LTRACE_1(_level, #_string " = ", MFX_TRACE_FORMAT_WS, _string)

#define MFX_LTRACE_P(_level, _arg1) \
    MFX_LTRACE_1(_level, #_arg1 " = ", MFX_TRACE_FORMAT_P, _arg1)

#define MFX_LTRACE_I(_level, _arg1) \
    MFX_LTRACE_1(_level, #_arg1 " = ", MFX_TRACE_FORMAT_I, _arg1)

#define MFX_LTRACE_X(_level, _arg1) \
    MFX_LTRACE_1(_level, #_arg1 " = ", MFX_TRACE_FORMAT_X, _arg1)

#define MFX_LTRACE_D(_level, _arg1) \
    MFX_LTRACE_2(_level, #_arg1 " = ", MFX_TRACE_FORMAT_D, _arg1, _arg1)

#define MFX_LTRACE_F(_level, _arg1) \
    MFX_LTRACE_1(_level, #_arg1 " = ", MFX_TRACE_FORMAT_F, _arg1)

#ifdef MFX_TRACE_ENABLE
#define MFX_LTRACE_BUFFER_S(_level, _name, _buffer, _size)  \
    if (_buffer)                                            \
    MFX_LTRACE_2(_level, _name, "%p[%d]", _buffer, _size)
#else
#define MFX_LTRACE_BUFFER_S(_level, _name, _buffer, _size)
#endif

#define MFX_LTRACE_BUFFER(_level, _buffer) \
    MFX_LTRACE_BUFFER_S(_level, #_buffer, _buffer, sizeof(*_buffer)) \

#define MFX_LTRACE_GUID(_level, _guid) \
    MFX_LTRACE((MFX_TRACE_PARAMS, _level, #_guid " = ", \
               MFX_TRACE_FORMAT_GUID, \
               (_guid).Data1, (_guid).Data2, (_guid).Data3, \
               (_guid).Data4[0], (_guid).Data4[1], (_guid).Data4[2], (_guid).Data4[3], \
               (_guid).Data4[4], (_guid).Data4[5], (_guid).Data4[6], (_guid).Data4[7]))

/*------------------------------------------------------------------------------*/
// these macroses uses default trace level

#define MFX_TRACE_1(_message, _format, _arg1) \
    MFX_LTRACE_1(MFX_TRACE_LEVEL, _message, _format, _arg1)

#define MFX_TRACE_2(_message, _format, _arg1, _arg2) \
    MFX_LTRACE_2(MFX_TRACE_LEVEL, _message, _format, _arg1, _arg2)

#define MFX_TRACE_S(_arg1) \
    MFX_LTRACE_S(MFX_TRACE_LEVEL, _arg1)

#define MFX_TRACE_WS(_message) \
    MFX_LTRACE_WS(MFX_TRACE_LEVEL, _message)

#define MFX_TRACE_P(_arg1) \
    MFX_LTRACE_P(MFX_TRACE_LEVEL, _arg1)

#define MFX_TRACE_I(_arg1) \
    MFX_LTRACE_I(MFX_TRACE_LEVEL, _arg1)

#define MFX_TRACE_X(_arg1) \
    MFX_LTRACE_X(MFX_TRACE_LEVEL, _arg1)

#define MFX_TRACE_D(_arg1) \
    MFX_LTRACE_D(MFX_TRACE_LEVEL, _arg1)

#define MFX_TRACE_F(_arg1) \
    MFX_LTRACE_F(MFX_TRACE_LEVEL, _arg1)

#define MFX_TRACE_GUID(_guid) \
    MFX_LTRACE_GUID(MFX_TRACE_LEVEL, _guid)

#define MFX_TRACE_BUFFER(_name, _buffer, _size) \
    MFX_LTRACE_BUFFER(MFX_TRACE_LEVEL, _name, _buffer, _size)

/*------------------------------------------------------------------------------*/

#ifdef __cplusplus
} // extern "C"
#endif

/*------------------------------------------------------------------------------*/
// C++ section

#ifdef __cplusplus

#ifdef MFX_TRACE_ENABLE
// C++ class for BeginTask/EndTask
class MFXTraceTask
{
public:
    MFXTraceTask(mfxTraceStaticHandle *static_handle,
                 const char *file_name, mfxTraceU32 line_num,
                 const char *function_name,
                 mfxTraceChar* category, mfxTraceLevel level,
                 const char *task_name,
                 const bool bCreateID = false);
    mfxTraceU32 GetID();
    void        Stop();
    ~MFXTraceTask();

private:
    bool                    m_bStarted;
    mfxTraceU32             m_TaskID;
    mfxTraceStaticHandle    *m_pStaticHandle;
    mfxTraceTaskHandle      m_TraceTaskHandle;
};
#endif // #ifdef MFX_TRACE_ENABLE

/*------------------------------------------------------------------------------*/
// auto tracing of the functions

#ifdef MFX_TRACE_ENABLE
    #define _MFX_AUTO_LTRACE_(_level, _task_name, _bCreateID)       \
        static mfxTraceStaticHandle _trace_static_handle;           \
        MFXTraceTask                _mfx_trace_task(MFX_TRACE_PARAMS, _level, _task_name, _bCreateID);
    #define MFX_AUTO_TRACE_STOP()   _mfx_trace_task.Stop()
    #define MFX_AUTO_TRACE_GETID()  _mfx_trace_task.GetID()
#else
    #define _MFX_AUTO_LTRACE_(_level, _task_name, _bCreateID)
    #define MFX_AUTO_TRACE_STOP()
    #define MFX_AUTO_TRACE_GETID()  0
#endif

#define MFX_AUTO_LTRACE(_level, _task_name)     \
    _MFX_AUTO_LTRACE_(_level, _task_name, false)

#define MFX_AUTO_TRACE(_task_name) \
    _MFX_AUTO_LTRACE_(MFX_TRACE_LEVEL, _task_name, false)

#define MFX_AUTO_TRACE_FUNC() \
    _MFX_AUTO_LTRACE_(MFX_TRACE_LEVEL, NULL, false)

#define MFX_AUTO_LTRACE_FUNC(_level) \
    _MFX_AUTO_LTRACE_(_level, NULL, false)

#define MFX_AUTO_LTRACE_WITHID(_level, _task_name)     \
    _MFX_AUTO_LTRACE_(_level, _task_name, true)

#define MFX_AUTO_TRACE_WITHID(_task_name) \
    _MFX_AUTO_LTRACE_(MFX_TRACE_LEVEL, _task_name, true)

#endif // ifdef __cplusplus

#endif // #ifndef __MFX_TRACE_H__
