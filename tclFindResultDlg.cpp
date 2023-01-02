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
#include "ContextMenu.h"
#include "tclFindResultDoc.h"
#include "tclFindResultDlg.h"
#include <commdlg.h>// For fileopen dialog.
#define MDBG_COMP "FRDlg:" 
#include "myDebug.h"
#include "resource.h"
#include "npp_exceptions.h"

#define STYLING_MASK 255
#define FNDRESDLG_LINE_HEAD ""
#define FNDRESDLG_LINE_COLON ": "
#define FNDRESDLG_LINE_HYPHEN "| "
#define FNDRESDLG_DEFAULT_STYLE STYLE_DEFAULT // style number for the default styling
#define FNDRESDLG_ACTIVATE_SEARCH 0x06

#ifdef UNICODE
#define filestat _wstat
#else
#define filestat _stat
#endif
// available style id -> sub sequent 0-based index
// counter definition is ScintillaSearchView::transStylePos for clipboard colors
const int tclFindResultDlg::transStyleIdTab[MY_STYLE_COUNT] = {
        1,  2,  3,  4,  5,  6,  7,  8,  9, // STYLE_DEFAULT is for default color
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
  120,121,122,123,124,125,126,127,128,129,
  130,131,132,133,134,135,136,137,138,139,
  140,141,142,143,144,145,146,147,148,149,
  150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,
  170,171,172,173,174,175,176,177,178,179,
  180,181,182,183,184,185,186,187,188,189,
  190,191,192,193,194,195,196,197,198,199,
  200,201,202,203,204,205,206,207,208,209,
  210,211,212,213,214,215,216,217,218,219,
  220,221,222,223,224,225,226,227,228,229,
  230,231,232,233,234,235,236,237,238,239,
  240,241,242,243,244,245,246,247,248,249,
  250,251,252,253,254,255
};

int tclFindResultDlg::transStyleId(unsigned int id) const {
   if (id < MY_STYLE_COUNT) {
      return transStyleIdTab[id];
   }
   else {
      //return transStyleIdTab[MY_STYLE_COUNT-1]; // last style repeated
      return STYLE_DEFAULT;
   }
}
tlpLinePosInfo tclFindResultDoc::mDefLineInfo = tlpLinePosInfo();

tclFindResultDlg::tclFindResultDlg() 
   : DockingDlgInterface(IDD_FIND_DLG_RESULT)
   , _pParent(0)
   , _markedLine(-1)
   , _lineCounter(0) 
   , miLineNumColSize(0)
   , miLineHeadSize(0)
   , mFontSize(8)
   , mUseBookmark(1)
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
   _scintView.execute(SCI_SETILEXER,SCLEX_CONTAINER);
