/* ****************************************************************************** *\

Copyright (C) 2008-2014 Intel Corporation.  All rights reserved.

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

File Name: mfx_enc_common.cpp

\* ****************************************************************************** */

#include "umc_defs.h"
#include "ipps.h"

#include <math.h>
#include <algorithm>
#include <limits.h>
#include "mfx_enc_common.h"
#include "mfx_utils.h"

#include "vm_debug.h"
//----MFX data -> UMC data--------------------------------------------
Ipp8u CalculateMAXBFrames (mfxU8 GopRefDist)
{
    if(GopRefDist)
        return (GopRefDist - 1);
    else
        return 0;
}

Ipp16u CalculateUMCGOPLength (mfxU16 GOPSize, mfxU8 targetUsage)
{
    if(GOPSize)
        return (GOPSize);
    else
        switch (targetUsage)
        {
        case MFX_TARGETUSAGE_BEST_QUALITY:
            return 200;
        case 2:
            return 200;
        case 3:
             return 200;
        case 4:
            return 20;
        case 5:
            return 10;
        case 6:
            return 4;
        case MFX_TARGETUSAGE_BEST_SPEED:
            return 1;
        }
        return 1;
}
bool SetUFParameters(mfxU8 TargetUsages, bool& mixed,Ipp32u& twoRef )
{
    switch (TargetUsages)
    {
    case MFX_TARGETUSAGE_BEST_QUALITY:
        mixed=0;twoRef=2;
        break;
    case 2:
        mixed=0;twoRef=2;
        break;
    case 3:
        mixed=0;twoRef=2;
        break;
    case 4:
        mixed=0;twoRef=1;
        break;
    case 5:
        mixed=0;twoRef=1;
        break;
    case 6:
        mixed=0;twoRef=1;
        break;
    case MFX_TARGETUSAGE_BEST_SPEED:
        mixed=0;twoRef=1;
        break;
    default:
        return false;
    }
    return true;
}
bool SetPROParameters (mfxU8 TargetUsages,Ipp8u &MESpeed, bool &UseFB, bool &FastFB, bool &bIntensityCompensation,
                       bool &bChangeInterpolationType, bool &bChangeVLCTables,bool &bTrellisQuantization, bool &bUsePadding,
                       bool &bVSTransform, bool &deblocking, mfxU8 &smoothing, bool &fastUVMC)
{
    switch (TargetUsages)
    {
    case MFX_TARGETUSAGE_BEST_QUALITY:
        MESpeed=25; UseFB=1; FastFB=0; bIntensityCompensation=0;
        bChangeInterpolationType=1;bTrellisQuantization = 1;
        bUsePadding  = 1; bChangeVLCTables =1;
        bVSTransform = 1; deblocking = 1;  smoothing = 0; fastUVMC = 0;
        break;
    case 2:
        MESpeed=25; UseFB=1; FastFB=0; bIntensityCompensation=0;
        bChangeInterpolationType=1;bTrellisQuantization = 1;
        bUsePadding  = 1; bChangeVLCTables =1;
        bVSTransform = 0; deblocking = 1;  smoothing = 0; fastUVMC = 0;
        break;
    case 3:
        MESpeed=25; UseFB=1; FastFB=0; bIntensityCompensation=0;
        bChangeInterpolationType=0;bTrellisQuantization = 0;
        bUsePadding  = 1; bChangeVLCTables =0;
        bVSTransform = 0; deblocking = 1;  smoothing = 0; fastUVMC = 0;
        break;
    case 4:
        MESpeed=25; UseFB=1; FastFB=0; bIntensityCompensation=0;
        bChangeInterpolationType=0;bTrellisQuantization = 0;
        bUsePadding  = 1; bChangeVLCTables =0;
        bVSTransform = 0; deblocking = 1;  smoothing = 0; fastUVMC = 0;
        break;
    case 5:
        MESpeed=25; UseFB=0; FastFB=0; bIntensityCompensation=0;
        bChangeInterpolationType=0;bTrellisQuantization = 0;
        bUsePadding  = 1; bChangeVLCTables =0;
        bVSTransform = 0; deblocking = 1;  smoothing = 0; fastUVMC = 0;
        break;
    case 6:
        MESpeed=25; UseFB=0; FastFB=0; bIntensityCompensation=0;
        bChangeInterpolationType=0;bTrellisQuantization = 0;
        bUsePadding  = 0; bChangeVLCTables =0;
        bVSTransform = 0; deblocking = 1;  smoothing = 0; fastUVMC = 1;
        break;
    case MFX_TARGETUSAGE_BEST_SPEED:
        MESpeed=25; UseFB=0; FastFB=0; bIntensityCompensation=0;
        bChangeInterpolationType=0;bTrellisQuantization = 0;
        bUsePadding  = 0; bChangeVLCTables =0;
        bVSTransform = 0; deblocking = 0;  smoothing = 0; fastUVMC = 1;
        break;
    default:
        return false;
    }
    return true;
}


