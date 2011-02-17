/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011 Matthias H. mattesh(at)gmx.net

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
#include "stdafx.h"
#include "precompiledHeaders.h"

#include "FindDlg.h"
#include "Scintilla.h"
#include "SciLexer.h"
#include "MyPlugin.h"
#include "ScintillaSearchView.h"
#include "tclFindResultDoc.h"
#include "myDebug.h"
#include "tclFindResultDlg.h"

#define STYLING_MASK 255
#define FNDRESDLG_LINE_HEAD ""
#define FNDRESDLG_LINE_COLON ": "
#define FNDRESDLG_DEFAULT_STYLE 0 // style number for the default styling
#define FNDRESDLG_ACTIVATE_SEARCH 0x06

#ifdef UNICODE
#define filestat _wstat
#else
#define filestat _stat
#endif

tlpLinePosInfo tclFindResultDoc::mDefLineInfo = tlpLinePosInfo();

tclFindResultDlg::tclFindResultDlg() 
   : DockingDlgInterface(IDD_FIND_DLG_RESULT)
   , _markedLine(-1)
   , _lineCounter(0) 
   , miLineNumColSize(0)
   , miLineHeadSize(0)
   , mFontSize(8)
{};

tclFindResultDlg::~tclFindResultDlg() {
   _scintView.destroy();
}

void tclFindResultDlg::setParent(MyPlugin* parent) {
   _pParent = parent;
}

void tclFindResultDlg::setLineNumColSize(int size) {
   miLineNumColSize = size;
   miLineHeadSize = miLineNumColSize+ (int)(strlen(FNDRESDLG_LINE_COLON)+strlen(FNDRESDLG_LINE_HEAD));
}

int tclFindResultDlg::getLineNumColSize() const {
   return miLineNumColSize;
}

void tclFindResultDlg::initEdit() {
   _scintView.init(_hInst, _hSelf);
   _scintView.display();
	setFinderReadOnly(true);
   RECT rect;
   getClientRect(rect);
   _scintView.reSizeToWH(rect);
	_scintView.execute(SCI_SETCODEPAGE, SC_CP_UTF8);
   // let window parent (this class) do the styling
   _scintView.execute(SCI_SETLEXER,SCLEX_CONTAINER);
   mFindResultSearchDlg.init(_hInst, _hParent, &_scintView);
   mFindResultSearchDlg.create(IDD_FIND_RES_DLG_SEARCH);
   // TODO _scintView.showMargin(ScintillaSearchView::_SC_MARGE_LINENUMBER);
}

/** remove all that line being referecend by result not no other pattern */
void tclFindResultDlg::removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult) 
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
               _pParent->execute(scnMainHandle, SCI_MARKERDELETE, thisLine, MARK_BOOKMARK);
               
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
            char num[20];
            std::string s;
            tlmIdxPosInfo::const_iterator i = l.posInfos().begin();
            sprintf(num,"%3.10f",i->first);
            s.append(num);
            for(; i != l.posInfos().end(); ++i) {
               s.append(", ");
               sprintf(num,"%3.10f",i->first);
               s.append(num);
            }
            DBG1("removeUnusedResultLines() patterns are: %s.",s.c_str()); 
#endif
            int line = mFindResults.getLineNoAtRes(thisLine);
            int startL = (int)_scintView.execute(SCI_POSITIONFROMLINE, line);
            int endL = (int)_scintView.execute(SCI_GETLINEENDPOSITION, line);
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
   data->pszAddInfo = TEXT("");

   // supported features by plugin
   data->uMask	= (DWS_DF_CONT_BOTTOM | DWS_PARAMSALL);

   // store for live time
   _data = data;
}

#ifdef FEATURE_HEADLINE
void tclFindResultDlg::addFileNameTitle(const TCHAR *fileName) {
   // TODO make sure headline is not calcuated in resline conversion
   string str = "[";
   str += fileName;
   str += "]\n";

   setFinderReadOnly(false);
   _scintView.execute(SCI_APPENDTEXT, str.length(), (LPARAM)str.c_str());
   setFinderReadOnly(true);
   _lineCounter++;
}
#endif

int tclFindResultDlg::insertPosInfo(tPatId patternId, tiLine iResultLine, tclPosInfo pos) {
   return mFindResults.insertPosInfo(patternId, iResultLine, pos);
}

bool tclFindResultDlg::getLineAvail(tiLine foundLine) const {
   return mFindResults.getLineAvail(foundLine);
}

const std::string& tclFindResultDlg::getLineText(int iResultLine) {
   return mFindResults.getLineText(iResultLine);
}

