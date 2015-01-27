/* ****************************************************************************** *\

Copyright (C) 2006-2013 Intel Corporation.  All rights reserved.

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

File Name: umc_par_reader.cpp

\* ****************************************************************************** */

#include "umc_par_reader.h"

using namespace UMC;

//! Maximum size of parfile is limited to avoid processing of big wrong files
#define MAX_PARFILE_SIZE 16384

//! \def DELIMITER VM_STRING(";, \t")
/*! Separator characters for arguments */
#define DELIMITER VM_STRING(";, \t")

//! \def CODEC_SECTION_TITLE VM_STRING(";, \t")
/*! Codec specific section title */
#define CODEC_SECTION_TITLE VM_STRING("property")

/*!
 OptionInfo array is is used to provide ParamList class with descriptions about
 command line and parfile parameters. It is passed to addOptionInfo function.
 All entries in limits field are delimited with ";, \t" (see DELIMITER).
 When synonym field is not null, it points to base name, while field name is
 a synonym name, which can be used too. In this case the next fields are ignored.
 The array has to be terminated with a record containing null in field name.
 0-length name is valid and used for options without a key.
*/
//example
#if(0)
const ParamList::OptionInfo SomeOptions[] = {
  {VM_STRING("parfile"),   0, 1, ParamList::argStr, ParamList::checkNone, 0, VM_STRING("parameter file name")},
  {VM_STRING("bitrate"),   0, 1, ParamList::argFlt, ParamList::checkMinMax, VM_STRING("0,1.e8"), VM_STRING("bitrate in bps")},
  {VM_STRING("b"), VM_STRING("bitrate"), },
  {VM_STRING("width"),     0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1;4096"), VM_STRING("frame width in pixels")},
  {VM_STRING("w"), VM_STRING("width"), },
  {VM_STRING("height"),    0, 1, ParamList::argInt, ParamList::checkMinMax, VM_STRING("1 4096"), VM_STRING("frame height in pixels")},
  {VM_STRING("h"), VM_STRING("height"), },
  {VM_STRING("codec"),     0, 1, ParamList::argStr, ParamList::checkSet, VM_STRING("mpeg2,mpeg4;264 263, 261;vc1 ;dv"), VM_STRING("codec name")},
  {VM_STRING("framerate"), 0, 1, ParamList::argFlt, ParamList::checkNone, 0, VM_STRING("frame rate in fps")},
  {0,} // list terminator
};
#endif

ParamList::ParamList() : m_plist(0), m_parfile_data(0), m_listOptionInfo(0)
{
  m_numParams = 0;
  m_parfile_size = 0;
  m_numOptionInfo = 0;
}

// destructor frees all allocated memory
ParamList::~ParamList()
{
  while (m_plist != 0) {
    Parameter* prev = m_plist;
    m_plist = m_plist->m_next;
    delete prev;
  }
  if(m_parfile_data != 0)
    delete m_parfile_data;
  while (m_listOptionInfo != 0) {
    OptionInfoGroup* pdescr = m_listOptionInfo;
    m_listOptionInfo = m_listOptionInfo->next;
    delete pdescr;
  }
}

// returns base name if name is known or is a known synonym
// returns 0 if the name is unknown
const vm_char* ParamList::getBaseName(const vm_char* name)
{
  const OptionInfo* info;
  Ipp32s index;

  if(name == 0 /*|| name[0] == 0*/)
    return 0;
  // find the name in description (incl. synonyms)
  for(index = 0; index < m_numOptionInfo; index++) {
    info = getOptionInfo(index);
    if(info==0 || info->name==0 /*|| info->name[0]==0*/)
      return 0;
    if(!vm_string_strcmp(name, info->name)) {
      if(info->synonym) {
        return info->synonym;
      }
      return info->name;
    }
  }
  return 0;
}

// finds description for parameter name or synonym
// returns 0 if the name is unknown
const ParamList::OptionInfo* ParamList::findOptionInfo(const vm_char* name)
{
  const OptionInfo* info;
  const vm_char* basename;
  Ipp32s index;

  basename = getBaseName(name);
  if(basename == 0) // unknown name
    return 0;
  // find the name in description (incl. synonyms)
  for(index = 0; index < m_numOptionInfo; index++) {
    info = getOptionInfo(index);
    if(info==0 || info->name==0 /*|| info->name[0]==0*/)
      return 0;
    if(!vm_string_strcmp(basename, info->name)) {
      return info;
    }
  }
  // will never be here
  return 0;
}

// finds the parameter record read by name or synonym
// returns 0 if not found
ParamList::Parameter* ParamList::findParam(const vm_char* name)
{
  const vm_char* basename;
  Parameter* ppar;

  if(name == 0 /*|| name[0] == 0*/)
    return 0;
  basename = getBaseName(name);
  if(basename == 0)
    basename = name; // name without description, use as is

  for(ppar = m_plist; ppar != 0; ppar = ppar->m_next) {
    if( !vm_string_strcmp(basename, ppar->m_name))
      return ppar;
  }
  return 0;
}

// adds new parameter record by name or synonym
// fills record with name and info description exists
// synonym is translated to base name
// returns 0 if a record with the name is present already
ParamList::Parameter* ParamList::addParam(const vm_char* name)
{
  Parameter* ppar;
  Parameter* plast = 0;
  const OptionInfo* info;

  // just for debug, can be removed
  ppar = findParam(name);

  // append new record
  for(ppar = m_plist; ppar != 0; ppar = ppar->m_next) {
    plast = ppar;
  }
  ppar = new Parameter;
  if(ppar == 0)
    return 0; // malloc failed

  if(plast)
    plast->m_next = ppar;
  else
    m_plist = ppar;
  m_numParams++;

  // find the name in description (incl. synonyms) and set name
  info = findOptionInfo(name);
  ppar->m_info = info;
  if(info != 0) {
    ppar->m_name = info->name;
  } else {
    ppar->m_name = name;
  }

  return ppar;
}

// reads parameter file of the following format:
// first word in each line is parameter name
// name can be preceded with "-" and is followed by '=' sign or/and spaces
// then follow parameters, separated by DELIMITER
// symbol '#' comments to the end of line.
// CODEC_SECTION_TITLE codec_name separates the section which is only read
// when codec argument matches codec_name
// parameters from file don't override command line parameters
// should not be called before readCLine or second time
// Examples:
//  -property h263 # comment:encoder to be used
//   smth = 2.54,  1e-2  -7 ;0 # 4 Ipp32f values
// # completely commented line

Status ParamList::readPFile(const vm_char * fname, const vm_char * codec)
{
  vm_file*    InputFile;
  vm_char* pfdata;
  Ipp32s   filelen;

  Ipp32s p, t, endl, endt, next;
  Ipp32s comment;
  Ipp32s wrong_section = 0;
  Ipp32s narg;
  Parameter* rec;
  Status ret;

  if(m_parfile_data)
    return -2; // single parfile for now

  InputFile = vm_file_fopen(fname, VM_STRING("rt"));
  if(InputFile == 0)
    return UMC_ERR_NOT_FOUND;
  vm_file_fseek( InputFile, 0, VM_FILE_SEEK_END);
  filelen = (Ipp32s)vm_file_ftell(InputFile);
  if(filelen <= 0 || filelen > MAX_PARFILE_SIZE) {
    vm_file_fclose(InputFile);
    return UMC_ERR_INVALID_STREAM;
  }
  vm_file_fseek( InputFile, 0, VM_FILE_SEEK_SET);

  pfdata = new vm_char[filelen+1];
  if(!pfdata) {
    vm_file_fclose(InputFile);
    return UMC_ERR_ALLOC;
  }
  m_parfile_data = pfdata;
  m_parfile_size = filelen+1;

  filelen = vm_file_read(pfdata, sizeof(vm_char), filelen, InputFile);
  vm_file_close(InputFile);
  if(filelen > m_parfile_size) // can be less
    return UMC_ERR_FAILED;
  m_parfile_size = filelen;
  pfdata[filelen] = 0;

  // parse strings now
  endl = 0;
  for(next = 0;next<filelen;) {
    comment = 0;
    for(p=next; p<filelen && pfdata[p] != '\n'; p++) {// find end of line
      if(pfdata[p]=='#' && !comment) {// comment
        pfdata[p] = 0;
        comment = 1;
        endl = p;
      }
    }
    if(!comment)
      endl = p;
    pfdata[p] = 0; // change '\n' to 0
    for(t = next; isspace(pfdata[t]) || pfdata[t]=='-'; t++); // start of 1st token (name)
    next = p+1; // now points to the next line
    for(endt = t; !isspace(pfdata[endt]) && pfdata[endt]!=0 && pfdata[endt]!='='; endt++); // end of 1st token (name)
    if(t==endt)
      continue; // empty line or EOF
    pfdata[endt] = 0; // end of token

    // If start of codec specific section
    // Skip to next if doesn't match codec name
    if( !vm_string_strcmp(pfdata + t, CODEC_SECTION_TITLE)) {
      for(t = endt+1; t<endl && (isspace(pfdata[t]) || pfdata[t]=='='); t++); // start 1st token
      endt = (Ipp32s)vm_string_strcspn(pfdata+t, DELIMITER);
      endt+=t;
      pfdata[endt] = 0;
      // check if no codec name or if it matches
      if(codec == 0)
        getValue(VM_STRING("codec"), &codec); // if set in parfile
      if( endt == t || (codec != 0 && !vm_string_stricmp(pfdata + t, codec)) ) {
        wrong_section = 0; // enable till next section
      } else {
        wrong_section = 1; // disable till next section
      }
      continue;
    }
    if(wrong_section != 0)
      continue;

    // have token (name)
    rec = findParam(pfdata + t);
    if(rec == 0) {
      rec = addParam(pfdata + t); // add record if new
      if(rec == 0)
        return UMC_ERR_ALLOC; // only if malloc failed (or -3)
    }
    else {
      continue; // don't overwrite from par-file
    }

    for(t = endt+1; t<endl && (isspace(pfdata[t]) || pfdata[t]=='='); t++); // start 1st token
    // now fill the record
    for(narg = 0; ;narg++) {
      endt = (Ipp32s)vm_string_strcspn(pfdata+t, DELIMITER);
      if(endt==0)
        break; // no token
      endt+=t;
      pfdata[endt] = 0;
      if(rec->m_info) { // have description
        if(narg >= rec->m_info->numArgs)
          break; // ignore extra args
        ret = rec->appendValue(pfdata+t);
        if(ret != UMC_OK)
          return ret;
      }
      else { // unknown parameter name - just remember
        rec->appendValue(pfdata+t);
      }
      t = endt+1+(Ipp32s)vm_string_strspn(pfdata+endt+1, DELIMITER);
    }
  }
  return UMC_OK;
}

// reads command line of the following format:
// each parameter name has to be separated from values
// numeric values can start with '-' only for known names
// command line does override parameters from parameter file
// if meets -parfile parfilename then proceed it in the end
// Examples:
//   -codec h263 -smth  2.54  1e-2  -7 0

Status ParamList::readCLine(vm_char ** cline, Ipp32s argc)
{
  Ipp32s arg, narg;
  Parameter* rec;
  Status ret;

  for(arg=0; arg<argc; arg++) {
    const vm_char* name;
    if(cline[arg][0]=='-') {
      // have token (name)
      name = cline[arg]+1;
    } else {
      name = VM_STRING(""); // will get without option key
      arg--;
    }

    rec = findParam(name);
    if(rec == 0) {
      rec = addParam(name);
      if(rec == 0)
        return UMC_ERR_ALLOC; // only if malloc failed (or -3)
    }
    // overwrites if exist
    for(narg = 0; arg+1<argc; arg++, narg++) {
      if(rec->m_info) { // have description
        if(narg >= rec->m_info->numArgs)
          break; // ignore extra args
        ret = rec->appendValue(cline[arg+1]);
        if(ret != UMC_OK)
          return ret;
      }
      else { // unknown parameter name - just remember till next option
        if(cline[arg+1][0]=='-')
          break;
        rec->appendValue(cline[arg+1]);
      }
    }
  }

  // read par-file if have met
  //rec = findParam(VM_STRING("parfile")); // name can be changed
  //if(rec != 0 && rec->m_nargs == 1) {
  //  ret = readPFile(rec->getValue(0));
  //  if(ret != UMC_OK)
  //    return ret;
  //}

  return UMC_OK;
}


// returns UMC_ERR_NOT_FOUND if no record,
//   UMC_ERR_NO_ARG if no such argument
// else sets *val and return UMC_OK if record type matches or
//   UMC_WRN_TYPE_MISMATCH otherwise
Status ParamList::getValue(const vm_char* name, Ipp32s* val, Ipp32s vnum)
{
  const vm_char* argstr;
  Parameter *rec = findParam(name);
  if(rec == 0)
    return UMC_ERR_NOT_FOUND;
  if(rec->m_nargs <= vnum)
    return UMC_ERR_NO_ARG;
  argstr = rec->getValue(vnum);
  if(argstr == 0)
    return UMC_ERR_NO_ARG;
  if(1 == vm_string_sscanf(argstr, VM_STRING("%d"), (int*)val) &&
     (rec->m_info == 0 || rec->m_info->argType == argInt))
  {
    return UMC_OK;
  }
  return UMC_WRN_TYPE_MISMATCH;
}

// returns UMC_ERR_NOT_FOUND if no record,
//   UMC_ERR_NO_ARG if no such argument
// else sets *val and return UMC_OK if record type matches or
//   UMC_WRN_TYPE_MISMATCH otherwise
Status ParamList::getValue(const vm_char* name, Ipp64f* val, Ipp32s vnum)
{
  const vm_char* argstr;
  Parameter *rec = findParam(name);
  if(rec == 0)
    return UMC_ERR_NOT_FOUND;
  if(rec->m_nargs <= vnum)
    return UMC_ERR_NO_ARG;
  argstr = rec->getValue(vnum);
  if(argstr == 0)
    return UMC_ERR_NO_ARG;
  if(1 == vm_string_sscanf(argstr, VM_STRING("%lf"), val) &&
    (rec->m_info == 0 || rec->m_info->argType == argFlt))
  {
    return UMC_OK;
  }
  return UMC_WRN_TYPE_MISMATCH;
}

// returns UMC_ERR_NOT_FOUND if no record,
//   UMC_ERR_NO_ARG if no such argument
// else sets *val and return UMC_OK if record type matches or
//   UMC_WRN_TYPE_MISMATCH otherwise
Status ParamList::getValue(const vm_char* name, Ipp32f* val, Ipp32s vnum)
{
  const vm_char* argstr;
  Parameter *rec = findParam(name);
  if(rec == 0)
    return UMC_ERR_NOT_FOUND;
  if(rec->m_nargs <= vnum)
    return UMC_ERR_NO_ARG;
  argstr = rec->getValue(vnum);
  if(argstr == 0)
    return UMC_ERR_NO_ARG;
  if(1 == vm_string_sscanf(argstr, VM_STRING("%f"), val) &&
    (rec->m_info == 0 || rec->m_info->argType == argFlt))
  {
    return UMC_OK;
  }
  return UMC_WRN_TYPE_MISMATCH;
}

// returns UMC_ERR_NOT_FOUND if no record,
//   UMC_ERR_NO_ARG if no such argument
// else sets *val and return UMC_OK if record type matches or
//   UMC_WRN_TYPE_MISMATCH otherwise
Status ParamList::getValue(const vm_char* name, const vm_char** val, Ipp32s vnum)
{
  Parameter *rec = findParam(name);
  if(rec == 0)
    return UMC_ERR_NOT_FOUND;
  if(rec->m_nargs <= vnum)
    return UMC_ERR_NO_ARG;
  *val = rec->getValue(vnum);
  if(*val == 0)
    return UMC_ERR_NO_ARG;
  if(rec->m_info == 0 || rec->m_info->argType == argStr) {
    return UMC_OK;
  }
  return UMC_WRN_TYPE_MISMATCH;
}

// returns UMC_ERR_NOT_FOUND if no record,
// UMC_WRN_TYPE_MISMATCH if type mismatch or the record has arguments
// else return UMC_OK with success
Status ParamList::getValue(const vm_char* name)
{
  Parameter *rec = findParam(name);
  if(rec == 0)
    return UMC_ERR_NOT_FOUND;
  if(rec->m_info == 0) {
    if(rec->m_nargs == 0)
      return UMC_OK;
  } else if(rec->m_info->argType == argOpt)
    return UMC_OK;
  return UMC_WRN_TYPE_MISMATCH;
}


// adds group of parameters descriptions
// see top of the file for descr
// group name is supposed to be codec class name and used now only for information
Status ParamList::addOptionInfo(const vm_char* group_name, const OptionInfo* descr)
{
  const OptionInfo* rec;
  OptionInfoGroup* entry;
  Ipp32s count;
  const vm_char* ptr;

  if(descr == 0 || descr->name == 0 /*|| descr->name[0] == 0*/)
    return UMC_ERR_NULL_PTR; // no descriptions or empty

  for(entry = m_listOptionInfo; entry != 0; entry = entry->next)
    if( 0 == vm_string_strcmp(group_name, entry->groupName))
      return UMC_OK; // already exist

  // count entries
  // can check input descriptions here
  // and return UMC_ERR_INVALID_STREAM; on error
  for(rec = descr, count = 0;
    rec->name != 0 /*&& rec->name[0] != 0*/; rec++, count++)
  {
    Ipp32s i;
    // check if name exists
    const OptionInfo* old = findOptionInfo(rec->name);
    if(old != 0)
      return UMC_ERR_INVALID_STREAM; // duplicates
    if(rec->synonym != 0) {
      if(rec->synonym[0] == 0)
        return UMC_ERR_INVALID_STREAM;
      if(0 == findOptionInfo(rec->synonym)) {
        Ipp32s found = 0;
        for(i=0; descr[i].name != 0 /*&& descr[i].name[0] != 0*/; i++) {
          if(0 == vm_string_strcmp(descr[i].name, rec->synonym)) {
            found = 1;
            break;
          }
        }
        if(found == 0) // synonym name is undefined
          return UMC_ERR_INVALID_STREAM;
      }
      continue; // to the next line
    } else {
      for(i=0; i<count; i++)
        if(0 == vm_string_strcmp(descr[i].name, rec->name))
          return UMC_ERR_INVALID_STREAM; // repeated name
    }
    ptr = rec->limits;
    // check description
    if(rec->checkType == checkMin || rec->checkType == checkMax || rec->checkType == checkMinMax) {
      if(rec->limits == 0)
        return UMC_ERR_INVALID_STREAM;
      if(rec->checkType == checkMinMax) {
        ptr += vm_string_strcspn(rec->limits, DELIMITER);
        ptr += vm_string_strspn(ptr, DELIMITER);
      }
      if(argInt == rec->argType) {
        Ipp32s imin, imax;
        if(1 != vm_string_sscanf(rec->limits, VM_STRING("%d"), (int*)&imin))
          return UMC_ERR_INVALID_STREAM;
        if(rec->checkType == checkMinMax) {
          if (1 != vm_string_sscanf(ptr, VM_STRING("%d"), (int*)&imax) || imin > imax)
            return UMC_ERR_INVALID_STREAM;
        }
      } else if(argFlt == rec->argType) {
        Ipp64f dmin, dmax;
        if(1 != vm_string_sscanf(rec->limits, VM_STRING("%lf"), &dmin))
          return UMC_ERR_INVALID_STREAM;
        if(rec->checkType == checkMinMax) {
          if (1 != vm_string_sscanf(ptr, VM_STRING("%lf"), &dmax) || dmin > dmax)
            return UMC_ERR_INVALID_STREAM;
        }
      } else if(argOther != rec->argType)
        return UMC_ERR_INVALID_STREAM;

    } else if(rec->checkType == checkSet) {
      if(rec->limits == 0 || rec->limits[0] == 0)
        return UMC_ERR_INVALID_STREAM; // must be at least 1 entry
      do {
        if(argInt == rec->argType) {
          Ipp32s ival;
          if(1 != vm_string_sscanf(ptr,VM_STRING("%d"),(int*)&ival))
            return UMC_ERR_INVALID_STREAM;
        } else if(argFlt == rec->argType) {
          Ipp64f dval;
          if(1 != vm_string_sscanf(ptr,VM_STRING("%lf"),&dval))
            return UMC_ERR_INVALID_STREAM;
        } else
          break; //don't check not numbers
        ptr += vm_string_strspn(ptr, DELIMITER);
        ptr++; // next element or 0 in the end
      } while(*ptr != 0);
    } else if(rec->checkType != checkNone)
      return UMC_ERR_INVALID_STREAM;
  } // end of description group checking

  entry = new OptionInfoGroup;
  if(entry == 0)
    return UMC_ERR_ALLOC;
  entry->count = count;
  entry->entry = descr;
  entry->groupName = group_name;
  // add to the beginning of the list
  entry->next = m_listOptionInfo;
  m_listOptionInfo = entry;
  m_numOptionInfo += count;

  // option could have been read before - update
  Parameter* ppar = m_plist;
  while(ppar != 0) {
    const OptionInfo *pInfo;
    if(ppar->m_info == 0) {
      pInfo = findOptionInfo(ppar->m_name);
      if(pInfo != 0 && ppar->m_nargs == pInfo->numArgs) {
        Ipp32s i;
        for(i=0; i<ppar->m_nargs; i++) {
          if(UMC_OK != ppar->checkValue(ppar->getValue(i)))
            break;
        }
        if(i == ppar->m_nargs) {
          ppar->m_name = pInfo->name;
          ppar->m_info = pInfo;
        }
      }
    }
    ppar = ppar->m_next;
  }

  return UMC_OK;
}

// return description by index
// used for access to accumulated descriptions
const ParamList::OptionInfo* ParamList::getOptionInfo(Ipp32s index)
{
  OptionInfoGroup* dentry = m_listOptionInfo;
  Ipp32s numrec = m_numOptionInfo;
  if(index >= m_numOptionInfo)
    return 0;

  while(dentry->count <= index) {
    index -= dentry->count;
    numrec -= dentry->count; // to verify
    dentry = dentry->next;
    if(dentry == 0 || numrec <= 0)
      return 0;
  }

  return dentry->entry + index;
}


// returns text description for parameter name
const vm_char* ParamList::helpParam(const vm_char* name)
{
  const vm_char* text;

  const OptionInfo* info = findOptionInfo(name);
  if(info == 0)
    text = VM_STRING("");
  else
    text = info->comment;

  return text;
}

Status ParamList::dumpHelp(vm_file* out)
{
  OptionInfoGroup  *dentry = m_listOptionInfo;
  const OptionInfo *entry;

  if(out == 0)
    return UMC_ERR_NULL_PTR;

  while(dentry != 0) {
    vm_string_fprintf(out, VM_STRING("[%.80s]\n"),
      (dentry->groupName!=0)?dentry->groupName:VM_STRING(" "));
    for( entry = dentry->entry; entry->name != 0 /*&& entry->name[0] != 0*/; entry++) {
      if(entry->name[0] != 0)
        vm_string_fprintf(out, VM_STRING("-%-11s"), entry->name);
      else // possible empty name, say for input file
        vm_string_fprintf(out, VM_STRING("%12s"), VM_STRING("[no prefix]"));
      if(entry->synonym) {
        vm_string_fprintf(out, VM_STRING(" same as -%s\n"), entry->synonym);
        continue;
      }
      if(entry->numArgs > 1)
        vm_string_fprintf(out, VM_STRING(" (%d x "), entry->numArgs);
      else if(entry->numArgs == 1)
        vm_string_fprintf(out, VM_STRING(" ("));
      if(entry->numArgs > 0)
        vm_string_fprintf(out, VM_STRING("%s)"),
         (entry->argType == argInt)?VM_STRING("integer") :
        ((entry->argType == argFlt)?VM_STRING("Ipp32f"  ) :
        ((entry->argType == argStr)?VM_STRING("str"):VM_STRING("unknown"))));
      if(entry->comment)
        vm_string_fprintf(out, VM_STRING(" %s"), entry->comment);
      vm_string_fprintf(out, VM_STRING("\n"));
    }
    dentry = dentry->next;
  }

  return UMC_OK;
}
/*! \fn Status ParamList::dumpParams(FILE* out)
Prints information about all parameters read with arguments to stream \a out
\param out stream pointer
\return UMC_OK on success
*/
Status ParamList::dumpParams(vm_file* out)
{
  if(out == 0)
    return UMC_ERR_NULL_PTR;

  Parameter* ppar = m_plist;
  while(ppar != 0) {
    Ipp32s i;
    vm_string_fprintf(out, VM_STRING("%11s "), ppar->m_name);
    for(i=0; i<ppar->m_nargs; i++) {
      if(i>0)
        vm_string_fprintf(out, VM_STRING("; "));
      vm_string_fprintf(out, VM_STRING("%s"), ppar->getValue(i));
    }
    const vm_char* helpstr = helpParam(ppar->m_name);
    if(vm_string_strlen(helpstr) > 0)
      vm_string_fprintf(out, VM_STRING("\t# %s"), helpParam(ppar->m_name));
    vm_string_fprintf(out, VM_STRING("\n"));
    ppar = ppar->m_next;
  }
  return UMC_OK;
}

ParamList::Parameter::~Parameter()
{
  ValueChain* ptr = m_values;
  while(ptr != 0) {
    ValueChain* todel = ptr;
    ptr = ptr->next;
    delete todel;
  }
}

Status ParamList::Parameter::appendValue(const vm_char* value)
{
  ValueChain* ptr = m_values;
  Ipp32s numcheck = 0;
  Status ret = UMC_OK;
  if(ptr != 0)
    for(numcheck=1; ptr->next != 0; ptr = ptr->next) {
      numcheck++;
    }
  //VM_ASSERT(numcheck == m_nargs);
  //VM_ASSERT(numcheck == m_nargs);
  //if(numcheck != m_nargs) {
  //  m_nargs = numcheck; // self fix
  //}
  if(m_info)
    ret = checkValue(value);
  if(0 == ptr) {
    m_values = new ValueChain;
    ptr = m_values;
  } else {
    ptr->next = new ValueChain;
    ptr = ptr->next;
  }
  if(ptr == 0)
    return UMC_ERR_ALLOC;
  ptr->next = 0;
  ptr->svalue = value;
  m_nargs++;
  return ret;
}
const vm_char* ParamList::Parameter::getValue(Ipp32s index)
{
  ValueChain* ptr = m_values;
  if(index>=m_nargs)
    return 0;
  while(index>0) {
    if(ptr->next == 0)
      return 0;
    ptr = ptr->next;
    index--;
  }
  return ptr->svalue;
}

// check values according to description's rules
// return UMC_OK on success
// UMC_ERR_INVALID_STREAM otherwise
Status ParamList::Parameter::checkValue(const vm_char* value)
{
  Ipp32s res;

  if(m_info != 0)
  switch(m_info->checkType) {
    case checkMin:
    case checkMax:
    case checkMinMax:
      {
        const vm_char *limmin, *limmax;
        limmax = limmin = m_info->limits;
        if(m_info->checkType == checkMinMax) {
          limmax += vm_string_strcspn(limmin, DELIMITER);
          limmax += vm_string_strspn(limmax, DELIMITER);
        }
        switch(m_info->argType) {
          case argFlt:
            {
              Ipp64f dval, dmin, dmax;
              //dval = atof(value);
              res = vm_string_sscanf(value, VM_STRING("%lf"), &dval);
              if(res != 1) {
                goto check_error;
              }
              if(m_info->checkType != checkMax) {
                vm_string_sscanf(limmin, VM_STRING("%lf"), &dmin);
                if(dmin>dval) {
                  //*value = limmin;
                  goto check_error;
                }
              }
              if(m_info->checkType != checkMin) {
                vm_string_sscanf(limmax, VM_STRING("%lf"), &dmax);
                if(dmax<dval) {
                  //*value = limmax;
                  goto check_error;
                }
              }
            } break;
          case argInt:
            {
              Ipp32s ival, imin, imax;
              //ival = vm_string_atoi(value);
              res = vm_string_sscanf(value, VM_STRING("%d"), (int*)&ival);
              if(res != 1) {
                goto check_error;
              }
              if(m_info->checkType != checkMax) {
                vm_string_sscanf(limmin, VM_STRING("%d"), (int*)&imin);
                if(imin>ival) {
                  //*value = limmin;
                  goto check_error;
                }
              }
              if(m_info->checkType != checkMin) {
                vm_string_sscanf(limmax, VM_STRING("%d"), (int*)&imax);
                if(imax<ival) {
                  //*value = limmax;
                  goto check_error;
                }
              }
            } break;
          case argOther:
            break;
          default:
            goto check_error;
        }
      } break;
    case checkSet:
      {
        size_t len;
        const vm_char *wrd = m_info->limits;
        switch(m_info->argType) {
          case argStr:
            do {
              len = vm_string_strcspn(wrd, DELIMITER);
              if( 0 == vm_string_strncmp(value, wrd, len))
                break;
              wrd += len + vm_string_strspn(wrd+len, DELIMITER);
            } while(*wrd != 0);
            break;
          case argFlt:
            {
              Ipp64f dval;// = atof(value);
              if(1 != vm_string_sscanf(value, VM_STRING("%lf"), &dval))
                break;
              for(; *wrd != 0; ) {
                Ipp64f dchk; // = atof(wrd);
                vm_string_sscanf(wrd, VM_STRING("%lf"), &dchk);
                if(dchk == dval)
                  break;
                wrd += vm_string_strcspn(wrd, DELIMITER);
                wrd += vm_string_strspn(wrd, DELIMITER);
              }
            } break;
          case argInt:
            {
              Ipp32s ival; // = vm_string_atoi(value);
              if(1 != vm_string_sscanf(value, VM_STRING("%d"), (int*)&ival))
                break;
              for(; *wrd != 0; ) {
                Ipp32s ichk; // = vm_string_atoi(wrd);
                vm_string_sscanf(wrd, VM_STRING("%d"), (int*)&ichk);
                if(ichk == ival)
                  break;
                wrd += vm_string_strcspn(wrd, DELIMITER);
                wrd += vm_string_strspn(wrd, DELIMITER);
              }
            } break;
          case argOpt:
            break;
          default:
            goto check_error;
      }
        if(*wrd == 0)
          goto check_error;
      } break;
    case checkNone:
      break;
    default:
      goto check_error;
  }
  return UMC_OK;
check_error:
  return UMC_ERR_INVALID_STREAM;
}

