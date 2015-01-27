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

File Name: mfx_vp8_encode_ddi_hw.cpp

\* ****************************************************************************** */

#include "mfx_vp8_encode_utils_hybrid_hw.h"
#if defined (MFX_ENABLE_VP8_VIDEO_ENCODE_HW)
#include "assert.h"

namespace MFX_VP8ENC
{    
    mfxStatus QueryHwCaps(mfxCoreInterface * pCore, ENCODE_CAPS_VP8 & caps)
    {
        std::auto_ptr<DriverEncoder> ddi;

        ddi.reset(CreatePlatformVp8Encoder());
        MFX_CHECK(ddi.get() != NULL, MFX_WRN_PARTIAL_ACCELERATION);

        mfxStatus sts = ddi.get()->CreateAuxilliaryDevice(pCore, DXVA2_Intel_Encode_VP8, 640, 480);
        MFX_CHECK_STS(sts);
            
        sts = ddi.get()->QueryEncodeCaps(caps);
        MFX_CHECK_STS(sts);

        return MFX_ERR_NONE;
    }
    mfxStatus CheckVideoParam(mfxVideoParam const & par, ENCODE_CAPS_VP8 const &caps)
    {
        mfxStatus sts = MFX_ERR_NONE;
        mfxExtVP8CodingOption * pExtVP8Opt = GetExtBuffer(par);
        MFX_CHECK_NULL_PTR1(pExtVP8Opt);

        MFX_CHECK(par.mfx.FrameInfo.Height <= caps.MaxPicHeight, MFX_WRN_PARTIAL_ACCELERATION);
        MFX_CHECK(par.mfx.FrameInfo.Width  <= caps.MaxPicWidth,  MFX_WRN_PARTIAL_ACCELERATION);
        if (pExtVP8Opt->EnableMultipleSegments && !caps.SegmentationAllowed)
        {
            pExtVP8Opt->EnableMultipleSegments = 0;
            sts =  MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;       
        }
        return sts;
    }

    DriverEncoder* CreatePlatformVp8Encoder()
    {
        return new VAAPIEncoder;
    }

#if defined(MFX_VA_LINUX)

#define ALIGN(x, align) (((x) + (align) - 1) & (~((align) - 1)))

 /* ----------- Functions to convert MediaSDK into DDI --------------------- */

    void FillSpsBuffer(mfxVideoParam const & par, VAEncSequenceParameterBufferVP8 & sps)
    {
        Zero(sps);

        sps.frame_width  = par.mfx.FrameInfo.Width;
        sps.frame_height = par.mfx.FrameInfo.Height; 
        sps.frame_width_scale = 0;
        sps.frame_height_scale = 0;

        /*OG: are those parameters used for full ENCODE Mode only ??? */

        sps.error_resilient = 0; 
        sps.kf_auto = 0; 
        sps.kf_min_dist = 1;
        sps.kf_max_dist = par.mfx.GopRefDist;
        sps.bits_per_second = par.mfx.TargetKbps*1000;    //     
        sps.intra_period  = par.mfx.GopPicSize;
    } 