void tclFindResultDlg::setLineText(int iFoundLine, const std::string& text) {
   bool bNewLine = mFindResults.setLineText(iFoundLine, text);
   // here we have to distinguish update and insert of lines in search result window
   int resLine = mFindResults.getLineNoAtRes(iFoundLine);
   //bool bVisible = mFindResults.getLineAtMain(iFoundLine).visible();
   int startPos = (int)_scintView.execute(SCI_POSITIONFROMLINE, resLine);
   if (startPos != -1) {
      setCurrentMarkedLine(-1);
      setFinderReadOnly(false);
      std::string s = FNDRESDLG_LINE_HEAD;
      char conv[20];
      s.append(miLineNumColSize-strlen(itoa(iFoundLine+1, conv, 10)), ' ');
      s.append(conv);
      s.append(FNDRESDLG_LINE_COLON);
      s.append(text);
      DBG3("setLineText() iFoundLine: %d resLine: %d text: \"%s\"", iFoundLine, resLine, s.c_str());
      if(bNewLine) {
         // adding a newlin into result -> set bookmark in main window
         _pParent->execute(scnMainHandle, SCI_MARKERADD, iFoundLine, MARK_BOOKMARK);
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
   unsigned iPat = FNDRESDLG_DEFAULT_STYLE;
   _scintView.execute(SCI_STYLESETFONT, iPat, (LPARAM)mFontName.c_str());
   _scintView.execute(SCI_STYLESETSIZE, iPat, (LPARAM)mFontSize);
   // copy styles into result window cache because while painting
   // user may have removed a pattern already
   for(unsigned iPat = 0; iPat < mPatStyleList.size(); ++iPat) 
   {
      const tclPattern& rPat = mPatStyleList.getPattern(mPatStyleList.getPatternId(iPat));
      _scintView.execute(SCI_STYLESETFONT, iPat+1, (LPARAM)mFontName.c_str());
      _scintView.execute(SCI_STYLESETSIZE, iPat+1, (LPARAM)mFontSize);
   } // for 
}

void tclFindResultDlg::updateWindowData(const std::string& fontName, unsigned fontSize) {
   mFontName = fontName;
   mFontSize = fontSize;
   setPatternFonts();
}

void tclFindResultDlg::clear() 
{
   setCurrentMarkedLine(-1);
   //_foundInfos.clear(); 
   mFindResults.clear();
   setFinderReadOnly(false);
   _pParent->execute(scnMainHandle, SCI_MARKERDELETEALL, MARK_BOOKMARK);
   _scintView.execute(SCI_CLEARALL);
   setFinderReadOnly(true);
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
   _pParent->execute(scnMainHandle, SCI_GOTOLINE, _markedLine);
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
   rtfColTbl += itoa(RTF_COL_R(defPat.getColorNum()),styleNum, 10);
   rtfColTbl += RTF_COLTAG_GREEN;
   rtfColTbl += itoa(RTF_COL_G(defPat.getColorNum()),styleNum, 10);
   rtfColTbl += RTF_COLTAG_BLUE;
   rtfColTbl += itoa(RTF_COL_B(defPat.getColorNum()),styleNum, 10);
   rtfColTbl += RTF_COLTAG_END;
   // copy styles into result window cache because while painting
   // user may have removed a pattern already
   for(unsigned iPat = 0; iPat < mPatStyleList.size(); ++iPat) 
   {
      const tclPattern& rPat = mPatStyleList.getPattern(mPatStyleList.getPatternId(iPat));
      _scintView.execute(SCI_STYLESETVISIBLE, iPat+1, !rPat.getIsHideText());
      _scintView.execute(SCI_STYLESETBOLD, iPat+1, rPat.getIsBold());
      _scintView.execute(SCI_STYLESETITALIC, iPat+1, rPat.getIsItalic());
      _scintView.execute(SCI_STYLESETUNDERLINE, iPat+1, rPat.getIsUnderlined());
      _scintView.execute(SCI_STYLESETFORE, iPat+1, rPat.getColorNum());
      _scintView.execute(SCI_STYLESETBACK, iPat+1, rPat.getBgColorNum());
      _scintView.execute(SCI_STYLESETEOLFILLED, iPat+1, (rPat.getSelectionType()==tclPattern::line));
      rtfColTbl += RTF_COLTAG_RED;
      rtfColTbl += itoa(RTF_COL_R(rPat.getColorNum()),styleNum, 10);
      rtfColTbl += RTF_COLTAG_GREEN;
      rtfColTbl += itoa(RTF_COL_G(rPat.getColorNum()),styleNum, 10);
      rtfColTbl += RTF_COLTAG_BLUE;
      rtfColTbl += itoa(RTF_COL_B(rPat.getColorNum()),styleNum, 10);
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

// message call back method
BOOL CALLBACK tclFindResultDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) 
   {
   case IDC_DO_CHECK_CONF: 
      {
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
               mFindResultSearchDlg.doDialog();
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
            vector<MenuItemUnit> tmp;
            //example tmp.push_back(MenuItemUnit(0, "Separator"));
            tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_COPY, TEXT("Copy")));
            tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SELECTALL, TEXT("Select All")));
            tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
            tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SEARCH, TEXT("Find...")));
            scintillaContextmenu.create(_scintView.getHSelf(), tmp);
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
         break;
      }

   case WM_NOTIFY:
      {
         // defer into extra function because of reinterpret cast 
         if(false == notify(reinterpret_cast<SCNotification *>(lParam))){
            // notify didn't know about the notification send to parent
            return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
         }
         break;
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
   u += 1; // shift by 1 to allow 0 being default style
   DBG4("setStyle() pattern %f style %d begin %d length %d", iPatternId, u, iBeginPos, iLength);
   _scintView.execute(SCI_STARTSTYLING, iBeginPos, 0xFF);
   _scintView.execute(SCI_SETSTYLING, iLength, u);
}

