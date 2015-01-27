/* ****************************************************************************** *\

Copyright (C) 2003-2008 Intel Corporation.  All rights reserved.

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

File Name: umc_data_writer.h

\* ****************************************************************************** */

#ifndef __UMC_DATA_WRITER_H__
#define __UMC_DATA_WRITER_H__

/*
//  Class:       DataWriter
//
//  Notes:       Base abstract class of data writer. Class describes
//               the high level interface of abstract source of data.
//               All specific ( reading from file, from network, inernet, etc ) must be implemented in
//               derevied classes.
//               Splitter uses this class to obtain data
//
*/

#include "umc_structures.h"
#include "umc_dynamic_cast.h"

namespace UMC
{

class DataWriterParams
{
    DYNAMIC_CAST_DECL_BASE(DataWriterParams)

public:
    // Default constructor
    DataWriterParams(void){}
    // Destructor
    virtual ~DataWriterParams(void){}
};

class DataWriter
{
    DYNAMIC_CAST_DECL_BASE(DataWriter)
public:
    // Default constructor
    DataWriter(void){}
    // Destructor
    virtual ~DataWriter(void){}

    // Initialization abstract destination media
    virtual Status Init(DataWriterParams *InitParams) = 0;

    // Close destination media
    virtual Status Close(void) = 0;

    // Reset all internal parameters to start writing from begin
    virtual Status Reset(void) = 0;

    // Write data to output stream
    virtual Status PutData(void *data, Ipp32s &nsize) = 0;

    // Set current position in destination media
    virtual Status SetPosition(Ipp32u /* nPosLow */,
                               Ipp32u * /* pnPosHigh */,
                               Ipp32u /* nMethod */)
    {   return UMC_ERR_FAILED;    }

    // Get current position in destination media
    virtual Status GetPosition(Ipp32u * /* pnPosLow */, Ipp32u * /* pnPosHigh */)
    {   return UMC_ERR_FAILED;    }
};

} // namespace UMC

#endif /* __UMC_DATA_WRITER_H__ */