    mfxStatus FillPpsBuffer(
        TaskHybridDDI const & task,
        mfxVideoParam const & par,
        VAEncPictureParameterBufferVP8 & pps,
        std::vector<ExtVASurface> const & reconQueue)
    {
        mfxExtVP8CodingOption * pExtVP8Opt = GetExtBuffer(par);
        MFX_CHECK_NULL_PTR1(pExtVP8Opt);

        Zero(pps);

        pps.pic_flags.bits.version     = pExtVP8Opt->Version;
        pps.pic_flags.bits.color_space = 0;

        pps.ref_last_frame = pps.ref_gf_frame = pps.ref_arf_frame = VA_INVALID_SURFACE;

        MFX_CHECK(task.m_pRecFrame->idInPool < reconQueue.size(), MFX_ERR_UNDEFINED_BEHAVIOR);
        pps.reconstructed_frame = reconQueue[task.m_pRecFrame->idInPool].surface;

        pps.ref_flags.value = 0; // use all references
        if ( task.m_pRecRefFrames[REF_BASE])
        {
            MFX_CHECK(task.m_pRecRefFrames[REF_BASE]->idInPool < reconQueue.size(), MFX_ERR_UNDEFINED_BEHAVIOR);
            pps.ref_last_frame = reconQueue[task.m_pRecRefFrames[REF_BASE]->idInPool].surface;
        }
        if ( task.m_pRecRefFrames[REF_GOLD])
        {
            MFX_CHECK(task.m_pRecRefFrames[REF_GOLD]->idInPool < reconQueue.size(), MFX_ERR_UNDEFINED_BEHAVIOR);
            pps.ref_gf_frame = reconQueue[task.m_pRecRefFrames[REF_GOLD]->idInPool].surface;
        }
        if ( task.m_pRecRefFrames[REF_ALT])
        {
            MFX_CHECK(task.m_pRecRefFrames[REF_ALT]->idInPool < reconQueue.size(), MFX_ERR_UNDEFINED_BEHAVIOR);
            pps.ref_arf_frame = reconQueue[task.m_pRecRefFrames[REF_ALT]->idInPool].surface;
        }

        pps.pic_flags.bits.frame_type                      = (task.m_sFrameParams.bIntra) ? 0 : 1;
        pps.pic_flags.bits.segmentation_enabled           = pExtVP8Opt->EnableMultipleSegments;

        pps.pic_flags.bits.loop_filter_type               = task.m_sFrameParams.LFType;
        pps.pic_flags.bits.loop_filter_adj_enable         = pExtVP8Opt->LoopFilterRefTypeDelta[0] || pExtVP8Opt->LoopFilterRefTypeDelta[1] || pExtVP8Opt->LoopFilterRefTypeDelta[2] || pExtVP8Opt->LoopFilterRefTypeDelta[3] ||
            pExtVP8Opt->LoopFilterMbModeDelta[0] || pExtVP8Opt->LoopFilterMbModeDelta[1] || pExtVP8Opt->LoopFilterMbModeDelta[2] || pExtVP8Opt->LoopFilterMbModeDelta[3];

        pps.pic_flags.bits.num_token_partitions           = pExtVP8Opt->NumTokenPartitions;

        if (pps.pic_flags.bits.frame_type)
        {
            pps.pic_flags.bits.refresh_golden_frame = 0; 
            pps.pic_flags.bits.refresh_alternate_frame = 0; 
            pps.pic_flags.bits.copy_buffer_to_golden = 1;            
            pps.pic_flags.bits.copy_buffer_to_alternate = 2; 
            pps.pic_flags.bits.refresh_last = 1;                   
        }  

        pps.pic_flags.bits.sign_bias_golden         = 0;
        pps.pic_flags.bits.sign_bias_alternate      = 0;
        pps.pic_flags.bits.mb_no_coeff_skip         = 1;
  
        pps.sharpness_level          = task.m_sFrameParams.Sharpness;;


        for (int i = 0; i < 4; i ++)
        {
            pps.loop_filter_level[i] = task.m_sFrameParams.LFLevel[i];
            if (pps.pic_flags.bits.loop_filter_adj_enable)
            {
                pps.ref_lf_delta[i]      = pExtVP8Opt->LoopFilterRefTypeDelta[i];
                pps.mode_lf_delta[i]     = pExtVP8Opt->LoopFilterMbModeDelta[i];
            }
        }

        pps.pic_flags.bits.refresh_entropy_probs          = 0;
        pps.pic_flags.bits.forced_lf_adjustment           = 0;
        pps.pic_flags.bits.show_frame                     = 1;
        pps.pic_flags.bits.recon_filter_type              = 3;
        pps.pic_flags.bits.auto_partitions                = 0;
        pps.pic_flags.bits.clamping_type                  = 0;
        pps.pic_flags.bits.update_mb_segmentation_map     = 0;
        pps.pic_flags.bits.update_segment_feature_data    = 0;

        /*OG: is this parameters used for full ENCODE Mode only ??? */
        pps.clamp_qindex_high = 127;
        pps.clamp_qindex_low = 0;

        return MFX_ERR_NONE;
    }

    mfxStatus FillQuantBuffer(TaskHybridDDI const & task, mfxVideoParam const & par,VAQMatrixBufferVP8& quant)
    {
        MFX_CHECK(par.mfx.RateControlMethod == MFX_RATECONTROL_CQP, MFX_ERR_UNSUPPORTED);
        mfxExtVP8CodingOption * pExtVP8Opt = GetExtBuffer(par);

        for (int i = 0; i < 4; i ++)
        {
            quant.quantization_index[i] = task.m_sFrameParams.bIntra ? par.mfx.QPI:par.mfx.QPP
            + pExtVP8Opt->SegmentQPDelta[i];
        }
        // delta for YDC, Y2AC, Y2DC, UAC, UDC
        for (int i = 0; i < 5; i ++)
        {
            quant.quantization_index_delta[i] = pExtVP8Opt->CoeffTypeQPDelta[i];
        }

        return MFX_ERR_NONE;
    }

#define LEFT(i) 3*i
#define RIGHT(i) 3*i + 1
#define ROIID(i) 3*i + 2

