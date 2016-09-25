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
a tclResult is the container for the result of searching for one pattern
*/

#ifndef TCLRESULT_H
#define TCLRESULT_H
#include <string>
#include <vector>
#include "tclPosInfo.h"

//struct tstLineInfo {
//   tstLineInfo(int thisLine, const char* thispText, int thisIndex)
//      :line(thisLine), pText(thispText), index(thisIndex){}
//   bool operator==(const tstLineInfo& right) const {
//      return ((line == right.line) &&
//              (pText == right.pText) &&
//              (index == right.index));
//   }
//   bool operator!=(const tstLineInfo& right) const {
//      return !operator==(right);
//   }
//   tstLineInfo& operator=(const tstLineInfo& right) {
//      if(&right == this) {
//         return *this;
//      }
//      new (this) tstLineInfo(right.line, right.pText, right.index);
//   }
//   const int line;
//   const char* pText;
//   const int index;
//};

/**
 * A result is the list of found positions for a given pattern.
 * it maintains the positions in an order to allow quick access to the original text. 
 */
class tclResult {
public:
   typedef std::vector<tclPosInfo> tlvPosInfo;

   tclResult();

   tclResult(const tclResult & right);

   virtual ~tclResult();

   tclResult & operator = (const tclResult & right);

   /**
    * remove all position entries in the result. 
    */
   void clear();
   /**
   * return index of a given line 
   * if line is not in return -1
   */
   int find(int line) const;
   
   unsigned size() const ;

   const tclPosInfo& getPosition(unsigned index) const;

   const tlvPosInfo& getPositions() const {
      return mlvPositions;
   }
   /**
    * add one position into the result 
    */
   void push_back(int targetStart, int targetEnd, int lineNumber/*, const char* pLine*/);

   void setDirty(bool dirty=true);
   bool getIsDirty() const ;

protected:
   bool mbDirty; // set to false if search is completed
   tlvPosInfo mlvPositions;
};
#endif //TCLRESULT_H
