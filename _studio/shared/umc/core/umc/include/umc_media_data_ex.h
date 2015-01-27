/* ****************************************************************************** *\

Copyright (C) 2003 - 2013 Intel Corporation.  All rights reserved.

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

File Name: umc_media_data_ex.h

\* ****************************************************************************** */

#ifndef __UMC_MEDIA_DATA_EX_H__
#define __UMC_MEDIA_DATA_EX_H__

#include "umc_media_data.h"

namespace UMC
{

class MediaDataEx : public MediaData
{
    DYNAMIC_CAST_DECL(MediaDataEx, MediaData)

public:
    class _MediaDataEx{
        DYNAMIC_CAST_DECL_BASE(_MediaDataEx)
        public:
        Ipp32u count;
        Ipp32u index;
        Ipp64u bstrm_pos;
        Ipp32u *offsets;
        Ipp32u *values;
        Ipp32u limit;

        _MediaDataEx()
        {
            count = 0;
            index = 0;
            bstrm_pos = 0;
            limit   = 2000;
            offsets = (Ipp32u*)malloc(sizeof(Ipp32u)*limit);
            values  = (Ipp32u*)malloc(sizeof(Ipp32u)*limit);
        }

        virtual ~_MediaDataEx()
        {
            if(offsets)
            {
                free(offsets);
                offsets = 0;
            }
            if(values)
            {
                free(values);
                values = 0;
            }
            limit   = 0;
        }
    };

    // Default constructor
    MediaDataEx()
    {
        m_exData = NULL;
    };

    // Destructor
    virtual ~MediaDataEx(){};

    _MediaDataEx* GetExData()
    {
        return m_exData;
    };

    void SetExData(_MediaDataEx* pDataEx)
    {
        m_exData = pDataEx;
    };

protected:
    _MediaDataEx *m_exData;
};

}

#endif //__UMC_MEDIA_DATA_EX_H__

