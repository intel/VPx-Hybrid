/* ****************************************************************************** *\

Copyright (C) 2011-2014 Intel Corporation.  All rights reserved.

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

File Name: libmfx_core_interface.h

\* ****************************************************************************** */

#ifndef __LIBMFX_CORE_INTERFACE_H__
#define __LIBMFX_CORE_INTERFACE_H__

#include "mfx_common.h"
#include <mfxvideo++int.h>


// {1F5BB140-6BB4-416e-81FF-4A8C030FBDC6}
static const 
MFX_GUID  MFXIVideoCORE_GUID = 
{ 0x1f5bb140, 0x6bb4, 0x416e, { 0x81, 0xff, 0x4a, 0x8c, 0x3, 0xf, 0xbd, 0xc6 } };

// {3F3C724E-E7DC-469a-A062-B6A23102F7D2}
static const 
MFX_GUID  MFXICORED3D_GUID = 
{ 0x3f3c724e, 0xe7dc, 0x469a, { 0xa0, 0x62, 0xb6, 0xa2, 0x31, 0x2, 0xf7, 0xd2 } };

// {C9613F63-3EA3-4D8C-8B5C-96AF6DC2DB0F}
static const MFX_GUID  MFXICORED3D11_GUID = 
{ 0xc9613f63, 0x3ea3, 0x4d8c, { 0x8b, 0x5c, 0x96, 0xaf, 0x6d, 0xc2, 0xdb, 0xf } };

// {B0FCB183-1A6D-4f00-8BAF-93F285ACEC93}
static const MFX_GUID MFXICOREVAAPI_GUID = 
{ 0xb0fcb183, 0x1a6d, 0x4f00, { 0x8b, 0xaf, 0x93, 0xf2, 0x85, 0xac, 0xec, 0x93 } };

// {86dc1aab-eb20-47a2-a461-428a7bd60183}
static const MFX_GUID MFXICOREVDAAPI_GUID =
{ 0x86dc1aab, 0xeb20, 0x47a2, {0xa4, 0x61, 0x42, 0x8a, 0x7b, 0xd6, 0x01, 0x83 } };

// {6ED94B99-DB70-4EBB-BC5C-C7E348FC2396}
static const 
MFX_GUID  MFXIHWCAPS_GUID = 
{ 0x6ed94b99, 0xdb70, 0x4ebb, {0xbc, 0x5c, 0xc7, 0xe3, 0x48, 0xfc, 0x23, 0x96 } };

// {0CF4CE38-EA46-456d-A179-8A026AE4E101}
static const 
MFX_GUID  MFXIHWMBPROCRATE_GUID = 
{0xcf4ce38, 0xea46, 0x456d, {0xa1, 0x79, 0x8a, 0x2, 0x6a, 0xe4, 0xe1, 0x1} };

// {6A0665ED-2DE0-403B-A5EE-944E5AAFA8E5}
static const
MFX_GUID  MFXID3D11DECODER_GUID = 
{0x6a0665ed, 0x2de0, 0x403b, {0xa5, 0xee, 0x94, 0x4e, 0x5a, 0xaf, 0xa8, 0xe5} };

static const
MFX_GUID MFXICORECM_GUID = 
{0xe0b78bba, 0x39d9, 0x48dc,{ 0x99, 0x29, 0xc5, 0xd6, 0x5e, 0xa, 0x6a, 0x66} };




// Try to obtain required interface
// Declare a template to query an interface
template <class T> inline
T *QueryCoreInterface(VideoCORE* pUnk, const MFX_GUID &guid = T::getGuid())
{
    void *pInterface = NULL;

    // query the interface
    if (pUnk)
    {
        pInterface = pUnk->QueryCoreInterface(guid);
        // cast pointer returned to the required core interface
        return reinterpret_cast<T *>(pInterface);
    }
    else
    {
        return NULL;
    }

} // T *QueryCoreInterface(MFXIUnknown *pUnk, const MFX_GUID &guid)

