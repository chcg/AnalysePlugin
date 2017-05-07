/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2017 Matthias H. mattesh(at)gmx.net

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
/* tclFindResultDlg.cpp 
This class implements the handling of the search result window 
it mainly contains the view based on scintilla and maintains parallel stored
search result string cache
*/
//#include "stdafx.h"
#include "FindDlg.h"
#include "Scintilla.h"
#include "SciLexer.h"
#include "MyPlugin.h"
#include "ScintillaSearchView.h"
#include "tclFindResultDoc.h"
#include "tclFindResultDlg.h"
#include <commdlg.h>// For fileopen dialog.
#define MDBG_COMP "FRDlg:" 
#include "myDebug.h"
#include "resource.h"
#include "npp_defines.h"

#define STYLING_MASK 255
#define FNDRESDLG_LINE_HEAD ""
#define FNDRESDLG_LINE_COLON ": "
#define FNDRESDLG_LINE_HYPHEN "| "
#define FNDRESDLG_DEFAULT_STYLE 0 // style number for the default styling
#define FNDRESDLG_ACTIVATE_SEARCH 0x06

#ifdef UNICODE
#define filestat _wstat
#else
#define filestat _stat
#endif
const int tclFindResultDlg::transStyleId[MY_STYLE_COUNT] = {
        1,  2,  3,  4,  5,  6,  7,  8,  9, // 0 is for default color
   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
   20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
   30, 31,                                 // scintilla defined numbers
   40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
   50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
   60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
   70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
   90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
  100,101,102,103,104,105,106,107,108,109,
  110,111,112,113,114,115,116,117,118,119,
  120,121,122,123,124,125,126,127
};
   
tlpLinePosInfo tclFindResultDoc::mDefLineInfo = tlpLinePosInfo();

tclFindResultDlg::tclFindResultDlg() 
   : DockingDlgInterface(IDD_FIND_DLG_RESULT)
   , _markedLine(-1)
   , _lineCounter(0) 
   , miLineNumColSize(0)
   , miLineHeadSize(0)
   , mFontSize(8)
   , mUseBookmark(1)
   , mDisplayLineNo(1)
   , mDisplayComment(0) // TODO check fact that pattern may be a different
#ifdef FEATURE_RESVIEW_POS_KEEP_AT_SEARCH
   , mCurrentViewLineNo(0)
#endif
   , mFromMainWindow(false)
   , mFromFindResult(false)
{
   _ResAdditionalInfo[0] = 0;
}

tclFindResultDlg::~tclFindResultDlg() {
   _scintView.destroy();
}

void tclFindResultDlg::setParent(MyPlugin* parent) {
   _pParent = parent;
}

void tclFindResultDlg::setCodePage(WPARAM cp) {
   _scintView.execute(SCI_SETCODEPAGE, cp);
}

void tclFindResultDlg::setLineNumColSize(int size) {
   miLineNumColSize = size;
   miLineHeadSize = miLineNumColSize+ (int)(strlen(FNDRESDLG_LINE_COLON)+strlen(FNDRESDLG_LINE_HEAD));
}

int tclFindResultDlg::getLineNumColSize() const {
   return miLineNumColSize;
}

void tclFindResultDlg::initEdit(const tclPattern& defaultPattern) {
   _scintView.init(_hInst, _hSelf);
   _scintView.display();
   setFinderReadOnly(true);
   RECT rect;
   getClientRect(rect);
   _scintView.reSizeToWH(rect);
   //_scintView.execute(SCI_SETCODEPAGE, SC_CP_UTF8);
   // let window parent (this class) do the styling
   _scintView.execute(SCI_SETLEXER,SCLEX_CONTAINER);
   _scintView.execute(SCI_SETSTYLEBITS, MY_STYLE_BITS); // maximum possible
   mFindResultSearchDlg.init(_hInst, _hParent, &_scintView);
   mFindResultSearchDlg.setdefaultPattern(defaultPattern);
   mFindResultSearchDlg.create(IDD_FIND_RES_DLG_SEARCH);
   // TODO _scintView.showMargin(ScintillaSearchView::_SC_MARGE_LINENUMBER);
}

