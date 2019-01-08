/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2019 Matthias H. mattesh(at)gmx.net

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
//#include "stdafx.h"
#include "resource.h"
#include "tclTableview.h"
#include "tclPatternList.h"
#include "tclResultList.h"
#include "chardefines.h"
#include <commctrl.h>// For ListView control APIs

#define MAX_CHAR_CELL 1000 // max chars in a cell including \0

// use teColumnNums to address the columns
const tstPatternConfTab tclTableview::gPatternConfTab[tclTableview::TBLVIEW_COL_MAX] = {
#ifdef COL_NUMBERING
    {TEXT("#"), 20 },
#endif
    {TEXT("Hits"),   0 }
   ,{TEXT("Active"), 20 }
   ,{TEXT("Search"), 100}
#ifdef RESULT_COLORING
   ,{TEXT("Color"),  40 }
   ,{TEXT("BgCol"),  40 }
#endif
   ,{TEXT("Type"),   50 }
   ,{TEXT("Case"),   20 }
   ,{TEXT("Word"),   20 }
   ,{TEXT("Select"), 35 }
   ,{TEXT("Hide"),   20 }
#ifdef RESULT_STYLING
   ,{TEXT("Bold"),   20 }
   ,{TEXT("Italic"), 20 }
   ,{TEXT("Underl."),20 }
#endif
   ,{TEXT("Comment"),200 }
};

generic_string tclTableview::getCell(int item, int column) const {
   static TCHAR cs[MAX_CHAR_CELL];
   ListView_GetItemText(mhList, item, column, cs, MAX_CHAR_CELL);
   TCHAR* cc = (TCHAR*)cs;
   int j = (int)generic_strlen(cs)+1; 
   j = (j>MAX_CHAR_CELL)?MAX_CHAR_CELL:j;
   for(int i=0; i<j; ++i) {
      cc[i] = (TCHAR)cs[i];
   }
   return generic_string(cc);
}

void tclTableview::refillTable(tclPatternList& pl) {
   if(mhList==0) {
      return; // not initialized
   }
   ListView_DeleteAllItems(mhList);
   tclPatternList::const_iterator iPattern = pl.begin();
   int item=0;
   for (; iPattern != pl.end(); ++iPattern) {
      const tclPattern& rp = iPattern.getPattern();
      LVITEM lvi;
      ZeroMemory(&lvi, sizeof(lvi));
      lvi.mask = LVIF_TEXT;
      // lvi.pszText = rp.getSearchText().c_str();
      lvi.iItem = pl.getPatternIndex(iPattern.getPatId());
      item = ListView_InsertItem(mhList, &lvi);
      updateRow(item, rp);
   }
}
int tclTableview::insertRow(){
   if(mhList==0) {
      return -1; // not initialized
   }
   LVITEM lvi;
   ZeroMemory(&lvi, sizeof(lvi));
   lvi.iItem = getSelectedRow();
   if(lvi.iItem == -1) {
      lvi.iItem =0;
   }
   lvi.mask = LVIF_TEXT;
   return ListView_InsertItem(mhList, &lvi);
}

int tclTableview::insertAfterRow(){
   if(mhList==0) {
      return -1; // not initialized
   }
   LVITEM lvi;
   ZeroMemory(&lvi, sizeof(lvi));
   lvi.iItem = getSelectedRow();
   if(lvi.iItem == -1) {
      lvi.iItem = getRowCount();
   } else {
      ++lvi.iItem;
   }
   lvi.mask = LVIF_TEXT;
   return ListView_InsertItem(mhList, &lvi);
}

void tclTableview::updateRow(int item, const tclPattern& rp) {
   //updateRowColor(item, rp);
#ifdef COL_NUMBERING
   TCHAR num[10];
   updateCell(item, TBLVIEW_COL_NUM, generic_itoa(item, num, 10));
#endif
   updateCell(item, TBLVIEW_COL_HITS, getHitCountStr(item));
   updateCell(item, TBLVIEW_COL_DO_SEARCH, rp.getDoSearch()?TEXT("X"):TEXT(""));
   updateCell(item, TBLVIEW_COL_SEARCH_TEXT, rp.getSearchText());
   updateCell(item, TBLVIEW_COL_SEARCH_TYPE, rp.getSearchTypeStr());
   updateCell(item, TBLVIEW_COL_MATCHCASE, rp.getIsMatchCase()?TEXT("X"):TEXT(""));
   updateCell(item, TBLVIEW_COL_WHOLEWORD, rp.getIsWholeWord()?TEXT("X"):TEXT(""));
   updateCell(item, TBLVIEW_COL_SELECT, rp.getSelectionTypeStr());
   updateCell(item, TBLVIEW_COL_HIDE, rp.getIsHideText()?TEXT("X"):TEXT(""));
   updateCell(item, TBLVIEW_COL_COMMENT, rp.getComment());
#ifdef RESULT_STYLING
   updateCell(item, TBLVIEW_COL_BOLD, rp.getIsBold()?TEXT("X"):TEXT(""));
   updateCell(item, TBLVIEW_COL_ITALIC, rp.getIsItalic()?TEXT("X"):TEXT(""));
   updateCell(item, TBLVIEW_COL_UNDERLINED, rp.getIsUnderlined()?TEXT("X"):TEXT(""));
#endif
#ifdef RESULT_COLORING
   updateCell(item, TBLVIEW_COL_COLOR, rp.getColorStr());
   updateCell(item, TBLVIEW_COL_BGCOLOR, rp.getBgColorStr());
#endif
}

