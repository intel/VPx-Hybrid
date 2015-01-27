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

File Name: vm_file_unux.h

\* ****************************************************************************** */

#ifndef VM_FILE_UNIX_H
#  define VM_FILE_UNIX_H
#  include <stdio.h>
#  include <stdarg.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <dirent.h>

# ifdef __APPLE__
#  define OSX
# endif
#if !defined MAX_PATH
#  define MAX_PATH 260
#endif

typedef FILE vm_file;
typedef DIR vm_dir;

# define vm_stderr  stderr
# define vm_stdout  stdout
# define vm_stdin    stdin
/*
 * file access functions
 */
# if defined(__ANDROID__) || defined(OSX) || defined(LINUX64)
/* native fopen is 64-bits on OSX */
#  define vm_file_fopen    fopen
# else
#  define vm_file_fopen    fopen64
# endif

# define vm_file_fclose     fclose
# define vm_file_feof         feof
# define vm_file_remove  remove

/*
 * binary file IO */
# define vm_file_fread    fread
# define vm_file_fwrite    fwrite

/*
 * character (string) file IO */
# define vm_file_fgets      fgets
# define vm_file_fputs      fputs
# define vm_file_fscanf     fscanf
# define vm_file_fprintf    fprintf

/* temporary file support */
# define vm_file_tmpfile      tmpfile
# define vm_file_tmpnam       tmpnam
# define vm_file_tmpnam_r     tmpnam_r
# define vm_file_tempnam      tempnam

#endif //VM_FILE_UNIX_H

