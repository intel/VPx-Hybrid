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

File Name: mfx_vp8_encode_utils_hw.h

\* ****************************************************************************** */

#include "mfx_common.h"
#ifndef _MFX_VP8_ENCODE_UTILS_HW_H_
#define _MFX_VP8_ENCODE_UTILS_HW_H_

//#define VP8_LOGGING

#ifdef VP8_LOGGING
#define VP8_LOG(S)     printf(S); fflush(0);
#define VP8_LOG_1(S,n) printf(S,n); fflush(0);
#else
#define VP8_LOG(S)
#define VP8_LOG_1(S,n)
#endif

#include "umc_mutex.h"

#if defined(_WIN32) || defined(_WIN64)
  #define VPX_FORCEINLINE __forceinline
  #define VPX_NONLINE __declspec(noinline)
  #define VPX_FASTCALL __fastcall
  #define ALIGN_DECL(X) __declspec(align(X))
#else
  #define VPX_FORCEINLINE __attribute__((always_inline)) inline
  #define VPX_NONLINE __attribute__((noinline))
  #define VPX_FASTCALL
  #define ALIGN_DECL(X) __attribute__ ((aligned(X)))
#endif

#include <vector>
#include <memory>

#include "mfxstructures.h"
#include "mfx_enc_common.h"

namespace MFX_VP8ENC
{

    typedef struct _sFrameParams
    {
        mfxU8    bIntra;
        mfxU8    bGold;
        mfxU8    bAltRef;
        mfxU8    LFType;    //Loop filter type [0, 1]
        mfxU8    LFLevel[4];   //Loop filter level [0,63], all segments
        mfxU8    Sharpness; //[0,7]
        mfxU8    copyToGoldRef;
        mfxU8    copyToAltRef;
    } sFrameParams;

    enum eTaskStatus
    {
        TASK_FREE =0,
        TASK_INITIALIZED,
        TASK_SUBMITTED,
        READY    
    };
    enum eRefFrames
    {
        REF_BASE  = 0,
        REF_GOLD  = 1,
        REF_ALT   = 2,
        REF_TOTAL = 3
    };
    enum
    {
        MFX_MEMTYPE_D3D_INT = MFX_MEMTYPE_FROM_ENCODE | MFX_MEMTYPE_DXVA2_DECODER_TARGET | MFX_MEMTYPE_INTERNAL_FRAME,
        MFX_MEMTYPE_D3D_EXT = MFX_MEMTYPE_FROM_ENCODE | MFX_MEMTYPE_DXVA2_DECODER_TARGET | MFX_MEMTYPE_EXTERNAL_FRAME,
        MFX_MEMTYPE_SYS_EXT = MFX_MEMTYPE_FROM_ENCODE | MFX_MEMTYPE_SYSTEM_MEMORY        | MFX_MEMTYPE_EXTERNAL_FRAME,
        MFX_MEMTYPE_SYS_INT = MFX_MEMTYPE_FROM_ENCODE | MFX_MEMTYPE_SYSTEM_MEMORY        | MFX_MEMTYPE_INTERNAL_FRAME
    };
    static const mfxU16 MFX_IOPATTERN_IN_MASK_SYS_OR_D3D =
        MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_IN_VIDEO_MEMORY;

    static const mfxU16 MFX_IOPATTERN_IN_MASK =
        MFX_IOPATTERN_IN_MASK_SYS_OR_D3D | MFX_IOPATTERN_IN_OPAQUE_MEMORY;

    struct sFrameEx
    {
        mfxFrameSurface1 *pSurface;
        mfxU32            idInPool;
        mfxU8             QP;
    };

