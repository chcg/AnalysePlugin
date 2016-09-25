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
tclResultList contains a list of results but also 
the corresponding patterns used.
the syncrosiastion between both lists is the index in the vectors
*/
//#include "stdafx.h"

#include "tclResultList.h"
#include "tclResult.h"
#include <assert.h>
#define MDBG_COMP "ResLst:" 
#include "myDebug.h"

void tclResultList::moveResult(tPatId oldPattId, tPatId newPattId) 
{
   DBG2("moveResult(old, new) %f %f", oldPattId, newPattId);
   tclPatternList::moveResult(oldPattId, newPattId);
   mlmResult[newPattId] = mlmResult[oldPattId];
   mlmResult.erase(oldPattId);
}

void tclResultList::clear(){
   tclPatternList::clear();
   mlmResult.clear();
}

bool tclResultList::setPattern(tPatId i, const tclPattern& pattern){
   tlmPatternList::iterator it = mlmPattern.find(i);
   if(it != mlmPattern.end()) 
   {
      tclPattern& myP = it->second;
      if(pattern==myP) {
         DBGW2("setPattern() pattern %s same in row %d don't change.", 
            pattern.getSearchText().c_str(), i);
         return true;
      } else {
         // make it dirty only if search pattern did change
         if(!myP.isSearchEqual(pattern)) {
            mlmResult[i].setDirty();
         }
         return tclPatternList::setPattern(i, pattern);
      }
   } else {
      // create a new entry for it
      mlmResult[i].setDirty();
      return tclPatternList::setPattern(i, pattern);
   }
}

bool tclResultList::getIsDirty() const {
   for (tlmResult::const_iterator it = mlmResult.begin();
      it != mlmResult.end();
      ++it) 
   {
      if(it->second.getIsDirty()) { 
         return true; 
      }
   }
   return false;
}

tPatId tclResultList::push_back(const tclPattern& pattern){
   tPatId id = tclPatternList::push_back(pattern);
   mlmResult[id] = tclResult();
   return id;
}

tPatId tclResultList::insert(tPatId before, const tclPattern& pattern){
   tPatId id = tclPatternList::insert(before, pattern);
   mlmResult[id] = tclResult();
   return id;
}
tPatId tclResultList::insertAfter(tPatId after, const tclPattern& pattern){
   tPatId id = tclPatternList::insertAfter(after, pattern);
   mlmResult[id] = tclResult();
   return id;
}

void tclResultList::remove(tPatId i){
   mlmResult.erase(i);
   tclPatternList::remove(i);
}
