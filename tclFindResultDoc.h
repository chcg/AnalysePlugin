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

#ifndef TCLFINDRESULTDOC_H
#define TCLFINDRESULTDOC_H

#include <map>
#include <set>
#include <vector>
#include <string>
#include "MyPlugin.h"
#include "tclPosInfo.h"

class tclResultLine
{
public:
   tclResultLine():bVisible(false), bValid(false){}
   tclResultLine(const tclResultLine& right)
      :bVisible(right.bVisible), bValid(right.bValid),text(right.text){}
   // special constructor for adding or deleting lines.
   // if str.size()==0 the line will become invisible but valid
   tclResultLine(const std::string& str)
      :bVisible(str.size()>0), bValid(true),text(str){}
   tclResultLine& operator=(const tclResultLine& right) {
      if(&right==this) {
         return *this;
      }
      bVisible = right.bVisible;
      bValid = right.bValid;
      text = right.text;
      return *this;
      }
   bool bVisible; // if to be displayed
   bool bValid; // if at least once set
   std::string text;
};


typedef std::set<tclPosInfo> tlsPosInfo; // second -> set of pos infos for one pattern per line 
typedef std::map<tPatId, tlsPosInfo> tlmIdxPosInfo; // position and according pattern

// second -> line and found positions
class tclLinePosInfo : public std::pair<tclResultLine, tlmIdxPosInfo> {
public:
   tclLinePosInfo()
      :std::pair<tclResultLine, tlmIdxPosInfo>() {}
   tclLinePosInfo(const std::string& line, const tlmIdxPosInfo& pos)
      :std::pair<tclResultLine, tlmIdxPosInfo>(line,pos) {}
   tclLinePosInfo(const tclLinePosInfo& other)
      :std::pair<tclResultLine, tlmIdxPosInfo>(other){}
   // my accessors
   const std::string& text() const { return first.text; }
   std::string& text() { return first.text; }
   const bool& visible() const { return first.bVisible; }
   bool& visible() { return first.bVisible; }
   const bool& valid() const { return first.bValid; }
   bool& valid() { return first.bValid; }
   const tlmIdxPosInfo& posInfos () const { return second; }
   tlmIdxPosInfo& posInfos () { return second; }
};

typedef std::pair<const tiLine, tclLinePosInfo> tlpLinePosInfo; // pair of the map
typedef std::map<tiLine, tclLinePosInfo> tlmLinePosInfo; // result list complete
typedef std::vector<tiLine> tlvLine;


/**
* the find result doc is an implementation of all searchresults ordered by their 
* line number of the found position. It maintains the relation ship between line 
* number in the edit window of the result, the line number in the main window and 
* the index information which pattern caused this find position.
* It maintains the data in a triple map which on first level qualifies the line
* and on second the pattern index and on third level a set of sorted positions.
*/
class tclFindResultDoc {
public:

   /**
   * insert the line into the result window if not already in.
   * @return the resultwindow line number; (-1 is error case and should never come)
   */
   int insertPosInfo(tPatId patternId, tiLine foundLine, tclPosInfo pos); 

   /** function creates an empty line if not available before */
   std::string& refLineText(tiLine foundLine); 

   bool getLineAvail(tiLine foundLine) const ;

   /** function creates an empty line if not available before */
   const std::string& getLineText(tiLine foundLine); 

   /** setLineText returns true in case that line was added or updated */
   bool setLineText(tiLine foundLine, const std::string& text); 

   void reserve(unsigned count); 

   void clear(); 

   int size() const; 

   void erase(tiLine foundLine);

   void moveResult(tPatId oldPattId, tPatId newPattId);

   /** make sure function is not called with resultWinLine >= size() */
   const tlpLinePosInfo& getLineAtRes(int resultWinLine) const; 
   
   /** function returns empty line (default CTOR) if not available before */
   const tclLinePosInfo& getLineAtMain(tiLine foundLine) const;

   /** returns the next found line number in main window for seeking to it after search has finished
       In case no more found lines follow after the given line the return is -1 == ERROR.
   **/
   tiLine getNextLineNoAtMain(tiLine iFirstLineInView) const;

   /** function returns empty line (default CTOR) if not available before */
   bool getLineAtMainAvail(tiLine foundLine) const; 

   /** function returns newly inserted element if not available before */
   tclLinePosInfo& refLineAtMain(tiLine foundLine); 


   int getLineNoAtRes(tiLine foundLine) const; 

   /** make sure function is not called with resultWinLine >= size() */
   tiLine getLineNoAtMain(int resultWinLine) const; 



protected:
   void insertResLine(tiLine foundLine); 
   void removeResLine(tiLine foundLine); 

   /**
   mLines map<line, map< patternIndex, tclPosInfo>>
   */
   tlmLinePosInfo mLines;
   // conversion from resLIne in Foundline and back
   tlvLine mReslines;

   static tlpLinePosInfo mDefLineInfo; // used for return of invalid value
};

#endif //TCLFINDRESULTDOC_H