Ipp32u CalculateUMCBitrate(mfxU16    TargetKbps)
{
    return TargetKbps*1000;
}

Ipp64f CalculateUMCFramerate( mfxU32 FrameRateExtN, mfxU32 FrameRateExtD)
{
    if (FrameRateExtN && FrameRateExtD)
        return (Ipp64f)FrameRateExtN / FrameRateExtD;
    else
        return 0;
}

void CalculateMFXFramerate(Ipp64f framerate, mfxU32* FrameRateExtN, mfxU32* FrameRateExtD)
{
  mfxU32 fr;
  if (!FrameRateExtN || !FrameRateExtD)
    return;

  fr = (mfxU32)(framerate + .5);
  if (fabs(fr - framerate) < 0.0001) {
    *FrameRateExtN = fr;
    *FrameRateExtD = 1;
    return;
  }

  fr = (mfxU32)(framerate * 1.001 + .5);
  if (fabs(fr * 1000 - framerate * 1001) < 10) {
    *FrameRateExtN = fr * 1000;
    *FrameRateExtD = 1001;
    return;
  }
  // can do more

  *FrameRateExtN = (mfxU32)(framerate * 10000 + .5);
  *FrameRateExtD = 10000;
  return;
}


UMC::FrameType GetFrameType (mfxU16 FrameOrder, mfxInfoMFX* info)
{
  mfxU16 GOPSize = info->GopPicSize;
  mfxU16 IPDist = info->GopRefDist;
  mfxU16 pos = (GOPSize)? FrameOrder %(GOPSize):FrameOrder;


  if (pos == 0 || IPDist == 0)
  {
      return UMC::I_PICTURE;
  }
  else
  {
      pos = pos % (IPDist);
      return (pos != 0) ? UMC::B_PICTURE : UMC::P_PICTURE;
  }
}


//----UMC data -> MFX data--------------------------------------------

mfxU16 CalculateMFXGOPLength (Ipp16u GOPSize)
{
    if(!GOPSize)
        return GOPSize;
    else
        return 15;
}

mfxU8 CalculateGopRefDist(mfxU8 BNum)
{
    return (BNum + 1);
}

mfxU32 TranslateMfxFRCodeMPEG2(mfxFrameInfo *info, mfxU32 *codeN, mfxU32* codeD)
{
  mfxU32 n = info->FrameRateExtN, d = info->FrameRateExtD, code = 0;
  mfxU32 flag1001 = 0;

  if (!n || !d) return 0;
  if(d%1001 == 0 && n%1000 == 0) {
    n /= 1000;
    d /= 1001;
    flag1001 = 1;
  }
  switch(n) {
    case 48: case 72: case 96:
    case 24 : code = 2 - flag1001; n /= 24; break;
    case 90:
    case 30 : code = 5 - flag1001; n /= 30; break;
    case 120: case 180: case 240:
    case 60 : code = 8 - flag1001; n /= 60; break;
    case 75:
    case 25 : code = 3; n /= 25; break;
    case 100: case 150: case 200:
    case 50 : code = 6; n /= 50; break;
    default: code = 0;
  }
  if( code != 0 && d <= 0x20 &&
    ((code !=3 && code != 6) || !flag1001) ) {
      *codeN = n-1;
      *codeD = d-1;
      return code;
  }
  // can do more
  return 0;
}

mfxExtBuffer* GetExtBuffer(mfxExtBuffer** ebuffers, mfxU32 nbuffers, mfxU32 BufferId)
{
    if (!ebuffers) return 0;
    for(mfxU32 i=0; i<nbuffers; i++) {
        if (!ebuffers[i]) continue;
        if (ebuffers[i]->BufferId == BufferId) {
            return ebuffers[i];
        }
    }
    return 0;
}

mfxExtCodingOption* GetExtCodingOptions(mfxExtBuffer** ebuffers, mfxU32 nbuffers)
{
    return (mfxExtCodingOption*)GetExtBuffer(ebuffers, nbuffers, MFX_EXTBUFF_CODING_OPTION);
}

mfxExtVideoSignalInfo* GetExtVideoSignalInfo(mfxExtBuffer** ebuffers, mfxU32 nbuffers)
{
    return (mfxExtVideoSignalInfo *)GetExtBuffer(ebuffers, nbuffers, MFX_EXTBUFF_VIDEO_SIGNAL_INFO);
}

