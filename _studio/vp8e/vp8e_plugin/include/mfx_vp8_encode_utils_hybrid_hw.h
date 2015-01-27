/* ****************************************************************************** *\

Copyright (C) 2012-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_vp8_encode_utils_hybrid_hw.h

\* ****************************************************************************** */

#include "mfx_common.h"

#ifndef _MFX_VP8_ENCODE_UTILS_HYBRID_HW_H_
#define _MFX_VP8_ENCODE_UTILS_HYBRID_HW_H_

#include "mfx_vp8_encode_utils_hw.h"
#include "mfx_vp8_encode_ddi_hw.h"
#include <queue>

#define MFX_CHECK_WITH_ASSERT(EXPR, ERR) { assert(EXPR); MFX_CHECK(EXPR, ERR); }


namespace MFX_VP8ENC
{
    struct sDDIFrames
    {
        sFrameEx *m_pMB_hw;
        sFrameEx *m_pMB_sw;

        sFrameEx *m_pDist_hw;
        sFrameEx *m_pDist_sw;

        sFrameEx *m_pSegMap_hw;

    };

    typedef struct 
    {
        mfxU16  RefFrameCost[4];
        mfxU16  IntraModeCost[4];
        mfxU16  InterModeCost[4];
        mfxU16  IntraNonDCPenalty16x16;
        mfxU16  IntraNonDCPenalty4x4;
    } VP8HybridCosts;

    class TaskHybridDDI: public Task
    {
    public:
        sDDIFrames ddi_frames;
        VP8HybridCosts m_costs;
        mfxU64         m_prevFrameSize;
        mfxU8          m_brcUpdateDelay;

        mfxU32         m_frameOrderOfPreviousFrame;

    public:
        TaskHybridDDI(): 
          Task() 
          {
              Zero (ddi_frames);
              m_prevFrameSize = 0;
              m_brcUpdateDelay = 0;
              m_frameOrderOfPreviousFrame = 0;
          }
        virtual ~TaskHybridDDI() {}
        virtual   mfxStatus SubmitTask (sFrameEx*  pRecFrame, sDpbVP8 &dpb, sFrameParams* pParams,  sFrameEx* pRawLocalFrame, 
                                        sDDIFrames *pDDIFrames)
        {
            /*printf("TaskHybridDDI::SubmitTask\n");*/
            mfxStatus sts = Task::SubmitTask(pRecFrame,dpb,pParams,pRawLocalFrame);

            MFX_CHECK(isFreeSurface(pDDIFrames->m_pMB_hw),MFX_ERR_UNDEFINED_BEHAVIOR);

            ddi_frames = *pDDIFrames;

            MFX_CHECK_STS(LockSurface(ddi_frames.m_pMB_hw,m_pCore));

            return sts;
        }
        virtual  mfxStatus FreeTask()
        {
            /*printf("TaskHybridDDI::FreeTask\n");*/
            MFX_CHECK_STS(FreeSurface(ddi_frames.m_pMB_hw,m_pCore));
            MFX_CHECK_STS(FreeSurface(ddi_frames.m_pMB_sw,m_pCore));

            //MFX_CHECK_STS(FreeSurface(ddi_frames.m_pDist_hw,m_pCore));
            //MFX_CHECK_STS(FreeSurface(ddi_frames.m_pDist_sw,m_pCore));

            return Task::FreeTask();
        }
    };

    struct FrameInfoFromPak
    {
        mfxU64         m_frameOrder;
        mfxU8          m_frameType;
        mfxU64         m_encodedFrameSize;
        VP8HybridCosts m_updatedCosts;
    };

    class TaskManagerHybridPakDDI : public TaskManager<TaskHybridDDI>
    {
    public:
        typedef TaskManager<TaskHybridDDI>  BaseClass;

        TaskManagerHybridPakDDI(): BaseClass()    {m_bUseSegMap = false;}
        virtual ~TaskManagerHybridPakDDI() {/*printf("~TaskManagerHybridPakDDI)\n");*/}
    
    // [SE] WA for Windows hybrid VP8 HSW driver (remove 'protected' here)
#if defined (MFX_VA_LINUX)
    protected:
#endif

        InternalFrames  m_MBDataDDI_hw;
        InternalFrames  m_DistDataDDI_hw;

        InternalFrames  m_MBDataDDI_sw;
        InternalFrames  m_DistDataDDI_sw;

        bool            m_bUseSegMap;
        InternalFrames  m_SegMapDDI_hw;

        mfxU16           m_maxBrcUpdateDelay;

        mfxU64           m_frameNumOfLastArrivedFrame;
        mfxI64           m_frameNumOfLastFrameSubmittedToDriver;
        mfxI64           m_frameNumOfLastEncodedFrame;

        std::queue<FrameInfoFromPak> m_cachedFrameInfoFromPak;
        FrameInfoFromPak             m_latestKeyFrame;
        FrameInfoFromPak             m_latestNonKeyFrame;