    mfxStatus FillSegMap(
        TaskHybridDDI const & task,
        mfxVideoParam const & par,
        mfxCoreInterface *    pCore,
        VAEncMiscParameterVP8SegmentMapParams& segMapPar)
    {
        // TODO: get real segmentation map here, from ROI or another interface
        if (task.ddi_frames.m_pSegMap_hw == 0)
            return MFX_ERR_NONE; // segment map isn't required
        FrameLocker lockSegMap(pCore, task.ddi_frames.m_pSegMap_hw->pSurface->Data, false);
        mfxU32 frameWidthInMBs  = par.mfx.FrameInfo.Width / 16;
        mfxU32 frameHeightInMBs = par.mfx.FrameInfo.Height / 16;
        mfxU32 segMapPitch = ALIGN(frameWidthInMBs, 64);
        mfxU32 segMapAlign = segMapPitch - frameWidthInMBs;
        mfxU8 *bufPtr = task.ddi_frames.m_pSegMap_hw->pSurface->Data.Y;

        // fill segmentation map from ROI
        mfxExtEncoderROI * pExtRoi = GetExtBuffer(par);
        mfxU32 roiMBHorizLimits[9];
        mfxU32 numRoiPerRow;
        mfxU32 i = 0;

        memset(segMapPar.yac_quantization_index_delta, 0, 4);

        if (pExtRoi->NumROI)
        {
            for (i = 0; i < pExtRoi->NumROI; i ++)
            {
                segMapPar.yac_quantization_index_delta[i] = pExtRoi->ROI[i].Priority;
            }
            for (mfxU32 row = 0; row < frameHeightInMBs; row ++)
            {
                numRoiPerRow = 0;
                for (i = 0; i < pExtRoi->NumROI; i ++)
                {
                    if (row >= (pExtRoi->ROI[i].Top >> 4) && (row < pExtRoi->ROI[i].Bottom >> 4))
                    {
                        roiMBHorizLimits[LEFT(numRoiPerRow)] = pExtRoi->ROI[i].Left >> 4;
                        roiMBHorizLimits[RIGHT(numRoiPerRow)] = pExtRoi->ROI[i].Right >> 4;
                        roiMBHorizLimits[ROIID(numRoiPerRow)] = i;
                        numRoiPerRow ++;
                    }
                }
                if (numRoiPerRow)
                {
                    for (mfxU32 col = 0; col < frameWidthInMBs; col ++)
                    {
                        for (i = 0; i < numRoiPerRow; i ++)
                        {
                            if (col >= roiMBHorizLimits[LEFT(i)] && col < roiMBHorizLimits[RIGHT(i)])
                            {
                                bufPtr[segMapPitch * row + col] = roiMBHorizLimits[ROIID(i)];
                                break;
                            }
                        }
                        if (i == numRoiPerRow)
                        {
                            if (pExtRoi->NumROI < 4)
                                bufPtr[segMapPitch * row + col] = pExtRoi->NumROI;
                            else
                                return MFX_ERR_DEVICE_FAILED;
                        }
                    }
                }
                else
                    memset(&(bufPtr[segMapPitch * row]), pExtRoi->NumROI, frameWidthInMBs);

                memset(&(bufPtr[segMapPitch * row + frameWidthInMBs]), 0, segMapAlign);
            }            
        }

#if 0        
        printf("\n\n");
        for (mfxU32 row = 0; row < frameHeightInMBs; row ++)
        {
            for (mfxU32 col = 0; col < segMapPitch; col ++)
            {
                printf("%02x(%3d) ", bufPtr[segMapPitch * row + col], pExtRoi->ROI[bufPtr[segMapPitch * row + col]].Priority);
            }
            printf("\n");
        }
        printf("\n\n");
        fflush(0);
#endif
        return MFX_ERR_NONE;
    }
    
    mfxStatus FillFrameUpdateBuffer(TaskHybridDDI const & task, VAEncMiscParameterVP8HybridFrameUpdate & frmUpdate)
    {
        frmUpdate.prev_frame_size = (UINT)task.m_prevFrameSize;
        frmUpdate.two_prev_frame_flag = task.m_brcUpdateDelay == 2 ? 1 : 0;
        for (mfxU8 i = 0; i < 4; i++)
        {
            frmUpdate.intra_mode_cost[i] = task.m_costs.IntraModeCost[i];
            frmUpdate.inter_mode_cost[i] = task.m_costs.InterModeCost[i];
            frmUpdate.ref_frame_cost[i]  = task.m_costs.RefFrameCost[i];
        }
        frmUpdate.intra_non_dc_penalty_16x16 = task.m_costs.IntraNonDCPenalty16x16;
        frmUpdate.intra_non_dc_penalty_4x4 = task.m_costs.IntraNonDCPenalty4x4;

        frmUpdate.ref_q_index[0] = frmUpdate.ref_q_index[1] = frmUpdate.ref_q_index[2] = 0;
        if ( task.m_pRecRefFrames[REF_BASE])
        {
            frmUpdate.ref_q_index[0] = task.m_pRecRefFrames[REF_BASE]->QP; // pass QP for last ref
        }
        if ( task.m_pRecRefFrames[REF_GOLD])
        {
            frmUpdate.ref_q_index[1] = task.m_pRecRefFrames[REF_GOLD]->QP; // pass QP for gold ref
        }
        if ( task.m_pRecRefFrames[REF_ALT])
        {
            frmUpdate.ref_q_index[2] = task.m_pRecRefFrames[REF_ALT]->QP; // pass QP for alt ref
        }

        return MFX_ERR_NONE;
    }
    
mfxU8 ConvertRateControlMFX2VAAPI(mfxU8 rateControl)
{
    switch (rateControl)
    {
    case MFX_RATECONTROL_CBR:  return VA_RC_CBR;
    case MFX_RATECONTROL_VBR:  return VA_RC_VBR;
    case MFX_RATECONTROL_AVBR: return VA_RC_VBR;
    case MFX_RATECONTROL_CQP:  return VA_RC_CQP;
    default: assert(!"Unsupported RateControl"); return 0;
    }

} // mfxU8 ConvertRateControlMFX2VAAPI(mfxU8 rateControl)

mfxStatus SetRateControl(
    mfxVideoParam const & par,
    VADisplay    m_vaDisplay,
    VAContextID  m_vaContextEncode,
    VABufferID & rateParamBuf_id)
{
    VAStatus vaSts;
    VAEncMiscParameterBuffer *misc_param;
    VAEncMiscParameterRateControl *rate_param;

    if ( rateParamBuf_id != VA_INVALID_ID)
    {
        vaDestroyBuffer(m_vaDisplay, rateParamBuf_id);
    }

    vaSts = vaCreateBuffer(m_vaDisplay,
                   m_vaContextEncode,
                   VAEncMiscParameterBufferType,
                   sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterRateControl),
                   1,
                   NULL,
                   &rateParamBuf_id);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    vaSts = vaMapBuffer(m_vaDisplay,
                 rateParamBuf_id,
                (void **)&misc_param);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    misc_param->type = VAEncMiscParameterTypeRateControl;
    rate_param = (VAEncMiscParameterRateControl *)misc_param->data;

    rate_param->bits_per_second = par.mfx.MaxKbps * 1000;

