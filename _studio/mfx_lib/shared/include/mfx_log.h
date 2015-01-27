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

File Name: mfx_log.h

\* ****************************************************************************** */
#if !defined(__MFX_LOG_H)
#define __MFX_LOG_H

#include <stddef.h>

// Declare the type of handle to logging object
typedef
void * log_t;

// Initialize the logging stuff. If log path is not set,
// the default log path and object is used.
extern "C"
log_t mfxLogInit(const char *pLogPath = 0);

// Write something to the log. If the log handle is zero,
// the default log file is used.
extern "C"
void mfxLogWriteA(const log_t log, const char *pString, ...);

// Close the specific log. Default log can't be closed.
extern "C"
void mfxLogClose(log_t log);

class mfxLog
{
public:
    // Default constructor
    mfxLog(void)
    {
        log = 0;
    }

    // Destructor
    ~mfxLog(void)
    {
        Close();
    }

    // Initialize the log
    log_t Init(const char *pLogPath)
    {
        // Close the object before initialization
        Close();

        log = mfxLogInit(pLogPath);

        return log;
    }

    // Initialize the log
    void Close()
    {
        mfxLogClose(log);

        log = 0;
    }

    // Handle to the protected log
    log_t log;
};

#endif // __MFX_LOG_H