    inline mfxStatus LockSurface(sFrameEx*  pFrame, mfxCoreInterface* pCore)
    {
        //printf("LockSurface: ");
        //if (pFrame) printf("%d\n",pFrame->idInPool); else printf("\n");
        return (pFrame) ? pCore->IncreaseReference(pCore->pthis, &pFrame->pSurface->Data) : MFX_ERR_NONE; 
    }
    inline mfxStatus FreeSurface(sFrameEx* &pFrame, mfxCoreInterface* pCore)
    {        
        mfxStatus sts = MFX_ERR_NONE;
        //printf("FreeSurface\n");
        //if (pFrame) printf("%d\n",pFrame->idInPool); else printf("\n");
        if (pFrame && pFrame->pSurface)
        {
            //printf("DecreaseReference\n");
            sts = pCore->DecreaseReference(pCore->pthis, &pFrame->pSurface->Data);
            pFrame = 0;
        }
        return sts;   
    }
    inline bool isFreeSurface (sFrameEx* pFrame)
    {
        //printf("isFreeSurface %d (%d)\n", pFrame->pSurface->Data.Locked, pFrame->idInPool);
        return (pFrame->pSurface->Data.Locked == 0);
    }
    
    inline bool isOpaq(mfxVideoParam const & video)
    {
        return (video.IOPattern & MFX_IOPATTERN_IN_OPAQUE_MEMORY)!=0;
    }


    inline mfxU32 CalcNumTasks(mfxVideoParam const & video)
    {
        return video.AsyncDepth + 1;
    }

    inline mfxU32 CalcNumSurfRawLocal(mfxVideoParam const & video, bool bHWImpl, bool bHWInput,bool bRawReference = false)
    {
        return (bHWInput !=  bHWImpl) ? CalcNumTasks(video) + ( bRawReference ? 3 : 0 ):0;
    }

    inline mfxU32 CalcNumSurfRecon(mfxVideoParam const & video)
    {
        //printf(" CalcNumSurfRecon %d (%d)\n", 3 + CalcNumTasks(video), CalcNumTasks(video));
        return 3 + CalcNumTasks(video);
    }
    inline mfxU32 CalcNumMB(mfxVideoParam const & video)
    {
        return (video.mfx.FrameInfo.Width>>4)*(video.mfx.FrameInfo.Height>>4);    
    }
    inline bool CheckPattern(mfxU32 inPattern)
    {
        inPattern = inPattern & MFX_IOPATTERN_IN_MASK;
        return ( inPattern == MFX_IOPATTERN_IN_SYSTEM_MEMORY ||
                 inPattern == MFX_IOPATTERN_IN_VIDEO_MEMORY ||
                 inPattern == MFX_IOPATTERN_IN_OPAQUE_MEMORY);           
    }
    inline bool CheckFrameSize(mfxU32 Width, mfxU32 Height)
    {
        return ( ((Width & 0x0f) == 0) && ((Height& 0x0f) == 0) &&
                 (Width > 0)  && (Width < 0x1FFF) &&
                 (Height > 0) && (Height < 0x1FFF));
    }


    bool isVideoSurfInput(mfxVideoParam const & video);   
    mfxStatus CheckEncodeFrameParam(mfxVideoParam const & video,
                                    mfxEncodeCtrl       * ctrl,
                                    mfxFrameSurface1    * surface,
                                    mfxBitstream        * bs,
                                    bool                  isExternalFrameAllocator);

    mfxStatus GetVideoParam(mfxVideoParam * parDst, mfxVideoParam *videoSrc);
    mfxStatus SetFramesParams(mfxVideoParam * par, mfxU16 forcedFrameType, mfxU32 frameOrder, sFrameParams *pFrameParams);


    /*------------------------ Utils------------------------------------------------------------------------*/
    
#define MFX_CHECK_WITH_ASSERT(EXPR, ERR) { assert(EXPR); MFX_CHECK(EXPR, ERR); }
#define STATIC_ASSERT(ASSERTION, MESSAGE) char MESSAGE[(ASSERTION) ? 1 : -1]; MESSAGE    

