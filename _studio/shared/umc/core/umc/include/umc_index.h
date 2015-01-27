/* ****************************************************************************** *\

Copyright (C) 2003-2009 Intel Corporation.  All rights reserved.

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

File Name: umc_index.h

\* ****************************************************************************** */

#ifndef __UMC_INDEX_H__
#define __UMC_INDEX_H__

#include "umc_linked_list.h"
#include "umc_mutex.h"

namespace UMC
{

struct IndexEntry
{
    // Constructor
    IndexEntry()
    {
        stPosition = 0;
        dPts = -1.0;
        dDts = -1.0;
        uiSize = 0;
        uiFlags = 0;
    }

    // Returns DTS if present otherwise PTS
    inline Ipp64f GetTimeStamp(void)
    {
        return dDts < 0.0 ? dPts : dDts;
    }

    // Aboslute position of the sample
    size_t stPosition;

    // Presentation time stamp in seconds
    Ipp64f dPts;

    // Decoding time stamp in seconds
    Ipp64f dDts;

    // Sample size in byte
    Ipp32u uiSize;

    // Flags (frame type for video samples)
    Ipp32u uiFlags;
};

struct IndexFragment
{
    // Constructor
    IndexFragment()
    {
        pEntryArray = NULL;
        iNOfEntries = 0;
    }

    // The pointer to array of entries
    IndexEntry *pEntryArray;

    // number of entries in the array
    Ipp32s iNOfEntries;

};

class TrackIndex
{
public:

    TrackIndex();
    ~TrackIndex();

    // Returns number of entries in ALL fragments
    Ipp32u NOfEntries(void);

    // Provides FIRST entry from the FIRST fragment
    Status First(IndexEntry &entry);

    // Provides LAST entry from the LAST fragment
    Status Last(IndexEntry &entry);

    // Provides next entry
    // If last returned entry is the last in the fragment,
    // first entry from the NEXT fragment will be returned
    Status Next(IndexEntry &entry);

    // Provides previous entry
    // If last returned entry is the first in the fragment,
    // last entry from the PREVIOUS fragment will be returned
    Status Prev(IndexEntry &entry);

    // Provides next key entry
    // If last returned entry is the last in the fragment,
    // first entry from the NEXT fragment will be returned
    Status NextKey(IndexEntry &entry);

    // Provides previous key entry
    // If last returned entry is the first in the fragment,
    // last entry from the PREVIOUS fragment will be returned
    Status PrevKey(IndexEntry &entry);

    // Provides last returned entry
    Status Get(IndexEntry &entry);

    // Provides entry at the specified position (through ALL fragments)
    Status Get(IndexEntry &entry, Ipp32s pos);

    // Provides entry with timestamp is less or equal to specified (through ALL fragments)
    Status Get(IndexEntry &entry, Ipp64f time);

    // Add whole fragment to the end of index
    Status Add(IndexFragment &newFrag);

    // Removes last fragment
    Status Remove(void);

/*
 * These functions are not necessary for AVI and MPEG4 splitters,
 * so they are temporary commented

    // Modifies last returned entry
    Status Modify(IndexEntry &entry);

    // Modifies entry at the specified position
    Status Modify(IndexEntry &entry, Ipp32s pos);
*/

protected:

    // Returns pointer to next entry
    // If last returned entry is the last in the fragment,
    // first entry from the NEXT fragment will be returned
    // Input parameter and current state are not checked
    // State variables will be modified
    IndexEntry *NextEntry(void);

    // Returns pointer to previous entry
    // If last returned entry is the first in the fragment,
    // last entry from the PREVIOUS fragment will be returned
    // Input parameter and current state are not checked
    // State variables will be modified
    IndexEntry *PrevEntry(void);

    // Returns element at a specified position
    // Input parameter and current state are not checked
    // State variables will be modified
    IndexEntry *GetEntry(Ipp32s pos);

    // Returns element with timestamp is less or equal to specified
    // Input parameter and current state are not checked
    // State variables will be modified
    IndexEntry *GetEntry(Ipp64f time);

    // Linked list of index fragments
    LinkedList<IndexFragment> m_FragmentList;

    // Total number of entries in all fragments
    Ipp32u m_uiTotalEntries;

    // Copy of fragment which contains last returned entry
    IndexFragment m_ActiveFrag;

    // absolute position in index of the first entry of active fragment
    Ipp32s m_iFirstEntryPos;

    // absolute position in index of the last entry of active fragment
    Ipp32s m_iLastEntryPos;

    // relative position inside active fragment of the last returned entry
    Ipp32s m_iLastReturned;

    // synchro object
    vm_mutex m_Mutex;

}; // class TrackIndex

} // namespace UMC

#endif // __UMC_INDEX_H__