    public:

        inline
        mfxStatus Init( mfxCoreInterface* pCore, mfxVideoParam *par, mfxU32 reconFourCC)
        {
            mfxStatus sts = BaseClass::Init(pCore,par,true,reconFourCC);
            MFX_CHECK_STS(sts);
            m_maxBrcUpdateDelay = par->AsyncDepth > 2 ? 2: par->AsyncDepth; // driver supports maximum 2-frames BRC update delay
            m_frameNumOfLastArrivedFrame = 0;
            m_frameNumOfLastFrameSubmittedToDriver = -1;
            m_frameNumOfLastEncodedFrame = -1;
            Zero(m_latestNonKeyFrame);
            Zero(m_latestKeyFrame);
            return MFX_ERR_NONE;
        }

        inline
        mfxStatus AllocInternalResources(mfxCoreInterface *pCore, 
                        mfxFrameAllocRequest reqMBData,
                        mfxFrameAllocRequest reqDist,
                        mfxFrameAllocRequest reqSegMap)
        {
            if (reqMBData.NumFrameMin < m_ReconFrames.Num())
                reqMBData.NumFrameMin = (mfxU16)m_ReconFrames.Num();
            MFX_CHECK_STS(m_MBDataDDI_hw.Init(pCore, &reqMBData,true));

            if (reqDist.NumFrameMin)
            {
                MFX_CHECK_STS(m_DistDataDDI_hw.Init(pCore, &reqDist,true));
                reqDist.Info.FourCC = MFX_FOURCC_P8;
                MFX_CHECK_STS(m_DistDataDDI_sw.Init(pCore, &reqDist,false));
            }

            if (reqSegMap.NumFrameMin)
            {
                m_bUseSegMap = true;
                MFX_CHECK_STS(m_SegMapDDI_hw.Init(pCore, &reqSegMap,true));
            }

            return MFX_ERR_NONE;
        }