//----------work with marker-----------------------------
mfxStatus SetFrameLockMarker(mfxFrameData* pFrame, mfxU8 LockMarker)
{
    MFX_CHECK_NULL_PTR1(pFrame);

    pFrame->Locked = LockMarker;

    return MFX_ERR_NONE;
}

mfxU8 GetFrameLockMarker(mfxFrameData* pFrame)
{
    return (mfxU8)pFrame->Locked;
}


/*mfxI16  GetExBufferIndex (mfxFrameCUC *cuc, mfxU32 cucID)
{
    if (!cuc->ExtBuffer)
        return -1;

    for (mfxI32 i = 0; i < cuc->NumExtBuffer; i ++)
    {
        if ((cuc->ExtBuffer[i]!=0) &&(((mfxU32*)(cuc->ExtBuffer[i]))[0] == cucID))
        {
            return (mfxI16)i;
        }
    }

    return -1;
}*/

mfxStatus CopyFrame(mfxFrameData *pIn, mfxFrameData *pOut, mfxFrameInfo *pInfoIn, mfxFrameInfo *pInfoOut)
{
    mfxStatus sts = MFX_ERR_NONE;
    if (pInfoOut)
    {
        if (pInfoIn->FourCC != pInfoOut->FourCC ||
            pInfoIn->Width   > pInfoOut->Width  ||
            pInfoIn->Height  > pInfoOut->Height)

            return MFX_ERR_UNSUPPORTED;
    }
    if (pInfoIn->FourCC == MFX_FOURCC_YV12)
    {
        if (pIn ->Y == 0 || pIn ->U == 0 || pIn ->V == 0 ||
            pOut->Y == 0 || pOut->U == 0 || pOut->V == 0)
        {
            return MFX_ERR_NULL_PTR;
        }
        mfxU8* p_in  = pIn ->Y;
        mfxU8* p_out = pOut ->Y;
        mfxU32 h     = pInfoIn->Height;
        mfxU32 w     = pInfoIn->Width;
        mfxU32 s_in  = pIn->PitchLow + ((mfxU32)pIn->PitchHigh << 16);
        mfxU32 s_out = pOut->PitchLow + ((mfxU32)pOut->PitchHigh << 16);

        for (mfxU32 i = 0 ; i < h; i++)
        {
            MFX_INTERNAL_CPY(p_out,p_in,w);
            p_in += s_in;
            p_out+= s_out;        
        }
        p_in  = pIn ->U;
        p_out = pOut->U;

        w     >>= 1;
        h     >>= 1;
        s_in  >>= 1;
        s_out >>= 1;

        for (mfxU32 i = 0 ; i < h; i++)
        {
            MFX_INTERNAL_CPY(p_out,p_in,w);
            p_in += s_in;
            p_out+= s_out;        
        }
        p_in  = pIn ->V;
        p_out = pOut->V;
        for (mfxU32 i = 0 ; i < h; i++)
        {
            MFX_INTERNAL_CPY(p_out,p_in,w);
            p_in += s_in;
            p_out+= s_out;        
        }
    }
    else if (pInfoIn->FourCC == MFX_FOURCC_NV12)
    {
        if (pIn ->Y == 0 || pIn ->UV == 0|| 
            pOut->Y == 0 || pOut->UV == 0 )
        {
            return MFX_ERR_NULL_PTR;
        }
        mfxU8* p_in  = pIn ->Y;
        mfxU8* p_out = pOut ->Y;
        mfxU32 h     = pInfoIn->Height;
        mfxU32 w     = pInfoIn->Width;
        mfxU32 s_in  = pIn->PitchLow + ((mfxU32)pIn->PitchHigh << 16);
        mfxU32 s_out = pOut->PitchLow + ((mfxU32)pOut->PitchHigh << 16);

        for (mfxU32 i = 0 ; i < h; i++)
        {
            MFX_INTERNAL_CPY(p_out,p_in,w);
            p_in += s_in;
            p_out+= s_out;        
        }
        p_in  = pIn ->UV;
        p_out = pOut->UV;
        h     >>= 1;
        for (mfxU32 i = 0 ; i < h; i++)
        {
            MFX_INTERNAL_CPY(p_out,p_in,w);
            p_in += s_in;
            p_out+= s_out;        
        }   
    }
    else
    {
        return MFX_ERR_UNSUPPORTED;
    }
    return sts;
}

/*
mfxI16  GetExBufferIndex (mfxVideoParam *par, mfxU32 cucID)
{
    mfxI16 index = -1;
    if (par->mfx.ExtBuffer)
    {
        for (mfxI32 i=0; i < par->mfx.NumExtBuffer; i ++)
        {
            if (((mfxU32*)(par->mfx.ExtBuffer[i]))[0] == cucID)
            {
                index = i;
                break;
            }
        }
    }
    return index;
}
*/

