/* ****************************************************************************** *\

Copyright (C) 2007-2013 Intel Corporation.  All rights reserved.

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

File Name: libmfx_core_operation.h

\* ****************************************************************************** */
#ifndef __LIBMFX_CORE_OPERATOR_H__
#define __LIBMFX_CORE_OPERATOR_H__

#include <vector>
#include <vm_interlocked.h>
#include <umc_mutex.h>

class VideoCORE;

class OperatorCORE
{
public:
    OperatorCORE(VideoCORE* pCore):m_refCounter(1)
    {
        m_Cores.push_back(pCore);
        pCore->SetCoreId(0);
    };

    void AddCore(VideoCORE* pCore)
    {
        UMC::AutomaticUMCMutex guard(m_guard);
        m_Cores.push_back(pCore);
        pCore->SetCoreId((mfxU32)m_Cores.size()-1);
    }
    void RemoveCore(VideoCORE* pCore)
    {
        UMC::AutomaticUMCMutex guard(m_guard);
        std::vector<VideoCORE*>::iterator it = m_Cores.begin();
        for (;it != m_Cores.end();it++)
        {
            if (*it == pCore)
            {
                m_Cores.erase(it);
                return;
            }
        }
    }

    // functor to run fuction from child cores
    bool  IsOpaqSurfacesAlreadyMapped(mfxFrameSurface1 **pOpaqueSurface, mfxU32 NumOpaqueSurface, mfxFrameAllocResponse *response)
    {
        bool sts;
        std::vector<VideoCORE*>::iterator it = m_Cores.begin();

        for (;it != m_Cores.end();it++)
        {
            sts = (*it)->IsOpaqSurfacesAlreadyMapped(pOpaqueSurface, NumOpaqueSurface, response, false);
            if (sts)
                return sts;
        }
        return false;
    }
    
    // functor to run fuction from child cores
    bool CheckOpaqRequest(mfxFrameAllocRequest *request, mfxFrameSurface1 **pOpaqueSurface, mfxU32 NumOpaqueSurface)
    {
        bool sts;
        std::vector<VideoCORE*>::iterator it = m_Cores.begin();

        for (;it != m_Cores.end();it++)
        {
            sts = (*it)->CheckOpaqueRequest(request, pOpaqueSurface, NumOpaqueSurface, false);
            if (!sts)
                return sts;
        }
        return true;
    }

    // functor to run fuction from child cores
    template <typename func, typename arg, typename arg2>
    mfxStatus DoFrameOperation(func functor, arg par, arg2 out)
    {
        mfxStatus sts;
        std::vector<VideoCORE*>::iterator it = m_Cores.begin();

        for (;it != m_Cores.end();it++)
        {
            sts = ((*it)->*functor)(par, out, false);
            // if it is correct Core we can return
            if (MFX_ERR_NONE == sts)
                return sts;
        }
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    // functor to run fuction from child cores
    template <typename func, typename arg>
    mfxStatus DoCoreOperation(func functor, arg par)
    {
        mfxStatus sts;
        std::vector<VideoCORE*>::iterator it = m_Cores.begin();

        for (;it != m_Cores.end();it++)
        {
            sts = ((*it)->*functor)(par, false);
            // if it is correct Core we can return
            if (MFX_ERR_NONE == sts)
                return sts;
        }
        return MFX_ERR_UNDEFINED_BEHAVIOR;
    }

    // functor to run fuction from child cores
    template <typename func, typename arg>
    mfxFrameSurface1* GetSurface(func functor, arg par)
    {
        mfxFrameSurface1* pSurf;
        std::vector<VideoCORE*>::iterator it = m_Cores.begin();
        for (;it != m_Cores.end();it++)
        {
            pSurf = ((*it)->*functor)(par, false);
            // if it is correct Core we can return
            if (pSurf)
                return pSurf;
        }
        return 0;
    }

    // Increment reference counter of the object.
    virtual
    void AddRef(void)
    {
        vm_interlocked_inc32(&m_refCounter);
    };
    // Decrement reference counter of the object.
    // If the counter is equal to zero, destructor is called and
    // object is removed from the memory.
    virtual
    void Release(void)
    {
        vm_interlocked_dec32(&m_refCounter);

        if (0 == m_refCounter)
        {
            delete this;
        }
    };
    // Get the current reference counter value
    virtual
    mfxU32 GetNumRef(void)
    {
        return m_refCounter;
    };
private:

    virtual ~OperatorCORE()
    {
        m_Cores.clear();
    };
    // self and child cores
    std::vector<VideoCORE*>  m_Cores;

    // Reference counters
    mfxU32 m_refCounter;

    UMC::Mutex m_guard;

    // Forbid the assignment operator
    OperatorCORE & operator = (const OperatorCORE &);

};
#endif