/** remove all that line being referecend by result not no other pattern */
void tclFindResultDlg::removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& DBGDEF(newResult))
{
   DBG2("removeUnusedResultLines() oldResult.size() %d newResult.size() %d.", 
      oldResult.size(), newResult.size());
   for(int iResultEntry = 0; iResultEntry < (int)oldResult.size();++iResultEntry)
   {
      tiLine thisLine = oldResult.getPosition(iResultEntry).line;
      // line not used in this result remove my link in it
      if(mFindResults.getLineAtMainAvail(thisLine)) {
         // if line is already removed don't remove it again
         tclLinePosInfo& l = mFindResults.refLineAtMain(thisLine);
         l.posInfos().erase(pattId);
         // lets check if other results use the line and remove if not
         if(l.posInfos().size() == 0) {
            int resultLine = mFindResults.getLineNoAtRes(thisLine);
            if(resultLine >= 0) {
               if(mUseBookmark){
                  _pParent->execute(scnActiveHandle, SCI_MARKERDELETE, thisLine, MARK_BOOKMARK);
               }
               int startL = (int)_scintView.execute(SCI_POSITIONFROMLINE, resultLine);
               int endL = (int)_scintView.execute(SCI_GETLINEENDPOSITION, resultLine);
               if (endL+2 <= _scintView.execute(SCI_GETLENGTH)) {
                  endL += 2;
               }
               DBG4("removeUnusedResultLines() removing line %d from %d to %d being main line %d", resultLine, startL, endL, thisLine);
               if(endL <= startL) {
                  ::MessageBox(0, TEXT("Line delete in result not correct"), TEXT("Analyse Plugin - Error"), 0);
               } else {
                  setFinderReadOnly(false);
                  _scintView.execute(SCI_SETTARGETSTART, startL);
                  _scintView.execute(SCI_SETTARGETEND, endL);
                  //SCI_REPLACETARGET(int length, const char *text)
                  _scintView.execute(SCI_REPLACETARGET, 0, (LPARAM)"");
                  DBG2("removeUnusedResultLines() tstart %d, tend %d.", startL, endL);
                  setFinderReadOnly(true);
               }
            } else {
               TCHAR num[20];
               ::MessageBox(0, TEXT("The line was not in ?"),generic_itoa(resultLine, num, 10), MB_OK);
            }
            mFindResults.erase(thisLine);
         } else {
            DBG2("removeUnusedResultLines() found line %d result not empty size %d", thisLine, l.posInfos().size());
#ifdef _DEBUG
            TCHAR num[20];
            generic_string s;
            tlmIdxPosInfo::const_iterator i = l.posInfos().begin();
            wsprintf(num,TEXT("%3.10f"),i->first);
            s.append(num);
            for(; i != l.posInfos().end(); ++i) {
               s.append(TEXT(", "));
               wsprintf(num,TEXT("%3.10f"),i->first);
               s.append(num);
            }
            DBGW1("removeUnusedResultLines() patterns are: %s.",s.c_str()); 
#endif
            //int line = mFindResults.getLineNoAtRes(thisLine);
            //int startL = (int)_scintView.execute(SCI_POSITIONFROMLINE, line);
            //int endL = (int)_scintView.execute(SCI_GETLINEENDPOSITION, line);
            //doStyle(line, startL, endL);
         }
      } else {
         DBG1("removeUnusedResultLines() found line %d already removed", thisLine);
      }
   }
   // clean markup
   _scintView.execute(SCI_SETSEL, -1);
   // update colors
   _scintView.execute(SCI_COLOURISE, 0, -1);
   _scintView.redraw();
}

void tclFindResultDlg::create(tTbData * data, bool isRTL)
{
   DockingDlgInterface::create(data, isRTL);
   // done by DockingInterface
   //data->hClient		= _hSelf;
   //data->pszName		= _pluginName;

   // user information
   data->dlgID       = _dlgID;
   data->pszModuleName = _moduleName.c_str();

   // icon
   data->hIconTab = ::LoadIcon(getHinst(), MAKEINTRESOURCE(IDI_ANALYSE));

   // additional info
   data->pszAddInfo = _ResAdditionalInfo;

   // supported features by plugin
   data->uMask	= (DWS_DF_CONT_BOTTOM | DWS_PARAMSALL);

   // store for live time
//   _data = data;
}

int tclFindResultDlg::insertPosInfo(tPatId patternId, tiLine iResultLine, tclPosInfo pos) {
   return mFindResults.insertPosInfo(patternId, iResultLine, pos);
}

bool tclFindResultDlg::getLineAvail(tiLine foundLine) const {
   return mFindResults.getLineAvail(foundLine);
}

int tclFindResultDlg::getNextFoundLine(int iEditorsLine) const {
   return (int)mFindResults.getNextLineNoAtMain((tiLine)iEditorsLine);
}

const std::string& tclFindResultDlg::getLineText(int iResultLine) {
   return mFindResults.getLineText(iResultLine);
}

