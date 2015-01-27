/* ****************************************************************************** *\

Copyright (C) 2008-2009 Intel Corporation.  All rights reserved.

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

File Name: mfx_thread_task.h

\* ****************************************************************************** */
#if !defined(__MFX_THREAD_TASK_H)
#define __MFX_THREAD_TASK_H

#include <mfxvideo.h>

#include <mfxvideo++int.h>

// Declare task's parameters structure
typedef
struct MFX_THREAD_TASK_PARAMETERS
{
    union
    {
        // ENCODE parameters
        struct
        {
            mfxEncodeCtrl *ctrl;                                // [IN] pointer to encode control
            mfxFrameSurface1 *surface;                          // [IN] pointer to the source surface
            mfxBitstream *bs;                                   // [OUT] pointer to the destination bitstream
            mfxEncodeInternalParams internal_params;            // output of EncodeFrameCheck(), input of EncodeFrame()

        } encode;

        // DECODE parameters
        struct
        {
            mfxBitstream *bs;                                   // [IN] pointer to the source bitstream
            mfxFrameSurface1 *surface_work;                     // [IN] pointer to the surface for decoding
            mfxFrameSurface1 *surface_out;                      // [OUT] pointer to the current being decoded surface

        } decode;

        // VPP parameters
        struct
        {
            mfxFrameSurface1 *in;                               // [IN] pointer to the source surface
            mfxFrameSurface1 *out;                              // [OUT] pointer to the destination surface
            mfxExtVppAuxData *aux;                              // [IN] auxilary encoding data

        } vpp;

        // ENC, PAK parameters
        struct
        {
            mfxFrameCUC *cuc;                                   // [IN, OUT] pointer to encoding parameters
            void *pBRC;                                         // [IN] pointer to the BRC object

        } enc, pak;
    };

} MFX_THREAD_TASK_PARAMETERS;

#endif // __MFX_THREAD_TASK_H