const Rational RATETAB[8]=
{
    {24000, 1001}, 
    {   24,    1}, 
    {   25,    1}, 
    {30000, 1001}, 
    {   30,    1}, 
    {   50,    1}, 
    {60000, 1001}, 
    {   60,    1},
};

static const Rational SORTED_RATIO[] =
{
    {1,32},{1,31},{1,30},{1,29},{1,28},{1,27},{1,26},{1,25},{1,24},{1,23},{1,22},{1,21},{1,20},{1,19},{1,18},{1,17},
    {1,16},{2,31},{1,15},{2,29},{1,14},{2,27},{1,13},{2,25},{1,12},{2,23},{1,11},{3,32},{2,21},{3,31},{1,10},{3,29},
    {2,19},{3,28},{1, 9},{3,26},{2,17},{3,25},{1, 8},{4,31},{3,23},{2,15},{3,22},{4,29},{1, 7},{4,27},{3,20},{2,13},
    {3,19},{4,25},{1, 6},{4,23},{3,17},{2,11},{3,16},{4,21},{1, 5},{4,19},{3,14},{2, 9},{3,13},{4,17},{1, 4},{4,15},
    {3,11},{2, 7},{3,10},{4,13},{1, 3},{4,11},{3, 8},{2, 5},{3, 7},{4, 9},{1, 2},{4, 7},{3, 5},{2, 3},{3, 4},{4, 5},
    {1,1},{4,3},{3,2},{2,1},{3,1},{4,1}
};

class FR_Compare
{
public:    

    bool operator () (Rational rfirst, Rational rsecond)
    {
        mfxF64 first = (mfxF64) rfirst.n/rfirst.d;
        mfxF64 second = (mfxF64) rsecond.n/rsecond.d;     
        return ( first < second );
    }
};


void ConvertFrameRateMPEG2(mfxU32 FrameRateExtD, mfxU32 FrameRateExtN, mfxI32 &frame_rate_code, mfxI32 &frame_rate_extension_n, mfxI32 &frame_rate_extension_d)
{
    Rational convertedFR;
    Rational bestFR = {INT_MAX, 1};
    mfxF64 minDifference = IPP_MAXABS_64F;

    for (mfxU32 i = 0; i < sizeof(RATETAB) / sizeof(RATETAB[0]); i++)
    {        
        if (FrameRateExtN * RATETAB[i].d == FrameRateExtD * RATETAB[i].n )
        {
            frame_rate_code = i + 1;      
            frame_rate_extension_n = 0;
            frame_rate_extension_d = 0;

            return;
        }
    }

    for (mfxU32 i = 0; i < sizeof(RATETAB) / sizeof(RATETAB[0]); i++)
    {        
        convertedFR.n = (mfxI64) FrameRateExtN * RATETAB[i].d;
        convertedFR.d = (mfxI64) FrameRateExtD * RATETAB[i].n;

        const Rational* it_lower = std::lower_bound(
            SORTED_RATIO,
            SORTED_RATIO + sizeof(SORTED_RATIO) / sizeof(SORTED_RATIO[0]),
            convertedFR,
            FR_Compare());

        if (it_lower == SORTED_RATIO + sizeof(SORTED_RATIO) / sizeof(SORTED_RATIO[0]))  // exceed max FR
        {
            it_lower--;            
        }
        else if (it_lower != SORTED_RATIO) // min FR not reached
        {            
            if ( abs( (mfxF64) (it_lower - 1)->n / (it_lower - 1)->d - (mfxF64) convertedFR.n / convertedFR.d) <
                abs( (mfxF64) it_lower->n / it_lower->d  - (mfxF64) convertedFR.n / convertedFR.d) )
            {
                it_lower--;
            }
        }

        if ( minDifference > abs((mfxF64) convertedFR.n / convertedFR.d  - (mfxF64)it_lower->n / it_lower->d))

        {
            minDifference = abs((mfxF64) convertedFR.n / convertedFR.d - (mfxF64)it_lower->n / it_lower->d);            
            frame_rate_code = i + 1;
            bestFR = *it_lower;
        } 
    }

    for (mfxU32 i = 0; i < sizeof(RATETAB) / sizeof(RATETAB[0]); i++)
    {// check that exist equal RATETAB with FR = 1:1        
        if (RATETAB[i].d * RATETAB[frame_rate_code - 1].n * bestFR.n == RATETAB[i].n * RATETAB[frame_rate_code - 1].d * bestFR.d)
        {
            frame_rate_code = i + 1;      
            frame_rate_extension_n = 0;
            frame_rate_extension_d = 0;
            return;
        }
    }

    frame_rate_extension_n = (mfxI32) bestFR.n - 1;
    frame_rate_extension_d = (mfxI32) bestFR.d - 1;
    return;
}