void tclFindResultDlg::setLineText(int iFoundLine, const std::string& text, const std::string& comment, unsigned commentWidth) {
   bool bNewLine = mFindResults.setLineText(iFoundLine, text);
   // here we have to distinguish update and insert of lines in search result window
   int resLine = mFindResults.getLineNoAtRes(iFoundLine);
   //bool bVisible = mFindResults.getLineAtMain(iFoundLine).visible();
   int startPos = (int)_scintView.execute(SCI_POSITIONFROMLINE, resLine);
   if (startPos != -1) {
      setCurrentMarkedLine(-1);
      setFinderReadOnly(false);
      std::string s = FNDRESDLG_LINE_HEAD;
      s.reserve(text.size() 
                + (mDisplayLineNo)?(miLineNumColSize + strlen(FNDRESDLG_LINE_COLON)):(0) 
                + (mDisplayComment)?(commentWidth + strlen(FNDRESDLG_LINE_HYPHEN)):(0));
      char conv[20];
      if (mDisplayLineNo) {
         s.append(miLineNumColSize-strlen(_itoa(iFoundLine+1, conv, 10)), ' ');
         s.append(conv);
         s.append(FNDRESDLG_LINE_COLON);
      }
      if (mDisplayComment) {
         s.append(comment);
         s.append(commentWidth-comment.length(), ' ');
         s.append(FNDRESDLG_LINE_HYPHEN);
      }
      s.append(text);
      DBGA3("setLineText() iFoundLine: %d resLine: %d text: \"%s\"", iFoundLine, resLine, s.c_str());
      if(bNewLine) {
         // adding a newline into result -> set bookmark in main window
         if(mUseBookmark) {
            _pParent->execute(scnActiveHandle, SCI_MARKERADD, iFoundLine, MARK_BOOKMARK);
         }
         _scintView.execute(SCI_INSERTTEXT, startPos, (LPARAM)s.c_str());
         ++_lineCounter;
      } else {
         int endPos = (int)_scintView.execute(SCI_GETLINEENDPOSITION, resLine);
         if (endPos+2 <= _scintView.execute(SCI_GETLENGTH)) {
            endPos += 2;
         }
         // hier text replace from startPos to endPos
         _scintView.execute(SCI_SETTARGETSTART, startPos);
         _scintView.execute(SCI_SETTARGETEND, endPos);
         //SCI_REPLACETARGET(int length, const char *text)
         _scintView.execute(SCI_REPLACETARGET, -1, (LPARAM)s.c_str());
         DBG2("setLineText() tstart %d, tend %d.", startPos, startPos);
      }
//      doStyle(resLine, startPos, (int)startPos+(int)s.length());
      setFinderReadOnly(true);
   } else {
      DBG1("setLineText() ERROR iFoundLine: %d not in editor! do  nothing",iFoundLine);
   }
}

void tclFindResultDlg::moveResult(tPatId oldPattId, tPatId newPattId)
{
   DBG2("moveResult(old, new) %f %f", oldPattId, newPattId);
   // change painting order but not resultlines themself
   // 1. iterate through result lines
   mFindResults.moveResult(oldPattId, newPattId);
   // 2. correct th pattern styles
   mPatStyleList.moveResult(oldPattId, newPattId);
   // 3. activate new coloring
   _scintView.execute(SCI_COLOURISE, 0, -1);
}
   
void tclFindResultDlg::setPatternFonts() {
   // set the result window font style
   unsigned iPat = FNDRESDLG_DEFAULT_STYLE;
   const char *fontName;
#ifdef UNICODE
   WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
   unsigned int cp = (unsigned int)_scintView.execute(SCI_GETCODEPAGE); 
   fontName = wmc->wchar2char(mFontName.c_str(), cp);
#else
   fontName = mFontName.c_str();
#endif
   _scintView.execute(SCI_STYLESETFONT, iPat, (LPARAM)fontName);
   _scintView.execute(SCI_STYLESETSIZE, iPat, (LPARAM)mFontSize);
   
   // copy styles into result window cache because while painting
   // user may have removed a pattern already
   for(unsigned iPat = 0; iPat < mPatStyleList.size(); ++iPat) 
   {
      //const tclPattern& rPat = mPatStyleList.getPattern(mPatStyleList.getPatternId(iPat));
      _scintView.execute(SCI_STYLESETFONT, transStyleId[iPat], (LPARAM)fontName);
      _scintView.execute(SCI_STYLESETSIZE, transStyleId[iPat], (LPARAM)mFontSize);
   } // for 
}

void tclFindResultDlg::updateWindowData(const generic_string& fontName, unsigned fontSize) {
   mFontName = fontName;
   mFontSize = fontSize;
   setPatternFonts();
}

void tclFindResultDlg::clear_view()
{
   if (_scintView.getHSelf() != NULL)
   {
       setFinderReadOnly(false);
      _scintView.execute(SCI_CLEARALL);
       setFinderReadOnly(true);
   }
}

void tclFindResultDlg::clear() 
{
   setCurrentMarkedLine(-1);
   //_foundInfos.clear(); 
   mFindResults.clear();
   if(mUseBookmark) {
      _pParent->execute(scnActiveHandle, SCI_MARKERDELETEALL, MARK_BOOKMARK);
   }
   clear_view();
   _lineCounter = 0;
}

int tclFindResultDlg::getCurrentMarkedLine() const 
{
   return _markedLine;
}

/**
* set the marked line with
*/ 
void tclFindResultDlg::setCurrentMarkedLine(int line) 
{  // we set the current mark here
   _markedLine = line;
   if(line != -1) {
      mFromFindResult = true;
      _pParent->execute(scnActiveHandle, SCI_GOTOLINE, _markedLine);
   }
}

void tclFindResultDlg::setSearchPatterns(const tclPatternList& list){
   // inform find window about new searches
   mFindResultSearchDlg.setSearchPatterns(list);
}

