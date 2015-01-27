/* ****************************************************************************** *\

Copyright (C) 2014 Intel Corporation.  All rights reserved.

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

File Name: vp8e_plugin.cpp

\* ****************************************************************************** */

#include "vp8e_plugin.h"
#include "mfx_plugin_module.h"
#include "mfxvideo++int.h"

PluginModuleTemplate g_PluginModule = {
    NULL,
    &MFX_VP8E_Plugin::Create,
    NULL,
    NULL,
    &MFX_VP8E_Plugin::CreateByDispatcher
};

MSDK_PLUGIN_API(MFXEncoderPlugin*) mfxCreateEncoderPlugin() {
    if (!g_PluginModule.CreateEncoderPlugin) {
        return 0;
    }
    return g_PluginModule.CreateEncoderPlugin();
}

MSDK_PLUGIN_API(MFXPlugin*) CreatePlugin(mfxPluginUID uid, mfxPlugin* plugin) {
    if (!g_PluginModule.CreatePlugin) {
        return 0;
    }
    return (MFXPlugin*) g_PluginModule.CreatePlugin(uid, plugin);
}

const mfxPluginUID MFX_VP8E_Plugin::g_PluginGuid = MFX_PLUGINID_VP8E_HW;

MFX_VP8E_Plugin::MFX_VP8E_Plugin(bool CreateByDispatcher)
    :m_adapter(0)
{
    m_pmfxCore = 0;
    memset(&m_PluginParam, 0, sizeof(mfxPluginParam));

    m_PluginParam.ThreadPolicy = MFX_THREADPOLICY_PARALLEL;
    m_PluginParam.MaxThreadNum = 1;
    m_PluginParam.APIVersion.Major = MFX_VERSION_MAJOR;
    m_PluginParam.APIVersion.Minor = MFX_VERSION_MINOR;
    m_PluginParam.PluginUID = g_PluginGuid;
    m_PluginParam.Type = MFX_PLUGINTYPE_VIDEO_ENCODE;
    m_PluginParam.CodecId = MFX_CODEC_VP8;
    m_PluginParam.PluginVersion = 1;
    m_createdByDispatcher = CreateByDispatcher;

    m_BSE = MFX_VP8ENC::Vp8CoreBSP::Create();

    m_pmfxCore = 0;
    memset(&m_mfxpar, 0, sizeof(mfxVideoParam));
}

mfxStatus MFX_VP8E_Plugin::PluginInit(mfxCoreInterface * pCore)
{
    if (!pCore)
        return MFX_ERR_NULL_PTR;

    m_pmfxCore = pCore;
    return MFX_ERR_NONE;
}

mfxStatus MFX_VP8E_Plugin::PluginClose()
{
    mfxStatus mfxRes = MFX_ERR_NONE;
    mfxStatus mfxRes2 = MFX_ERR_NONE;

    if (m_createdByDispatcher) {
        delete this;
    }

    return mfxRes2;
}

mfxStatus MFX_VP8E_Plugin::GetPluginParam(mfxPluginParam *par)
{
    if (!par)
        return MFX_ERR_NULL_PTR;
    *par = m_PluginParam;

    return MFX_ERR_NONE;
}

mfxStatus MFX_VP8E_Plugin::EncodeFrameSubmit(mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxThreadTask *task)
{
    MFX_VP8ENC::Task* pTask = 0;
    mfxStatus sts  = MFX_ERR_NONE;

    mfxStatus checkSts = MFX_VP8ENC::CheckEncodeFrameParam(
        m_video,
        ctrl,
        surface,
        bs,
        true);

    MFX_CHECK(checkSts >= MFX_ERR_NONE, checkSts);

    {
        UMC::AutomaticUMCMutex guard(m_taskMutex);
        sts = m_pTaskManager->InitTask(surface,bs,pTask);
        MFX_CHECK_STS(sts);
        if (ctrl)
            pTask->m_ctrl = *ctrl;

    }

    *task = (mfxThreadTask*)pTask;

    return checkSts;
}