class EncodeHWCaps
{
public:
    static const MFX_GUID getGuid()
    {
        return MFXIHWCAPS_GUID;
    }
    EncodeHWCaps()
    {
        m_caps = NULL;
        m_size = 0;
    };
    virtual ~EncodeHWCaps()
    {
        if (m_caps)
        {
            free(m_caps);
            m_caps = NULL;
        }
    };
    template <class CAPS> 
    mfxStatus GetHWCaps(GUID encode_guid, CAPS *hwCaps, mfxU32 array_size = 1)
    {
        if (m_caps)
        {
            if (encode_guid == m_encode_guid && m_size == array_size)
            {
                memcpy_s(hwCaps, sizeof(CAPS)*array_size, m_caps, sizeof(CAPS)*array_size);
                return MFX_ERR_NONE;
            }
        }
        return MFX_ERR_UNDEFINED_BEHAVIOR;
            
    }
    template <class CAPS> 
    mfxStatus SetHWCaps(GUID encode_guid, CAPS *hwCaps, mfxU32 array_size = 1)
    {
        if (!m_caps)
        {
            m_encode_guid = encode_guid;
            m_size = array_size;
            m_caps = malloc(sizeof(CAPS)*array_size);
            memcpy_s(m_caps, sizeof(CAPS)*array_size, hwCaps, sizeof(CAPS)*array_size);
            return MFX_ERR_NONE;
        }
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    }
protected:
    GUID   m_encode_guid;
    mfxHDL m_caps;
    mfxU32 m_size;

};

template <class T>
class ComPtrCore
{
public: 
    static const MFX_GUID getGuid()
    {
        return MFXID3D11DECODER_GUID;
    }
    ComPtrCore():m_pComPtr(NULL)
    {
    };
    virtual ~ComPtrCore()
    {
        if (m_pComPtr)
        {
            m_pComPtr->Release();
            m_pComPtr = NULL;
        }
    };
    ComPtrCore& operator = (T* ptr)
    {
        if (m_pComPtr != ptr)
        {
            if (m_pComPtr)
                m_pComPtr->Release();

            m_pComPtr = ptr;
        }
        return *this;
    };
    T* get()
    {
        return m_pComPtr;
    };

protected:
T* m_pComPtr;
};

#if defined (MFX_VA_WIN)

struct IDirectXVideoDecoderService;
struct IDirect3DDeviceManager9;

// to obtaion D3D services from the Core
struct D3D9Interface
{
    static const MFX_GUID & getGuid()
    {
        return MFXICORED3D_GUID;
    }

    virtual mfxStatus              GetD3DService(mfxU16 width,
                                                 mfxU16 height,
                                                 IDirectXVideoDecoderService **ppVideoService = NULL,
                                                 bool isTemporal = false) = 0;
    
    virtual IDirect3DDeviceManager9 * GetD3D9DeviceManager(void) = 0;
};

struct ID3D11Device;
struct ID3D11VideoDevice;
struct ID3D11DeviceContext;
struct ID3D11VideoContext;


struct D3D11Interface
{
    static const MFX_GUID getGuid()
    {
        return MFXICORED3D11_GUID;
    }

    virtual ID3D11Device * GetD3D11Device(bool isTemporal = false) = 0;
    virtual ID3D11VideoDevice * GetD3D11VideoDevice(bool isTemporal = false) = 0;

    virtual ID3D11DeviceContext * GetD3D11DeviceContext(bool isTemporal = false) = 0;
    virtual ID3D11VideoContext * GetD3D11VideoContext(bool isTemporal = false) = 0;
};


#elif defined (MFX_VA_LINUX)
    struct VAAPIInterface
    {
        static const MFX_GUID & getGuid()
        {
            return MFXICOREVAAPI_GUID;
        }
    
    //    virtual mfxStatus              GetD3DService(mfxU16 width,
    //                                                 mfxU16 height,
    //                                                 VADisplay** pDisplay = NULL) = 0;      
    };

#elif defined (MFX_VA_OSX)
struct VDAAPIInterface
{
    static const MFX_GUID & getGuid()
    {
        return MFXICOREVDAAPI_GUID;
    }
};

#endif

#endif // __LIBMFX_CORE_INTERFACE_H__
/* EOF */