/**
function is called whenever the styles in the result window have to be changed
*/
void tclFindResultDlg::setPatternStyles(const tclPatternList& list)
{
   // remove styles from mPatStyleList in case nothing is left in result window
   if (mFindResults.size() == 0) { 
      DBG0("setPatternStyles() clearing style list.");
      mPatStyleList.clear();
   }
   // avoiding setting if nothing changed is done by scintilla window
   // the selection type anyhow is not part of style and needs to be 
   // detected manually
   bool bReStyle = false;
   tclPatternList::const_iterator it = list.begin();
   for (; it != list.end(); ++it) {
      tclPatternList::const_iterator iIntern = mPatStyleList.find(it.getPatId());
      if ( iIntern != mPatStyleList.end()) {
         // style already exists check selection type
         if(iIntern.getPattern().getSelectionType() != 
            it.getPattern().getSelectionType()){
               bReStyle = true;
         }
      }
      mPatStyleList.setPattern(it.getPatId(), it.getPattern());
   }

   /*
   Propably we need some more features here ?

   SCI_STYLERESETDEFAULT stelle default ein
   SCI_STYLECLEARALL setzen alle zeichen auf default

   used for longer lived parse states like language 
   SCI_SETLINESTATE(int line, int value)
   SCI_GETLINESTATE(int line)
   SCI_GETMAXLINESTATE
   */
    
   std::string rtfColTbl;
   char styleNum[4];

   unsigned iPat = FNDRESDLG_DEFAULT_STYLE;
   tclPattern defPat;
   _scintView.execute(SCI_STYLESETVISIBLE, iPat, !defPat.getIsHideText());
   _scintView.execute(SCI_STYLESETBOLD, iPat, defPat.getIsBold());
   _scintView.execute(SCI_STYLESETITALIC, iPat, defPat.getIsItalic());
   _scintView.execute(SCI_STYLESETUNDERLINE, iPat, defPat.getIsUnderlined());
   _scintView.execute(SCI_STYLESETFORE, iPat, defPat.getColorNum());
   _scintView.execute(SCI_STYLESETBACK, iPat, defPat.getBgColorNum());
   _scintView.execute(SCI_STYLESETEOLFILLED, iPat, (defPat.getSelectionType()==tclPattern::line));
   rtfColTbl += RTF_COLTAG_RED;
   rtfColTbl += _itoa(RTF_COL_R(defPat.getColorNum()),styleNum, 10);
   rtfColTbl += RTF_COLTAG_GREEN;
   rtfColTbl += _itoa(RTF_COL_G(defPat.getColorNum()),styleNum, 10);
   rtfColTbl += RTF_COLTAG_BLUE;
   rtfColTbl += _itoa(RTF_COL_B(defPat.getColorNum()),styleNum, 10);
   rtfColTbl += RTF_COLTAG_END;
   // copy styles into result window cache because while painting
   // user may have removed a pattern already
   for(unsigned iPat = 0; iPat < mPatStyleList.size(); ++iPat) 
   {  
      int i = iPat < MY_STYLE_COUNT ? iPat : FNDRESDLG_DEFAULT_STYLE;
      const tclPattern& rPat = mPatStyleList.getPattern(mPatStyleList.getPatternId(iPat));
      _scintView.execute(SCI_STYLESETVISIBLE, transStyleId[i], !rPat.getIsHideText());
      _scintView.execute(SCI_STYLESETBOLD, transStyleId[i], rPat.getIsBold());
      _scintView.execute(SCI_STYLESETITALIC, transStyleId[i], rPat.getIsItalic());
      _scintView.execute(SCI_STYLESETUNDERLINE, transStyleId[i], rPat.getIsUnderlined());
      _scintView.execute(SCI_STYLESETFORE, transStyleId[i], rPat.getColorNum());
      _scintView.execute(SCI_STYLESETBACK, transStyleId[i], rPat.getBgColorNum());
      _scintView.execute(SCI_STYLESETEOLFILLED, transStyleId[i], (rPat.getSelectionType()==tclPattern::line));
      rtfColTbl += RTF_COLTAG_RED;
      rtfColTbl += _itoa(RTF_COL_R(rPat.getColorNum()),styleNum, 10);
      rtfColTbl += RTF_COLTAG_GREEN;
      rtfColTbl += _itoa(RTF_COL_G(rPat.getColorNum()),styleNum, 10);
      rtfColTbl += RTF_COLTAG_BLUE;
      rtfColTbl += _itoa(RTF_COL_B(rPat.getColorNum()),styleNum, 10);
      rtfColTbl += RTF_COLTAG_END;
   } // for
   
   // set the actual color table to the rtf clipboard func
   _scintView.setRtfColorTable(rtfColTbl.c_str());
   setPatternFonts();

   if(bReStyle) {
      _scintView.execute(SCI_COLOURISE, 0, -1);
   }
}

void tclFindResultDlg::updateDockingDlg(void) {
   _scintView.updateLineNumberWidth(true);
   _scintView.execute(SCI_COLOURISE, 0, -1);
   DockingDlgInterface::updateDockingDlg();
   saveSearchDoc();
}

void tclFindResultDlg::setFileName(const generic_string& str) {
   if(mSearchFileName != str) {
      mSearchFileName = str;
      updateHeadline();
   }
}

