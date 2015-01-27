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

File Name: umc_sys_info.cpp

\* ****************************************************************************** */

#include "umc_sys_info.h"
#include "vm_sys_info.h"
#include "vm_debug.h"
#include <string.h>

#if defined(LINUX32)
#include <sys/resource.h>
#include <unistd.h>

#define TIME_MICRO 1000000
#define TIMEVAL_TO_TICK(tv) \
    (vm_tick)((vm_tick)(tv).tv_sec * (vm_tick)TIME_MICRO + (vm_tick)(tv).tv_usec);
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif // if defined(_WIN32) || defined(_WIN64)

namespace UMC
{

// platform-depended function for cpu usage measurement (there is alternative implementation in vm)
static void GetCpuUseTime(vm_char*, vm_tick* process_use, vm_tick* total_use);

SysInfo::SysInfo(vm_char *pProcessName)
{
    memset(&m_sSystemInfo,0,sizeof(sSystemInfo));
    if (!pProcessName)
    {
        vm_sys_info_get_program_name(m_sSystemInfo.program_name);
    } else
    {
#if defined(_WIN32) || defined(_WIN64)
        vm_string_strcpy_s(m_sSystemInfo.program_name,
                           sizeof(m_sSystemInfo.program_name) / sizeof(m_sSystemInfo.program_name[0]),
                           pProcessName);
#endif
#if defined(LINUX32)
        vm_string_strcpy(m_sSystemInfo.program_name, pProcessName);
#endif
    }
    vm_sys_info_get_program_path(m_sSystemInfo.program_path);
    vm_sys_info_get_os_name(m_sSystemInfo.os_name);
    vm_sys_info_get_cpu_name(m_sSystemInfo.proc_name);
    vm_sys_info_get_vga_card(m_sSystemInfo.video_card);
    vm_sys_info_get_computer_name(m_sSystemInfo.computer_name);
#if defined(LINUX32)
    { // obtaining user name on Linux system
        vm_char* username = getenv("LOGNAME");
        if (username && (vm_string_strlen(username) < _MAX_LEN))
        {
            vm_string_strcpy(m_sSystemInfo.user_name, username);
        }
        else m_sSystemInfo.user_name[0] = 0;
    }
#endif
    m_sSystemInfo.cpu_freq = vm_sys_info_get_cpu_speed();
    m_sSystemInfo.phys_mem = vm_sys_info_get_mem_size();
    m_sSystemInfo.num_proc = vm_sys_info_get_cpu_num();

    CpuUsageRelease();
    ResetMemUsage();
    m_FuncGetMemUsage = NULL;
} // SysInfo::SysInfo(void)

SysInfo::~SysInfo(void)
{
} // SysInfo::~SysInfo(void)

void SysInfo::ResetMemUsage(void)
{
    m_MemoryMax = 0;
    m_MemorySum = 0;
    m_MemoryCount = 0;
}

Ipp64f SysInfo::GetMemUsage(void)
{
    if (!m_FuncGetMemUsage) return 0;
    Ipp64f mem_usage = m_FuncGetMemUsage()/(1024.0*1024.0);
    m_MemorySum += mem_usage;
    m_MemoryCount++;
    if (mem_usage > m_MemoryMax) m_MemoryMax = mem_usage;
    return mem_usage;
}

Ipp64f SysInfo::GetAvgMemUsage(void)
{
    return (m_MemoryCount) ? m_MemorySum/m_MemoryCount : 0;
}

Ipp64f SysInfo::GetMaxMemUsage(void)
{
    return m_MemoryMax;
}

Ipp64f SysInfo::GetCpuUsage(void)
{
    Ipp64f cpu_use = 0;
    vm_tick user_time_cur = 0;
    vm_tick total_time_cur = 0;
    GetCpuUseTime(m_sSystemInfo.program_name, &user_time_cur, &total_time_cur);
    if (!user_time_cur)
    {
        return cpu_use;
    }
    if ((user_time) && (total_time)) {
        Ipp64f dUserTime = (Ipp64f)(Ipp64s)((user_time_cur - user_time));
        Ipp64f dTotalTime = (Ipp64f)(Ipp64s)((total_time_cur - total_time));
        Ipp64f dUserTimeAvg = (Ipp64f)(Ipp64s)((user_time_cur - user_time_start));
        Ipp64f dTotalTimeAvg = (Ipp64f)(Ipp64s)((total_time_cur - total_time_start));
#if defined(_WIN32_WCE)
        cpu_use = 100 - ((dUserTime) / (dTotalTime)) * 100;
        avg_cpuusage = 100 - ((dUserTimeAvg) / (dTotalTimeAvg)) * 100;
#else
        cpu_use = ((dUserTime) / (dTotalTime)) * 100;
        avg_cpuusage = ((dUserTimeAvg) / (dTotalTimeAvg)) * 100;
#endif
     }
     else {
         user_time_start = user_time_cur;
         total_time_start = total_time_cur;
     }
     if ((cpu_use >= 0) && (cpu_use <= 100))
         last_cpuusage = cpu_use;
     else
         cpu_use = last_cpuusage;
     if (cpu_use > max_cpuusage)
         max_cpuusage = cpu_use;
    user_time = user_time_cur;
    total_time = total_time_cur;
    return cpu_use;
}

sSystemInfo *SysInfo::GetSysInfo(void)
{
    return &m_sSystemInfo;

} // sSystemInfo *SysInfo::GetSysInfo(void)

void GetCpuUseTime(vm_char* proc_name, vm_tick* process_use, vm_tick* total_use)
{
#if (defined(_WIN32) || defined(_WIN64)) || defined(_WIN32_WCE)
#if (defined(_WIN32_WCE))
    *process_use = GetIdleTime();
    *total_use = GetTickCount();
#else //(defined(_WIN32_WCE))
    Ipp32s Status;
    PSYSTEM_PROCESSES pProcesses;
    HINSTANCE hNtDll;
    HANDLE hHeap = GetProcessHeap();
    ULONG cbBuffer = 0x8000;
    PVOID pBuffer = NULL;

    Ipp32s (WINAPI * _ZwQuerySystemInformation)(UINT, PVOID, ULONG, PULONG);

    // get handle NTDLL.DLL
    hNtDll = GetModuleHandle(_T("ntdll.dll"));
    if (NULL == hNtDll)
        return;

    // find  address ZwQuerySystemInformation
    *(FARPROC *)&_ZwQuerySystemInformation =
        GetProcAddress(hNtDll, "ZwQuerySystemInformation");
    if (_ZwQuerySystemInformation == NULL)
        return; //SetLastError(ERROR_PROC_NOT_FOUND), FALSE;
    do
    {
        pBuffer = HeapAlloc(hHeap, 0, cbBuffer);
        if (pBuffer == NULL)
            return; //SetLastError(ERROR_NOT_ENOUGH_MEMORY), FALSE;

        Status = _ZwQuerySystemInformation(5, pBuffer, cbBuffer, NULL);

        if (Status == NEXT_AVAILABLE)
        {
            HeapFree(hHeap, 0, pBuffer);
            cbBuffer *= 2;
        }
        else if (!IF_SUCCESS(Status))
        {
            HeapFree(hHeap, 0, pBuffer);
            return; //SetLastError(Status), FALSE;
        }
    }
    while (Status == NEXT_AVAILABLE);

    pProcesses = (PSYSTEM_PROCESSES)pBuffer;

    for (;;)
    {
        PCWSTR pszProcessName = pProcesses->Application_Name.Ptr;
        if (pszProcessName == NULL)
            pszProcessName = L"Idle";
        const vm_char *pProcessName;

#ifdef UNICODE
        pProcessName = pszProcessName;
#else
        vm_char szProcessName[MAX_PATH];

        WideCharToMultiByte(CP_ACP, 0, pszProcessName, -1,
            szProcessName, MAX_PATH, NULL, NULL);
        pProcessName = szProcessName;
#endif

        if (!vm_string_strncmp(pProcessName, proc_name, 15)) {
            *process_use += pProcesses->APP_Time;
            *process_use += pProcesses->OS_Time;
        }

        *total_use += pProcesses->APP_Time;
        *total_use += pProcesses->OS_Time;

        if (pProcesses->OffsetOfNextEntry == 0)
            break;

        // find the address of the next process structure
        pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses)
            + pProcesses->OffsetOfNextEntry);
    }

    HeapFree(hHeap, 0, pBuffer);