    template<class T> inline void Zero(T & obj)                { memset(&obj, 0, sizeof(obj)); }
    template<class T> inline void Zero(std::vector<T> & vec)   { memset(&vec[0], 0, sizeof(T) * vec.size()); }
    template<class T> inline void Zero(T * first, size_t cnt)  { memset(first, 0, sizeof(T) * cnt); }
    template<class T> inline bool Equal(T const & lhs, T const & rhs) { return memcmp(&lhs, &rhs, sizeof(T)) == 0; }
    template<class T, class U> inline void Copy(T & dst, U const & src)
    {
        STATIC_ASSERT(sizeof(T) == sizeof(U), copy_objects_of_different_size);
        memcpy(&dst, &src, sizeof(dst));
    }
    template<class T> inline T & RemoveConst(T const & t) { return const_cast<T &>(t); }
    template<class T> inline T * RemoveConst(T const * t) { return const_cast<T *>(t); }
    template<class T, size_t N> inline T * Begin(T(& t)[N]) { return t; }
    template<class T, size_t N> inline T * End(T(& t)[N]) { return t + N; }
    template<class T, size_t N> inline size_t SizeOf(T(&)[N]) { return N; }
    template<class T> inline T const * Begin(std::vector<T> const & t) { return &*t.begin(); }
    template<class T> inline T const * End(std::vector<T> const & t) { return &*t.begin() + t.size(); }
    template<class T> inline T * Begin(std::vector<T> & t) { return &*t.begin(); }
    template<class T> inline T * End(std::vector<T> & t) { return &*t.begin() + t.size(); }

    /* copied from H.264 */
    //-----------------------------------------------------
    // Helper which checks number of allocated frames and auto-free.
    //-----------------------------------------------------
    template<class T> struct ExtBufTypeToId {};

#define BIND_EXTBUF_TYPE_TO_ID(TYPE, ID) template<> struct ExtBufTypeToId<TYPE> { enum { id = ID }; }
    BIND_EXTBUF_TYPE_TO_ID (mfxExtOpaqueSurfaceAlloc,MFX_EXTBUFF_OPAQUE_SURFACE_ALLOCATION);
    BIND_EXTBUF_TYPE_TO_ID (mfxExtVP8CodingOption,MFX_EXTBUFF_VP8_CODING_OPTION);
    BIND_EXTBUF_TYPE_TO_ID (mfxExtEncoderROI,MFX_EXTBUFF_ENCODER_ROI);
#undef BIND_EXTBUF_TYPE_TO_ID

    template <class T> inline void InitExtBufHeader(T & extBuf)
    {
        Zero(extBuf);
        extBuf.Header.BufferId = ExtBufTypeToId<T>::id;
        extBuf.Header.BufferSz = sizeof(T);
    }

    template <> inline void InitExtBufHeader<mfxExtVP8CodingOption>(mfxExtVP8CodingOption & extBuf)
    {
        Zero(extBuf);
        extBuf.Header.BufferId = ExtBufTypeToId<mfxExtVP8CodingOption>::id;
        extBuf.Header.BufferSz = sizeof(mfxExtVP8CodingOption);

        // align with hardcoded parameters that were used before
        extBuf.NumTokenPartitions = 1;
    }

    // temporal application of nonzero defaults to test ROI for VP8
    // TODO: remove this when testing os finished
    template <> inline void InitExtBufHeader<mfxExtEncoderROI>(mfxExtEncoderROI & extBuf)
    {
        Zero(extBuf);
        extBuf.Header.BufferId = ExtBufTypeToId<mfxExtEncoderROI>::id;
        extBuf.Header.BufferSz = sizeof(mfxExtEncoderROI);

        // no default ROI
        /*extBuf.NumROI = 2;
        extBuf.ROI[0].Left     = 16;
        extBuf.ROI[0].Right    = 128;
        extBuf.ROI[0].Top      = 16;
        extBuf.ROI[0].Bottom   = 128;
        extBuf.ROI[0].Priority = 0;

        extBuf.ROI[1].Left     = 64;
        extBuf.ROI[1].Right    = 256;
        extBuf.ROI[1].Top      = 64;
        extBuf.ROI[1].Bottom   = 256;
        extBuf.ROI[1].Priority = 0;*/
    }