void tclFindResultDlg::updateHeadline() {
   generic_string str = mSearchFileName;
   if (mSearchResultFile.size()) {
      str += generic_string(TEXT(" -> ")) + mSearchResultFile;
   }
   (void)generic_strncpy(_ResAdditionalInfo, str.c_str(), COUNTCHAR(_ResAdditionalInfo));
   _ResAdditionalInfo[COUNTCHAR(_ResAdditionalInfo)-1]=0;
   ::SendMessage(_hParent, NPPM_DMMUPDATEDISPINFO, 0, (LPARAM)_hSelf);
}

#ifdef FEATURE_RESVIEW_POS_KEEP_AT_SEARCH
void tclFindResultDlg::saveCurrentViewPos() {
   mCurrentViewLineNo = (int)_scintView.execute(SCI_GETFIRSTVISIBLELINE);
   DBG1("tclFindResultDlg::saveCurrentViewPos() VisibleLine %d", mCurrentViewLineNo);
}

void tclFindResultDlg::restoreCurrentViewPos() {
   int res = (int)_scintView.execute(SCI_SETFIRSTVISIBLELINE, mCurrentViewLineNo);
   DBG1("tclFindResultDlg::restoreCurrentViewPos() Setting the line returns %d", res);
}
#endif

/** this function expects iThisMainLine to be a line with search result found in it with help of getNextFoundLine() 
    Special handling! -1 will scroll always to the end of the document.
**/
void tclFindResultDlg::setCurrentViewPos(int iThisMainLine) {
   int iResLine;
   if (-1 == iThisMainLine) {
      int len = (int)_scintView.execute(SCI_GETLENGTH);
      iResLine = (int)_scintView.execute(SCI_LINEFROMPOSITION, len);
   }
   else {
      iResLine = mFindResults.getLineNoAtRes(iThisMainLine);
   }
   DBGDEF(int res = (int) ) _scintView.execute(SCI_SETFIRSTVISIBLELINE, iResLine);
   DBG2("tclFindResultDlg::setCurrentViewPos() Setting the line to %d returns %d", iResLine, res);
}

void tclFindResultDlg::updateViewScrollState(int iLineInMain, bool bInMain) {
   static int topLine = 0;
   if (topLine != iLineInMain){
      topLine = iLineInMain;
      if (bInMain && !mFromFindResult) {
         // text window moves search result
         int line = getNextFoundLine(topLine);
         mFromMainWindow = bInMain;
         setCurrentViewPos(line);
      }
      mFromFindResult = false;
   }
}
// public version calls internal with correct start and end values
void tclFindResultDlg::doStyle(int iFoundLine) {
   int iLineNumber = mFindResults.getLineNoAtRes(iFoundLine);
   DBG2("doStyle(iFoundLine) iFoundLine %d resLine %d", iFoundLine, iLineNumber);
   int styleBegin = (int)_scintView.execute(SCI_POSITIONFROMLINE, iLineNumber);
   int endOfLine = (int)_scintView.execute(SCI_GETLINEENDPOSITION, iLineNumber);
   doStyle(iLineNumber, styleBegin, endOfLine);
}

void tclFindResultDlg::doFindResultSearchDlg() {
   mFindResultSearchDlg.doDialog();
}

void tclFindResultDlg::doSaveToFile() {
   OPENFILENAME ofn;       // common dialog box structure
   TCHAR szFile[MAX_PATH]=TEXT("");       // buffer for file name
   (void)generic_strncpy(szFile, mSearchResultFile.c_str(), COUNTCHAR(szFile));
   szFile[COUNTCHAR(szFile)-1]=0;
   
   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = _hSelf;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = COUNTCHAR(szFile);
   ofn.lpstrFilter = TEXT("All\0*.*\0");
   ofn.nFilterIndex = 2;
   ofn.lpstrFileTitle = TEXT("Save Analyse Search Result to File");
   ofn.nMaxFileTitle = 0;// strlen("Open Analyse Config File")+1;
   ofn.lpstrInitialDir = szFile;
   ofn.Flags = OFN_OVERWRITEPROMPT; // OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   if (GetSaveFileName(&ofn)==TRUE) 
   {   
      if(0 != (int)generic_stricmp(mSearchResultFile.c_str(), ofn.lpstrFile)) {
         mSearchResultFile = ofn.lpstrFile;
         updateHeadline();
      }
   }
}