// deprecated, always 8   _scintView.execute(SCI_SETSTYLEBITS, MY_STYLE_BITS); // maximum possible
   mFindResultSearchDlg.init(_hInst, _hParent, &_scintView);
   mFindResultSearchDlg.setdefaultPattern(defaultPattern);
   mFindResultSearchDlg.create(IDD_FIND_RES_DLG_SEARCH);
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
            tiLine resultLine = mFindResults.getLineNoAtRes(thisLine);
            if(resultLine >= 0) {
               if(mUseBookmark){
                  _pParent->execute(teNppWindows::scnActiveHandle, SCI_MARKERDELETE, thisLine, _pParent->getBookmarkId());
               }
               tiLine startL = (tiLine)_scintView.execute(SCI_POSITIONFROMLINE, resultLine);
               tiLine endL = (tiLine)_scintView.execute(SCI_GETLINEENDPOSITION, resultLine);
               if (endL+2 <= _scintView.execute(SCI_GETLENGTH)) {
                  endL += 2;
               }
               DBG4("removeUnusedResultLines() removing line %d from %d to %d being main line %d", (int)resultLine, (int)startL, (int)endL, (int)thisLine);
               if(endL <= startL) {
                  ::MessageBox(0, TEXT("Line delete in result not correct"), TEXT("Analyse Plugin - Error"), 0);
               } else {
                  setFinderReadOnly(false);
                  _scintView.execute(SCI_SETTARGETSTART, startL);
                  _scintView.execute(SCI_SETTARGETEND, endL);
                  //SCI_REPLACETARGET(int length, const char *text)
                  _scintView.execute(SCI_REPLACETARGET, 0, (LPARAM)"");
                  DBG2("removeUnusedResultLines() tstart %d, tend %d.", (int)startL, (int)endL);
                  setFinderReadOnly(true);
               }
            } else {
               TCHAR num[20];
               ::MessageBox(0, TEXT("The line was not in ?"), generic_i64toa(resultLine, num, 10), MB_OK);
            }
            mFindResults.erase(thisLine);
         } else {
            DBG2("removeUnusedResultLines() found line %d result not empty size %d", (int)thisLine, (int)l.posInfos().size());
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

tiLine tclFindResultDlg::insertPosInfo(tPatId patternId, tiLine iResultLine, tclPosInfo pos) {
   return mFindResults.insertPosInfo(patternId, iResultLine, pos);
}

bool tclFindResultDlg::getLineAvail(tiLine foundLine) const {
   return mFindResults.getLineAvail(foundLine);
}

tiLine tclFindResultDlg::getNextFoundLine(tiLine iEditorsLine) const {
   return mFindResults.getNextLineNoAtMain(iEditorsLine);
}

const std::string& tclFindResultDlg::getLineText(tiLine iResultLine) {
   return mFindResults.getLineText(iResultLine);
}

void tclFindResultDlg::setLineText(tiLine iFoundLine, const std::string& text, const std::string& comment, unsigned commentWidth) {
   bool bNewLine = mFindResults.setLineText(iFoundLine, text);
   // here we have to distinguish update and insert of lines in search result window
   tiLine resLine = mFindResults.getLineNoAtRes(iFoundLine);
   //bool bVisible = mFindResults.getLineAtMain(iFoundLine).visible();
   tiLine startPos = (tiLine)_scintView.execute(SCI_POSITIONFROMLINE, resLine);
   if (startPos != -1) {
      setCurrentMarkedLine(-1);
      setFinderReadOnly(false);
      std::string s = FNDRESDLG_LINE_HEAD;
      s.reserve(text.size() 
                + (_scintView.getLineNumbersInResult())?(miLineNumColSize + strlen(FNDRESDLG_LINE_COLON)):(0)
                + (mDisplayComment)?(commentWidth + strlen(FNDRESDLG_LINE_HYPHEN)):(0));
      char conv[20];
      if (_scintView.getLineNumbersInResult()) {
         s.append(miLineNumColSize-strlen(_i64toa(iFoundLine+1, conv, 10)), ' ');
         s.append(conv);
         s.append(FNDRESDLG_LINE_COLON);
      }
      if (mDisplayComment) {
         s.append(comment);
         s.append(commentWidth-comment.length(), ' ');
         s.append(FNDRESDLG_LINE_HYPHEN);
      }
      s.append(text);
      DBGA3("setLineText() iFoundLine: %d resLine: %d text: \"%s\"", (int)iFoundLine, (int)resLine, s.c_str());
      if(bNewLine) {
         // adding a newline into result -> set bookmark in main window
         if(mUseBookmark) {
            _pParent->execute(teNppWindows::scnActiveHandle, SCI_MARKERADD, iFoundLine, _pParent->getBookmarkId());
         }
         _scintView.execute(SCI_INSERTTEXT, startPos, (LPARAM)s.c_str());
         ++_lineCounter;
      } else {
         tiLine endPos = (tiLine)_scintView.execute(SCI_GETLINEENDPOSITION, resLine);
         if (endPos+2 <= _scintView.execute(SCI_GETLENGTH)) {
            endPos += 2;
         }
         // hier text replace from startPos to endPos
         _scintView.execute(SCI_SETTARGETSTART, startPos);
         _scintView.execute(SCI_SETTARGETEND, endPos);
         //SCI_REPLACETARGET(int length, const char *text)
         _scintView.execute(SCI_REPLACETARGET, -1, (LPARAM)s.c_str());
         DBG2("setLineText() tstart %d, tend %d.", (int)startPos, (int)endPos);
      }
//      doStyle(resLine, startPos, (int)startPos+(int)s.length());
      setFinderReadOnly(true);
   } else {
      DBG1("setLineText() ERROR iFoundLine: %d not in editor! do  nothing", (int)iFoundLine);
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
   WcharMbcsConvertor *wmc = &WcharMbcsConvertor::getInstance();
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
      _scintView.execute(SCI_STYLESETFONT, transStyleId(iPat), (LPARAM)fontName);
      _scintView.execute(SCI_STYLESETSIZE, transStyleId(iPat), (LPARAM)mFontSize);
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

void tclFindResultDlg::clear(bool initial)
{
   setCurrentMarkedLine(-1);
   //_foundInfos.clear(); 
   mFindResults.clear();
   if(mUseBookmark && !initial) {
      _pParent->execute(teNppWindows::scnActiveHandle, SCI_MARKERDELETEALL, _pParent->getBookmarkId());
   }
   clear_view();
   _lineCounter = 0;
}

tiLine tclFindResultDlg::getCurrentMarkedLine() const
{
   return _markedLine;
}

/**
* set the marked line with
*/ 
void tclFindResultDlg::setCurrentMarkedLine(tiLine line)
{  // we set the current mark here
   _markedLine = line;
   if(line != -1) {
      mFromFindResult = true;
      _pParent->execute(teNppWindows::scnActiveHandle, SCI_GOTOLINE, _markedLine);
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
    
   unsigned iDefPat = FNDRESDLG_DEFAULT_STYLE;
   tclPattern defPat = mFindResultSearchDlg.getdefaultPattern();
   _scintView.execute(SCI_STYLESETVISIBLE, iDefPat, !defPat.getIsHideText());
   _scintView.execute(SCI_STYLESETBOLD, iDefPat, defPat.getIsBold());
   _scintView.execute(SCI_STYLESETITALIC, iDefPat, defPat.getIsItalic());
   _scintView.execute(SCI_STYLESETUNDERLINE, iDefPat, defPat.getIsUnderlined());
   _scintView.execute(SCI_STYLESETFORE, iDefPat, defPat.getColorNum());
   _scintView.execute(SCI_STYLESETBACK, iDefPat, defPat.getBgColorNum());
   _scintView.execute(SCI_STYLESETEOLFILLED, iDefPat, (defPat.getSelectionType()==tclPattern::line));
   // prepare rtfColTbl as color table for richtext support color 0 = default color
   _scintView.startRtfColorTable(defPat.getColorNum());
   // copy styles into result window cache because while painting
   // user may have removed a pattern already
   // we can maximally style MY_STYLE_COUNT patterns
   for(unsigned iPat = 0; (iPat < mPatStyleList.size()); ++iPat)
   {  
      if (iPat >= MY_STYLE_COUNT) {
         // no additional colors possible
         int e = 1;
      }
      else {
         const tclPattern& rPat = mPatStyleList.getPattern(mPatStyleList.getPatternId(iPat));
         _scintView.execute(SCI_STYLESETVISIBLE, transStyleId(iPat), !rPat.getIsHideText());
         _scintView.execute(SCI_STYLESETBOLD, transStyleId(iPat), rPat.getIsBold());
         _scintView.execute(SCI_STYLESETITALIC, transStyleId(iPat), rPat.getIsItalic());
         _scintView.execute(SCI_STYLESETUNDERLINE, transStyleId(iPat), rPat.getIsUnderlined());
         _scintView.execute(SCI_STYLESETFORE, transStyleId(iPat), rPat.getColorNum());
         _scintView.execute(SCI_STYLESETBACK, transStyleId(iPat), rPat.getBgColorNum());
         _scintView.execute(SCI_STYLESETEOLFILLED, transStyleId(iPat), (rPat.getSelectionType() == tclPattern::line));
         // colors 1 based
         _scintView.addRtfColor2Table(rPat.getColorNum());
      }
   } // for
   
   _scintView.finalizeRtfColorTable();
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
   mCurrentViewLineNo = (tiLine)_pParent->execute(teNppWindows::scnActiveHandle, SCI_GETFIRSTVISIBLELINE);
   DBG1("tclFindResultDlg::saveCurrentViewPos() VisibleLine %d", (int)mCurrentViewLineNo);
}

void tclFindResultDlg::restoreCurrentViewPos() {
   DBG1("tclFindResultDlg::restoreCurrentViewPos() Re-setting the line %d", (int)mCurrentViewLineNo);
   updateViewScrollState(mCurrentViewLineNo, true, true);
}
#endif

/** this function expects iThisMainLine to be a line with search result found in it with help of getNextFoundLine() 
    Special handling! -1 will scroll always to the end of the document.
**/
void tclFindResultDlg::setCurrentViewPos(tiLine iThisMainLine) {
   tiLine iResLine;
   if (-1 == iThisMainLine) {
      tiLine len = (tiLine)_scintView.execute(SCI_GETLENGTH);
      iResLine = (tiLine)_scintView.execute(SCI_LINEFROMPOSITION, len);
   }
   else {
      iResLine = mFindResults.getLineNoAtRes(iThisMainLine);
   }
   DBGDEF(tiLine res = (tiLine) ) _scintView.execute(SCI_SETFIRSTVISIBLELINE, iResLine);
   DBG2("tclFindResultDlg::setCurrentViewPos() Setting the line to %d returns %d", (int)iResLine, (int)res);
}

void tclFindResultDlg::updateViewScrollState(tiLine iLineInMain, bool bInMain, bool bAnyway) {
   static tiLine topLine = 0;
   DBG3("updateViewScrollState() topLine %d iLineInMain %d bInMain %d", (int)topLine, (int)iLineInMain, (int)bInMain);
   if (bAnyway || topLine != iLineInMain){
      topLine = iLineInMain;
      if (bAnyway || (bInMain && !mFromFindResult)) {
         // text window moves search result
         tiLine line = getNextFoundLine(topLine);
         mFromMainWindow = bInMain;
         setCurrentViewPos(line);
      }
      mFromFindResult = false;
   }
}
// public version calls internal with correct start and end values
void tclFindResultDlg::doStyle(tiLine iFoundLine) {
   tiLine iLineNumber = mFindResults.getLineNoAtRes(iFoundLine);
   DBG2("doStyle(iFoundLine) iFoundLine %d resLine %d", (int)iFoundLine, (int)iLineNumber);
   tiLine styleBegin = (tiLine)_scintView.execute(SCI_POSITIONFROMLINE, iLineNumber);
   tiLine endOfLine = (tiLine)_scintView.execute(SCI_GETLINEENDPOSITION, iLineNumber);
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
         if (mUseBookmark != _pParent->getUseBookmark()) {
            _pParent->execute(teNppWindows::scnActiveHandle, SCI_MARKERDELETEALL, _pParent->getBookmarkId());
         }
         mUseBookmark = _pParent->getUseBookmark();
         if(_scintView.getLineNumbersInResult() != (_pParent->getDisplayLineNo() != 0)) {
            _scintView.setLineNumbersInResult((_pParent->getDisplayLineNo() != 0));
            clear();
            _pParent->runSearch();
         }
         updateWindowData(_pParent->getResultFontName(), _pParent->getResultFontSize());
         const tclPattern* p = (const tclPattern*)lParam;
         mFindResultSearchDlg.setdefaultPattern(*p);
         break;
      }
   case WM_COMMAND : 
      {
         switch (wParam)
         {
         case FNDRESDLG_WRAP_MODE:
            setWrapMode(!getWrapMode());
            break;

         case FNDRESDLG_SCINTILLAFINFER_SAVE_RTF:
            {
            _scintView.doSaveRichtext();
            return TRUE;
         }
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
         case FNDRESDLG_SHOW_LINE_NUMBERS:
            {
               _scintView.setLineNumbersInResult(!_scintView.getLineNumbersInResult());
               _pParent->setDisplayLineNo(_scintView.getLineNumbersInResult()); // toggle value in config dialog
               clear();
               _pParent->runSearch();
               return TRUE;
            }
         case FNDRESDLG_SHOW_OPTIONS:
         {
            _pParent->showConfigDlg();
            return TRUE;
         }
         case FNDRESDLG_SHOW_CONTEXTMENU:
         {
            POINT pt = { static_cast<short>(LOWORD(lParam)),
                        static_cast<short>(HIWORD(lParam)) };
            std::vector<MenuItemUnit> tmp = _scintView.getContextMenu();
            tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
            tmp.push_back(MenuItemUnit(FNDRESDLG_ACTIVATE_PATTERN_LIST, TEXT("matching patterns:")));
            {
               int pos = (int)_scintView.execute(SCI_GETCURRENTPOS);
               int line = (int)_scintView.execute(SCI_LINEFROMPOSITION, pos);
               const tlmIdxPosInfo& p = mFindResults.getLineAtRes(line).second.posInfos();
               tlmIdxPosInfo::const_iterator it = p.begin();
               for (; it != p.end(); ++it) {
                  int idx = _pParent->getPatternIndex(it->first); // list is zero based
                  generic_string s = _pParent->getPatternIdentification(it->first);
                  s += TEXT(":");
                  s += _pParent->getPatternSearchText(it->first);
                  DBG4("Line %d has pattern %f line %d text %s", line, it->first, idx, s.c_str());
                  int range = (FNDRESDLG_ACTIVATE_PATTERN_END - FNDRESDLG_ACTIVATE_PATTERN_BASE);
                  if (idx > range) {
                     break;
                  }
                  tmp.push_back(MenuItemUnit((FNDRESDLG_ACTIVATE_PATTERN_BASE + idx), s));
               }
            }
            // create the menu
            ContextMenu scintillaContextmenu;
            scintillaContextmenu.create(getHSelf(), tmp);
            scintillaContextmenu.enableItem(FNDRESDLG_ACTIVATE_PATTERN_LIST, false); // disable it because it's headline text
            scintillaContextmenu.checkItem(FNDRESDLG_WRAP_MODE, _scintView.getWrapMode());
            scintillaContextmenu.checkItem(FNDRESDLG_SHOW_LINE_NUMBERS, _scintView.getLineNumbersInResult());
            scintillaContextmenu.display(pt);
            return TRUE;
         }
         default :
            if ((wParam >= FNDRESDLG_ACTIVATE_PATTERN_BASE) && (wParam < FNDRESDLG_ACTIVATE_PATTERN_END)) {
               int index = (int)(wParam - FNDRESDLG_ACTIVATE_PATTERN_BASE);
               DBG1("FNDRESDLG_ACTIVATE_PATTERN for index %d", index);
               _pParent->setSelectedPattern(index);
            } else {
               break;
            }
         }; // switch
         break;
      }
   case WM_CONTEXTMENU :
      {
         POINT p;
         ::GetCursorPos(&p);
         ::SendMessage(getHSelf(), WM_COMMAND, FNDRESDLG_SHOW_CONTEXTMENU, MAKELONG(p.x, p.y));
         return TRUE;
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
void tclFindResultDlg::setStyle(tPatId iPatternId, tiLine iBeginPos, tiLine iLength)
{
   unsigned u = mPatStyleList.getPatternIndex(iPatternId);
   u = transStyleId(u); // shift by table lookup to avoid predefined default styles
   DBG4("setStyle() pattern %f style %d begin %d length %d", iPatternId, u, (int)iBeginPos, (int)iLength);
   _scintView.execute(SCI_STARTSTYLING, iBeginPos, MY_STYLE_MASK);
   _scintView.execute(SCI_SETSTYLING, iLength, u);
}

/** 
this function sends the series of commands to 
scintilla win with the corresponding style id 
**/
void tclFindResultDlg::setDefaultStyle(tiLine iBeginPos, tiLine iLength)
{
   DBG2("setDefaultStyle() begin %d length %d", (int)iBeginPos, (int)iLength);
   _scintView.execute(SCI_STARTSTYLING, iBeginPos, MY_STYLE_MASK);
   _scintView.execute(SCI_SETSTYLING, iLength, FNDRESDLG_DEFAULT_STYLE);
}

// callback from scintilla to colorize the search window
void tclFindResultDlg::doStyle(tiLine startResultLineNo, tiLine startStyleNeeded, tiLine endStyleNeeded)
{
   DBG3("doStyle() startResultLineNo %d startStyleNeeded %d endStyleNeeded %d", 
      (int)startResultLineNo, (int)startStyleNeeded, (int)endStyleNeeded);
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
   tiLine resultLineNum = startResultLineNo;
   tiLine styleBegin = startStyleNeeded;
   tiLine maxResultLines = (tiLine)_scintView.execute(SCI_GETLINECOUNT);
   tiLine endOfLine = (tiLine)(_scintView.execute(SCI_GETLINEENDPOSITION, resultLineNum)
                     +((resultLineNum<maxResultLines)?2:0)); // CRLF is not in last line
   //int charsHidden = 0;
   do // for all lines to be styled
   {
      if(resultLineNum < mFindResults.size() ) 
      {
         const tlpLinePosInfo& rlpi = mFindResults.getLineAtRes(resultLineNum);
         // for each pattern applicable for this line 
         DBG2("doStyle() resLine %d size of patterns %d. ",
            (int)resultLineNum, (int)rlpi.second.posInfos().size());
         
         tiLine iLength = endOfLine - styleBegin;                  
         if(iLength < 0 ) {
            DBG0("doStyle() ERROR negative line length");
            continue;
         } 
         // default style
         setDefaultStyle(styleBegin, iLength);
         int iThisLineHead = _scintView.getLineNumbersInResult() ? miLineHeadSize : 0;
         if(iLength <= iThisLineHead) {
            DBG0("doStyle() line length 0 nothing to style");
            ++resultLineNum;
            styleBegin = (tiLine)_scintView.execute(SCI_POSITIONFROMLINE, resultLineNum);
            endOfLine = (tiLine)_scintView.execute(SCI_GETLINEENDPOSITION, resultLineNum);
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
               setStyle(iPosInfo->first, styleBegin+ iThisLineHead, iLength- iThisLineHead); // until end of line
            } else {
               // style per found positions
               // iPosInfo calculates on mainWin text
               tlsPosInfo::const_iterator iFoundPos = iPosInfo->second.begin();
               for(; iFoundPos != iPosInfo->second.end(); ++iFoundPos) {
                  tiLine iPosInfoLength = (iFoundPos->end - iFoundPos->start);
                  tiLine iPosLineBegin = iFoundPos->start - (tiLine)_pParent->execute(teNppWindows::scnActiveHandle,SCI_POSITIONFROMLINE,iFoundPos->line);
                  if((iPosInfoLength>0) && (iPosLineBegin>=0) &&
                     ((styleBegin+ iThisLineHead +iPosLineBegin) >= styleBegin) &&
                     (iPosInfoLength <= (endOfLine-styleBegin))) {
                     // do styling 
                        setStyle(iPosInfo->first, styleBegin+ iThisLineHead +iPosLineBegin, iPosInfoLength);
                  } else {
                     DBG4("doStyle() ERROR word styling pos illegal pos.start %d styleBegin %d pos.end %d endOfLine %d.",
                        (int)styleBegin+ iThisLineHead +iPosLineBegin, (int)styleBegin,
                        (int)styleBegin+ iThisLineHead +iPosLineBegin+iPosInfoLength, (int)endOfLine);
                  }
               }
            }
         }
      } else {
         // line in result is not in result list (one empty line break at the end)
         // make style marker is moved until end as requested
         DBG1("doStyle() called on empty line %d", (int)resultLineNum);
      }
      ++resultLineNum;
      styleBegin = (tiLine)_scintView.execute(SCI_POSITIONFROMLINE, resultLineNum);
      endOfLine = (tiLine)_scintView.execute(SCI_GETLINEENDPOSITION, resultLineNum
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
         DBG1("findresult SCN_STYLENEEDED end %d ", (int)notification->position);
         tiLine startPos = (tiLine)_scintView.execute(SCI_GETENDSTYLED);
         tiLine lineNumber = (tiLine)_scintView.execute(SCI_LINEFROMPOSITION,startPos);
         startPos = (tiLine)_scintView.execute(SCI_POSITIONFROMLINE,lineNumber);
         doStyle(lineNumber, startPos, (tiLine)notification->position);
         ret = true;
         break;
      }
   case SCN_MARGINCLICK: 
      {   
         DBG2("findresult SCN_MARGINCLICK margin %d line %d", 
               notification->margin, (int)notification->position);
         
         tiLine lineEnd = (tiLine)_scintView.execute(SCI_LINEFROMPOSITION, notification->position);
         lineEnd = (tiLine)_scintView.execute(SCI_GETLINEENDPOSITION, lineEnd);
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
            tiLine currentPos = (tiLine)_scintView.execute(SCI_GETCURRENTPOS);
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
            tiLine resLineNo = (tiLine)_scintView.execute(SCI_LINEFROMPOSITION, currentPos);
            if ((resLineNo >= _scintView.execute(SCI_GETLINECOUNT)) || (mFindResults.size() == 0)) {
               // we are out of editable range. don't do anything
               return TRUE;
            }
            const tlpLinePosInfo& lineInfo = mFindResults.getLineAtRes(resLineNo);
            tiLine lineMain = lineInfo.first;
            //int startMain = (int)_pParent->execute(scnActiveHandle, SCI_POSITIONFROMLINE, lineMain, 0);

            //int cmd = NPPM_SWITCHTOFILE;//getMode()==FILES_IN_DIR?WM_DOOPEN:NPPM_SWITCHTOFILE;
            int iSuccess = (int)_pParent->execute(teNppWindows::nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)getszFileName());
            if (iSuccess == 0) {
               struct _stat st;
               iSuccess = (filestat(getszFileName(),&st))?0:1; // check if file exist -> ret == 0
               // try to open it
               if (iSuccess != 0) {
                  iSuccess = (int)_pParent->execute(teNppWindows::nppHandle, NPPM_DOOPEN, 0, (LPARAM)getszFileName());
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

            DBG3("notify(SCNotification) SCN_DOUBLECLICK to line  %d in %s line in result %d", 
               (int)lineMain, getszFileName(), resLineNo);

            ret = true;

         } NPP_CATCH_ALL {
            DBG0("notify(SCNotification) ERROR SCN_DOUBLECLICK problem.");
         }
         break;
      }
   case SCN_PAINTED:
   case SCN_MODIFIED:
   // case SCN_SCROLLED: // TODO // replacement for obsolete custom SCN_SCROLLED:   if (notification->updated & SC_UPDATE_V_SCROLL)
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
         tiLine currTopLine = (tiLine)_scintView.execute(SCI_GETFIRSTVISIBLELINE);
         DBG3("notify() SCN_UPDATEUI: Scroll to currTopLine=%d from main %d result %d", (int)currTopLine, mFromMainWindow, mFromFindResult);
         // setting mainwindow based on result windows setting is disabled to avoid echo causing 
         if (!mFromMainWindow) {
            mFromFindResult = true;
            tiLine mainLine = mFindResults.getLineNoAtMain(currTopLine);
            generic_string currFile;
            if (_pParent->bCheckLastFileNameSame(currFile)) {
               _pParent->execute(teNppWindows::scnActiveHandle, SCI_ENSUREVISIBLEENFORCEPOLICY, (WPARAM)mainLine);
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
      DBGDEF(int i = notification->nmhdr.code;)
      DBG2("notify() unused notification code %d 0x%x", i, i);
      break;
   }
   return ret;
}

void tclFindResultDlg::setFinderReadOnly(bool isReadOnly) {
   DBG1("setFinderReadOnly(): %d", (int)isReadOnly);
   _scintView.execute(SCI_SETREADONLY, isReadOnly);
}

void tclFindResultDlg::saveSearchDoc() {
   if (0 == mSearchResultFile.size()) {
      // no saving configured
      return;
   }
   Utf8_16_Write UnicodeConvertor;
   UnicodeConvertor.setEncoding(uniUTF8);
   if (UnicodeConvertor.openFile(mSearchResultFile.c_str()))
   {
      char* buf = (char*) _scintView.execute(SCI_GETCHARACTERPOINTER); // vs. SCI_GETDOCPOINTER
      unsigned long lengthDoc = (unsigned long)_scintView.getCurrentDocLen(); // TODO check 64-bit issue
      if (lengthDoc && buf) {
         bool items_written = UnicodeConvertor.writeFile(buf, lengthDoc);
         UnicodeConvertor.closeFile();

         // Error, we didn't write the entire document to disk.
         // Note that fwrite() doesn't return the number of bytes written, but rather the number of ITEMS.
         if(!items_written)
         {
            MessageBox(NULL, TEXT("Problem in save"), TEXT("tclFindResultDlg::saveSearchDoc"), MB_OK);
         }
      }
   }
}