    if(par.mfx.MaxKbps)
        rate_param->target_percentage = (unsigned int)(100.0 * (mfxF64)par.mfx.TargetKbps / (mfxF64)par.mfx.MaxKbps);

    vaUnmapBuffer(m_vaDisplay, rateParamBuf_id);

    return MFX_ERR_NONE;
}

mfxStatus SetHRD(
    mfxVideoParam const & par,
    VADisplay    m_vaDisplay,
    VAContextID  m_vaContextEncode,
    VABufferID & hrdBuf_id)
{
    VAStatus vaSts;
    VAEncMiscParameterBuffer *misc_param;
    VAEncMiscParameterHRD *hrd_param;

    if ( hrdBuf_id != VA_INVALID_ID)
    {
        vaDestroyBuffer(m_vaDisplay, hrdBuf_id);
    }
    vaSts = vaCreateBuffer(m_vaDisplay,
                   m_vaContextEncode,
                   VAEncMiscParameterBufferType,
                   sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterHRD),
                   1,
                   NULL,
                   &hrdBuf_id);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    vaSts = vaMapBuffer(m_vaDisplay,
                 hrdBuf_id,
                (void **)&misc_param);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    misc_param->type = VAEncMiscParameterTypeHRD;
    hrd_param = (VAEncMiscParameterHRD *)misc_param->data;

    hrd_param->initial_buffer_fullness = par.mfx.InitialDelayInKB * 8000;
    hrd_param->buffer_size = par.mfx.BufferSizeInKB * 8000;

    vaUnmapBuffer(m_vaDisplay, hrdBuf_id);

    return MFX_ERR_NONE;
}

mfxStatus SetFrameRate(
    mfxVideoParam const & par,
    VADisplay    m_vaDisplay,
    VAContextID  m_vaContextEncode,
    VABufferID & frameRateBufId)
{
    VAStatus vaSts;
    VAEncMiscParameterBuffer *misc_param;
    VAEncMiscParameterFrameRate *frameRate_param;

    if ( frameRateBufId != VA_INVALID_ID)
    {
        vaDestroyBuffer(m_vaDisplay, frameRateBufId);
    }

    vaSts = vaCreateBuffer(m_vaDisplay,
                   m_vaContextEncode,
                   VAEncMiscParameterBufferType,
                   sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterFrameRate),
                   1,
                   NULL,
                   &frameRateBufId);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    vaSts = vaMapBuffer(m_vaDisplay,
                 frameRateBufId,
                (void **)&misc_param);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    misc_param->type = VAEncMiscParameterTypeFrameRate;
    frameRate_param = (VAEncMiscParameterFrameRate *)misc_param->data;

    frameRate_param->framerate = (unsigned int)(100.0 * (mfxF64)par.mfx.FrameInfo.FrameRateExtN / (mfxF64)par.mfx.FrameInfo.FrameRateExtD);

    vaUnmapBuffer(m_vaDisplay, frameRateBufId);

    return MFX_ERR_NONE;
}

mfxStatus SendMiscBufferFrameUpdate(
    VADisplay    m_vaDisplay,
    VAContextID  m_vaContextEncode,
    VAEncMiscParameterVP8HybridFrameUpdate & frameUpdateBuf,
    VABufferID & frameUpdateBufId)
{
    VAStatus vaSts;
    VAEncMiscParameterBuffer *misc_param;
    VAEncMiscParameterVP8HybridFrameUpdate *frameUpdate_param;

    if ( frameUpdateBufId != VA_INVALID_ID)
    {
        vaDestroyBuffer(m_vaDisplay, frameUpdateBufId);
    }

    vaSts = vaCreateBuffer(m_vaDisplay,
                   m_vaContextEncode,
                   VAEncMiscParameterBufferType,
                   sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterVP8HybridFrameUpdate),
                   1,
                   NULL,
                   &frameUpdateBufId);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    vaSts = vaMapBuffer(m_vaDisplay,
                 frameUpdateBufId,
                (void **)&misc_param);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    misc_param->type = (VAEncMiscParameterType)VAEncMiscParameterTypeVP8HybridFrameUpdate;
    frameUpdate_param = (VAEncMiscParameterVP8HybridFrameUpdate *)misc_param->data;
    *frameUpdate_param = frameUpdateBuf;

    vaUnmapBuffer(m_vaDisplay, frameUpdateBufId);

    return MFX_ERR_NONE;
}

mfxStatus SendMiscBufferSegMapPar(
    VADisplay    m_vaDisplay,
    VAContextID  m_vaContextEncode,
    VAEncMiscParameterVP8SegmentMapParams & segMapParBuf,
    VABufferID & segMapParBufId)
{
    VAStatus vaSts;
    VAEncMiscParameterBuffer *misc_param;
    VAEncMiscParameterVP8SegmentMapParams *frameUpdate_param;

    if ( segMapParBufId != VA_INVALID_ID)
    {
        vaDestroyBuffer(m_vaDisplay, segMapParBufId);
    }

    vaSts = vaCreateBuffer(m_vaDisplay,
                   m_vaContextEncode,
                   VAEncMiscParameterBufferType,
                   sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterVP8SegmentMapParams),
                   1,
                   NULL,
                   &segMapParBufId);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    vaSts = vaMapBuffer(m_vaDisplay,
                 segMapParBufId,
                (void **)&misc_param);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    misc_param->type = (VAEncMiscParameterType)VAEncMiscParameterTypeVP8SegmentMapParams;
    frameUpdate_param = (VAEncMiscParameterVP8SegmentMapParams *)misc_param->data;
    *frameUpdate_param = segMapParBuf;

    vaUnmapBuffer(m_vaDisplay, segMapParBufId);

    return MFX_ERR_NONE;
}

