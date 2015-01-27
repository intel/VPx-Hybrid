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

File Name: vm_file_unix.c

\* ****************************************************************************** */
/*
 * VM 64-bits buffered file operations library
 *       UNIX implementation
 */
/* codecws compilation fence */
#if defined(LINUX32) || defined(__APPLE__)

#if defined(LINUX32) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(LINUX64)
/* These defines are needed to get access to 'struct stat64'. stat64 function is seen without them, but
 * causes segmentation faults working with 'struct stat'.
 */
#define __USE_LARGEFILE64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#if defined(__ANDROID__)
#include <sys/statfs.h>
#else
#include <sys/statvfs.h>
#endif /* #if !defined(__ANDROID__) */
#include <dirent.h>
#include "vm_file.h"
#include <ipps.h>


/* obtain file info. return 0 if file is not accessible,
   file_size or file_attr can be NULL if either is not interested */
Ipp32s vm_file_getinfo(const vm_char *filename, Ipp64u *file_size, Ipp32u *file_attr) {
#if defined(__APPLE__) || defined(__ANDROID__) || defined(LINUX64)
   struct stat buf;
   if (stat(filename,&buf) != 0) return 0;
#else
   struct stat64 buf;
   if (stat64(filename,&buf) != 0) return 0;
#endif
   if (file_size) *file_size=buf.st_size;
   if (file_attr) {
      *file_attr=0;
      if (buf.st_mode & S_IFREG) *file_attr|=VM_FILE_ATTR_FILE;
      if (buf.st_mode & S_IFDIR) *file_attr|=VM_FILE_ATTR_DIRECTORY;
      if (buf.st_mode & S_IFLNK) *file_attr|=VM_FILE_ATTR_LINK;
   }
   return 1;
  }



Ipp64u vm_file_fseek(vm_file *fd, Ipp64s position, VM_FILE_SEEK_MODE mode) {
#if defined(__APPLE__) || defined(__ANDROID__) || defined(LINUX64)
  return fseeko(fd, (off_t)position, mode);
#else
  return fseeko64(fd, (__off64_t)position, mode);
#endif
  }

Ipp64u vm_file_ftell(vm_file *fd) {
#if defined(__APPLE__) || defined(__ANDROID__) || defined(LINUX64)
  return (Ipp64u) ftello(fd);
#else
  return (Ipp64u)ftello64(fd);
#endif
  }

/*
 *   Directory manipulations
 */
Ipp32s vm_dir_remove(vm_char *path) {
   return !remove(path);
}

Ipp32s vm_dir_mkdir(vm_char *path) {
   return !mkdir(path,0777);
}


static vm_char *d_name = NULL;
Ipp32s vm_dir_open(vm_dir **dd, vm_char *path) {
   if ((dd[0]=opendir(path)) != NULL) {
    d_name = NULL;
    getcwd(d_name, 0);
    chdir(path);
    }
   return (dd[0] != NULL) ? 1 : 0;
}

/*
 * directory traverse */
Ipp32s vm_dir_read(vm_dir *dd, vm_char *filename,int nchars) {
  Ipp32s rtv = 0;
  if (dd != NULL) {
   struct dirent *ent=readdir(dd);
   if (ent) {
     vm_string_strncpy(filename,ent->d_name,nchars);
     rtv = 1;
     }
   }
   return rtv;
}

void vm_dir_close(vm_dir *dd) {
  if (dd != NULL) {
    if (d_name != NULL) {
      chdir(d_name);
      free(d_name);
      d_name = NULL;
      }
    closedir(dd);
    }
}

/*
 * findfirst, findnext, findclose direct emulation
 * for old ala Windows applications
 */
Ipp32s vm_string_findnext(vm_findptr handle, vm_finddata_t *fileinfo) {
  Ipp32s rtv = 1;
  Ipp64u sz;
  Ipp32u atr;
  if (vm_dir_read(handle, fileinfo[0].name, MAX_PATH))
    if (vm_file_getinfo(fileinfo[0].name, &sz, &atr)) {
      fileinfo[0].size = sz;
      fileinfo[0].attrib = atr;
      rtv = 0;
    }
  return rtv;
  }

vm_findptr vm_string_findfirst(vm_char *filespec, vm_finddata_t *fileinfo) {
  vm_findptr dd;
  vm_dir_open(&dd, filespec);
  if (dd != NULL)
    vm_string_findnext(dd, fileinfo);
  return dd;
  }

Ipp32s vm_string_findclose(vm_findptr handle) {
  return closedir(handle);
  }

Ipp64u vm_dir_get_free_disk_space( void ) {
  Ipp64u rtv = 0;
#if defined(__ANDROID__)
  struct statfs fst;
  if (statfs(".", &fst) >= 0) {
    rtv = fst.f_bsize*fst.f_bavail;
    }
#else
  struct statvfs fst;
  if (statvfs(".", &fst) >= 0) {
    rtv = fst.f_bsize*fst.f_bavail;
    }
#endif
  return rtv;
  }

void vm_string_splitpath(const vm_char *path, char *drive, char *dir, char *fname, char *ext) {

    if (path && drive && dir && fname && ext) {
        drive[0] = '\0';
        dir[0] = '\0';
        strcpy(fname, path);
        ext[0] = '\0';
    }
}

Ipp32s vm_file_vfprintf(vm_file *fd, vm_char* format, va_list argptr)
{
    Ipp32s sts = 0;
    va_list copy;
    va_copy(copy, argptr);
    sts = vfprintf(fd, format,  copy);
    va_end(argptr);
    return sts;
}

#else
# pragma warning( disable: 4206 )
#endif