namespace
{
#if defined(MFX_VA_WIN)
    void ConvertFrameRateMPEG2_test()
    {

        int pass  = 0;
        int fail  = 0;
        int total = 0;
        mfxU32 FrameRateExtD;
        mfxU32 FrameRateExtN;
        mfxI32 frame_rate_code;
        mfxI32 frame_rate_extension_n;
        mfxI32 frame_rate_extension_d;
        Rational CurrentFR;    
        Rational ChangedFR;    
        const Rational deltaFR = {3, 10000};
        for (int i = 0; i < sizeof(RATETAB) / sizeof(RATETAB[0]); i++)
        {     
            for(int j = 0; j < sizeof(SORTED_RATIO) / sizeof(SORTED_RATIO[0]); j++ )
            {

                CurrentFR.n = RATETAB[i].n * SORTED_RATIO[j].n;
                CurrentFR.d = RATETAB[i].d * SORTED_RATIO[j].d;            
                /////// Exact

                ConvertFrameRateMPEG2((mfxI32) CurrentFR.d, (mfxI32) CurrentFR.n, frame_rate_code, frame_rate_extension_n, frame_rate_extension_d);
                FrameRateExtD = (mfxU32) ((frame_rate_extension_d + 1) * RATETAB[frame_rate_code - 1].d);
                FrameRateExtN = (mfxU32) ((frame_rate_extension_n + 1) * RATETAB[frame_rate_code - 1].n);

                if ( ((SORTED_RATIO[j].n == 1 && SORTED_RATIO[j].d == 1) && (FrameRateExtN == CurrentFR.n && FrameRateExtD == CurrentFR.d)) ||
                    ((SORTED_RATIO[j].n != 1 || SORTED_RATIO[j].d != 1) && (FrameRateExtN  * CurrentFR.d == FrameRateExtD  * CurrentFR.n)) )
                {
                    pass++;
                    total++;            
                }
                else
                {
                    fail++;
                    total++;
                    printf("\n! FAILED in exact_test:\n");
                    printf("SORTED_RATIO  = %I64d / %I64d\n", SORTED_RATIO[j].n, SORTED_RATIO[j].d);                
                    printf("RATETAB       = %I64d / %I64d\n", RATETAB[i].n, RATETAB[i].d);                                
                    printf("CurrentFR     = %I64d / %I64d\n", CurrentFR.n, CurrentFR.d);
                    printf("deltaFR       = %I64d / %I64d\n", deltaFR.n, deltaFR.d);                    
                    printf("RATIO_chng    = %I64d / %I64d\n", (mfxI64)(frame_rate_extension_n + 1), (mfxI64)(frame_rate_extension_d + 1));
                    printf("RATETAB_chng  = %I64d / %I64d\n", RATETAB[frame_rate_code - 1].n, RATETAB[frame_rate_code - 1].d);                
                    printf("FrameRateExt  = %I64d / %I64d\n", (mfxI64)FrameRateExtN, (mfxI64)FrameRateExtD);
                    printf("\n");
                }

                /////// minus
                CurrentFR.n = RATETAB[i].n * SORTED_RATIO[j].n;
                CurrentFR.d = RATETAB[i].d * SORTED_RATIO[j].d;            
                if (RATETAB[i].d == 1001)
                {
                    ChangedFR.n = CurrentFR.n * deltaFR.d / 1000 - deltaFR.n;
                    ChangedFR.d = CurrentFR.d * deltaFR.d / 1000;                    

                }
                else
                {
                    ChangedFR.n = CurrentFR.n * deltaFR.d - deltaFR.n * CurrentFR.d;
                    ChangedFR.d = CurrentFR.d * deltaFR.d;
                }

                ConvertFrameRateMPEG2((mfxI32) ChangedFR.d, (mfxI32) ChangedFR.n, frame_rate_code, frame_rate_extension_n, frame_rate_extension_d);
                FrameRateExtD = (mfxU32) ((frame_rate_extension_d + 1) * RATETAB[frame_rate_code - 1].d);
                FrameRateExtN = (mfxU32) ((frame_rate_extension_n + 1) * RATETAB[frame_rate_code - 1].n);

                if ( ((SORTED_RATIO[j].n == 1 && SORTED_RATIO[j].d == 1) && (FrameRateExtN == CurrentFR.n && FrameRateExtD == CurrentFR.d)) ||
                    ((SORTED_RATIO[j].n != 1 || SORTED_RATIO[j].d != 1) && (FrameRateExtN  * CurrentFR.d == FrameRateExtD  * CurrentFR.n)) )
                {
                    pass++;
                    total++;            
                }
                else
                {
                    fail++;
                    total++;
                    printf("\n! FAILED in minus_test:\n");
                    printf("SORTED_RATIO  = %I64d / %I64d\n", SORTED_RATIO[j].n, SORTED_RATIO[j].d);                
                    printf("RATETAB       = %I64d / %I64d\n", RATETAB[i].n, RATETAB[i].d);                                
                    printf("CurrentFR     = %I64d / %I64d\n", CurrentFR.n, CurrentFR.d);
                    printf("deltaFR       = %I64d / %I64d\n", deltaFR.n, deltaFR.d);
                    printf("ChangedFR     = %I64d / %I64d\n", ChangedFR.n, ChangedFR.d);  
                    printf("frame_rate    = %I64d / %I64d\n", (mfxI64)(frame_rate_extension_n + 1), (mfxI64)(frame_rate_extension_d + 1));
                    printf("RATETAB_chng  = %I64d / %I64d\n", RATETAB[frame_rate_code - 1].n, RATETAB[frame_rate_code - 1].d);                
                    printf("FrameRateExt  = %I64d / %I64d\n", (mfxI64)FrameRateExtN, (mfxI64)FrameRateExtD);
                    printf("\n");
                }

                /////// plus
                CurrentFR.n = RATETAB[i].n * SORTED_RATIO[j].n;
                CurrentFR.d = RATETAB[i].d * SORTED_RATIO[j].d;            
                if (RATETAB[i].d == 1001)
                {
                    ChangedFR.n = CurrentFR.n * deltaFR.d / 1000 + deltaFR.n;
                    ChangedFR.d = CurrentFR.d * deltaFR.d / 1000;                    
                }
                else
                {
                    ChangedFR.n = CurrentFR.n * deltaFR.d + deltaFR.n * CurrentFR.d;
                    ChangedFR.d = CurrentFR.d * deltaFR.d;
                }

                ConvertFrameRateMPEG2((mfxI32) ChangedFR.d, (mfxI32) ChangedFR.n, frame_rate_code, frame_rate_extension_n, frame_rate_extension_d);
                FrameRateExtD = (mfxU32) ((frame_rate_extension_d + 1) * RATETAB[frame_rate_code - 1].d);
                FrameRateExtN = (mfxU32) ((frame_rate_extension_n + 1) * RATETAB[frame_rate_code - 1].n);

                if ( ((SORTED_RATIO[j].n == 1 && SORTED_RATIO[j].d == 1) && (FrameRateExtN == CurrentFR.n && FrameRateExtD == CurrentFR.d)) ||
                    ((SORTED_RATIO[j].n != 1 || SORTED_RATIO[j].d != 1) && (FrameRateExtN  * CurrentFR.d == FrameRateExtD  * CurrentFR.n)) )
                {
                    pass++;
                    total++;            
                }
                else
                {
                    fail++;
                    total++;
                    printf("\n! FAILED in plus_test:\n");
                    printf("SORTED_RATIO  = %I64d / %I64d\n", SORTED_RATIO[j].n, SORTED_RATIO[j].d);                
                    printf("RATETAB       = %I64d / %I64d\n", RATETAB[i].n, RATETAB[i].d);                                
                    printf("CurrentFR     = %I64d / %I64d\n", CurrentFR.n, CurrentFR.d);
                    printf("deltaFR       = %I64d / %I64d\n", deltaFR.n, deltaFR.d);
                    printf("ChangedFR     = %I64d / %I64d\n", ChangedFR.n, ChangedFR.d);  
                    printf("frame_rate    = %I64d / %I64d\n", (mfxI64)(frame_rate_extension_n + 1), (mfxI64)(frame_rate_extension_d + 1));
                    printf("RATETAB_chng  = %I64d / %I64d\n", RATETAB[frame_rate_code - 1].n, RATETAB[frame_rate_code - 1].d);                
                    printf("FrameRateExt  = %I64d / %I64d\n", (mfxI64)FrameRateExtN, (mfxI64)FrameRateExtD);
                    printf("\n");
                }

            }

        }
        printf("ConvertFrameRateMPEG2_test Complete:\n");
        printf("Pass       : %d\n", pass);
        printf("Fail       : %d\n", fail);
        printf("Total tests: %d\n", total); 

    }
#endif
}