    template <class T> struct GetPointedType {};
    template <class T> struct GetPointedType<T *> { typedef T Type; };
    template <class T> struct GetPointedType<T const *> { typedef T Type; };

    struct mfxExtBufferProxy
    {
    public:
        template <typename T> operator T()
        {
            mfxExtBuffer * p = GetExtBuffer(
                m_extParam,
                m_numExtParam,
                ExtBufTypeToId<typename GetPointedType<T>::Type>::id);
            return reinterpret_cast<T>(p);
        }

        template <typename T> friend mfxExtBufferProxy GetExtBuffer(const T & par);

    protected:
        mfxExtBufferProxy(mfxExtBuffer ** extParam, mfxU32 numExtParam)
            : m_extParam(extParam)
            , m_numExtParam(numExtParam)
        {
        }

    private:
        mfxExtBuffer ** m_extParam;
        mfxU32          m_numExtParam;
    };

    template <typename T> mfxExtBufferProxy GetExtBuffer(T const & par)
    {
        return mfxExtBufferProxy(par.ExtParam, par.NumExtParam);
    }

    class MfxFrameAllocResponse : public mfxFrameAllocResponse
    {
    public:
        MfxFrameAllocResponse();

        ~MfxFrameAllocResponse();

        mfxStatus Alloc(
            mfxCoreInterface * pCore,
            mfxFrameAllocRequest & req);

        mfxFrameInfo               m_info;

    private:
        MfxFrameAllocResponse(MfxFrameAllocResponse const &);
        MfxFrameAllocResponse & operator =(MfxFrameAllocResponse const &);

        mfxCoreInterface * m_pCore;
        mfxU16      m_numFrameActualReturnedByAllocFrames;

        std::vector<mfxFrameAllocResponse> m_responseQueue;
        std::vector<mfxMemId>              m_mids;        
    }; 

    struct FrameLocker
    {
        FrameLocker(mfxCoreInterface * pCore, mfxFrameData & data, bool external = false)
            : m_core(pCore)
            , m_data(data)
            , m_memId(data.MemId)
            , m_status(Lock(external))
        {
        }

        FrameLocker(mfxCoreInterface * pCore, mfxFrameData & data, mfxMemId memId, bool external = false)
            : m_core(pCore)
            , m_data(data)
            , m_memId(memId)
            , m_status(Lock(external))
        {
        }

        ~FrameLocker() { Unlock(); }

        mfxStatus Unlock()
        {
            mfxStatus mfxSts = MFX_ERR_NONE;
            mfxSts = m_core->FrameAllocator.Unlock(m_core->FrameAllocator.pthis, m_memId, &m_data);
            m_status = LOCK_NO;
            return mfxSts;
        }

    protected:
        enum { LOCK_NO, LOCK_DONE };

        mfxU32 Lock(bool external)
        {
            mfxU32 status = LOCK_NO;

            if (m_data.Y == 0 &&
                MFX_ERR_NONE == m_core->FrameAllocator.Lock(m_core->FrameAllocator.pthis, m_memId, &m_data))
                status = LOCK_DONE;

            return status;
        }

    private:
        FrameLocker(FrameLocker const &);
        FrameLocker & operator =(FrameLocker const &);

        mfxCoreInterface * m_core;
        mfxFrameData &     m_data;
        mfxMemId           m_memId;
        mfxU32             m_status;
    };

    class VP8MfxParam : public mfxVideoParam
    {
    public:
        VP8MfxParam();
        VP8MfxParam(VP8MfxParam const &);
        VP8MfxParam(mfxVideoParam const &);