VAAPIEncoder::VAAPIEncoder()
: m_pmfxCore(NULL)
, m_vaDisplay(0)
, m_vaContextEncode(0)
, m_vaConfig(0)
, m_spsBufferId(VA_INVALID_ID)
, m_ppsBufferId(VA_INVALID_ID)
, m_qMatrixBufferId(VA_INVALID_ID)
, m_frmUpdateBufferId(VA_INVALID_ID)
, m_segMapParBufferId(VA_INVALID_ID)
, m_frameRateBufferId(VA_INVALID_ID)
, m_rateCtrlBufferId(VA_INVALID_ID)
, m_hrdBufferId(VA_INVALID_ID)
{
} // VAAPIEncoder::VAAPIEncoder(VideoCORE* core)


VAAPIEncoder::~VAAPIEncoder()
{
    Destroy();

} // VAAPIEncoder::~VAAPIEncoder()

#define MFX_CHECK_WITH_ASSERT(EXPR, ERR) { assert(EXPR); MFX_CHECK(EXPR, ERR); }

mfxStatus VAAPIEncoder::CreateAuxilliaryDevice(
    mfxCoreInterface* pCore,
    GUID /*guid*/,
    mfxU32 width,
    mfxU32 height)
{
    m_pmfxCore = pCore;

    if(pCore)
    {
        mfxStatus mfxSts = pCore->GetHandle(pCore->pthis, MFX_HANDLE_VA_DISPLAY, &m_vaDisplay);
        MFX_CHECK_STS(mfxSts);
    }

    m_width  = width;
    m_height = height;

    // set encoder CAPS on our own for now
    memset(&m_caps, 0, sizeof(m_caps));
    m_caps.MaxPicWidth = 1920;
    m_caps.MaxPicHeight = 1080;
    m_caps.HybridPakFunc = 1;
    m_caps.SegmentationAllowed = 1;
    
    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::CreateAuxilliaryDevice(VideoCORE* core, GUID guid, mfxU32 width, mfxU32 height)


mfxStatus VAAPIEncoder::CreateAccelerationService(mfxVideoParam const & par)
{
    if(0 == m_reconQueue.size())
    {
    /* We need to pass reconstructed surfaces when call vaCreateContext().
     * Here we don't have this info.
     */
        m_video = par;
        return MFX_ERR_NONE;
    }

    MFX_CHECK(m_vaDisplay, MFX_ERR_DEVICE_FAILED);
    VAStatus vaSts;

    mfxI32 entrypointsIndx = 0;
    mfxI32 numEntrypoints = vaMaxNumEntrypoints(m_vaDisplay);
    MFX_CHECK(numEntrypoints, MFX_ERR_DEVICE_FAILED);

    std::vector<VAEntrypoint> pEntrypoints(numEntrypoints);

    vaSts = vaQueryConfigEntrypoints(
                m_vaDisplay,
                VAProfileVP8Version0_3,
                Begin(pEntrypoints),
                &numEntrypoints);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    bool bEncodeEnable = false;
    for( entrypointsIndx = 0; entrypointsIndx < numEntrypoints; entrypointsIndx++ )
    {
        // [SE] VAEntrypointHybridEncSlice is entry point for Hybrid VP8 encoder        
        if( VAEntrypointHybridEncSlice == pEntrypoints[entrypointsIndx] )        
        {
            bEncodeEnable = true;
            break;
        }
    }
    if( !bEncodeEnable )
    {
        return MFX_ERR_DEVICE_FAILED;
    }

    // Configuration
    VAConfigAttrib attrib[2];

    attrib[0].type = VAConfigAttribRTFormat;
    attrib[1].type = VAConfigAttribRateControl;
    vaSts = vaGetConfigAttributes(m_vaDisplay,
                          VAProfileVP8Version0_3,
                          (VAEntrypoint)VAEntrypointHybridEncSlice,
                          &attrib[0], 2);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    if ((attrib[0].value & VA_RT_FORMAT_YUV420) == 0)
        return MFX_ERR_DEVICE_FAILED;

    mfxU8 vaRCType = ConvertRateControlMFX2VAAPI(par.mfx.RateControlMethod); // [SE] at the moment there is no BRC for VP8 on driver side

    if ((attrib[1].value & vaRCType) == 0)
        return MFX_ERR_DEVICE_FAILED;

    attrib[0].value = VA_RT_FORMAT_YUV420;
    attrib[1].value = vaRCType;

    vaSts = vaCreateConfig(
        m_vaDisplay,
        VAProfileVP8Version0_3,
        (VAEntrypoint)VAEntrypointHybridEncSlice,
        attrib,
        2,
        &m_vaConfig);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    std::vector<VASurfaceID> reconSurf;
    for(int i = 0; i < m_reconQueue.size(); i++)
        reconSurf.push_back(m_reconQueue[i].surface);

    // Encoder create
    vaSts = vaCreateContext(
        m_vaDisplay,
        m_vaConfig,
        m_width,
        m_height,
        VA_PROGRESSIVE,
        Begin(reconSurf),
        reconSurf.size(),
        &m_vaContextEncode);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    Zero(m_sps);
    Zero(m_pps);
 
    //------------------------------------------------------------------

    FillSpsBuffer(par, m_sps);
    SetHRD(par, m_vaDisplay, m_vaContextEncode, m_hrdBufferId);
    SetRateControl(par, m_vaDisplay, m_vaContextEncode, m_frameRateBufferId);
    SetFrameRate(par, m_vaDisplay, m_vaContextEncode, m_rateCtrlBufferId);

    hybridQueryBufferAttributes pfnVaQueryBufferAttr = NULL;
    pfnVaQueryBufferAttr = (hybridQueryBufferAttributes)vaGetLibFunc(m_vaDisplay, FUNC_QUERY_BUFFER_ATTRIBUTES);

    if (pfnVaQueryBufferAttr)
    {
        //VAEncMbDataLayout VaMbLayout;
        memset(&m_layout, 0, sizeof(m_layout));
        mfxU32 bufferSize = sizeof(VAEncMbDataLayout);
        vaSts = pfnVaQueryBufferAttr(
          m_vaDisplay,
          m_vaContextEncode,
          (VABufferType)VAEncMbDataBufferType,
          &m_layout,
          &bufferSize);
        MFX_CHECK_WITH_ASSERT(bufferSize == sizeof(VAEncMbDataLayout), MFX_ERR_UNDEFINED_BEHAVIOR);
        MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);
    }
    else
      return MFX_ERR_DEVICE_FAILED;

    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::CreateAccelerationService(MfxVideoParam const & par)


mfxStatus VAAPIEncoder::Reset(mfxVideoParam const & par)
{
    m_video = par;

    FillSpsBuffer(par, m_sps);
    SetHRD(par, m_vaDisplay, m_vaContextEncode, m_hrdBufferId);
    SetRateControl(par, m_vaDisplay, m_vaContextEncode, m_frameRateBufferId);
    SetFrameRate(par, m_vaDisplay, m_vaContextEncode, m_frameRateBufferId);

    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::Reset(MfxVideoParam const & par)

mfxU32 VAAPIEncoder::GetReconSurfFourCC()
{
    return MFX_FOURCC_VP8_NV12;
} // mfxU32 VAAPIEncoder::GetReconSurfFourCC()

#define HYBRIDPAK_PER_MB_DATA_SIZE     816
#define HYBRIDPAK_PER_MB_MV_DATA_SIZE  64

DWORD CalculateMbSurfaceHeight (DWORD dwNumMBs)
{
    DWORD dwMemoryNeededPerMb           = HYBRIDPAK_PER_MB_DATA_SIZE + HYBRIDPAK_PER_MB_MV_DATA_SIZE; // Both MB code & Motion vectors are in same surface
    DWORD dwMemoryAllocatedPerMb        = ALIGN((HYBRIDPAK_PER_MB_DATA_SIZE + HYBRIDPAK_PER_MB_MV_DATA_SIZE), 32); // 2D Linear Surface's picth is atleast aligned to 32 bytes
    DWORD dwBytesNeededFor4KAlignment   = 0x1000 - ((HYBRIDPAK_PER_MB_DATA_SIZE * dwNumMBs) & 0xfff); // The MV data shall start at 4k aligned offset in the MB surface
    DWORD dwExtraBytesAvailableInBuffer = (dwMemoryAllocatedPerMb  -  dwMemoryNeededPerMb) * dwNumMBs; // Extra bytes available due to pitch alignment

    if (dwBytesNeededFor4KAlignment > dwExtraBytesAvailableInBuffer)
    {
        // Increase the Surface height to cater to the 4k alignment
        dwNumMBs += ((dwBytesNeededFor4KAlignment - dwExtraBytesAvailableInBuffer) + dwMemoryAllocatedPerMb - 1) / dwMemoryAllocatedPerMb;
    }

    return dwNumMBs;
}

mfxStatus VAAPIEncoder::QueryCompBufferInfo(D3DDDIFORMAT type, mfxFrameAllocRequest& request, mfxU32 frameWidth, mfxU32 frameHeight)
{
    if (type == D3DDDIFMT_INTELENCODE_MBDATA)
    {
        mfxU32 frameSizeInMBs = (frameWidth * frameHeight) / 256;
        request.Info.FourCC = MFX_FOURCC_VP8_MBDATA; // vaCreateSurface is required to allocate surface for MB data
        request.Info.Width = ALIGN((HYBRIDPAK_PER_MB_DATA_SIZE + HYBRIDPAK_PER_MB_MV_DATA_SIZE), 32);
        request.Info.Height = CalculateMbSurfaceHeight(frameSizeInMBs + 1); // additional "+1" memory is used to return QP and loop filter levels from BRC to MSDK
    }
    else if (type == D3DDDIFMT_INTELENCODE_SEGMENTMAP)
    {
        request.Info.FourCC = MFX_FOURCC_VP8_SEGMAP;
        request.Info.Width  = ALIGN(frameWidth / 16, 64);
        request.Info.Height = frameHeight / 16;
    }

    if (type == D3DDDIFMT_INTELENCODE_DISTORTIONDATA)
    {
        return MFX_ERR_UNSUPPORTED;
    }

    // context_id required for allocation video memory (tmp solution) 
    request.reserved[0] = m_vaContextEncode;
    
    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::QueryCompBufferInfo(D3DDDIFORMAT type, mfxFrameAllocRequest& request, mfxU32 frameWidth, mfxU32 frameHeight)


mfxStatus VAAPIEncoder::QueryEncodeCaps(ENCODE_CAPS_VP8& caps)
{
    caps = m_caps;

    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::QueryEncodeCaps(ENCODE_CAPS& caps)

mfxStatus VAAPIEncoder::QueryMBLayout(MBDATA_LAYOUT & layout)
{
    layout = m_layout;

    return MFX_ERR_NONE;
}

mfxStatus VAAPIEncoder::Register(mfxFrameAllocResponse& response, D3DDDIFORMAT type)
{
    std::vector<ExtVASurface> * pQueue;
    mfxStatus sts;

    if( D3DDDIFMT_INTELENCODE_MBDATA == type
    )
    {
        pQueue = &m_mbDataQueue;
    }
    else if (D3DDDIFMT_INTELENCODE_SEGMENTMAP == type)
    {
        pQueue = &m_segMapQueue;
    }
    else
    {
        pQueue = &m_reconQueue;
    }

    {
        // we should register allocated HW bitstreams and recon surfaces
        MFX_CHECK( response.mids, MFX_ERR_NULL_PTR );

        ExtVASurface extSurf;
        VASurfaceID *pSurface = NULL;

        for (mfxU32 i = 0; i < response.NumFrameActual; i++)
        {
            sts = m_pmfxCore->FrameAllocator.GetHDL(m_pmfxCore->FrameAllocator.pthis, response.mids[i], (mfxHDL *)&pSurface);
            MFX_CHECK_STS(sts);

            extSurf.surface = *pSurface;

            pQueue->push_back( extSurf );
        }
    }

    if( D3DDDIFMT_INTELENCODE_BITSTREAMDATA != type &&
        D3DDDIFMT_INTELENCODE_MBDATA != type &&
        D3DDDIFMT_INTELENCODE_SEGMENTMAP != type)
    {
        sts = CreateAccelerationService(m_video);
        MFX_CHECK_STS(sts);
    }

    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::Register(mfxFrameAllocResponse& response, D3DDDIFORMAT type)


mfxStatus VAAPIEncoder::Register(mfxMemId memId, D3DDDIFORMAT type)
{
    memId;
    type;

    return MFX_ERR_UNSUPPORTED;

} // mfxStatus VAAPIEncoder::Register(mfxMemId memId, D3DDDIFORMAT type)

mfxStatus VAAPIEncoder::Execute(
    TaskHybridDDI const & task,
    mfxHDL          surface)
{
    VAStatus vaSts;

    VASurfaceID *inputSurface = (VASurfaceID*)surface;
    VASurfaceID reconSurface;
    VABufferID codedBuffer;
    mfxU32 i;    

    std::vector<VABufferID> configBuffers;
    configBuffers.resize(MAX_CONFIG_BUFFERS_COUNT);
    mfxU16 buffersCount = 0;

    // update params
    FillPpsBuffer(task, m_video, m_pps, m_reconQueue);
    FillQuantBuffer(task, m_video, m_quant);
    FillSegMap(task, m_video, m_pmfxCore, m_segMapPar);
    FillFrameUpdateBuffer(task, m_frmUpdate);

//===============================================================================================    

    //------------------------------------------------------------------
    // find bitstream    
    mfxU32 idxInPool = task.m_pRecFrame->idInPool;    
    if( idxInPool < m_mbDataQueue.size() )
    {
        codedBuffer = m_mbDataQueue[idxInPool].surface;
    }
    else
    {
        return MFX_ERR_UNKNOWN;
    }

    // [SE] should we recieve MB data through coded_buf?
    m_pps.coded_buf = codedBuffer;

    //------------------------------------------------------------------
    // buffer creation & configuration
    //------------------------------------------------------------------
    {
        // 1. sequence level
        {
            MFX_DESTROY_VABUFFER(m_spsBufferId, m_vaDisplay);
            vaSts = vaCreateBuffer(m_vaDisplay,
                                   m_vaContextEncode,
                                   VAEncSequenceParameterBufferType,
                                   sizeof(m_sps),
                                   1,
                                   &m_sps,
                                   &m_spsBufferId);
            MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

            configBuffers[buffersCount++] = m_spsBufferId;
        }

        // 2. Picture level
        {
            MFX_DESTROY_VABUFFER(m_ppsBufferId, m_vaDisplay);
            vaSts = vaCreateBuffer(m_vaDisplay,
                                   m_vaContextEncode,
                                   VAEncPictureParameterBufferType,
                                   sizeof(m_pps),
                                   1,
                                   &m_pps,
                                   &m_ppsBufferId);
            MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

            configBuffers[buffersCount++] = m_ppsBufferId;
        }

        // 3. Quantization matrix
        {
            MFX_DESTROY_VABUFFER(m_qMatrixBufferId, m_vaDisplay);
            vaSts = vaCreateBuffer(m_vaDisplay,
                                   m_vaContextEncode,
                                   VAQMatrixBufferType,
                                   sizeof(m_quant),
                                   1,
                                   &m_quant,
                                   &m_qMatrixBufferId);
            MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

            configBuffers[buffersCount++] = m_qMatrixBufferId;
        }

        // 4. Segmentation map
        if (task.ddi_frames.m_pSegMap_hw != 0)
        {
            // segmentation map buffer is already allocated and filled. Need just to attach it
            configBuffers[buffersCount++] = m_segMapQueue[idxInPool].surface;
        }

        // 5. Per-segment parameters
        if (task.m_frameOrder)
        {
            vaSts = SendMiscBufferSegMapPar(m_vaDisplay,
                                            m_vaContextEncode,
                                            m_segMapPar,
                                            m_segMapParBufferId);
            MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);
            configBuffers[buffersCount++] = m_segMapParBufferId;
        }

        // 6. Frame update
        if (task.m_frameOrder)
        {
            vaSts = SendMiscBufferFrameUpdate(m_vaDisplay,
                                              m_vaContextEncode,
                                              m_frmUpdate,
                                              m_frmUpdateBufferId);
            MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);
            configBuffers[buffersCount++] = m_frmUpdateBufferId;
        }

        // 7. hrd parameters
        configBuffers[buffersCount++] = m_hrdBufferId;
        // 8. RC parameters
        configBuffers[buffersCount++] = m_rateCtrlBufferId;
        // 9. frame rate
        configBuffers[buffersCount++] = m_frameRateBufferId;
    }

    assert(buffersCount <= configBuffers.size());

    //------------------------------------------------------------------
    // Rendering
    //------------------------------------------------------------------
    {
        vaSts = vaBeginPicture(
            m_vaDisplay,
            m_vaContextEncode,
            *inputSurface);
        MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);
    }
    {
        vaSts = vaRenderPicture(
            m_vaDisplay,
            m_vaContextEncode,
            Begin(configBuffers),
            buffersCount);
        MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);
    }
    {
        vaSts = vaEndPicture(m_vaDisplay, m_vaContextEncode);
        MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);
    }

    //------------------------------------------------------------------
    // PostStage
    //------------------------------------------------------------------
    // put to cache
    {
        UMC::AutomaticUMCMutex guard(m_guard);
        ExtVASurface currentFeedback;
        currentFeedback.surface = *inputSurface;
        currentFeedback.number = task.m_frameOrder;
        currentFeedback.idxBs    = idxInPool;
        m_feedbackCache.push_back( currentFeedback );
    }

    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::Execute(ExecuteBuffers& data, mfxU32 fieldId)


mfxStatus VAAPIEncoder::QueryStatus(
    Task & task)
{
    VAStatus vaSts;

    //------------------------------------------
    // (1) mapping feedbackNumber -> surface & mb data buffer
    bool isFound = false;
    VASurfaceID waitSurface;
    mfxU32 waitIdxBs;
    mfxU32 indxSurf;

    {
        UMC::AutomaticUMCMutex guard(m_guard);
        for( indxSurf = 0; indxSurf < m_feedbackCache.size(); indxSurf++ )
        {
            ExtVASurface currentFeedback = m_feedbackCache[ indxSurf ];

            if( currentFeedback.number == task.m_frameOrder)
            {
                waitSurface = currentFeedback.surface;
                waitIdxBs   = currentFeedback.idxBs;

                isFound  = true;

                break;
            }
        }
    }

    if( !isFound )
    {
        return MFX_ERR_UNKNOWN;
    }

    // find used mb data buffer
    VABufferID codedBuffer;
    if( waitIdxBs < m_mbDataQueue.size())
    {
        codedBuffer = m_mbDataQueue[waitIdxBs].surface;
    }
    else
    {
        return MFX_ERR_UNKNOWN;
    }
#if defined(SYNCHRONIZATION_BY_VA_SYNC_SURFACE)
    vaSts = vaSyncSurface(m_vaDisplay, waitSurface);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    {
        UMC::AutomaticUMCMutex guard(m_guard);
        m_feedbackCache.erase( m_feedbackCache.begin() + indxSurf );
    }

    return MFX_ERR_NONE;
#else
    VASurfaceStatus surfSts = VASurfaceSkipped;

    vaSts = vaQuerySurfaceStatus(m_vaDisplay, waitSurface, &surfSts);
    MFX_CHECK_WITH_ASSERT(VA_STATUS_SUCCESS == vaSts, MFX_ERR_DEVICE_FAILED);

    switch (surfSts)
    {
        case VASurfaceReady:
            // remove task
            m_feedbackCache.erase( m_feedbackCache.begin() + indxSurf );
            return MFX_ERR_NONE;

        case VASurfaceRendering:
        case VASurfaceDisplaying:
            return MFX_WRN_DEVICE_BUSY;

        case VASurfaceSkipped:
        default:
            assert(!"bad feedback status");
            return MFX_ERR_DEVICE_FAILED;
    }
#endif
} // mfxStatus VAAPIEncoder::QueryStatus(mfxU32 feedbackNumber, mfxU32& bytesWritten)


mfxStatus VAAPIEncoder::Destroy()
{
    if( m_vaContextEncode )
    {
        vaDestroyContext( m_vaDisplay, m_vaContextEncode );
        m_vaContextEncode = 0;
    }

    if( m_vaConfig )
    {
        vaDestroyConfig( m_vaDisplay, m_vaConfig );
        m_vaConfig = 0;
    }

    MFX_DESTROY_VABUFFER(m_spsBufferId, m_vaDisplay);    
    MFX_DESTROY_VABUFFER(m_ppsBufferId, m_vaDisplay);
    MFX_DESTROY_VABUFFER(m_qMatrixBufferId, m_vaDisplay);
    MFX_DESTROY_VABUFFER(m_segMapParBufferId, m_vaDisplay);
    MFX_DESTROY_VABUFFER(m_frmUpdateBufferId, m_vaDisplay);
    MFX_DESTROY_VABUFFER(m_frameRateBufferId, m_vaDisplay);
    MFX_DESTROY_VABUFFER(m_rateCtrlBufferId, m_vaDisplay);
    MFX_DESTROY_VABUFFER(m_hrdBufferId, m_vaDisplay);

    return MFX_ERR_NONE;

} // mfxStatus VAAPIEncoder::Destroy()
#endif
}

#endif 