mfxStatus CheckFrameRateMPEG2(mfxU32 &FrameRateExtD, mfxU32 &FrameRateExtN)
{
    mfxI32 frame_rate_code        = 0;
    mfxI32 frame_rate_extension_n = 0;
    mfxI32 frame_rate_extension_d = 0;
    mfxF64 input_ratio = (mfxF64) FrameRateExtN / FrameRateExtD;
    mfxF64 difference_ratio = 0;

    ConvertFrameRateMPEG2(FrameRateExtD, FrameRateExtN, frame_rate_code, frame_rate_extension_n, frame_rate_extension_d);
    difference_ratio =  fabs(input_ratio - (mfxF64)(frame_rate_extension_n + 1) / (frame_rate_extension_d + 1) * RATETAB[frame_rate_code - 1].n / RATETAB[frame_rate_code - 1].d);    

    if (difference_ratio < input_ratio / 50000)
    { //difference less than 0.05%
       return MFX_ERR_NONE;
    }
    else 
    {
       FrameRateExtD = (mfxU32) ((frame_rate_extension_d + 1) * RATETAB[frame_rate_code - 1].d);
       FrameRateExtN = (mfxU32) ((frame_rate_extension_n + 1) * RATETAB[frame_rate_code - 1].n);

       if (difference_ratio < input_ratio / 1000)
           return MFX_WRN_INCOMPATIBLE_VIDEO_PARAM;
       else //difference more than 0.1%
           return MFX_ERR_INVALID_VIDEO_PARAM;
    }
}
mfxStatus CheckAspectRatioMPEG2 (mfxU16 &aspectRatioW, mfxU16 &aspectRatioH, mfxU32 frame_width, mfxU32 frame_heigth, mfxU16 cropW, mfxU16 cropH)
{
    mfxU32 width = (cropW != 0) ? cropW : frame_width;
    mfxU32 height =(cropH != 0) ? cropH : frame_heigth;

    if ((aspectRatioW == 0 && aspectRatioH == 0) || (aspectRatioW == 1 && aspectRatioH == 1))
        return MFX_ERR_NONE;
    if (aspectRatioW == 0 || aspectRatioH == 0)
        return MFX_ERR_INVALID_VIDEO_PARAM;
    if (width != 0 && height != 0)
    {
        mfxU64 k   = ((mfxU64)aspectRatioW * width * 100000) / (aspectRatioH * height);

        if ((aspectRatioW * width * 3 == aspectRatioH * height * 4) ||
            (aspectRatioW * width * 9 == aspectRatioH * height * 16) ||
            (aspectRatioW * width * 100 == aspectRatioH * height * 221) )
        {
            return MFX_ERR_NONE;
        }

        if (k > (400000/3 - 133) && k < (400000/3 + 133))
        {
            return MFX_ERR_NONE;
        } 
        else if ( k > ( 1600000/9 - 177) && k < (1600000/9 + 177) )
        {
            return MFX_ERR_NONE;
        } 
        else if (k > (22100000/100 - 221) && k < (22100000/100 + 221))
        {
             return MFX_ERR_NONE;
        } 
        else
            return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
    } 
    else 
    {
        if (width == 0 && height == 0)
            return MFX_ERR_NONE;
        else
            return MFX_ERR_INVALID_VIDEO_PARAM;
    }
}
mfxU8 GetAspectRatioCode (mfxU32 dispAspectRatioW, mfxU32 dispAspectRatioH)
{
    if (dispAspectRatioH == 0)
    {
        return 1;    
    }
    mfxU64 k  = ((mfxU64)dispAspectRatioW*1000)/(dispAspectRatioH);

     if  (k <= (4000/3  + 1)  && k >= (4000/3 - 1))
     {
         return 2;         
     }
     if (k <= (16000/9  + 1)  && k >= (16000/9 - 1))
     {
         return 3;
     }
     if (k <= (221000/100  + 1)  && k >= (221000/100 - 1))
     {
         return 4;
     }
     return 1;
}