// message call back method
INT_PTR CALLBACK tclFindResultDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) 
   {
   case IDC_DO_CHECK_CONF: 
      {
         mUseBookmark = _pParent->getUseBookmark();
         if(mDisplayLineNo != _pParent->getDisplayLineNo()) {
            mDisplayLineNo = _pParent->getDisplayLineNo();
            clear();
            _pParent->runSearch();
         }
         updateWindowData(_pParent->getResultFontName(), _pParent->getResultFontSize());
         break;
      }
   case WM_COMMAND : 
      {
         switch (wParam)
         {
         case FNDRESDLG_SCINTILLAFINFER_COPY :
            {
               _scintView.execute(SCI_COPY);
               _scintView.doRichTextCopy();
               return TRUE;
            }
         case FNDRESDLG_SCINTILLAFINFER_SELECTALL :
            {
               _scintView.execute(SCI_SELECTALL);
               return TRUE;
            }
         case FNDRESDLG_SCINTILLAFINFER_SEARCH:
            {
               doFindResultSearchDlg();
               return TRUE;
            }
         case FNDRESDLG_SCINTILLAFINFER_SAVEFILE:
            {
               doSaveToFile();
               saveSearchDoc();
               return TRUE;
            }
         case FNDRESDLG_SCINTILLAFINFER_SAVE_CLR:
            {
               mSearchResultFile.clear();
               updateHeadline();
               return TRUE;
            }
         default :
            {
               break;
            }
         }; // switch
         break;
      }
   case WM_CONTEXTMENU :
      {
         if (HWND(wParam) == _scintView.getHSelf())
         {
            POINT p;
            ::GetCursorPos(&p);
            ContextMenu scintillaContextmenu;
            scintillaContextmenu.create(_scintView.getHSelf(), _scintView.getContextMenu());
            scintillaContextmenu.display(p);
            return TRUE;
         }
         return (BOOL)::DefWindowProc(_hSelf, message, wParam, lParam);
      }

   case WM_SIZE :
      {
         RECT rc;
         getClientRect(rc);
         _scintView.reSizeTo(rc);
         return TRUE;
      }

   case WM_NOTIFY:
      {
         // defer into extra function because of reinterpret cast 
         if(false == notify(reinterpret_cast<SCNotification *>(lParam))){
            // notify didn't know about the notification send to parent
            return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
         }
         return TRUE;
      }
   //case WM_RBUTTONDOWN:
 //     {
 //        int i = 10;
 //     }

   default :
      // call base class
      return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
   }
   return FALSE;
}

/** 
this function sends the series of commands to 
scintilla win with the corresponding style id converted from the mPatStyleList
**/
void tclFindResultDlg::setStyle(tPatId iPatternId, int iBeginPos, int iLength)
{
   unsigned u = mPatStyleList.getPatternIndex(iPatternId);
   u = transStyleId[u]; // shift by table lookup to avoid predefined default styles
   DBG4("setStyle() pattern %f style %d begin %d length %d", iPatternId, u, iBeginPos, iLength);
   _scintView.execute(SCI_STARTSTYLING, iBeginPos, MY_STYLE_MASK);
   _scintView.execute(SCI_SETSTYLING, iLength, u);
}

/** 
this function sends the series of commands to 
scintilla win with the corresponding style id 
**/
void tclFindResultDlg::setDefaultStyle(int iBeginPos, int iLength)
{
   DBG2("setDefaultStyle() begin %d length %d", iBeginPos, iLength);
   _scintView.execute(SCI_STARTSTYLING, iBeginPos, MY_STYLE_MASK);
   _scintView.execute(SCI_SETSTYLING, iLength, FNDRESDLG_DEFAULT_STYLE);
}