void tclTableview::updateCell(int item, int column, const generic_string& s){
   //static TCHAR cs[MAX_CHAR_CELL];
   //int j = (int)s.length()+1; 
   //j = (j>MAX_CHAR_CELL)?MAX_CHAR_CELL:j;
   //for(int i=0; i<j; ++i) {cs[i] = (TCHAR)(s[i]&0xff);}
   ListView_SetItemText(mhList, item, column, (TCHAR*)s.c_str());
}

void tclTableview::setHitsRowVisible(bool bVisible, const tclResultList& results) {
   mbHitsVisible = bVisible;
   size_t max = 0;
   TCHAR num[20];
   int row = 0;
   for (tclResultList::const_iterator it = results.begin(); it != results.end(); ++it , ++row) {
      if (bVisible) {
         const tclResult& r = it.getResult();
         int n = (int)r.getPositions().size();
         generic_itoa(n, num, 10);
         updateCell(row, TBLVIEW_COL_HITS, num);
         max = (n > max) ? n : max;
      }
      else {
         updateCell(row, TBLVIEW_COL_HITS, TEXT(""));
      }
   }
   int iLineNumColSize =   (max<10) ? 20 :
                           (max<1000) ? 30 :
                           (max<10000) ? 35 :
                           (max<100000) ? 40 : 50;
   ListView_SetColumnWidth(mhList, TBLVIEW_COL_HITS, bVisible ? iLineNumColSize : 0);
}

generic_string tclTableview::getHitCountStr(int row) const {
   return generic_string(TEXT(""));
}

void tclTableview::create(){
   if(mhList == 0) {
      return; // not initialized
   }

   ListView_SetExtendedListViewStyle(mhList,
      LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
   //::SendMessage(mhList,GWL_STYLE,lngOldStyle | LVS_SHOWSELALWAYS );
   LVCOLUMN lvc = {0};
   lvc.mask = LVCF_TEXT  | LVCF_WIDTH;
   for (int col = 0; col < TBLVIEW_COL_MAX; ++col) {
      lvc.pszText = gPatternConfTab[col].szColumnName;
      lvc.cx = gPatternConfTab[col].iColumnSize;
      ListView_InsertColumn(mhList, col, &lvc);
   }
}

#ifdef COL_NUMBERING
generic_string tclTableview::getItemNumStr() const { return getItem(TBLVIEW_COL_NUM);}
#endif
generic_string tclTableview::getHitsStr() const { return getItem(TBLVIEW_COL_HITS); }
generic_string tclTableview::getDoSearchStr() const { return getItem(TBLVIEW_COL_DO_SEARCH);}
generic_string tclTableview::getSearchTextStr() const { return getItem(TBLVIEW_COL_SEARCH_TEXT);}
generic_string tclTableview::getSearchTypeStr() const {return getItem(TBLVIEW_COL_SEARCH_TYPE);}
generic_string tclTableview::getMatchCaseStr() const {return getItem(TBLVIEW_COL_MATCHCASE );}
generic_string tclTableview::getWholeWordStr() const {return getItem(TBLVIEW_COL_WHOLEWORD);}
generic_string tclTableview::getSelectStr() const {return getItem(TBLVIEW_COL_SELECT );}
generic_string tclTableview::getHideStr() const {return getItem(TBLVIEW_COL_HIDE);}
generic_string tclTableview::getCommentStr() const { return getItem(TBLVIEW_COL_COMMENT);}
#ifdef RESULT_STYLING
generic_string tclTableview::getBoldStr() const {return getItem(TBLVIEW_COL_BOLD);}
generic_string tclTableview::getItalicStr() const {return getItem(TBLVIEW_COL_ITALIC );}
generic_string tclTableview::getUnderlinedStr() const {return getItem(TBLVIEW_COL_UNDERLINED );}
#endif
#ifdef RESULT_COLORING
generic_string tclTableview::getColorStr() const {return getItem(TBLVIEW_COL_COLOR );}
generic_string tclTableview::getBgColorStr() const {return getItem(TBLVIEW_COL_BGCOLOR );}
#endif

int tclTableview::getSelectedRow() const {
   if(mhList) {
      return ListView_GetSelectionMark(mhList); // 0-n entry ; -1 nothing
   } else {
      return -1; // nothing selected
   }
}

// returns the row selected before
int tclTableview::setSelectedRow(int index) const {
   if(mhList) {
      int prevPos = ListView_SetSelectionMark(mhList, index); // 0-n entry ; -1 nothing      
      ListView_SetItemState(mhList, index, (LVIS_FOCUSED|LVIS_SELECTED),(LVIS_FOCUSED|LVIS_SELECTED));
      return prevPos;
   } else {
      return -1; // nothing selected
   }
}

int tclTableview::getRowCount() const {
   if(mhList) {
      return ListView_GetItemCount(mhList); // 0-n entries 
   } else {
      return 0; // nothing 
   }
}

void tclTableview::setRowItems(const tclPattern& pattern){
   if(mhList==0) {
      return;
   }
   if(getSelectedRow()!=-1){
      updateRow(getSelectedRow(), pattern);
   }
}

void tclTableview::removeRow(int row) {
   if(mhList==0) {
      return;
   }
   if(getRowCount()>row && row>=0) {
      ListView_DeleteItem(mhList, row);
   }
}

void tclTableview::removeAll() {
   ListView_DeleteAllItems(mhList);
}