mfxStatus MFX_VP8E_Plugin::Execute(mfxThreadTask task, mfxU32 , mfxU32 )
{
    MFX_VP8ENC::TaskHybridDDI       *pTask = (MFX_VP8ENC::TaskHybridDDI*)task;
    MFX_CHECK(pTask->m_status == MFX_VP8ENC::TASK_INITIALIZED || pTask->m_status == MFX_VP8ENC::TASK_SUBMITTED, MFX_ERR_UNDEFINED_BEHAVIOR);

    if (pTask->m_status == MFX_VP8ENC::TASK_INITIALIZED)
    {
        mfxStatus sts = MFX_ERR_NONE;
        {
            UMC::AutomaticUMCMutex guard(m_taskMutex);
            if (MFX_ERR_NONE != m_pTaskManager->CheckHybridDependencies(*pTask))
              return MFX_TASK_BUSY;
        }
        MFX_VP8ENC::sFrameParams        frameParams={0};
        mfxFrameSurface1    *pSurface=0;
        bool                bExternalSurface = true;

        mfxHDL surfaceHDL = 0;
        mfxHDL *pSurfaceHdl = (mfxHDL *)&surfaceHDL;

        {
            UMC::AutomaticUMCMutex guard(m_taskMutex);
            sts = SetFramesParams(&m_video,pTask->m_ctrl.FrameType,pTask->m_frameOrder, &frameParams);
            MFX_CHECK_STS(sts);
            sts = m_pTaskManager->SubmitTask(pTask,&frameParams);
            MFX_CHECK_STS(sts);
        }

        sts = pTask->GetInputSurface(pSurface, bExternalSurface);
        MFX_CHECK_STS(sts);

        sts = m_pmfxCore->FrameAllocator.GetHDL(m_pmfxCore->FrameAllocator.pthis, pSurface->Data.MemId, pSurfaceHdl);
        MFX_CHECK_STS(sts);

        MFX_CHECK(surfaceHDL != 0, MFX_ERR_UNDEFINED_BEHAVIOR);
        sts = m_ddi->Execute(*pTask, surfaceHDL);
        MFX_CHECK_STS(sts);

        {
            UMC::AutomaticUMCMutex guard(m_taskMutex);
            m_pTaskManager->RememberSubmittedTask(*pTask);
        }

        return MFX_TASK_WORKING;
    }
    else
    {
        mfxStatus           sts = MFX_ERR_NONE;

        {
            UMC::AutomaticUMCMutex guard(m_taskMutex);
            if (MFX_ERR_NONE != m_pTaskManager->CheckHybridDependencies(*pTask))
              return MFX_TASK_BUSY;
        }

        MFX_VP8ENC::MBDATA_LAYOUT layout={0};
        if ((sts = m_ddi->QueryStatus(*pTask) )== MFX_WRN_DEVICE_BUSY)
        {
            return MFX_TASK_WORKING;
        }

        MFX_CHECK_STS(sts);
        MFX_CHECK_STS(m_ddi->QueryMBLayout(layout));

        sts = m_BSE->SetNextFrame(0, 0, pTask->m_sFrameParams,pTask->m_frameOrder);
        MFX_CHECK_STS(sts);

        mfxExtVP8CodingOption * pExtVP8Opt = GetExtBuffer(m_video);
        bool bInsertIVF = (pExtVP8Opt->WriteIVFHeaders != MFX_CODINGOPTION_OFF);
        bool bInsertSH  = bInsertIVF && pTask->m_frameOrder==0 && m_bStartIVFSequence;

        sts = m_BSE->RunBSP(bInsertIVF, bInsertSH, pTask->m_pBitsteam, (MFX_VP8ENC::TaskHybridDDI *)pTask, layout, m_pmfxCore);
        MFX_CHECK_STS(sts);

        {
            UMC::AutomaticUMCMutex guard(m_taskMutex);
            m_pTaskManager->RememberEncodedTask(*pTask);
        }

        pTask->m_pBitsteam->TimeStamp = pTask->m_timeStamp;
        pTask->m_pBitsteam->FrameType = mfxU16(pTask->m_sFrameParams.bIntra ? MFX_FRAMETYPE_I | MFX_FRAMETYPE_IDR : MFX_FRAMETYPE_P);

        sts = pTask->CompleteTask();
        MFX_CHECK_STS(sts);

        {
            MFX_VP8ENC::VP8HybridCosts updatedCosts = m_BSE->GetUpdatedCosts();
            UMC::AutomaticUMCMutex guard(m_taskMutex);
            m_pTaskManager->CacheInfoFromPak(*pTask,updatedCosts);
            pTask->FreeTask();
        }

        return MFX_TASK_DONE;
    }
}

