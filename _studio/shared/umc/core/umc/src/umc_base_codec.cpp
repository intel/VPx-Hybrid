/* ****************************************************************************** *\

Copyright (C) 2007-2008 Intel Corporation.  All rights reserved.

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

File Name: umc_base_codec.cpp

\* ****************************************************************************** */

#include "umc_base_codec.h"
#include "umc_default_memory_allocator.h"

using namespace UMC;

// Default constructor
BaseCodecParams::BaseCodecParams(void)
{
    m_SuggestedOutputSize = 0;
    m_SuggestedInputSize = 0;
    lpMemoryAllocator = 0;
    numThreads = 0;

    m_pData=NULL;

    profile = 0;
    level = 0;
}

// Constructor
BaseCodec::BaseCodec(void)
{
    m_pMemoryAllocator = 0;
    m_bOwnAllocator = false;
}

// Destructor
BaseCodec::~BaseCodec(void)
{
  BaseCodec::Close();
}

// Initialize codec with specified parameter(s)
// Has to be called if MemoryAllocator interface is used
Status BaseCodec::Init(BaseCodecParams *init)
{
  if (init == 0)
    return UMC_ERR_NULL_PTR;

  // care about reentering as well
  if (init->lpMemoryAllocator) {
    if (m_bOwnAllocator && m_pMemoryAllocator != init->lpMemoryAllocator) {
      vm_debug_trace(VM_DEBUG_ERROR, VM_STRING("can't replace external allocator\n"));
      return UMC_ERR_INIT;
    }
    m_pMemoryAllocator = init->lpMemoryAllocator;
    m_bOwnAllocator = false;
  } else {
    if (m_pMemoryAllocator != 0 && !m_bOwnAllocator) {
      vm_debug_trace(VM_DEBUG_ERROR, VM_STRING("can't replace external allocator\n"));
      return UMC_ERR_INIT;
    }
    if (m_pMemoryAllocator == 0)
      m_pMemoryAllocator = new DefaultMemoryAllocator;
    m_bOwnAllocator = true;
  }
  return UMC_OK;
}

// Close all codec resources
Status BaseCodec::Close(void)
{
  if ( m_bOwnAllocator && m_pMemoryAllocator != 0 )
  {
    delete m_pMemoryAllocator;
    m_bOwnAllocator = false;
    m_pMemoryAllocator = 0;
  }
  return UMC_OK;
}