        VP8MfxParam & operator = (VP8MfxParam const &);
        VP8MfxParam & operator = (mfxVideoParam const &);


    protected:
        void Construct(mfxVideoParam const & par);


    private:
        mfxExtBuffer *              m_extParam[3];
        mfxExtOpaqueSurfaceAlloc    m_extOpaque;
        mfxExtVP8CodingOption       m_extVP8Opt;
        mfxExtEncoderROI            m_extROI;
    };


    class ExternalFrames
    {
    protected:
        std::vector<sFrameEx>  m_frames;
    public:
        ExternalFrames() {}
        void Init();
        mfxStatus GetFrame(mfxFrameSurface1 *pInFrame, sFrameEx *&pOutFrame);     
     };


    class InternalFrames
    {
    protected:
        std::vector<sFrameEx>           m_frames;
        MfxFrameAllocResponse           m_response;
        std::vector<mfxFrameSurface1>   m_surfaces;
    public:
        InternalFrames() {}
        mfxStatus Init(mfxCoreInterface *pCore, mfxFrameAllocRequest *pAllocReq, bool bHW);
        sFrameEx * GetFreeFrame();
        mfxStatus  GetFrame(mfxU32 numFrame, sFrameEx * &Frame);

        inline mfxU16 Height()
        {
            return m_response.m_info.Height;
        }
        inline mfxU16 Width()
        {
            return m_response.m_info.Width;
        }
        inline mfxU32 Num()
        {
            return m_response.NumFrameActual;
        }
        inline MfxFrameAllocResponse& GetFrameAllocReponse()  {return m_response;}
    };


    typedef struct _sDpbVP8
    {
        sFrameEx* m_refFrames[REF_TOTAL];
        mfxU8     m_refMap[REF_TOTAL];
    } sDpbVP8;


    class Task
    {
    public:

        eTaskStatus          m_status;
        sFrameEx            *m_pRawFrame;
        sFrameEx            *m_pRawLocalFrame;
        mfxBitstream        *m_pBitsteam;
        mfxCoreInterface    *m_pCore;

        sFrameParams         m_sFrameParams;
        sFrameEx            *m_pRecFrame;
        sFrameEx            *m_pRecRefFrames[3];
        bool                 m_bOpaqInput;
        mfxU32              m_frameOrder;
        mfxU64              m_timeStamp;

        mfxEncodeCtrl       m_ctrl;

    protected:

        mfxStatus CopyInput();

    public:

        Task ():
          m_status(TASK_FREE),
              m_pRawFrame(0),
              m_pRawLocalFrame(0),
              m_pBitsteam(0),
              m_pCore(0),
              m_pRecFrame(0),
              m_bOpaqInput(false),
              m_frameOrder (0)
          {
              Zero(m_pRecRefFrames);
              Zero(m_sFrameParams);
              Zero(m_ctrl);
          }
          virtual
          ~Task ()
          {
              FreeTask();
          }
          mfxStatus GetOriginalSurface(mfxFrameSurface1 *& pSurface, bool &bExternal);
          mfxStatus GetInputSurface(mfxFrameSurface1 *& pSurface, bool &bExternal);

          virtual
          mfxStatus Init(mfxCoreInterface * pCore, mfxVideoParam *par);
          
          virtual
          mfxStatus InitTask( sFrameEx     *pRawFrame, 
                              mfxBitstream *pBitsteam,
                              mfxU32        frameOrder);
          virtual
          mfxStatus SubmitTask (sFrameEx*  pRecFrame, sDpbVP8 &dpb, sFrameParams* pParams, sFrameEx* pRawLocalFrame);
 
          inline mfxStatus CompleteTask ()
          {
              m_status = READY;
              return MFX_ERR_NONE;
          }
          inline bool isReady()
          {
            return m_status == READY;
          }

          virtual 
          mfxStatus FreeTask();
          mfxStatus FreeDPBSurfaces();

          inline bool isFreeTask()
          {
              return (m_status == TASK_FREE);        
          }
    };