mfxStatus MFX_VP8E_Plugin::FreeResources(mfxThreadTask task, mfxStatus )
{
    task; return MFX_ERR_NONE;
}
mfxStatus MFX_VP8E_Plugin::Query(mfxVideoParam *in, mfxVideoParam *out)
{
    MFX_CHECK_NULL_PTR1(out);

    MFX_VP8ENC::ENCODE_CAPS_VP8             caps = {};
    MFX_CHECK(MFX_ERR_NONE == MFX_VP8ENC::QueryHwCaps(m_pmfxCore, caps), MFX_WRN_PARTIAL_ACCELERATION);

    return   (in == 0) ? MFX_VP8ENC::SetSupportedParameters(out):
        MFX_VP8ENC::CheckParameters(in,out);
}
mfxStatus MFX_VP8E_Plugin::QueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest *in, mfxFrameAllocRequest *out)
{
    MFX_CHECK_NULL_PTR2(par,in);

    MFX_CHECK(MFX_VP8ENC::CheckPattern(par->IOPattern), MFX_ERR_INVALID_VIDEO_PARAM);
    MFX_CHECK(MFX_VP8ENC::CheckFrameSize(par->mfx.FrameInfo.Width, par->mfx.FrameInfo.Height),MFX_ERR_INVALID_VIDEO_PARAM);

    in->Type = mfxU16((par->IOPattern & MFX_IOPATTERN_IN_SYSTEM_MEMORY)? MFX_VP8ENC::MFX_MEMTYPE_SYS_EXT:MFX_VP8ENC::MFX_MEMTYPE_D3D_EXT) ;

    in->NumFrameMin =  (par->AsyncDepth ? par->AsyncDepth: 1)  + 1; // default AsyncDepth is 1
    in->NumFrameSuggested = in->NumFrameMin;

    in->Info = par->mfx.FrameInfo;
    return MFX_ERR_NONE;
}
mfxStatus MFX_VP8E_Plugin::Init(mfxVideoParam *par)
{
    mfxStatus sts  = MFX_ERR_NONE;
    mfxStatus sts1 = MFX_ERR_NONE; // to save warnings ater parameters checking

    m_video = *par;

    m_pTaskManager = new MFX_VP8ENC::TaskManagerHybridPakDDI;

    mfxExtVP8CodingOption* pExtVP8Opt    = GetExtBuffer(m_video);
    {
        mfxExtOpaqueSurfaceAlloc   * pExtOpaque = GetExtBuffer(m_video);

        MFX_CHECK(MFX_VP8ENC::CheckFrameSize(par->mfx.FrameInfo.Width, par->mfx.FrameInfo.Height),MFX_ERR_INVALID_VIDEO_PARAM);

        sts1 = MFX_VP8ENC::CheckParametersAndSetDefault(par,&m_video, pExtVP8Opt, pExtOpaque, true ,false);
        MFX_CHECK(sts1 >=0, sts1);
    }
    m_ddi.reset(MFX_VP8ENC::CreatePlatformVp8Encoder());
    MFX_CHECK(m_ddi.get() != 0, MFX_WRN_PARTIAL_ACCELERATION);

    sts = m_ddi->CreateAuxilliaryDevice(m_pmfxCore,DXVA2_Intel_Encode_VP8,
        m_video.mfx.FrameInfo.Width, m_video.mfx.FrameInfo.Height);
    MFX_CHECK(sts == MFX_ERR_NONE, MFX_WRN_PARTIAL_ACCELERATION);

    MFX_VP8ENC::ENCODE_CAPS_VP8 caps = {};
    sts = m_ddi->QueryEncodeCaps(caps);
    if (sts != MFX_ERR_NONE)
        return MFX_WRN_PARTIAL_ACCELERATION;

    sts = CheckVideoParam(m_video, caps);
    MFX_CHECK(sts>=0,sts);

    sts = m_ddi->CreateAccelerationService(m_video);
    MFX_CHECK_STS(sts);

    mfxFrameAllocRequest reqMB     = {};
    mfxFrameAllocRequest reqDist   = {};
    mfxFrameAllocRequest reqSegMap = {};

    // on Linux we should allocate recon surfaces first, and then create encoding context and use it for allocation of other buffers
    // initialize task manager, including allocation of recon surfaces chain
    sts = m_pTaskManager->Init(m_pmfxCore,&m_video,m_ddi->GetReconSurfFourCC());
    MFX_CHECK_STS(sts);

    // encoding device/context is created inside this Register() call
    sts = m_ddi->Register(m_pTaskManager->GetRecFramesForReg(), D3DDDIFMT_NV12);
    MFX_CHECK_STS(sts);

    sts = m_ddi->QueryCompBufferInfo(D3DDDIFMT_INTELENCODE_MBDATA, reqMB, m_video.mfx.FrameInfo.Width, m_video.mfx.FrameInfo.Height);
    MFX_CHECK_STS(sts);
    sts = m_ddi->QueryCompBufferInfo(D3DDDIFMT_INTELENCODE_DISTORTIONDATA, reqDist, m_video.mfx.FrameInfo.Width, m_video.mfx.FrameInfo.Height);
    if (sts == MFX_ERR_NONE)
        reqDist.NumFrameMin = reqDist.NumFrameSuggested = (mfxU16)CalcNumSurfRecon(m_video);
    sts = m_ddi->QueryCompBufferInfo(D3DDDIFMT_INTELENCODE_SEGMENTMAP, reqSegMap, m_video.mfx.FrameInfo.Width, m_video.mfx.FrameInfo.Height);
    if (sts == MFX_ERR_NONE && pExtVP8Opt->EnableMultipleSegments == MFX_CODINGOPTION_ON)
        reqSegMap.NumFrameMin = reqSegMap.NumFrameSuggested = (mfxU16)CalcNumSurfRecon(m_video);

    sts = m_pTaskManager->AllocInternalResources(m_pmfxCore ,reqMB,reqDist,reqSegMap);
    MFX_CHECK_STS(sts);

    sts = m_ddi->Register(m_pTaskManager->GetMBFramesForReg(), D3DDDIFMT_INTELENCODE_MBDATA);
    MFX_CHECK_STS(sts);
    if (reqDist.NumFrameMin)
    {
        sts = m_ddi->Register(m_pTaskManager->GetDistFramesForReg(), D3DDDIFMT_INTELENCODE_DISTORTIONDATA);
        MFX_CHECK_STS(sts);
    }
    if (reqSegMap.NumFrameMin)
    {
        sts = m_ddi->Register(m_pTaskManager->GetSegMapFramesForReg(), D3DDDIFMT_INTELENCODE_SEGMENTMAP);
        MFX_CHECK_STS(sts);
    }

    sts = m_BSE->Init(m_video);
    MFX_CHECK_STS(sts);

    m_bStartIVFSequence = true;

    return sts1;
}
mfxStatus MFX_VP8E_Plugin::Reset(mfxVideoParam *par)
{
       mfxStatus sts  = MFX_ERR_NONE;
        mfxStatus sts1 = MFX_ERR_NONE;

        //printf("HybridPakDDIImpl::Reset\n");

        MFX_CHECK_NULL_PTR1(par);
        MFX_CHECK(par->IOPattern == m_video.IOPattern, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        MFX_VP8ENC::VP8MfxParam parBeforeReset = m_video;
        MFX_VP8ENC::VP8MfxParam parAfterReset = *par;

        {
            mfxExtVP8CodingOption*       pExtVP8Opt = GetExtBuffer(parAfterReset);
            mfxExtOpaqueSurfaceAlloc*    pExtOpaque = GetExtBuffer(parAfterReset);

            sts1 = MFX_VP8ENC::CheckParametersAndSetDefault(par,&parAfterReset, pExtVP8Opt,pExtOpaque,true,true);
            MFX_CHECK(sts1>=0, sts1);
        }

        MFX_CHECK(parBeforeReset.AsyncDepth == parAfterReset.AsyncDepth
            && parBeforeReset.mfx.RateControlMethod == parAfterReset.mfx.RateControlMethod,
            MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

        parAfterReset.mfx.BufferSizeInKB = m_video.mfx.BufferSizeInKB; // inherit HRD buffer size from initialization parameters

        m_video = parAfterReset;

        sts = m_pTaskManager->Reset(&m_video);
        MFX_CHECK_STS(sts);

        sts = m_ddi->Reset(m_video);
        MFX_CHECK_STS(sts);

        sts = m_BSE->Reset(m_video);
        MFX_CHECK_STS(sts);

        m_bStartIVFSequence = false;

        return sts1;
}
mfxStatus MFX_VP8E_Plugin::Close()
{
    if (m_pTaskManager)
    {
        delete m_pTaskManager;
        m_pTaskManager = 0;
    }
    return MFX_ERR_NONE;
}

mfxStatus MFX_VP8E_Plugin::GetVideoParam(mfxVideoParam *par)
{
    MFX_CHECK_NULL_PTR1(par);
    return MFX_VP8ENC::GetVideoParam(par,&m_video);
}