        inline 
        mfxStatus Reset(mfxVideoParam *par)
        {
            m_maxBrcUpdateDelay = par->AsyncDepth > 2 ? 2: par->AsyncDepth; // driver supports maximum 2-frames BRC update delay

            if (IsResetOfPipelineRequired(m_video, *par))
            {
                m_frameNumOfLastArrivedFrame = 0;
                m_frameNumOfLastFrameSubmittedToDriver = -1;
                m_frameNumOfLastEncodedFrame = -1;
            }
            return BaseClass::Reset(par);
        }
        inline
        mfxStatus InitTask(mfxFrameSurface1* pSurface, mfxBitstream* pBitstream, Task* & pOutTask)
        {
            if (m_frameNum >= m_maxBrcUpdateDelay && m_cachedFrameInfoFromPak.size() == 0)
            {
                // MSDK should send frame update to driver but required previous frame isn't encoded yet
                // let's wait for encoding of this frame
                return MFX_WRN_DEVICE_BUSY;
            }

            mfxStatus sts = BaseClass::InitTask(pSurface,pBitstream,pOutTask);
            TaskHybridDDI *pHybridTask = (TaskHybridDDI*)pOutTask;
            Zero(pHybridTask->m_costs);
            if (pOutTask->m_frameOrder < m_maxBrcUpdateDelay && m_cachedFrameInfoFromPak.size() == 0)
            {
                pHybridTask->m_frameOrderOfPreviousFrame = m_frameNumOfLastArrivedFrame;
                m_frameNumOfLastArrivedFrame = m_frameNum - 1;
                // MSDK can't send frame update to driver but it's OK for initial encoding stage
                return MFX_ERR_NONE;
            }

            mfxU64 minFrameOrderToUpdateBrc = 0;
            if (pOutTask->m_frameOrder > m_maxBrcUpdateDelay)
            {
                minFrameOrderToUpdateBrc = pOutTask->m_frameOrder - m_maxBrcUpdateDelay;
            }
            FrameInfoFromPak newestFrame = m_cachedFrameInfoFromPak.back();
            if (newestFrame.m_frameOrder < minFrameOrderToUpdateBrc)
            {
                return MFX_ERR_UNDEFINED_BEHAVIOR;
            }
            pHybridTask->m_prevFrameSize = newestFrame.m_encodedFrameSize;
            pHybridTask->m_brcUpdateDelay = mfxU8(pOutTask->m_frameOrder - newestFrame.m_frameOrder);

            while (m_cachedFrameInfoFromPak.size())
            {
                // store costs from last key and last non-key frames
                if (m_cachedFrameInfoFromPak.front().m_frameType == 0)
                    m_latestKeyFrame = m_cachedFrameInfoFromPak.front();
                else
                    m_latestNonKeyFrame = m_cachedFrameInfoFromPak.front();

                if (m_cachedFrameInfoFromPak.size() > 1 ||
                  m_cachedFrameInfoFromPak.front().m_frameOrder < minFrameOrderToUpdateBrc ||
                  m_cachedFrameInfoFromPak.front().m_frameOrder == minFrameOrderToUpdateBrc && pOutTask->m_frameOrder >= m_maxBrcUpdateDelay)
                {
                    // remove cached information if it's not longer required
                    m_cachedFrameInfoFromPak.pop();
                }
                else
                {
                    // all remaining cached information could be required for future frames
                    break;
                }
            }

            if (pHybridTask->m_frameOrder % m_video.mfx.GopPicSize == 0)
            {
                // key-frame
                pHybridTask->m_costs = m_latestKeyFrame.m_updatedCosts;
            }
            else
            {
                // non key-frame
                pHybridTask->m_costs = m_latestNonKeyFrame.m_updatedCosts;
            }

            pHybridTask->m_frameOrderOfPreviousFrame = m_frameNumOfLastArrivedFrame;
            m_frameNumOfLastArrivedFrame = m_frameNum - 1;

            return sts;
        }
        inline
        mfxStatus CacheInfoFromPak(Task &task, VP8HybridCosts & updatedCosts)
        {
            if (task.m_status != READY)
                return MFX_ERR_UNDEFINED_BEHAVIOR;

            FrameInfoFromPak infoAboutJustEncodedFrame;
            infoAboutJustEncodedFrame.m_frameOrder = task.m_frameOrder;
            infoAboutJustEncodedFrame.m_encodedFrameSize = task.m_pBitsteam->DataLength;
            infoAboutJustEncodedFrame.m_updatedCosts = updatedCosts;
            infoAboutJustEncodedFrame.m_frameType = !task.m_sFrameParams.bIntra;
            m_cachedFrameInfoFromPak.push(infoAboutJustEncodedFrame);

            return MFX_ERR_NONE;
        }
        inline 
        mfxStatus SubmitTask(Task*  pTask, sFrameParams *pParams)
        {
            sFrameEx* pRecFrame = 0;
            sFrameEx* pRawLocalFrame = 0;
            sDDIFrames ddi_frames = {0};

            mfxStatus sts = MFX_ERR_NONE;

            MFX_CHECK(m_pCore!=0, MFX_ERR_NOT_INITIALIZED);

            pRecFrame = m_ReconFrames.GetFreeFrame();
            MFX_CHECK(pRecFrame!=0,MFX_WRN_DEVICE_BUSY);

            if (m_bHWFrames != m_bHWInput)
            {
                pRawLocalFrame = m_LocalRawFrames.GetFreeFrame();
                MFX_CHECK(pRawLocalFrame!= 0,MFX_WRN_DEVICE_BUSY);
            } 
            MFX_CHECK_STS(m_MBDataDDI_hw.GetFrame(pRecFrame->idInPool,ddi_frames.m_pMB_hw));

            if (m_bUseSegMap)
            {
                MFX_CHECK_STS(m_SegMapDDI_hw.GetFrame(pRecFrame->idInPool,ddi_frames.m_pSegMap_hw));
            }
            else
                ddi_frames.m_pSegMap_hw = 0;

            sts = ((TaskHybridDDI*)pTask)->SubmitTask(  pRecFrame,m_dpb,
                                                        pParams, pRawLocalFrame,
                                                        &ddi_frames);
            MFX_CHECK_STS(sts);

            UpdateDpb(pParams, pRecFrame);

            return sts;
        }
        inline
        void RememberSubmittedTask(Task &task)
        {
            m_frameNumOfLastFrameSubmittedToDriver = task.m_frameOrder;
        }
        inline
        void RememberEncodedTask(Task &task)
        {
            m_frameNumOfLastEncodedFrame = task.m_frameOrder;
        }
        inline
        mfxStatus CheckHybridDependencies(TaskHybridDDI &task)
        {
            if (task.m_frameOrder == 0)
                return MFX_ERR_NONE;

            if (task.m_status == TASK_INITIALIZED &&
                m_frameNumOfLastFrameSubmittedToDriver >= task.m_frameOrderOfPreviousFrame)
                return MFX_ERR_NONE;

            if (task.m_status == TASK_SUBMITTED &&
                m_frameNumOfLastEncodedFrame >= task.m_frameOrderOfPreviousFrame)
                return MFX_ERR_NONE;

            return MFX_WRN_IN_EXECUTION;
        }
        inline MfxFrameAllocResponse& GetRecFramesForReg()
        {
            return m_ReconFrames.GetFrameAllocReponse();
        }
        inline MfxFrameAllocResponse& GetMBFramesForReg()
        {
            return m_MBDataDDI_hw.GetFrameAllocReponse();
        }

        inline MfxFrameAllocResponse& GetDistFramesForReg()
        {
            return m_DistDataDDI_hw.GetFrameAllocReponse();
        }

        inline MfxFrameAllocResponse& GetSegMapFramesForReg()
        {
            return m_SegMapDDI_hw.GetFrameAllocReponse();
        }
    };
}
#endif