    template <class TTask>
    inline TTask* GetFreeTask(std::vector<TTask> & tasks)
    {
        typename std::vector<TTask>::iterator task = tasks.begin();
        for (;task != tasks.end(); task ++)
        {
            if (task->isFreeTask())
            {
                return &task[0]; 
            }            
        } 
        return 0;
    }
    template <class TTask>
    inline mfxStatus FreeTasks(std::vector<TTask> & tasks)
    {
        mfxStatus sts = MFX_ERR_NONE;
        typename std::vector<TTask>::iterator task = tasks.begin();
        for (;task != tasks.end(); task ++)
        {
            if (!task->isFreeTask())
            {
                sts = task->FreeTask(); 
                MFX_CHECK_STS(sts);
            }            
        } 
        return sts;
    }

    inline bool IsResetOfPipelineRequired(const mfxVideoParam& oldPar, const mfxVideoParam& newPar)
    {
        return oldPar.mfx.FrameInfo.Width != newPar.mfx.FrameInfo.Width
              || oldPar.mfx.FrameInfo.Height != newPar.mfx.FrameInfo.Height
              ||  oldPar.mfx.GopPicSize != newPar.mfx.GopPicSize;
    }

    /*for Full Encode (SW or HW)*/
    template <class TTask>
    class TaskManager
    {
    protected:

        mfxCoreInterface*       m_pCore;
        VP8MfxParam      m_video;

        bool    m_bHWFrames;
        bool    m_bHWInput;

        ExternalFrames  m_RawFrames;
        InternalFrames  m_LocalRawFrames;
        InternalFrames  m_ReconFrames;

        std::vector<TTask>  m_Tasks;
        sDpbVP8             m_dpb;

        mfxU32              m_frameNum;


    public:
        TaskManager ():
          m_pCore(0),
          m_bHWFrames(false),
          m_bHWInput(false),
          m_frameNum(0)
        {
            memset(&m_dpb, 0, sizeof(m_dpb));
        }

          ~TaskManager()
          {
            FreeTasks(m_Tasks);
          }

          mfxStatus Init(mfxCoreInterface* pCore, mfxVideoParam *par, bool bHWImpl, mfxU32 reconFourCC)
          {
              mfxStatus sts = MFX_ERR_NONE;

              MFX_CHECK(!m_pCore, MFX_ERR_UNDEFINED_BEHAVIOR);

              m_pCore   = pCore;
              m_video   = *par;
              m_bHWFrames = bHWImpl;
              m_bHWInput = isVideoSurfInput(m_video);

              memset(&m_dpb, 0, sizeof(m_dpb));

              m_frameNum = 0;

              m_RawFrames.Init();

              mfxFrameAllocRequest request     = {};
              request.Info = m_video.mfx.FrameInfo;
              request.Info.FourCC = MFX_FOURCC_NV12; // only NV12 is supported as driver input
              request.NumFrameMin = request.NumFrameSuggested = (mfxU16)CalcNumSurfRawLocal(m_video, bHWImpl, m_bHWInput);

              sts = m_LocalRawFrames.Init(pCore, &request, m_bHWFrames);
              MFX_CHECK_STS(sts);

              request.NumFrameMin = request.NumFrameSuggested = (mfxU16)CalcNumSurfRecon(m_video);
              request.Info.FourCC = reconFourCC;

              sts = m_ReconFrames.Init(pCore, &request, m_bHWFrames);
              MFX_CHECK_STS(sts);

              {
                  mfxU32 numTasks = CalcNumTasks(m_video);
                  m_Tasks.resize(numTasks);

                  for (mfxU32 i = 0; i < numTasks; i++)
                  {
                      sts = m_Tasks[i].Init(m_pCore,&m_video);
                      MFX_CHECK_STS(sts);
                  }
              }

              return sts;
          }

