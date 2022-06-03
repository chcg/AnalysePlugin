/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (c) 2022 Matthias H. mattesh(at)gmx.net

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
------------------------------------- */
/**
a tclResult is the container for the result of searching for one pattern
*/

//#include "stdafx.h"
#include "tclResult.h"
//#include <algorithm> 
using namespace std;

bool line_less(const tclPosInfo& one, const tclPosInfo& two) {
   return one.line < two.line;
}

tclResult::tclResult():mbDirty(true){}

tclResult::~tclResult(){}

tclResult::tclResult(const tclResult & right):mbDirty(true){
   mbDirty = right.mbDirty;
   mlvPositions = tlvPosInfo(right.mlvPositions);
}

tclResult & tclResult::operator= (const tclResult & right){
   if(&right == this){
      return *this;
   }
   mbDirty = right.mbDirty;
   mlvPositions = right.mlvPositions;
   return *this;
}

void tclResult::clear(){
   mlvPositions.clear();
   mbDirty = true;
}

unsigned tclResult::size() const {
   return (unsigned)mlvPositions.size();
}

int tclResult::find(int line) const {
   tlvPosInfo::const_iterator it = mlvPositions.begin();
   for(;it!= mlvPositions.end();++it) {
      if (it->line == line) {
         return (int)(mlvPositions.begin() - it);
      }
   }
   return -1;
}

const tclPosInfo& tclResult::getPosition(unsigned index) const {
   return mlvPositions[index];
}

void tclResult::push_back(int targetStart, int targetEnd, int lineNumber/*, const char* pLine*/){
   mlvPositions.push_back(tclPosInfo(targetStart, targetEnd, lineNumber/*, std::string(pLine)*/));
}

void tclResult::setDirty(bool dirty){
   mbDirty = dirty;
}

bool tclResult::getIsDirty() const {
   return mbDirty;
}
