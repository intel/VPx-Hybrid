/* ****************************************************************************** *\

Copyright (C) 2006-2010 Intel Corporation.  All rights reserved.

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

File Name: umc_par_reader.h

\* ****************************************************************************** */

#ifndef __UMC_PAR_READER_H__
#define __UMC_PAR_READER_H__

#include "vm_types.h"
#include "vm_file.h"
#include "vm_strings.h"
#include "umc_structures.h"


/*! \note
Implementation PROBLEMS and limitations:
name for unknown parameters has to be const
arguments have to be const
*/

//! Common namespace for Unified Media Components
namespace UMC
{

// additional error codes
//! Object not found.
#define UMC_ERR_NOT_FOUND -899
//! Argument number is greater than argument count.
#define UMC_ERR_NO_ARG -898
//! Request type differs from option description.
#define UMC_WRN_TYPE_MISMATCH 80


  //! Parameter List class.
  /*!
  ParamList class is used to parse, store command line options and
  parameters from parameter file and then access them.
  */

class ParamList {
  public:
    //! Types for option arguments. Used in OptionInfo.
    enum ArgType {
      argOther = 0,  //!< Undefined argument type
      argStr,        //!< String argument
      argInt,        //!< Integer argument
      argFlt,        //!< Float point value argument
      argOpt         //!< Option without arguments (flag)
    };

    //! Check modes for option arguments. Used in OptionInfo.
    enum CheckType {
      checkNone,     //!< No argument checking
      checkMin,      //!< Checks if argument is not less
      checkMax,      //!< Checks if argument is not greater
      checkMinMax,   //!< Checks if argument is inside range
      checkSet       //!< dictionary check (argument belongs to provided set)
    };

    //! Option Information. Contains information about the option and description of arguments
    /*!
    OptionInfo array is is used to provide ParamList class with descriptions about
    command line and parfile parameters. It is passed to addOptionInfo function.
    All entries in limits field are delimited with ";, \t" (see DELIMITER).
    When synonym field is not null, it points to base name, while field name is
    a synonym name, which can be used too. In this case the next fields are ignored.
    The array has to be terminated with a record containing null in field name.
    0-length name is valid and is used for options without a key.
    */
    struct OptionInfo {
      const vm_char* name;      //!< Base name when synonym == 0
      const vm_char* synonym;   //!< Nonzero value means the option is synonym to pointed  base name
      Ipp32s         numArgs;   //!< Number of option arguments
      ArgType        argType;   //!< Type of the option arguments
      CheckType      checkType; //!< Check modes for option arguments
      const vm_char* limits;    //!< Min, max or set of possible values
      const vm_char* comment;   //!< Textual description of the option
    };

  public:
    //! Constructor.
    ParamList();
    //! Destructor.
    virtual ~ParamList();

    /** \overload
    */
    Status getValue(const vm_char* name, Ipp32s* val, Ipp32s vnum=0);
    /*! \overload
    */
    Status getValue(const vm_char* name, Ipp64f* val, Ipp32s vnum=0);
    /*! \overload
    */
    Status getValue(const vm_char* name, Ipp32f* val, Ipp32s vnum=0);
    //! Writes vnum argument of option name to the address pointed by val.
    /*!
    If argument type doesn't match, returns string representation.
    \param name name of the option.
    \param val destination address.
    \param vnum argument number.
    \return UMC_OK on success,
       UMC_WRN_TYPE_MISMATCH when argument type doesn't match,
       UMC_ERR_NOT_FOUND if no record,
       UMC_ERR_NO_ARG when \a vnum is grater than argument count.
    */
    Status getValue(const vm_char* name, const vm_char** val, Ipp32s vnum=0);
    /** \overload
    \return UMC_OK if the option is met. UMC_WRN_TYPE_MISMATCH if it has arguments.
    UMC_ERR_NOT_FOUND if there was no such option
    */
    Status getValue(const vm_char* name);

    //! Reads command line options.
    /*!
    Loads options from command line. Arguments in recognized options are checked
    if they should. (argv+1, argc-1) is typical parameters. Reads parameter file as well
    if it is specified in command line. Command line does override parameters from parameter file.
    \par Command line has the following format:
    Each option name has to be separated from values.
    Numeric values can start with '-' only for known (described) names.\n
    Examples:\n
      -codec h263 -smth  2.54  1e-2  -7 0
    \param cline address of option pointers array
    \param argc number of options to read
    \return UMC_OK on success
    */
    Status readCLine(vm_char ** cline, Ipp32s argc);
    //! Reads parameter file.
    /*!
    Loads parameters from the parameter file. Arguments in recognized parameters are checked
    if they should.
    \par Parameter file has the following format:
    First word in each line is parameter name,
    name can be preceded with "-" and is followed by '=' sign or/and spaces,
    then follow parameters, separated by DELIMITER.
    Symbol '#' comments to the end of line.
    CODEC_SECTION_TITLE codec_name separates the section which is only read
    when \codec argument matches codec_name.
    Parameters from file don't override command line parameters.
    Should not be called before readCLine or second time.\n
    Examples:\n
    bitrate 1500000\n
    -property h263    # comment: h263 specific\n
      smth = 2.54,  1e-2  -7 ;0 # 4 Ipp32f values\n
     property = mpeg2 # mpeg2 specific params\n
      interlaced\n
     property         # common again\n
    nthreads 2\n
    # completely commented line
    \param fname name of the parameter file
    \param codec codec name for specific parameters
    \return UMC_OK on success
    */
    Status readPFile(const vm_char * fname, const vm_char * codec);