/** 
this function sends the series of commands to 
scintilla win with the corresponding style id 
**/
void tclFindResultDlg::setDefaultStyle(int iBeginPos, int iLength)
{
   DBG2("setDefaultStyle() begin %d length %d", iBeginPos, iLength);
   _scintView.execute(SCI_STARTSTYLING, iBeginPos, 0xFF);
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
   int charsHidden = 0;
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
         if(iLength <= miLineHeadSize) {
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
               setStyle(iPosInfo->first, styleBegin+miLineHeadSize, iLength-miLineHeadSize); // until end of line
            } else {
               // style per found positions
               // iPosInfo calculates on mainWin text
               tlsPosInfo::const_iterator iFoundPos = iPosInfo->second.begin();
               for(; iFoundPos != iPosInfo->second.end(); ++iFoundPos) {
                  int iPosInfoLength = (iFoundPos->end - iFoundPos->start);
                  int iPosLineBegin = iFoundPos->start - (int)_pParent->execute(scnMainHandle,SCI_POSITIONFROMLINE,iFoundPos->line);
                  if((iPosInfoLength>0) && (iPosLineBegin>=0) &&
                     ((styleBegin+miLineHeadSize+iPosLineBegin) >= styleBegin) &&
                     (iPosInfoLength <= (endOfLine-styleBegin))) {
                     // do styling 
                     setStyle(iPosInfo->first, styleBegin+miLineHeadSize+iPosLineBegin, iPosInfoLength);
                  } else {
                     DBG4("doStyle() ERROR word styling pos illegal pos.start %d styleBegin %d pos.end %d endOfLine %d.",
                        styleBegin+miLineHeadSize+iPosLineBegin, styleBegin, 
                        styleBegin+miLineHeadSize+iPosLineBegin+iPosInfoLength, endOfLine);
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
   
   DBG2("notify() code %x line %x.", notification->nmhdr.code, notification->line);

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
         try {
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
            /*std::*/ generic_string fullPath = _pParent->getSearchFileName();

            // get currently marked line in result doc
            int resLineNo = (int)_scintView.execute(SCI_LINEFROMPOSITION, currentPos);
            if(resLineNo >= _scintView.execute(SCI_GETLINECOUNT)) {
               // we are out of editable range. don't do anything
               return TRUE;
            }
            const tlpLinePosInfo& lineInfo = mFindResults.getLineAtRes(resLineNo);
            int lineMain = (int)lineInfo.first;
            int startMain = (int)_pParent->execute(scnMainHandle, SCI_POSITIONFROMLINE, lineMain, 0);

            //int cmd = NPPM_SWITCHTOFILE;//getMode()==FILES_IN_DIR?WM_DOOPEN:NPPM_SWITCHTOFILE;
            int iSuccess = (int)_pParent->execute(nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)fullPath.c_str());
            if (iSuccess == 0) {
               struct _stat st;
               iSuccess = (filestat(fullPath.c_str(),&st))?0:1; // check if file exist -> ret == 0
               // try to open it
               if (iSuccess != 0) {
                  iSuccess = (int)_pParent->execute(nppHandle, NPPM_DOOPEN, 0, (LPARAM)fullPath.c_str());
               }
            }
            if(iSuccess == 0) {
               ::MessageBox(0, TEXT("File could not be opened, were this search result comes from. \nRestart search with [Search]"), TEXT("Analyse Plugin - Sorry"), 0);
            } else {
               _pParent->displaySectionCentered(startMain, startMain);
               // we set the current mark here
               setCurrentMarkedLine(lineMain);
            }

            char line[1000]="";
            wcstombs(line, fullPath.c_str(),1000);
            DBG3("notify(SCNotification) SCN_DOUBLECLICK to line  %d in %s line in result", 
               lineMain, line, resLineNo);
   
            ret = true;

         } catch(...){
            DBG0("notify(SCNotification) ERROR SCN_DOUBLECLICK problem.");
            //printStr(TEXT("SCN_DOUBLECLICK problem"));
         }
         break;
      }
   case SCN_PAINTED:
   case SCN_MODIFIED:
   case SCN_UPDATEUI:
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
