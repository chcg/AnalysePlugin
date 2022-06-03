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
/* tclFindResultDlg.h 
This class implements the handling of the search result window 
it mainly contains the view based on scintilla and maintains parallel stored
search result string cache
*/

#ifndef TCLFINDRESULTDLG_H
#define TCLFINDRESULTDLG_H

#include "FindDlg.h"
#include "Scintilla.h"
#include "SciLexer.h"
#include "MyPlugin.h"
#include "ScintillaSearchView.h"
#include "tclFindResultDoc.h"
#include "tclFindResultSearchDlg.h"

#define MY_STYLE_COUNT (MY_STYLE_MASK-8) // 0 and 32-39 are defaults

class tclFindResultDlg : public DockingDlgInterface {

public:
   tclFindResultDlg();

   ~tclFindResultDlg();

   void setParent(MyPlugin* parent);

   void setCodePage(WPARAM cp);

   void setLineNumColSize(int size);

   int getLineNumColSize() const ;

   void initEdit(const tclPattern& defaultPattern); 

   void reserve(unsigned count) {
      mFindResults.reserve(count);
   }

   /** remove all that line being referecend by result not no other pattern */
   void removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult); 
   

   void create(tTbData * data, bool isRTL = false);
   
   tiLine insertPosInfo(tPatId patternId, tiLine iResultLine, tclPosInfo pos);

   bool getLineAvail(tiLine foundLine) const ;
   tiLine getNextFoundLine(tiLine iEdittorsLine) const;

   const std::string& getLineText(tiLine iResultLine);

   void setLineText(tiLine iFoundLine, const std::string& text, const std::string& comment, unsigned commentWidth);
   
   void moveResult(tPatId oldPattId, tPatId newPattId);

   void updateWindowData(const generic_string& fontName, unsigned fontSize);
   
   void clear_view();
   void clear(bool initial = false);

   tiLine getCurrentMarkedLine() const ;
   
   /**
   * set the marked line with
   */ 
   void setCurrentMarkedLine(tiLine line);
   
   /**
   function is called whenever the styles in the result window have to be changed
   */
   void setPatternStyles(const tclPatternList& list);
   
   void setSearchPatterns(const tclPatternList& list);
  	
   virtual void updateDockingDlg(void);

   void setUseBookmark(int useIt){
      mUseBookmark = useIt;
   }
   void setWrapMode(bool bOn) {
      _scintView.setWrapMode(bOn);
   }
   bool getWrapMode() const {
      return _scintView.getWrapMode();
   }

   bool getDisplayLineNo() const {
      return _scintView.getLineNumbersInResult();
   }
   void setDisplayLineNo(bool useIt){
      if (_scintView.getLineNumbersInResult() != useIt) {
         _scintView.setLineNumbersInResult(useIt);
      }
   }

   void setFileName(const generic_string& str);
   
   void updateHeadline();

   const TCHAR* getszFileName() const {
      return mSearchFileName.c_str();
   }
   const generic_string& getSearchResultFileName() const {
      return mSearchResultFile;
   }
#ifdef FEATURE_RESVIEW_POS_KEEP_AT_SEARCH
   // CurrentViewPos can be modified to avoid moving content in result window
   void saveCurrentViewPos();
   void restoreCurrentViewPos();
#endif
   void setCurrentViewPos(tiLine iThisMainLine);
   void updateViewScrollState(tiLine iLineInMain, bool bInMain, bool bAnyway=false);

protected :
   static const int transStyleId[MY_STYLE_COUNT];
   // public version calls internal with correct start and end values
   void doStyle(tiLine iFoundLine);

   // open search dialog in result window
   void doFindResultSearchDlg();

   // identify the file to which the result shall be stored
   void doSaveToFile();
   // message call back method
   virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

   /** 
   this function sends the series of commands to 
   scintilla win with the corresponding style id 
   **/
   void setStyle(tPatId iPatternId, tiLine iBeginPos, tiLine iLength);
   void setDefaultStyle(tiLine iBeginPos, tiLine iLength);

   // callback from scintilla to colorize the search window
   void doStyle(tiLine startResultLineNo, tiLine startStyleNeeded, tiLine endStyleNeeded);

   bool notify(SCNotification *notification);
   
   void setFinderReadOnly(bool isReadOnly); 
   void saveSearchDoc();
   void setPatternFonts();

   MyPlugin* _pParent;

   //   ScintillaSearchView **_ppEditView;
   //	std::vector<FoundInfo> _foundInfos;
   tclFindResultDoc mFindResults;

   ScintillaSearchView _scintView;
   tiLine _markedLine;  // -1 or the line actually marked in the main window
   // InWhat _mode;
   size_t _lineCounter; // incremented when a line has been added to the edit window

   // count of chars reserved for line number
   int miLineNumColSize;
   // count of chars reserved for num header per line
   int miLineHeadSize;

   tclPatternList mPatStyleList;

   tclFindResultSearchDlg mFindResultSearchDlg;

   generic_string mSearchFileName;
   TCHAR _ResAdditionalInfo[MAX_HEADLINE]; // file name as in edit tab 
   generic_string mFontName;
   unsigned mFontSize;
   int mUseBookmark;
   int mDisplayComment;
   generic_string mSearchResultFile;
#ifdef FEATURE_RESVIEW_POS_KEEP_AT_SEARCH
   tiLine mCurrentViewLineNo;
#endif
   bool mFromMainWindow; // flag if main window moves the result window
   bool mFromFindResult; // flag set if double click moves the main window
};
#endif //TCLFINDRESULTDLG_H
