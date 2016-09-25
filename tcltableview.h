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
tclTableview implements the WINAPI functionality for the table of patterns 
in the find config dock window
*/

#ifndef TCLTABLEVIEW_H
#define TCLTABLEVIEW_H

#include <windows.h>
#include <string>
#include "Common.h"

class tclPatternList;
class tclPattern;


struct tstPatternConfTab {
   TCHAR* szColumnName;
   int iColumnSize;
};

/**
 * encapsulates all necessary functionality to meintain the list view and its content. 
 */
class tclTableview {
public:
   enum teColumnNums {
#ifdef COL_NUMBERING
      TBLVIEW_COL_NUM,
#endif
      TBLVIEW_COL_DO_SEARCH,    
      TBLVIEW_COL_SEARCH_TEXT,
   #ifdef RESULT_COLORING
      TBLVIEW_COL_COLOR,
      TBLVIEW_COL_BGCOLOR,
   #endif
      TBLVIEW_COL_SEARCH_TYPE,
      TBLVIEW_COL_MATCHCASE,
      TBLVIEW_COL_WHOLEWORD,
      TBLVIEW_COL_SELECT,
      TBLVIEW_COL_HIDE,
   #ifdef RESULT_STYLING
      TBLVIEW_COL_BOLD,
      TBLVIEW_COL_ITALIC,
      TBLVIEW_COL_UNDERLINED,
   #endif
      TBLVIEW_COL_COMMENT,
      TBLVIEW_COL_MAX
   };

   tclTableview():mhList(0){}

   ~tclTableview(){
      mhList=0;
   }

   void setListViewHandle(HWND hwnd){
      mhList = hwnd;
   }

   HWND getListViewHandle() const {
      return mhList;
   }

   void refillTable(tclPatternList& pl);

   void create();

   int getSelectedRow() const ;

   int setSelectedRow(int index) const ;

   int getRowCount() const ;

   generic_string getSearchTextStr() const ;
   generic_string getSearchTypeStr() const ;
   generic_string getMatchCaseStr() const ;
   generic_string getWholeWordStr() const ;
   generic_string getSelectStr() const ;
   generic_string getHideStr() const ;
   generic_string getCommentStr() const ;
   generic_string getDoSearchStr() const ;
#ifdef COL_NUMBERING
   generic_string getItemNumStr() const ;
#endif

#ifdef RESULT_STYLING
   generic_string getBoldStr() const ;
   generic_string getItalicStr() const ;
   generic_string getUnderlinedStr() const ;
#endif
#ifdef RESULT_COLORING
   generic_string getColorStr() const ;
   generic_string getBgColorStr() const ;
#endif

   generic_string getItem(int column) const { 
      if(mhList==0) {
         return generic_string(); // not initialized
      }
      return getCell(getSelectedRow(), column);
   }

   void setRowItems(const tclPattern& pattern);
   int insertRow();
   int insertAfterRow();
   void removeRow(int row);
   void removeAll();

protected:
   static const tstPatternConfTab gPatternConfTab[TBLVIEW_COL_MAX];

   generic_string getCell(int item, int column) const;
   void updateRow(int item, const tclPattern& rp);
   void updateCell(int item, int column, const generic_string& s);
   void updateRowColor(int item, const tclPattern& rp);

   HWND mhList;
};
#endif //TCLTABLEVIEW_H