void CorrectProfileLevelMpeg2(mfxU16 &profile, mfxU16 & level, mfxU32 w, mfxU32 h, mfxF64 frame_rate, mfxU32 bitrate)
{
    if (MFX_LEVEL_MPEG2_HIGH !=  level && MFX_LEVEL_MPEG2_HIGH1440 !=  level && MFX_LEVEL_MPEG2_MAIN !=  level &&  MFX_LEVEL_MPEG2_LOW !=  level)
        level = MFX_LEVEL_MPEG2_MAIN;

    if (MFX_PROFILE_MPEG2_SIMPLE != profile && MFX_PROFILE_MPEG2_MAIN != profile && MFX_PROFILE_MPEG2_HIGH != profile)
        profile = MFX_PROFILE_MPEG2_MAIN;

    
    if (w > 1440 || h > 1152 || frame_rate*w*h > 47001600.0 || bitrate > 60000000)
    {
        level = MFX_LEVEL_MPEG2_HIGH;
    }
    else if ((w > 720 || h > 576 || frame_rate > 30 || frame_rate*w*h > 10368000.0 || bitrate > 15000000) && (level != MFX_LEVEL_MPEG2_HIGH))
    {
        level = MFX_LEVEL_MPEG2_HIGH1440;
    }
    else if ((w > 352 || h > 288 || frame_rate*w*h > 3041280.0 || bitrate > 4000000) && (level != MFX_LEVEL_MPEG2_HIGH && level != MFX_LEVEL_MPEG2_HIGH1440))
    {
        level = MFX_LEVEL_MPEG2_MAIN;
    }

    if (MFX_PROFILE_MPEG2_SIMPLE == profile && MFX_LEVEL_MPEG2_MAIN !=  level)
    {
        profile = MFX_PROFILE_MPEG2_MAIN;
    }

    if (MFX_PROFILE_MPEG2_HIGH == profile && MFX_LEVEL_MPEG2_LOW == level )
    {
        profile = MFX_PROFILE_MPEG2_MAIN;
    } 
}
mfxStatus InputSurfaces::Reset(mfxVideoParam *par, mfxU16 NumFrameMin)
{
    mfxStatus sts = MFX_ERR_NONE;

    mfxU32 ioPattern = par->IOPattern & (MFX_IOPATTERN_IN_VIDEO_MEMORY|MFX_IOPATTERN_IN_SYSTEM_MEMORY|MFX_IOPATTERN_IN_OPAQUE_MEMORY);
    if (ioPattern & (ioPattern - 1))
        return MFX_ERR_INVALID_VIDEO_PARAM;
    
    MFX_INTERNAL_CPY(&m_Info,&par->mfx.FrameInfo,sizeof(mfxFrameInfo));

    bool bOpaq = (par->IOPattern & MFX_IOPATTERN_IN_OPAQUE_MEMORY)!=0;

    MFX_CHECK(bOpaq == m_bOpaq || !m_bInitialized, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);     

    if (bOpaq)
    {
        MFX_CHECK (m_pCore->IsCompatibleForOpaq(), MFX_ERR_UNDEFINED_BEHAVIOR);

        mfxExtOpaqueSurfaceAlloc * pOpaqAlloc = (mfxExtOpaqueSurfaceAlloc *)GetExtendedBuffer(par->ExtParam, par->NumExtParam, MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION);
        MFX_CHECK (pOpaqAlloc, MFX_ERR_INVALID_VIDEO_PARAM);

        switch (pOpaqAlloc->In.Type & (MFX_MEMTYPE_DXVA2_DECODER_TARGET|MFX_MEMTYPE_SYSTEM_MEMORY|MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET))
        {
        case MFX_MEMTYPE_SYSTEM_MEMORY:
            m_bSysMemFrames = true;
            break;
        case MFX_MEMTYPE_DXVA2_DECODER_TARGET:
        case MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET:
            m_bSysMemFrames = false;
            break;
        default:
            return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;        
        }

       if (pOpaqAlloc->In.NumSurface < NumFrameMin)
            return m_bInitialized ? MFX_ERR_INCOMPATIBLE_VIDEO_PARAM : MFX_ERR_INVALID_VIDEO_PARAM;
        if (pOpaqAlloc->In.NumSurface > m_request.NumFrameMin && m_bInitialized)
            return MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;

        if (!m_bInitialized)
        {
            m_request.Info = par->mfx.FrameInfo;
            m_request.NumFrameMin = m_request.NumFrameSuggested = (mfxU16)pOpaqAlloc->In.NumSurface;
            m_request.Type = (mfxU16)pOpaqAlloc->In.Type;

            sts = m_pCore->AllocFrames(&m_request,
                &m_response,
                pOpaqAlloc->In.Surfaces, 
                pOpaqAlloc->In.NumSurface);

            if (MFX_ERR_UNSUPPORTED == sts && (pOpaqAlloc->In.Type & MFX_MEMTYPE_FROM_ENCODE) == 0)  sts = MFX_ERR_NONE;
            MFX_CHECK_STS(sts);
        }
        m_bOpaq = true;
    }
    else
    {
        bool bSysMemFrames = (par->IOPattern & MFX_IOPATTERN_IN_SYSTEM_MEMORY) != 0;
        MFX_CHECK(bSysMemFrames == m_bSysMemFrames || !m_bInitialized, MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);
        m_bSysMemFrames = bSysMemFrames;
    }
    m_bInitialized = true;
    return sts;     
}
mfxStatus InputSurfaces::Close()
{
    if (m_response.NumFrameActual != 0)
    {
        m_pCore->FreeFrames (&m_response);         
    }
    m_bOpaq = false;
    m_bSysMemFrames = false;
    m_bInitialized = false;

    memset(&m_request,  0, sizeof(mfxFrameAllocRequest));
    memset(&m_response, 0, sizeof (mfxFrameAllocResponse)); 

    return MFX_ERR_NONE;
}
