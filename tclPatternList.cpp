/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2016 Matthias H. mattesh(at)gmx.net

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
class tclPatternList contains a vector of tclPatterns
*/
//#include "stdafx.h"

#include "tclPatternList.h"
#include "tclPattern.h"
#define MDBG_COMP "PatLst:" 
#include "myDebug.h"

// this value is used for the initial, first entry
// it is intentionally high as inserting in front cause ID/2 values before
#define PAT_INIT_ID 1000 

tclPattern tclPatternList::mDefault = tclPattern();

const tclPattern& tclPatternList::getPattern(tPatId i ) const {
   tlmPatternList::const_iterator it = mlmPattern.find(i);
   if(it!=mlmPattern.end()) {
      return it->second;
   } else {
      return mDefault;
   }
}

tPatId tclPatternList::getPatternId(unsigned index) const {
   tlmPatternList::const_iterator it = mlmPattern.begin();
   if(size() ==0) {
      return 0;
   }
   unsigned i = 0;
   while (i < index) {
      if(it == mlmPattern.end()) {
         return tPatId(unsigned(-1));
      }
      ++i;
      ++it;
   }
   return it->first;
}

unsigned tclPatternList::getPatternIndex(tPatId id) const {
   tlmPatternList::const_iterator it = mlmPattern.find(id);
   unsigned i = 0;
   if(it == mlmPattern.end()) {
      i = unsigned(-1);
   } else {
      while(it != mlmPattern.begin()) {
         --it;
         ++i;
      }
   }
   return i;
}

tPatId tclPatternList::push_back(const tclPattern& pattern) {
   tPatId id = PAT_INIT_ID;
   if(mlsPatIds.size()>0) {
      tPatId last = *mlsPatIds.rbegin();
      id = tPatId(int(last) + 1);
   }
   mlsPatIds.insert(id);
   mlmPattern[id] = pattern;
   DBG1("push_back() adding id %f.", id);
   return id;
}

unsigned tclPatternList::size() const {
   return (unsigned)mlmPattern.size();
}
 
tPatId tclPatternList::insert(tPatId before, const tclPattern& pattern) {
   tPatId newId = 0;
   if(mlsPatIds.size() > 0) {
      tlsPatId::iterator it1 = mlsPatIds.find(before);
      if(it1 == mlsPatIds.end()) {
         // given before id is not available add it to the begin
         it1 = mlsPatIds.begin();
      }
      if(it1 != mlsPatIds.begin()) {
         // before is not first; calc PatId
         tlsPatId::iterator it2 = it1;
         --it2;
         newId = tPatId(((*it2 - *it1) / 2) + *it1);
      } else {
         newId = tPatId(int(before) - 1);
      }
      DBG2("insert(before) before %f new %f.", before, newId);
   } else {
      DBG1("insert(before) before <not found> new %f.", newId);
   }
   mlsPatIds.insert(newId);
   mlmPattern[newId] = pattern;
   return newId;
}

tPatId tclPatternList::insertAfter(tPatId after, const tclPattern& pattern) {
   tPatId newId = 0;
   if(mlsPatIds.size() > 0) {
      tlsPatId::iterator it1 = mlsPatIds.find(after);
      if(it1 == mlsPatIds.end()) {
         // given after id is not available add it to the end
         newId = tPatId(int(*mlsPatIds.rbegin()) + 1);
      } else {
         tlsPatId::iterator it2 = it1;
         ++it2;
         if(it2 == mlsPatIds.end()) {
            // given after is the last valid add to the end
            newId = tPatId(int(after) + 1);
         } else {
            // put id between it1 and it2
            newId = tPatId(((*it2 - *it1) / 2) + *it1);
         }
      }
      DBG2("insert(after) after %f new %f.", after, newId);
   } else {
      DBG1("insert(after) after <not found> new %f.", newId);
   }
   mlsPatIds.insert(newId);
   mlmPattern[newId] = pattern;
   return newId;
}

void tclPatternList::moveResult(tPatId oldPattId, tPatId newPattId) 
{
   DBG2("moveResult(old, new) %f %f", oldPattId, newPattId);
   tlmPatternList::iterator iOld = mlmPattern.find(oldPattId);
   if(iOld != mlmPattern.end()) {
      mlmPattern[newPattId] = iOld->second;
      tclPatternList::remove(oldPattId);
   }
}

void tclPatternList::clear(){
   mlsPatIds.clear();
   mlmPattern.clear();
}

bool tclPatternList::setPattern(tPatId i, const tclPattern& pattern){
   tlmPatternList::iterator it = mlmPattern.find(i);
   if(it != mlmPattern.end()) {
      DBG1("setPattern() id %f done.", i);
      it->second = pattern;
      return true;
   }else {
      DBG1("setPattern() id %f was not in list! adding...", i);
      mlmPattern[i] = pattern;
      mlsPatIds.insert(i);
      return false;
   }
}

unsigned tclPatternList::getCommentWidth() const {
   tlmPatternList::const_iterator it = mlmPattern.begin();
   size_t max = 0;
   for (;it != mlmPattern.end();++it) {
      size_t s = it->second.getComment().size();
      max = (max<s)?s:max;
   }
   return (int)max;
}

void tclPatternList::remove(tPatId i){
   // removal only in pattern list Ids will remain
   mlmPattern.erase(i); 
   if(mlmPattern.size() == 0) {
      mlsPatIds.clear();
   }
}