          mfxStatus ReleaseDpbFrames()
          {
              for (mfxU8 refIdx = 0; refIdx < REF_TOTAL; refIdx ++)
                  MFX_CHECK_STS(FreeSurface(m_dpb.m_refFrames[refIdx],m_pCore));

              return MFX_ERR_NONE;
          }

          mfxStatus Reset(mfxVideoParam *par)
          {
              mfxStatus sts = MFX_ERR_NONE;
              MFX_CHECK(m_pCore!=0, MFX_ERR_NOT_INITIALIZED);

              MFX_CHECK(m_ReconFrames.Height() >= par->mfx.FrameInfo.Height,MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);
              MFX_CHECK(m_ReconFrames.Width()  >= par->mfx.FrameInfo.Width,MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

              MFX_CHECK(mfxU16(CalcNumSurfRawLocal(*par,m_bHWFrames,isVideoSurfInput(*par))) <= m_LocalRawFrames.Num(),MFX_ERR_INCOMPATIBLE_VIDEO_PARAM); 
              MFX_CHECK(mfxU16(CalcNumSurfRecon(*par)) <= m_ReconFrames.Num(),MFX_ERR_INCOMPATIBLE_VIDEO_PARAM);

              m_video = *par;

              if (IsResetOfPipelineRequired(m_video, *par))
              {
                  m_frameNum = 0;
                  ReleaseDpbFrames();
                  memset(&m_dpb, 0, sizeof(m_dpb));
                  sts = FreeTasks(m_Tasks);
                  MFX_CHECK_STS(sts);
              }
              return MFX_ERR_NONE;
          }
          mfxStatus InitTask(mfxFrameSurface1* pSurface, mfxBitstream* pBitstream, Task* & pOutTask)
          {
              mfxStatus sts = MFX_ERR_NONE;
              Task*     pTask = GetFreeTask(m_Tasks);
              sFrameEx* pRawFrame = 0;

              //printf("Init frame\n");

              MFX_CHECK(m_pCore!=0, MFX_ERR_NOT_INITIALIZED);
              MFX_CHECK(pTask!=0,MFX_WRN_DEVICE_BUSY);

              sts = m_RawFrames.GetFrame(pSurface, pRawFrame);
              MFX_CHECK_STS(sts);  

              //printf("Init frame - 1\n");

              sts = pTask->InitTask(pRawFrame,pBitstream, m_frameNum);
              pTask->m_timeStamp =pSurface->Data.TimeStamp;
              MFX_CHECK_STS(sts);

              //printf("Init frame - End\n");

              pOutTask = pTask;

              m_frameNum++;

              return sts;
          }

          inline
          mfxStatus UpdateDpb(sFrameParams *pParams, sFrameEx *pRecFrame)
          {
              mfxU8   freeSurfCnt[REF_TOTAL] = {0, };
              sDpbVP8 dpbBeforeUpdate = m_dpb;

              // reset mapping of ref frames
              m_dpb.m_refMap[REF_BASE] = m_dpb.m_refMap[REF_GOLD] = m_dpb.m_refMap[REF_ALT] = 0;

              if (pParams->bIntra)
              {
                  m_dpb.m_refFrames[REF_BASE] = pRecFrame;
                  m_dpb.m_refFrames[REF_GOLD] = pRecFrame;
                  m_dpb.m_refFrames[REF_ALT] =  pRecFrame;

                  // treat all references as free - they all were replaced with current frame
                  freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_BASE]] ++;
                  freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_GOLD]] ++;
                  freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_ALT]]  ++;
              }
              else
              {
                  // refresh last_ref with current frame
                  m_dpb.m_refFrames[REF_BASE] = pRecFrame;
                  m_dpb.m_refMap[REF_BASE] = 0;
                  // treat last_ref as free - it was replaced with current frame
                  freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_BASE]] ++;

                  if (pParams->bGold)
                  {
                      // refresh gold_ref with current frame
                      m_dpb.m_refFrames[REF_GOLD] = pRecFrame;
                      m_dpb.m_refMap[REF_GOLD] = 0;
                      // treat gold_ref as free - it was replaced with current frame
                      freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_GOLD]] ++;
                  }
                  else
                  {
                      if (pParams->copyToGoldRef == 1)
                      {
                          // replace gold_ref with last_ref
                          m_dpb.m_refFrames[REF_GOLD] = dpbBeforeUpdate.m_refFrames[REF_BASE];                        
                          // treat gold_ref as free - it was replaced with last_ref
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_GOLD]] ++;
                          // treat last_ref as occupied - gold_ref was replaced with it
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_BASE]] = 0;
                      }
                      else if (pParams->copyToGoldRef == 2)
                      {
                          // replace gold_ref with alt_ref
                          m_dpb.m_refFrames[REF_GOLD] = dpbBeforeUpdate.m_refFrames[REF_ALT];
                          // treat gold_ref as free - it was replaced with alt_ref
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_GOLD]] ++;
                          // treat alt_ref as occupied - gold_ref was replaced with it
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_ALT]] = 0;
                      }
                      m_dpb.m_refMap[REF_GOLD] = 1;
                  }

                  if (pParams->bAltRef)
                  {
                      // refresh alt_ref with current frame
                      m_dpb.m_refFrames[REF_ALT] = pRecFrame;
                      m_dpb.m_refMap[REF_ALT] = 0;
                      // treat alt_ref as free - it was replaced with current frame
                      freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_ALT]] ++;
                  }
                  else
                  {
                      if (pParams->copyToAltRef == 1)
                      {
                          // replace alt_ref with last_ref
                          m_dpb.m_refFrames[REF_ALT] = dpbBeforeUpdate.m_refFrames[REF_BASE];
                          // treat alt_ref as free - it was replaced with last_ref
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_ALT]] ++;
                          // treat last_ref as occupied - alt_ref was replaced with it
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_BASE]] = 0;
                      }
                      else if (pParams->copyToAltRef == 2)
                      {
                          // replace alt_ref with gold_ref
                          m_dpb.m_refFrames[REF_ALT] = dpbBeforeUpdate.m_refFrames[REF_GOLD];
                          // treat alt_ref as free - it was replaced with gold_ref
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_ALT]] ++;
                          // treat gold_ref as occupied - alt_ref was replaced with it
                          freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_GOLD]] = 0;
                      }

                      if (m_dpb.m_refMap[REF_GOLD] == 0) // gold_ref was replaced with current frames
                          m_dpb.m_refMap[REF_ALT] = 1;
                      else
                          m_dpb.m_refMap[REF_ALT] = (m_dpb.m_refFrames[REF_ALT] == m_dpb.m_refFrames[REF_GOLD]) ? 1 : 2;
                  }
              }

              if (m_dpb.m_refFrames[REF_GOLD] == dpbBeforeUpdate.m_refFrames[REF_GOLD])
                  freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_GOLD]] = 0;
              if (m_dpb.m_refFrames[REF_ALT] == dpbBeforeUpdate.m_refFrames[REF_ALT])
                  freeSurfCnt[dpbBeforeUpdate.m_refMap[REF_ALT]] = 0;

              // release reconstruct surfaces that didn't fit into updated dpb
              for (mfxU8 refIdx = 0; refIdx < REF_TOTAL; refIdx ++)
              {
                  if (freeSurfCnt[dpbBeforeUpdate.m_refMap[refIdx]])
                  {
                      MFX_CHECK_STS(FreeSurface(dpbBeforeUpdate.m_refFrames[refIdx],m_pCore));
                      freeSurfCnt[dpbBeforeUpdate.m_refMap[refIdx]] = 0; // reset counter to 0 to avoid second FreeSurface() call for same surface
                  }
              }

              return MFX_ERR_NONE;
          }

    }; 
}
#endif