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

File Name: vm_sys_info.h

\* ****************************************************************************** */

#ifndef __VM_SYS_INFO_H__
#define __VM_SYS_INFO_H__


#include "vm_types.h"
#include "vm_strings.h"

#undef UMC_VERSION_INFO
#ifdef UMC_VERSION_INFO
/*
 * umc_version.gen file contains
 *     vm_char *umc_version_string = "Current UMC version";
 *     Ipp64u   umc_version_number = 0xVERSION_NUMBER;
 */
#  include "umc_version.gen"
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Processor's feature bits */
enum
{
    CMOV_FEATURE                = 0x00001,
    MMX_FEATURE                 = 0x00002,
    MMX_EXT_FEATURE             = 0x00004,
    SSE_FEATURE                 = 0x00008,
    SSE2_FEATURE                = 0x00010,
    SSE3_FEATURE                = 0x00020,

    UNK_MANUFACTURE             = 0x00000,
    INTEL_MANUFACTURE           = 0x10000
};

/* Processors ID list */
enum
{
    UNKNOWN_CPU                 = 0,

    PENTIUM                     = INTEL_MANUFACTURE,
    PENTIUM_PRO                 = PENTIUM | CMOV_FEATURE,
    PENTIUM_MMX                 = PENTIUM | MMX_FEATURE,
    PENTIUM_2                   = PENTIUM_MMX | CMOV_FEATURE,
    PENTIUM_3                   = PENTIUM_2 | MMX_EXT_FEATURE | SSE_FEATURE,
    PENTIUM_4                   = PENTIUM_3 | SSE2_FEATURE,
    PENTIUM_4_PRESCOTT          = PENTIUM_4 | SSE3_FEATURE
};

typedef enum
{
    DDMMYY = 0,
    MMDDYY = 1,
    YYMMDD = 2
} DateFormat;

typedef enum
{
    HHMMSS      = 0,
    HHMM        = 1,
    HHMMSSMS1   = 2,
    HHMMSSMS2   = 3,
    HHMMSSMS3   = 4
} TimeFormat;

#ifdef UMC_VERSION_INFO
    /* Functions to obtain UMC version information */
    vm_char *vm_get_version_string( void );
    Ipp64u vm_get_version_number( void );
#endif

/* Functions to obtain processor's specific information */
Ipp32u vm_sys_info_get_cpu_num(void);
void vm_sys_info_get_cpu_name(vm_char *cpu_name);
void vm_sys_info_get_date(vm_char *m_date, DateFormat df);
void vm_sys_info_get_time(vm_char *m_time, TimeFormat tf);
void vm_sys_info_get_vga_card(vm_char *vga_card);
void vm_sys_info_get_os_name(vm_char *os_name);
void vm_sys_info_get_computer_name(vm_char *computer_name);
void vm_sys_info_get_program_name(vm_char *program_name);
void vm_sys_info_get_program_path(vm_char *program_path);
void vm_sys_info_get_program_description(vm_char *program_description);
Ipp32u vm_sys_info_get_cpu_speed(void);
Ipp32u vm_sys_info_get_mem_size(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VM_SYS_INFO_H__ */
