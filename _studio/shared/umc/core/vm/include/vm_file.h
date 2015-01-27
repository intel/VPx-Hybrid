/* ****************************************************************************** *\

Copyright (C) 2003-2014 Intel Corporation.  All rights reserved.

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

File Name: vm_file.h

\* ****************************************************************************** */
/*
 * VM 64-bits buffered file operations library
 *       common definitions
 */
#ifndef VM_FILE_H
#  define VM_FILE_H

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#  include "ippdefs.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif



#  include "vm_types.h"
#  include "vm_strings.h"
#  if defined(LINUX32) || defined(__APPLE__)
#     include "sys/vm_file_unix.h"
#  else
#     include "sys/vm_file_win.h"
#  endif

/*
 * bit or-ed file attribute */
enum {
   VM_FILE_ATTR_FILE        =0x0001,
   VM_FILE_ATTR_DIRECTORY   =0x0002,
   VM_FILE_ATTR_LINK        =0x0004,
   VM_FILE_ATTR_HIDDEN      =0x0008
};

/*
 * seek whence */
typedef enum {
   VM_FILE_SEEK_SET=0,
   VM_FILE_SEEK_CUR=1,
   VM_FILE_SEEK_END=2
} VM_FILE_SEEK_MODE;


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 * file access functions
 */
Ipp64u vm_file_fseek(vm_file *fd, Ipp64s position, VM_FILE_SEEK_MODE mode);
Ipp64u vm_file_ftell(vm_file *fd);
Ipp32s vm_file_getinfo(const vm_char *filename, Ipp64u *file_size, Ipp32u *file_attr);
Ipp32s vm_file_vfprintf(vm_file *fd, vm_char *format,  va_list argptr);

#if defined(LINUX32) || defined(OSX)

    #define vm_file_fflush fflush

#else /* #if defined(LINUX32) || defined(OSX) */

Ipp32s vm_file_fflush(vm_file *fd);
vm_file *vm_file_fopen(const vm_char *fname, const vm_char *mode);
Ipp32s vm_file_fclose(vm_file *fd);
Ipp32s vm_file_feof(vm_file* fd);
Ipp32s vm_file_remove(vm_char *path);
/*
 * binary file IO */
Ipp32s vm_file_fread(void *buf, Ipp32u itemsz, Ipp32s nitems, vm_file *fd);
Ipp32s vm_file_fwrite(void *buf, Ipp32u itemsz, Ipp32s nitems, vm_file *fd);

/*
 * character (string) file IO */
vm_char *vm_file_fgets(vm_char *str, int nchar, vm_file *fd);
Ipp32s vm_file_fputs(vm_char *str, vm_file *fd);
Ipp32s vm_file_fscanf(vm_file *fd,vm_char *format, ...);
Ipp32s vm_file_fprintf(vm_file *fd,vm_char *format, ...);
Ipp32s vm_file_vfprintf(vm_file *fd, vm_char *format,  va_list argptr);

/*
 * Temporary files support
 * ISO/IEC 9899:1990 emulation
 * Without stdlib.h defined
 *             mktemp, mkstemp and mkdtemp functions emulation for the first time.
 */
vm_file *vm_file_tmpfile(void);
vm_char *vm_file_tmpnam(vm_char *RESULT);
vm_char *vm_file_tmpnam_r(vm_char *RESULT); /* the same as tmpnam_r except that returns NULL if RESULT is NULL  -- fully reentrant function */
vm_char *vm_file_tempnam(vm_char *DIR, vm_char *PREFIX);


#endif /* #if defined(LINUX32) || defined(OSX) */

/*
 * file name manipulations */
void vm_file_getpath(vm_char *filename, vm_char *path, int nchars);
void vm_file_getbasename(vm_char *filename, vm_char *base, int nchars);
void vm_file_getsuffix(vm_char *filename, vm_char *suffix, int nchars);
void vm_file_makefilename(vm_char *path, vm_char *base, vm_char *suffix, vm_char *filename, int nchars);


/*
 *   Directory manipulations
 */
Ipp32s vm_dir_remove(vm_char *path);
Ipp32s vm_dir_mkdir(vm_char *path);

/* directory traverse */
Ipp32s vm_dir_open(vm_dir **dd, vm_char *path);
Ipp32s vm_dir_read(vm_dir *dd, vm_char *filename,int nchars);
void vm_dir_close(vm_dir *dd);

Ipp64u vm_dir_get_free_disk_space( void );

/*
 * defines for non-buffered file operations
 */
#define vm_file_open  vm_file_fopen
#define vm_file_read  vm_file_fread
#define vm_file_write vm_file_fwrite
#define vm_file_close vm_file_fclose
#define vm_file_seek  vm_file_fseek
#define vm_file_tell  vm_file_ftell
#define vm_file_gets  vm_file_fgets

/*
 */
# ifdef __cplusplus
  }
# endif
#endif /* VM_FILE_H */