// callback from scintilla to colorize the search window
void tclFindResultDlg::doStyle(int startResultLineNo, int startStyleNeeded, int endStyleNeeded) 
{
   DBG3("doStyle() startResultLineNo %d startStyleNeeded %d endStyleNeeded %d", 
      startResultLineNo, startStyleNeeded, endStyleNeeded);
   /*
   Procedure
   1. get the last time styled position to start on SCI_GETENDSTYLED
   startPos = SCI_GETENDSTYLED();
   2. from 1) until endStyleNeeded do:
   lineNumber = SCI_LINEFROMPOSITION(startPos);
   startPos = SCI_POSITIONFROMLINE(lineNumber);
   doStyle(lineNumber, startPos, endStyleNeeded);
   Concept of styling
   each pattern has a color and style attributes bold, underline and italic
   this data is implemented in styles having STYLE_LASTPREDEFINED + patId no.
   the styles are become set at time of activating a search and only changes
   are put through because the change of style invalidates previously set 
   styles
   a pattern later in list overrides the previous one
   a pattern can restrict the styling to the search word
   Process per line
   from result look which Positions were found
   from first to last position look 
   if the line shall be styled or the word
   execute the styling
   */
   int resultLineNum = startResultLineNo;
   int styleBegin = startStyleNeeded;
   int maxResultLines = (int)_scintView.execute(SCI_GETLINECOUNT);
   int endOfLine = (int)(_scintView.execute(SCI_GETLINEENDPOSITION, resultLineNum)
                     +((resultLineNum<maxResultLines)?2:0)); // CRLF is not in last line
   //int charsHidden = 0;
   do // for all lines to be styled
   {
      if(resultLineNum < mFindResults.size() ) 
      {
         const tlpLinePosInfo& rlpi = mFindResults.getLineAtRes(resultLineNum);
         // for each pattern applicable for this line 
         DBG2("doStyle() resLine %d size of patterns %d. ",
            resultLineNum, rlpi.second.posInfos().size());
         
         int iLength = endOfLine - styleBegin;                  
         if(iLength < 0 ) {
            DBG0("doStyle() ERROR negative line length");
            continue;
         } 
         // default style
         setDefaultStyle(styleBegin, iLength);
         if(iLength <= (mDisplayLineNo?miLineHeadSize:0)) {
            DBG0("doStyle() line length 0 nothing to style");
            ++resultLineNum;
            styleBegin = (int)_scintView.execute(SCI_POSITIONFROMLINE, resultLineNum);
            endOfLine = (int)_scintView.execute(SCI_GETLINEENDPOSITION, resultLineNum);
            continue;
         } 
         tlmIdxPosInfo::const_iterator iPosInfo = rlpi.second.posInfos().begin();
         for(; iPosInfo != rlpi.second.posInfos().end(); ++iPosInfo) 
         {
            // iPosInfo (PatternId, FoundPositions)
            tclPatternList::const_iterator iPattern = mPatStyleList.find(iPosInfo->first);
            if(iPattern == mPatStyleList.end()) {
               DBG1("doStyle() ERROR invalid iPosInfo id %f! use default painter",
                  iPosInfo->first);
               // default style already done.
               continue;
            }
            if(iPattern.getPattern().getSelectionType() == tclPattern::line) {
               // style whole line if chars left after line header
               DBG0("doStyle() style for line");
               setStyle(iPosInfo->first, styleBegin+(mDisplayLineNo?miLineHeadSize:0), iLength-(mDisplayLineNo?miLineHeadSize:0)); // until end of line
            } else {
               // style per found positions
               // iPosInfo calculates on mainWin text
               tlsPosInfo::const_iterator iFoundPos = iPosInfo->second.begin();
               for(; iFoundPos != iPosInfo->second.end(); ++iFoundPos) {
                  int iPosInfoLength = (iFoundPos->end - iFoundPos->start);
                  int iPosLineBegin = iFoundPos->start - (int)_pParent->execute(scnActiveHandle,SCI_POSITIONFROMLINE,iFoundPos->line);
                  if((iPosInfoLength>0) && (iPosLineBegin>=0) &&
                     ((styleBegin+(mDisplayLineNo?miLineHeadSize:0)+iPosLineBegin) >= styleBegin) &&
                     (iPosInfoLength <= (endOfLine-styleBegin))) {
                     // do styling 
                        setStyle(iPosInfo->first, styleBegin+(mDisplayLineNo?miLineHeadSize:0)+iPosLineBegin, iPosInfoLength);
                  } else {
                     DBG4("doStyle() ERROR word styling pos illegal pos.start %d styleBegin %d pos.end %d endOfLine %d.",
                        styleBegin+(mDisplayLineNo?miLineHeadSize:0)+iPosLineBegin, styleBegin, 
                        styleBegin+(mDisplayLineNo?miLineHeadSize:0)+iPosLineBegin+iPosInfoLength, endOfLine);
                  }
               }
            }
         }
      } else {
         // line in result is not in result list (one empty line break at the end)
         // make style marker is moved until end as requested
         DBG1("doStyle() called on empty line %d", resultLineNum);
      }
      ++resultLineNum;
      styleBegin = (int)_scintView.execute(SCI_POSITIONFROMLINE, resultLineNum);
      endOfLine = (int)_scintView.execute(SCI_GETLINEENDPOSITION, resultLineNum
                       +((resultLineNum<maxResultLines)?2:0)); // CRLF is not in last line
   } while ((styleBegin < endStyleNeeded)&&(resultLineNum < maxResultLines));
}