    //! Adds group of parameters descriptions.
    /*! Accepts pointer \a descr to the array of OptionInfo descriptions named by \a group_name
    and performs checking of descriptions. Description must have no
    duplicated names, synonyms to not described option names, improper limits.
    \param group_name inessential textual name of the new group
    \param descr pointer to the array with OptionInfo descriptions
    \return UMC_OK on success, UMC_ERR_INVALID_STREAM on errors in descriptions.
    */
    Status addOptionInfo(const vm_char* group_name, const OptionInfo* descr);

    //! Returns text description of the parameter.
    /*! \return pointer to text description for parameter \a name. */
    const vm_char* helpParam(const vm_char* name);

    //! Dumps all option information.
    /*! Prints information about all legal parameters and option to stream \a out
    \param out stream pointer
    \return UMC_OK on success
    */
    Status dumpHelp(vm_file * out);

    //! Dumps all parameters read.
    /*! Prints information about all parameters read with arguments to stream \a out
    \param out stream pointer
    \return UMC_OK on success
    */
    Status dumpParams(vm_file* out);

  protected:
    //! Parameters or options related information.
    /*!
    Contains information of options met and argument values.
    Parameters are stored in chain.
    */
    class Parameter {
    public:
      //! Constructor
      Parameter() : m_info(0), m_next(0), m_values(0) {m_name=0; m_nargs=0;}
      //! Destructor
      ~Parameter();

      const vm_char    *m_name;  //!< Base option name
      const OptionInfo *m_info;  //!< Pointer to option description, if non-zero
      Ipp32s            m_nargs; //!< Number of arguments read
      Parameter        *m_next;  //!< Pointer to next parameter

      //! Appends next value to the parameter.
      Status appendValue(const vm_char* value);
      //! Returns pointer to argument.
      const vm_char* getValue(Ipp32s index);
      //! Verifies argument value according to check type.
      Status checkValue(const vm_char* value);

    protected:
      //! Argument values chain
      struct ValueChain {
        const vm_char *svalue;   //!< Pointer to string representation of the value
        ValueChain    *next;     //!< Points to next ValueChain
      };
      ValueChain *m_values;      //!< Pointer to the beginning of the value chain
    };

    Ipp32s m_numParams;             //!< Number of parameters read
    Parameter *m_plist;          //!< Head of list of parameters read
    Ipp32s   m_parfile_size;     //!< Size of loaded parameter file
    vm_char *m_parfile_data;     //!< Loaded parameter file

    //! \brief Adds new Parameter.
    //! Creates new Parameter object and appends to the end of the list
    //! then looks for description by name or synonym and sets pointer to it if found.
    //! \return new Parameter or 0 if a record with the name is present already
    Parameter* addParam(const vm_char* name);
    //! \brief Finds Parameter.
    //! Looks for Parameter with given name, if exist means that parameter is read.
    //! \return the Parameter found or 0 otherwise.
    Parameter* findParam(const vm_char* name);

    //! \brief Return base name if the name is a synonym.
    //! \return base name if the \a name is a synonym or 0 if the name is unknown
    const vm_char* getBaseName(const vm_char* name);
    //! \brief Finds description for parameter \a name.
    //! \return description for parameter \a name or 0 if the name is unknown
    const OptionInfo* findOptionInfo(const vm_char* name);

    Ipp32s m_numOptionInfo;           //!< Total number of descriptions

    //! List of description groups, linked to chain
    struct OptionInfoGroup {
      const vm_char    *groupName; //!< Name for the option description group
      const OptionInfo *entry;     //!< Pointer to the beginning of array (the group)
      Ipp32s            count;     //!< Number of descriptions in the group
      OptionInfoGroup  *next;      //!< Next group or 0 when last
    } *m_listOptionInfo;           //!< Beginning of the list

    //! Returns option description by index. Is used for access to accumulated descriptions.
    const OptionInfo* getOptionInfo(Ipp32s index);

private:
    // Declare private copy constructor to avoid accidental assignment
    // and klocwork complaining.
    ParamList(const ParamList &);
    ParamList & operator = (const ParamList &);
};

}//namespace UMC

#endif /* __UMC_FILE_READER_H__ */
