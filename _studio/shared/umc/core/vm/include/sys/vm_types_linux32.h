/* ****************************************************************************** *\

Copyright (C) 2003-2013 Intel Corporation.  All rights reserved.

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

File Name: vm_types_linux32.h

\* ****************************************************************************** */

#if defined(LINUX32) || defined(__APPLE__)

#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef __INTEL_COMPILER
/* ICC and Fedora Core 3 incompatibility */
#define __interface xxinterface
#include <netinet/in.h>
#undef __interface
#else /* __INTEL_COMPILER */
#include <netinet/in.h>
#endif /* __INTEL_COMPILER */

#include <ippcore.h>
#include <ipps.h>
#if defined(_SOCKET_SUPPORT)
#include <sys/socket.h>
#endif /* defined(_SOCKET_SUPPORT) */
#include <sys/select.h>

#define vm_main main


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define ENABLE_COUNTER          100
#define DISABLE_COUNTER         101
#define GET_TSC_LOW             102
#define GET_TSC_HIGH            103

#define VM_ALIGN_DECL(X,Y) __attribute__ ((aligned(X))) Y

#define CONST_LL(X) X##LL
#define CONST_ULL(X) X##ULL

#define vm_timeval timeval
#define vm_timezone timezone



/* vm_event.h */
typedef struct vm_event
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    Ipp32s manual;
    Ipp32s state;
} vm_event;

/* vm_mmap.h */
typedef struct vm_mmap
{
    Ipp32s fd;
    void *address;
    size_t sizet;
    Ipp32s fAccessAttr;
} vm_mmap;

/* vm_mutex.h */
typedef struct vm_mutex
{
    pthread_mutex_t handle;
    Ipp32s is_valid;
} vm_mutex;

/* vm_semaphore.h */
typedef struct vm_semaphore
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    Ipp32s count;
    Ipp32s max_count;
} vm_semaphore;

/* vm_thread.h */
typedef struct vm_thread
{
    pthread_t handle;
    Ipp32s is_valid;
    Ipp32u (*p_thread_func)(void *);
    void *p_arg;
    vm_event exit_event;
    vm_mutex access_mut;
    Ipp32s i_wait_count;
} vm_thread;

#if defined(_SOCKET_SUPPORT)
/* vm_socket.h */
#define VM_SOCKET_QUEUE 20
typedef struct vm_socket
{
   fd_set r_set, w_set;
   Ipp32s chns[VM_SOCKET_QUEUE+1];
    struct sockaddr_in sal;
   struct sockaddr_in sar;
   struct sockaddr_in peers[VM_SOCKET_QUEUE+1];
   Ipp32s flags;
} vm_socket;
#endif /* defined(_SOCKET_SUPPORT) */

typedef struct vm_time
{
   Ipp64s start;
   Ipp64s diff;
   Ipp64s freq;
} vm_time;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LINUX32 */