#endif //(defined(_WIN32_WCE))
#elif defined(LINUX32)
    struct rusage ru;

    ippsZero_8u((Ipp8u*)&ru, sizeof(rusage));
    getrusage(RUSAGE_SELF, &ru); // values for process
    *process_use  = TIMEVAL_TO_TICK(ru.ru_utime);
    *process_use += TIMEVAL_TO_TICK(ru.ru_stime);

    ippsZero_8u((Ipp8u*)&ru, sizeof(rusage));
    getrusage(RUSAGE_CHILDREN, &ru); // values for dead child processes
    *process_use += TIMEVAL_TO_TICK(ru.ru_utime);
    *process_use += TIMEVAL_TO_TICK(ru.ru_stime);
// NOTE: enother way is to use clock function, but this will be incorrect for long tasks:
//    *process_use = (vm_tick)(clock() + 1);
    // approximation
    *process_use = (vm_tick)(((*process_use)-(*process_use)%100000)+1);
    *total_use   = vm_time_get_tick();
#endif //#if (defined(_WIN32) || defined(_WIN64)) && !defined(_WIN32_WCE)
}/* GetCpuUseTime */

void SysInfo::CpuUsageRelease(void)
{
    user_time = 0;
    max_cpuusage = 0;
    avg_cpuusage = 0;
    total_time = 0;
    last_cpuusage = 0;
}

} // namespace UMC
