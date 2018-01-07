/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2018 Matthias H. mattesh(at)gmx.net

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
------------------------------------- */
/** 
* implementation of tclFindResultDoc
*/
//#include "stdafx.h"
#include "tclFindResultDoc.h"
#include <algorithm>
#include "tclResult.h"
#include <assert.h>
#define MDBG_COMP "FRDoc:" 
#include "myDebug.h"

/**
* insert the line into the result window if not already in.
* @return the resultwindow line number; (-1 is error case and should never come)
*/
int tclFindResultDoc::insertPosInfo(tPatId patternId, tiLine foundLine, tclPosInfo pos) {
   // check line in
   mLines[foundLine].posInfos()[patternId].insert(pos);
   insertResLine(foundLine);
   return getLineNoAtRes(foundLine);
}

/** function creates an empty line if not available before */
std::string& tclFindResultDoc::refLineText(tiLine foundLine) {
   return mLines[foundLine].text();
}

bool tclFindResultDoc::getLineAvail(tiLine foundLine) const {
   tlmLinePosInfo::const_iterator it = mLines.find(foundLine);
   if(it!=mLines.end()) {
      return it->second.valid();
   }
   return false;
}

/** function creates an empty line if not available before */
const std::string& tclFindResultDoc::getLineText(tiLine foundLine) {
   tlmLinePosInfo::const_iterator it = mLines.find(foundLine);
   if(it!=mLines.end()) {
      return it->second.text();
   } else {
      return mDefLineInfo.second.text();
   }
}

/** setLineText returns true in case that line was added or updated */
bool tclFindResultDoc::setLineText(tiLine foundLine, const std::string& text) {
   tlmLinePosInfo::iterator it = mLines.find(foundLine); 
   bool bNew;
   if(it != mLines.end()) {
      bNew = !it->second.valid();
      it->second.first = tclResultLine(text);
   } else {
      bNew = true;
      mLines[foundLine].first = tclResultLine(text);
      insertResLine(foundLine);
   }
   return bNew;
}

void tclFindResultDoc::reserve(unsigned count) {
   mReslines.reserve(count);
}

void tclFindResultDoc::clear() {
   mLines.clear();
   mReslines.clear();
}

int tclFindResultDoc::size() const {
   return (int)mLines.size();
}

void tclFindResultDoc::erase(tiLine foundLine){
   removeResLine(foundLine);
   mLines.erase(foundLine);
}

void tclFindResultDoc::moveResult(tPatId oldPattId, tPatId newPattId)
{
   DBG2("moveResult(old, new) %f %f", oldPattId, newPattId);
   // iterate over all lines and change old in new id
   tlmLinePosInfo::iterator it = mLines.begin();
   for(;it != mLines.end(); ++it) {
      tlmIdxPosInfo& posInfos = it->second.posInfos();
      tlmIdxPosInfo::iterator iOld = posInfos.find(oldPattId);
      if(iOld != posInfos.end()) {
         // old pattern found in this line -> move
         posInfos[newPattId] = iOld->second;
         posInfos.erase(iOld);
      }
   }
}

/** make sure function is not called with resultWinLine >= size() */
const tlpLinePosInfo& tclFindResultDoc::getLineAtRes(int resultWinLine) const {
   if(resultWinLine >= size()) {
      assert(resultWinLine < size()); // index out of range
      return mDefLineInfo;
   }
   tiLine foundLine = mReslines[resultWinLine];
   tlmLinePosInfo::const_iterator it = mLines.lower_bound(foundLine);
   if((it!= mLines.end())&&(it->first==foundLine)) {
      return *it;
   } else {
      assert((it!= mLines.end())&&(it->first==foundLine)); // index out of range
      return mDefLineInfo;
   }
}

/** function returns empty line (default CTOR) if not available before */
const tclLinePosInfo& tclFindResultDoc::getLineAtMain(tiLine foundLine) const {
   tlmLinePosInfo::const_iterator it = mLines.lower_bound(foundLine);
   if((it!= mLines.end())&&(it->first==foundLine)) {
      return it->second;
   } else {
      assert((it!= mLines.end())&&(it->first==foundLine)); // index out of range
      return mDefLineInfo.second;
   }
}

tiLine tclFindResultDoc::getNextLineNoAtMain(tiLine iFirstLineInView) const {
   tlmLinePosInfo::const_iterator it = mLines.lower_bound(iFirstLineInView);
   if (it != mLines.end()) {
      return it->first;
   }
   else {
      return (tiLine)-1;
   }
}

/** function returns true if available */
bool tclFindResultDoc::getLineAtMainAvail(tiLine foundLine) const {
   tlmLinePosInfo::const_iterator it = mLines.lower_bound(foundLine);
   if((it!= mLines.end())&&(it->first==foundLine)) {
      return true;
   } else {
      return false;
   }
}

/** function returns newly inserted element if not available before */
tclLinePosInfo& tclFindResultDoc::refLineAtMain(tiLine foundLine) {
   return mLines[foundLine];
}

int tclFindResultDoc::getLineNoAtRes(tiLine foundLine) const {
   //tlmLinePosInfo::const_iterator it = mLines.find(foundLine);
   // inform window to redraw the given line array
   int i=-1;
   //// go the shorteset way to the end and count the lines before or after
   tlvLine::const_iterator first = mReslines.begin();
   tlvLine::const_iterator last, it;

   if ( (int)mReslines.size() > foundLine ) {
      last = mReslines.begin() + foundLine;
   } else {
      last = mReslines.end();
   }
   it = std::lower_bound(first, last, foundLine);
   if((it != mReslines.end())&&(*it == foundLine)) {
      i = (int)(it - first);
   } else {assert(0); }
   return i;
}

/** make sure function is not called with resultWinLine >= size() */
tiLine tclFindResultDoc::getLineNoAtMain(int resultWinLine) const {
   if(resultWinLine >= size() ) {
      assert(resultWinLine <= size()); // index out of range but 0 should be not be an error as it is reset scintilla
      return -1;
   }
   return mReslines.at(resultWinLine);
}

void tclFindResultDoc::insertResLine(tiLine foundLine) {
   // from resline == foundline we start searching for the real place
   // foundline cannot be bigger as resline
   tlvLine::iterator first = mReslines.begin(),
      last, it;
   if ( (int)mReslines.size() > foundLine) {
      last = mReslines.begin() + foundLine;
   } else {
      last = mReslines.end();
   }
   it = std::lower_bound(first, last, foundLine);
   if(it == mReslines.end()) {
      mReslines.insert(it, foundLine);
   } else if(*it > foundLine) {
      mReslines.insert(it, foundLine);
   }
}

void tclFindResultDoc::removeResLine(tiLine foundLine) {
   // from resline == foundline we start searching for the real place
   // foundline cannot be bigger as resline
   tlvLine::iterator first = mReslines.begin(),
      last, it;
   if ( (int)mReslines.size() > foundLine) {
      last = mReslines.begin() + foundLine;
   } else {
      last = mReslines.end();
   }
   it = std::lower_bound(first, last, foundLine);
   if(it == mReslines.end()) {
      assert(0); // line allready removed!
   } else if(*it == foundLine) {
      mReslines.erase(it);
   } else if(*it > foundLine) {
      assert(0); // line allready removed!
   }
}
