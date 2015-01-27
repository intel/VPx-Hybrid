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

File Name: mfx_interface.h

\* ****************************************************************************** */
#ifndef __MFX_INTERFACE_H
#define __MFX_INTERFACE_H

#include <mfxdefs.h>
#include <stdio.h>

#if defined(_MSC_VER)
// disable warning C4584
// << 'class1' : base-class 'class2' is already a base-class of 'class3' >>.
// we use multiple inheritance for interfaces only,
// so there is no interference on data members.
#pragma warning(disable : 4584)
#endif // defined(_MSC_VER)

#pragma pack(1)

// Declare the internal GUID type
typedef
struct MFX_GUID
{
    mfxU32 Data1;
    mfxU16 Data2;
    mfxU16 Data3;
    mfxU8 Data4[8];

} MFX_GUID;

#pragma pack()

// Declare the function to compare GUIDs
inline
bool operator == (const MFX_GUID &guid0, const MFX_GUID &guid1)
{
    const mfxU32 *pGUID0 = (const mfxU32 *) &guid0;
    const mfxU32 *pGUID1 = (const mfxU32 *) &guid1;

    if ((pGUID0[0] != pGUID1[0]) ||
        (pGUID0[1] != pGUID1[1]) ||
        (pGUID0[2] != pGUID1[2]) ||
        (pGUID0[3] != pGUID1[3]))
    {
        return false;
    }

    return true;

} // bool operator == (const MFX_GUID &guid0, const MFX_GUID &guid1)

// Declare base interface class
class MFXIUnknown
{
public:
    virtual ~MFXIUnknown(void){}
    // Query another interface from the object. If the pointer returned is not NULL,
    // the reference counter is incremented automatically.
    virtual
    void *QueryInterface(const MFX_GUID &guid) = 0;

    // Increment reference counter of the object.
    virtual
    void AddRef(void) = 0;
    // Decrement reference counter of the object.
    // If the counter is equal to zero, destructor is called and
    // object is removed from the memory.
    virtual
    void Release(void) = 0;
    // Get the current reference counter value
    virtual
    mfxU32 GetNumRef(void) const = 0;
};

// Declaration of function to create object to support required interface
template <class T>
T* CreateInterfaceInstance(const MFX_GUID &guid);

// Declare a template to query an interface
template <class T> inline
T *QueryInterface(MFXIUnknown* &pUnk, const MFX_GUID &guid)
{
    void *pInterface = NULL;

    // have to create instance of required interface corresponding to GUID
    if (!pUnk)
    {
        pUnk = CreateInterfaceInstance<T>(guid);
    }

    // query the interface
    if (pUnk)
    {
        pInterface = pUnk->QueryInterface(guid);
    }

    // cast pointer returned to the required interface
    return (T *) pInterface;

} // T *QueryInterface(MFXIUnknown *pUnk, const MFX_GUID &guid)


template <class T>
class MFXIPtr
{
public:
    // Default constructor
    MFXIPtr(void)
    {
        m_pInterface = NULL;
    }
    // Constructors
    explicit
    MFXIPtr(MFXIPtr &iPtr)
    {
        operator = (iPtr);
    }
    explicit
    MFXIPtr(void *pInterface)
    {
        operator = (pInterface);
    }

    // Destructor
    ~MFXIPtr(void)
    {
        Release();
    }

    // Cast operator
    T * operator -> (void) const
    {
        return m_pInterface;
    }

    MFXIPtr & operator = (MFXIPtr &iPtr)
    {
        // release the interface before setting new one
        Release();

        // save the pointer to interface
        m_pInterface = iPtr.m_pInterface;
        // increment the reference counter
        m_pInterface->AddRef();

        return *this;
    }

    MFXIPtr & operator = (void *pInterface)
    {
        // release the interface before setting new one
        Release();

        // save the pointer to interface
        m_pInterface = (T *) pInterface;
        // There is no need to increment the reference counter,
        // because it is supposed that the pointer is returned from
        // QueryInterface function.

        return *this;
    }

    inline
    bool operator == (const void *p) const
    {
        return (p == m_pInterface);
    }

protected:
    void Release(void)
    {
        if (m_pInterface)
        {
            // decrement the reference counter of the interface
            m_pInterface->Release();
            m_pInterface = NULL;
        }
    }

    T *m_pInterface;                                            // (T *) pointer to the interface
};

template <class T> inline
bool operator == (void *p, const MFXIPtr<T> &ptr)
{
    return (ptr == p);

} // bool operator == (void *p, const MFXIPtr &ptr)

#endif // __MFX_INTERFACE_H