bool tclFindResultDlg::notify(SCNotification *notification)
{
   bool ret = false; // true if message processed
   if(notification == 0) return ret;
   static bool bDoTrace = false; // set to true via debugger to trace this line
   if (bDoTrace) {
      DBG2("notify() code %x line %x.", notification->nmhdr.code, notification->line);
   }
   switch (notification->nmhdr.code) 
   {
   case SCN_STYLENEEDED:
      {
         //scn.nmhdr.code = SCN_STYLENEEDED;
         //scn.position = endStyleNeeded;
         DBG1("findresult SCN_STYLENEEDED end %d ", notification->position);
         int startPos = (int)_scintView.execute(SCI_GETENDSTYLED);
         int lineNumber = (int)_scintView.execute(SCI_LINEFROMPOSITION,startPos);
         startPos = (int)_scintView.execute(SCI_POSITIONFROMLINE,lineNumber);
         doStyle(lineNumber, startPos, notification->position);
         ret = true;
         break;
      }
   case SCN_MARGINCLICK: 
      {   
         DBG2("findresult SCN_MARGINCLICK margin %d line %d", 
               notification->margin, notification->position);
         
         int lineEnd = (int)_scintView.execute(SCI_LINEFROMPOSITION, notification->position);
         lineEnd = (int)_scintView.execute(SCI_GETLINEENDPOSITION, lineEnd);
         lineEnd = lineEnd + 2; // for line end
         _scintView.execute(SCI_SETSEL, notification->position, lineEnd);
         if (notification->margin == ScintillaSearchView::_SC_MARGE_FOLDER)
         {
            _scintView.marginClick(notification->position, notification->modifiers);
         }
         ret = true;
         break;
      }

   case SCN_DOUBLECLICK :
      {
         NPP_TRY {
            int currentPos = (int)_scintView.execute(SCI_GETCURRENTPOS);
            if (currentPos>1)
            {
               char prevChar = (char)_scintView.execute(SCI_GETCHARAT, currentPos - 1);
               if (prevChar == 0x0A) {
                  if(currentPos < _scintView.execute(SCI_GETLENGTH))
                      currentPos -= 2; // step to last char of line before CR LF
               }
            }

            // if getInfo() method the previous line is renew as current for next call
            // old generic_string fullPath = _pParent->getSearchFileName();

            // get currently marked line in result doc
            int resLineNo = (int)_scintView.execute(SCI_LINEFROMPOSITION, currentPos);
            if ((resLineNo >= _scintView.execute(SCI_GETLINECOUNT)) || (mFindResults.size() == 0)) {
               // we are out of editable range. don't do anything
               return TRUE;
            }
            const tlpLinePosInfo& lineInfo = mFindResults.getLineAtRes(resLineNo);
            int lineMain = (int)lineInfo.first;
            //int startMain = (int)_pParent->execute(scnActiveHandle, SCI_POSITIONFROMLINE, lineMain, 0);

            //int cmd = NPPM_SWITCHTOFILE;//getMode()==FILES_IN_DIR?WM_DOOPEN:NPPM_SWITCHTOFILE;
            int iSuccess = (int)_pParent->execute(nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)getszFileName());
            if (iSuccess == 0) {
               struct _stat st;
               iSuccess = (filestat(getszFileName(),&st))?0:1; // check if file exist -> ret == 0
               // try to open it
               if (iSuccess != 0) {
                  iSuccess = (int)_pParent->execute(nppHandle, NPPM_DOOPEN, 0, (LPARAM)getszFileName());
               } else {
                  DBG0("notify(SCNotification) SCN_DOUBLECLICK filestat() didn't find the file");
               }
            }
            if(iSuccess == 0) {
               ::MessageBox(0, TEXT("File could not be opened, were this search result comes from. \nRestart search with [Search]"), TEXT("Analyse Plugin - Sorry"), 0);
            } else {
               //_pParent->displaySectionCentered(startMain, startMain);
               // we set the current mark here
               setCurrentMarkedLine(lineMain);
               if (!_pParent->getDblClickJumps2EditView()) {
                  getFocus();
               }
            }

            DBG3("notify(SCNotification) SCN_DOUBLECLICK to line  %d in %s line in result", 
               lineMain, getszFileName(), resLineNo);

            ret = true;

         } NPP_CATCH_ALL{
            DBG0("notify(SCNotification) ERROR SCN_DOUBLECLICK problem.");
         }
         break;
      }
   case SCN_PAINTED:
   case SCN_MODIFIED:
   case SCN_SCROLLED:
      break;
   case SCN_CHARADDED:
      {
         DBG1("notify(char added) notification->ch = 0x%x.", notification->ch);
         if(notification->ch == FNDRESDLG_ACTIVATE_SEARCH) {
            mFindResultSearchDlg.display();
         }
         break;
      }
   case SCN_UPDATEUI:
   {
      if (((notification->updated & SC_UPDATE_V_SCROLL) != 0) && _pParent->getIsSyncScroll()) {
         int currTopLine = (int)_scintView.execute(SCI_GETFIRSTVISIBLELINE);
         DBG1("notify() SCN_UPDATEUI: Scrolled to currTopLine=%d", currTopLine);
         // setting mainwondow based on result windows setting is disabled to avoid echo causing 
         if (!mFromMainWindow) {
            mFromFindResult = true;
            tiLine mainLine = mFindResults.getLineNoAtMain(currTopLine);
            generic_string currFile;
            if (_pParent->bCheckLastFileNameSame(currFile)) {
               _pParent->execute(scnActiveHandle, SCI_ENSUREVISIBLEENFORCEPOLICY, (WPARAM)mainLine);
            }
         }
         else {
            mFromMainWindow = false;
            mFromFindResult = false;
         }
      }
      break;
   }
   default :
      int i = notification->nmhdr.code;
      DBG2("notify() unused notification code %d 0x%x", i, i);
      break;
   }
   return ret;
}

void tclFindResultDlg::setFinderReadOnly(bool isReadOnly) {
   _scintView.execute(SCI_SETREADONLY, isReadOnly);
}

void tclFindResultDlg::saveSearchDoc() {
   if (0 == mSearchResultFile.size()) {
      // no saving configured
      return;
   }
   Utf8_16_Write UnicodeConvertor;
   UnicodeConvertor.setEncoding(uniUTF8);
   FILE *fp = UnicodeConvertor.fopen(mSearchResultFile.c_str(), TEXT("wb"));
   if (fp)
   {
      char* buf = (char*) _scintView.execute(SCI_GETCHARACTERPOINTER); // vs. SCI_GETDOCPOINTER
      int lengthDoc = _scintView.getCurrentDocLen();
      if (lengthDoc && buf) {
         size_t items_written;
         items_written = UnicodeConvertor.fwrite(buf, lengthDoc);
         UnicodeConvertor.fclose();

         // Error, we didn't write the entire document to disk.
         // Note that fwrite() doesn't return the number of bytes written, but rather the number of ITEMS.
         if(items_written != 1)
         {
            MessageBox(NULL, TEXT("Problem in save"), TEXT("tclFindResultDlg::saveSearchDoc"), MB_OK);
         }
      }
   }
}
