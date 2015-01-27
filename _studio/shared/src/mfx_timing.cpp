/* ****************************************************************************** *\

Copyright (C) 2008-2013 Intel Corporation.  All rights reserved.

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

File Name: mfx_timing.cpp

\* ****************************************************************************** */

#include "mfx_timing.h"
#include "umc_mutex.h"
#include "umc_automatic_mutex.h"

#ifdef MFX_TIMING

#include <windows.h>
#include <evntprov.h>

#pragma warning(disable:4996)
#pragma warning(disable:4311)
#pragma warning(disable:4100)

#define MFX_TIMING_ERR  (-1000)

#undef  MFX_CHECK
#define MFX_CHECK(EXPR) { if (!(EXPR)) return MFX_TIMING_ERR; }

namespace MFX
{

static const GUID MSDK_GUID_TIMING = { 0x2D6B112A, 0xD21C, 0x4a40, 0x9B, 0xF2, 0xA3, 0xED, 0xF2, 0x12, 0xF6, 0x24 };
static const GUID ETW_GUID_APP =     { 0x102562d4, 0x61c6, 0x435a, 0xac, 0x2d, 0xf0, 0x4a, 0xf2, 0xd5, 0x07, 0xa0 };

#define MFX_TIMING_START_EVENT 0x01
#define MFX_TIMING_STOP_EVENT  0x02

static UMC::Mutex   g_InitMutex;
static bool         g_Init            = false;
static int          g_InitSts         = 0;
static DWORD        g_dwTimingLevel   = 0;
static REGHANDLE    g_EventRegistered = 0;
static unsigned int g_tasksId         = 0;

static class Unregister
{
public:
    ~Unregister()
    {
        EventUnregister(g_EventRegistered);
    }
} unregistrator;

int InitOnRegistryKeys(/*int additional_flags*/);

//////////////////////////////////////////////////////////////////
// C++ AutoTimer implementation

int AutoTimer::Init(const void *filename, int level)
{
    if (level > 0 && !g_Init)
    {
        g_InitSts = EventRegister(&ETW_GUID_APP, NULL, NULL, &g_EventRegistered);
        g_Init = true;
        return g_InitSts;
    }
    return 0;
}

unsigned int AutoTimer::CreateUniqTaskId()
{
    return InterlockedIncrement(&g_tasksId);
}

void AutoTimer::SetInput(void* handle, int component_index)
{
}

void AutoTimer::SetOutput(void* handle, int component_index)
{
}

void AutoTimer::AddParam(const char *param_name, const char *param_value)
{
}

void AutoTimer::AddParam(const char *param_name, int param_value)
{
}

void AutoTimer::Reset()
{
    m_Name = 0;
    m_timerid = 0;
    m_parentid = 0;
    m_childid = 0;
}

void AutoTimer::SendNamedEvent(int opcode)
{
    if (!g_dwTimingLevel)
        return;
    //event start
    EVENT_DESCRIPTOR Descriptor = {0};
    EVENT_DATA_DESCRIPTOR EventData[4];

    //number of ids appened to event
    int iDesc = 0;

    //storing name
    EventDataDescCreate(&EventData[iDesc++], m_Name, strlen(m_Name) + 1);
    Descriptor.Opcode = (UCHAR)opcode;

    if (opcode == MFX_TIMING_START_EVENT)
    {
        unsigned int id_list[] = {m_timerid, m_parentid, m_childid};
        for (int i = 0; i<sizeof(id_list) / sizeof(id_list[0]); i++)
        {
            EventDataDescCreate(&EventData[iDesc++], &id_list[i], sizeof(id_list[i]));
        }
    }

    EventWrite(g_EventRegistered, &Descriptor, iDesc, EventData);
}

void AutoTimer::Start(const char *name)
{
    if (InitOnRegistryKeys()) return;
    m_Name = name;

    SendNamedEvent(MFX_TIMING_START_EVENT);
}

void AutoTimer::Stop(int return_code)
{
    if (InitOnRegistryKeys()) return;
    if (!m_Name) return;

    if (return_code != MFX_INVALID_STATUS_CODE)
    {
        AddParam("Return code", return_code);
    }

    // stop
    SendNamedEvent(MFX_TIMING_STOP_EVENT);

    // clear
    Reset();
}

AutoTimer::AutoTimer()
{
    Reset();
}

AutoTimer::AutoTimer(const char *name)
{
    if (InitOnRegistryKeys()) return;
    Reset();
    Start(name);

}

AutoTimer::AutoTimer( const char *name
                     , const char *param
                     , int parami)
{
    if (InitOnRegistryKeys()) return;
    Reset();
    Start(name);
}

AutoTimer::AutoTimer( const char *name
                    , bool bCreateId
                    , unsigned int child_of
                    , unsigned int parent_of)
{
    if (InitOnRegistryKeys()) return;
    Reset();
    if (bCreateId)
    {
        m_timerid = CreateUniqTaskId();
    }
    m_childid  = parent_of;
    m_parentid = child_of;

    Start(name);
}

AutoTimer::~AutoTimer()
{
    if (InitOnRegistryKeys()) return;
    Stop();
}

////////////////////////////////////////////////////////////////////////////
// Registry keys

#define REG_ROOT                    HKEY_CURRENT_USER
#define REG_PATH_MEDIASDK           TEXT("Software\\Intel\\MediaSDK")
#define REG_KEY_TIMING_LEVEL        TEXT("EtwTrace")

int InitOnRegistryKeys(/*int additional_flags*/)
{
    if (g_Init)
        return g_InitSts;

    {
        UMC::AutomaticUMCMutex guard(g_InitMutex);

        if (g_Init)
            return g_InitSts;

        HKEY hKey;
        //DWORD dwLevel = 0;
        DWORD size1 = sizeof(DWORD);

        if (ERROR_SUCCESS != RegOpenKeyEx(REG_ROOT, REG_PATH_MEDIASDK, 0, KEY_QUERY_VALUE, &hKey))
        {
            // not enough permissions
            g_InitSts = MFX_TIMING_ERR;
        }

        if (!g_InitSts && ERROR_SUCCESS != RegQueryValueEx(hKey, REG_KEY_TIMING_LEVEL, NULL, NULL, (LPBYTE)&g_dwTimingLevel, &size1))
        {
            g_InitSts = MFX_TIMING_ERR;
        }

        if (!g_InitSts && g_dwTimingLevel && ERROR_SUCCESS != EventRegister(&MSDK_GUID_TIMING, NULL, NULL, &g_EventRegistered))
        {
            g_dwTimingLevel = 0;
            g_InitSts = MFX_TIMING_ERR;
        }

        RegCloseKey(hKey);

        g_Init = true;

        return g_InitSts;
    }
}

} // namespace

#endif // MFX_TIMING
