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

File Name: umc_sys_info.h

\* ****************************************************************************** */

#ifndef __SYS_INFO_H__
#define __SYS_INFO_H__

#include <stdio.h>
#include "umc_structures.h"
#include "vm_time.h"
#include "vm_types.h"

namespace UMC
{

typedef struct sSystemInfo
{
    Ipp32u num_proc;                                           // (Ipp32u) number of processor(s)
    Ipp32u cpu_freq;                                           // (Ipp32u) CPU frequency
    vm_char os_name[_MAX_LEN];                                 // (vm_char []) OS name
    vm_char proc_name[_MAX_LEN];                               // (vm_char []) processor's name
    vm_char computer_name[_MAX_LEN];                           // (vm_char []) computer's name
    vm_char user_name[_MAX_LEN];                               // (vm_char []) user's name
    vm_char video_card[_MAX_LEN];                              // (vm_char []) video adapter's name
    vm_char program_path[_MAX_LEN];                            // (vm_char []) program path
    vm_char program_name[_MAX_LEN];                            // (vm_char []) program name
    vm_char description[_MAX_LEN];
    Ipp32u phys_mem;
} sSystemInfo;

#if ! (defined(_WIN32_WCE) || defined(__linux) || defined(__APPLE__))
/* obtain  */

typedef struct _VIRTUAL_MACHINE_COUNTERS {
    size_t        size1;
    size_t        size2;
    Ipp32u        size3;
    size_t        size4;
    size_t        size5;
    size_t        size6;
    size_t        size7;
    size_t        size8;
    size_t        size9;
    size_t        size10;
    size_t        size11;
} VIRTUAL_MACHINE_COUNTERS;

typedef struct _TASK_ID {
    Ipp32u        ProcessID;
    Ipp32u        ThreadID;
} TASK_ID;

typedef struct _ST {
    Ipp64u        TimeOfKernel;
    Ipp64u        TimeOfUser;
    Ipp64u        TimeOfCreation;
    Ipp32u        TimeOfWaiting;
    void          *BaseAddress;
    TASK_ID       TaskID;
    Ipp32s        Priority;
    Ipp32s        StartPriority;
    Ipp32u        CSCount;
    Ipp32s        State;
    Ipp32s        ReasonOfWaiting;
} ST;

typedef struct _WSTRING {
    Ipp16u        Len;
    Ipp16u        MaxLen;
    wchar_t       *Ptr;
} WSTRING;

typedef struct _SYSTEM_PROCESSES {
    Ipp32u             OffsetOfNextEntry;
    Ipp32u             Reserved1;
    Ipp32u             Reserved2[6];
    Ipp64u             Reserved3;
    Ipp64u             APP_Time;
    Ipp64u             OS_Time;
    WSTRING            Application_Name;
    Ipp32s             Start_priority;
    Ipp32u             PID;
    Ipp32u             Reserved4;
    Ipp32u             Reserved5;
    Ipp32u             Reserved6[2];
    VIRTUAL_MACHINE_COUNTERS       Reserved7;
#if _WIN32_WINNT >= 0x500
    IO_COUNTERS        Reserved8;
#endif
    ST                 Reserved9[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;

typedef Ipp32s    COUNTERS_STATUS;
#define IF_SUCCESS(Status) ((COUNTERS_STATUS)(Status) >= 0)
#define NEXT_AVAILABLE     ((COUNTERS_STATUS)0xC0000004L)

#endif // if !(defined(_WIN32_WCE) || defined(__linux) || defined(__APPLE__))

typedef Ipp32u (*FuncGetMemUsage)();

class SysInfo
{
public:
    // Default constructor
    SysInfo(vm_char *pProcessName = NULL);
    // Destructor
    virtual ~SysInfo(void);

    // Get system information
    sSystemInfo *GetSysInfo(void);

    // CPU usage
    Ipp64f GetCpuUsage(void);
    Ipp64f GetAvgCpuUsage(void) {return avg_cpuusage;};
    Ipp64f GetMaxCpuUsage(void) {return max_cpuusage;};
    void CpuUsageRelease(void);

    // Memory usage
    void   SetFuncGetMemUsage(FuncGetMemUsage pFunc) { m_FuncGetMemUsage = pFunc; }
    Ipp64f GetMemUsage(void);
    Ipp64f GetAvgMemUsage(void);
    Ipp64f GetMaxMemUsage(void);
    void   ResetMemUsage(void);

protected:
    // system info struct
    sSystemInfo m_sSystemInfo;

    // CPU usage
    vm_tick user_time;
    vm_tick total_time;
    vm_tick user_time_start;
    vm_tick total_time_start;
    Ipp64f last_cpuusage;
    Ipp64f max_cpuusage;
    Ipp64f avg_cpuusage;

    // memory usage
    FuncGetMemUsage m_FuncGetMemUsage;
    Ipp64f  m_MemoryMax;
    Ipp64f  m_MemorySum;
    Ipp32s  m_MemoryCount;
};

} // namespace UMC

#endif // __SYS_INFO_H__
