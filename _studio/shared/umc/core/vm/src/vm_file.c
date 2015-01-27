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

File Name: vm_file.c

\* ****************************************************************************** */
/*
 * VM 64-bits buffered file operations library
 *       common implementation
 */
#include "vm_file.h"
#if defined(LINUX32) || defined(__APPLE__)
# define SLASH '/'
#else
# define SLASH '\\'
# if _MSC_VER >= 1400
#   pragma warning( disable:4996 )
# endif
#endif
#if defined (__ICL)
/* non-pointer conversion from "unsigned __int64" to "Ipp32s={signed int}" may lose significant bits */
#pragma warning(disable:2259)
#endif
/*
 * file name manipulations */
/*
 * return only path of file name */
void vm_file_getpath(vm_char *filename, vm_char *path, int nchars) {
  /* go to end of line and then move up until first SLASH will be found */
  Ipp32s len;
  path[0] = '\0';
  len = (Ipp32s) vm_string_strlen(filename);
  while(len && (filename[len--] != SLASH));
  if (len) {
      memcpy_s((void *)path, nchars, (const void *)filename, (len <= nchars) ? len+1 : nchars);
      path[(len <= nchars) ? len+1 : nchars] = '\0';
    }
  }

/*
 * return base file name free of path and all suffixes
 */
void vm_file_getbasename(vm_char *filename, vm_char *base, int nchars) {
  Ipp32s chrs = 0;
  vm_char *p, *q0, *q1, *s;
  base[0] = '\0';
  q0 = q1 = NULL;
  p = vm_string_strchr(filename, '.'); /* first invocation of . */
  s = filename;
  do {
    q0 = vm_string_strchr(s, SLASH);
    if (q0 != NULL) {
      q1 = q0;
      s = q0+1;
      }
    } while( q0 != NULL );
  if (p == NULL)
    p = &filename[vm_string_strlen(filename)];
  if ( q1 == NULL )
    q1 = filename;
  chrs = (Ipp32s) (p - q1);
  if (chrs) {
    if (q1[0] == SLASH) {
      ++q1;
      --chrs;
      }
    if (chrs > nchars)
      chrs = nchars-1;
    memcpy_s((void *)base, nchars, (const void *)q1, chrs);
    base[chrs] = '\0';
    }
  }
/*
 * return full file suffix or nchars of suffix if nchars is to small to fit the suffix
 * !!! if more then one suffix applied then only first from the end of filename will be found
 */
void vm_file_getsuffix(vm_char *filename, vm_char *suffix, int nchars) {
  /* go to end of line and then go up until we will meet the suffix sign . or
   * to begining of line if no suffix found */
  Ipp32s len, i = 0;
  len = (Ipp32s) vm_string_strlen(filename);
  suffix[0] = '\0';
  while(len && (filename[len--] != '.'));
  if (len) {
    len += 2;
    for( ; filename[len]; ++len) {
      suffix[i] = filename[len];
      if (++i >= nchars)
        break;
      }
    suffix[i] = '\0';
    }
  }

#define ADDPARM(A)                    \
  if ((Ipp32u)nchars > vm_string_strlen(A)) {   \
    vm_string_strcat_s(filename, nchars, A);              \
    offs = (Ipp32u) vm_string_strlen(filename);          \
    nchars -= offs;                   \
    if (nchars)                       \
      filename[offs] = SLASH;         \
    ++offs;                           \
    --nchars;                         \
    filename[offs] = '\0';            \
    }
/*
 * prepare complex file name according with OS rules:
 *    / delimiter for unix and \ delimiter for Windows */
void vm_file_makefilename(vm_char *path, vm_char *base, vm_char *suffix, vm_char *filename, int nchars) {
  Ipp32u offs = 0;
  filename[0] = '\0';
  if ((path != NULL) && (vm_string_strlen(path) < (Ipp32u)nchars))
    ADDPARM(path)
  if (nchars && (base != NULL))
    ADDPARM(base)
    if (nchars && (suffix != NULL)) {
      if (offs == 0) {
        filename[offs++] = '.';
        filename[offs] = '\0';
        --nchars;
        }
      else
        if (filename[offs-1] == SLASH)
          filename[offs-1] = '.';
      ADDPARM(suffix)
      }
    /* remove SLASH if exist */
    if (filename[offs-1] == SLASH)
      filename[offs-1] = '\0';
  }



