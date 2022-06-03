/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C) 2022 Matthias H. mattesh(at)gmx.net

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
 * tclPosInfo is the pure Information where a pattern has been found. 
 */
#ifndef TCLPOSINFO_H
#define TCLPOSINFO_H

typedef intptr_t tiLine;

/**
 * Position info is the pure Information where a pattern has been found. 
 * The string from start until end the values correspond to the search
 * result as provided by SCI_SEARCHINTARGET. Additionally the line of start
 * is stored for optimization reasons. It is not part of the identity, because 
 * it is derived from the start position.
 */
class tclPosInfo {
public:

   tclPosInfo(int thisStart, int thisEnd, int thisLine/*, const std::string& thisText*/):
      start(thisStart), end(thisEnd), line(thisLine)/*, text(thisText) */{}

   bool operator<(const tclPosInfo& right) const {
      return (start == right.start)?(end < right.end):(start < right.start);
   }
   bool operator==(const tclPosInfo& right) const {
      return (start == right.start)&&(end == right.end);
   }
   bool operator!=(const tclPosInfo& right) const {
      return !operator==(right);
   }
   tiLine start;  // start position of found text
   tiLine end;    // end position of found text
   tiLine line;   // line number of begin position
   //std::string text; // line of the first position
};
#endif //TCLPOSINFO_H
