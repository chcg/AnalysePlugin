/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011 Matthias H. mattesh(at)gmx.net
partly copied from the NotePad++ project from 
Don HO donho(at)altern.org 

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
#include "stdafx.h"
#include "precompiledHeaders.h"

#include "AnalysePlugin.h"
#include "SciLexer.h"
#include "tclFindResultDoc.h"
#include "chardefines.h"
#include "PleaseWaitDlg.h"

AnalysePlugin g_plugin;

const TCHAR AnalysePlugin::PLUGIN_NAME[] = TEXT("AnalysePlugin");
const TCHAR AnalysePlugin::ADDITIONALINFO[] = TEXT("Analyse additional info");
const TCHAR AnalysePlugin::KEYNAME[] = TEXT("doAnalyse");
const TCHAR AnalysePlugin::KEYSEARCHHISTORY[] = TEXT("searchHistory");
const TCHAR AnalysePlugin::KEYCOMMENTHISTORY[] = TEXT("commentHistory");
const TCHAR AnalysePlugin::KEYDEFAULTOPTIONS[] = TEXT("defaultOptions");
const TCHAR AnalysePlugin::KEYONAUTOUPDATE[] = TEXT("onAutoUpdate");
const TCHAR AnalysePlugin::KEYUSEBOOKMARK[] = TEXT("useBookmark");
const TCHAR AnalysePlugin::KEYONENTERACTION[] = TEXT("onEnterAction");
const TCHAR AnalysePlugin::KEYLASTFILENAME[] = TEXT("lastFile");
const TCHAR AnalysePlugin::KEYFONTNAME[] = TEXT("resultFontName");
const TCHAR AnalysePlugin::KEYFONTSIZE[] = TEXT("resultFontSize");
const TCHAR AnalysePlugin::SECTIONNAME[] = TEXT("Analyse Plugin");
const TCHAR AnalysePlugin::LOCALCONFFILE[] = TEXT("doLocalConf.xml");
const TCHAR AnalysePlugin::ANALYSE_INIFILE[] = TEXT("AnalysePlugin.ini");

// +++++++++++++ plugin dll interface ++++++++++++++++++++++++++++++++++++++ 

BOOL APIENTRY DllMain( HANDLE hModule, 
                      DWORD  reasonForCall, 
                      LPVOID lpReserved ) {
   return g_plugin.dllmain(hModule, reasonForCall, lpReserved);
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData) {
   g_plugin.setInfo(notpadPlusData);
}

extern "C" __declspec(dllexport) const TCHAR * getName() {
   return g_plugin.getName();
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF) {
   return g_plugin.getFuncsArray(nbF);
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notification) {
   g_plugin.beNotified(notification);
}

// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) {
   return g_plugin.messageProc(Message, wParam, lParam);
}

// +++++++++++++ plugin implementation ++++++++++++++++++++++++++++++++++++++ 

// the function needs to be static when called by notepad++
void MenuAnalyseToggle () {
   g_plugin.toggleShowFindDlg();
}
void MenuShowHelpDialog () {
   g_plugin.showHelpDialog();
}
void MenuShowConfigDialog () {
   g_plugin.showConfigDialog();
}

BOOL AnalysePlugin::dllmain(HANDLE hModule, 
                            DWORD  reasonForCall, 
                            LPVOID lpReserved )
{

   switch (reasonForCall)
   {
   case DLL_PROCESS_ATTACH:
      {   
         DBG0("DLL_PROCESS_ATTACH");
         _hModule = (HINSTANCE)hModule;
         // init all to zero
         memset(funcItem, 0, sizeof(funcItem));
         funcItem[SHOWFINDDLG]._pFunc = MenuAnalyseToggle;
         funcItem[SHOWHELPDLG]._pFunc = MenuShowHelpDialog;
#ifdef CONFIG_DIALOG
         funcItem[SHOWCNFGDLG]._pFunc = MenuShowConfigDialog;
#endif
         ::LoadString((HINSTANCE)_hModule, IDS_SHOW_ANALYSE_DIAG, funcItem[SHOWFINDDLG]._itemName, nbChar);
         ::LoadString((HINSTANCE)_hModule, IDS_SHOW_ANALYSE_HELP, funcItem[SHOWHELPDLG]._itemName, nbChar);
#ifdef CONFIG_DIALOG
         ::LoadString((HINSTANCE)_hModule, IDS_SHOW_ANALYSE_CONFIG, funcItem[SHOWCNFGDLG]._itemName, nbChar);
#endif
         // Shortcut :
         // Following code makes the first command
         // bind to the shortcut Ctrl-Alt-F
         // TODO doesn't work jet funcItem[SHOWFINDDLG]._pShKey = new MyShortcutKey(0x46, true, true, false); //VK_F
         //funcItem[INSERTLONGDATETIME]._pShKey = new MyShortcutKey(0x51, false, true, false); //VK_Q
         // may be to check NPPM_MODELESSDIALOG
         GetModuleFileName((HMODULE)_hModule, _szPluginFileName, COUNTCHAR(_szPluginFileName));
      }
      break;

   case DLL_PROCESS_DETACH:
      {
         DBG0("DLL_PROCESS_DETACH");
         if (g_TBSearchInFiles.hToolbarBmp)
            ::DeleteObject(g_TBSearchInFiles.hToolbarBmp);
         for( int i = 0; i < LAST_PLUGINFUNCID; i++) {
            if (funcItem[i]._pShKey) {
               delete funcItem[i]._pShKey;
               funcItem[i]._pShKey =0;
            }
         }
      }
      break;

   case DLL_THREAD_ATTACH:
      break;

   case DLL_THREAD_DETACH:
      break;
   }

   return TRUE;
}

void AnalysePlugin::loadSettings() 
{
   DBG0("loadSettings()");
   TCHAR nppPath[MAX_PATH];
   generic_strncpy(nppPath, _szPluginFileName, MAX_PATH);
   // remove the module name : get plugins directory path
   PathRemoveFileSpec(nppPath);

   // cd .. : get npp executable path
   PathRemoveFileSpec(nppPath);

   // Make localConf.xml path
   TCHAR localConfPath[MAX_PATH];
   generic_strncpy(localConfPath, nppPath, MAX_PATH);
   PathAppend(localConfPath, localConfFile);

   // Test if localConf.xml exist
   bool isLocal = (PathFileExists(localConfPath) == TRUE);

   if (isLocal) 
   {
      generic_strncpy(iniFilePath, nppPath, MAX_PATH);
      PathAppend(iniFilePath, TEXT("plugins\\config\\AnalysePlugin.ini"));
   }
   else 
   {
      ITEMIDLIST *pidl;
      SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
      SHGetPathFromIDList(pidl, iniFilePath);
      PathAppend(iniFilePath, TEXT("NotePad++\\plugins\\config\\AnalysePlugin.ini"));
   }
   TCHAR tmp[MAX_CHAR_CELL];
   gbPluginVisible = (0 != ::GetPrivateProfileInt(SECTIONNAME, KEYNAME, 0, iniFilePath));
   funcItem[SHOWFINDDLG]._init2Check = gbPluginVisible;  
   // the information is inverted here because ShowFindDialog() will invert it again
   ::GetPrivateProfileString(SECTIONNAME, KEYLASTFILENAME, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mSearchPatternFileName = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYSEARCHHISTORY, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mSearchHistory = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYCOMMENTHISTORY, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mCommentHistory = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYDEFAULTOPTIONS, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mDefaultOptions = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYUSEBOOKMARK, TEXT("1"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setUseBookmark(generic_atoi(tmp));
   _findResult.setUseBookmark(generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYONAUTOUPDATE, TEXT("0"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setOnAutoUpdate(generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYONENTERACTION, TEXT("0"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setOnEnterAction((teOnEnterAction)generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYFONTNAME, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setFontText(generic_string(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYFONTSIZE, TEXT("8"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setFontSize(generic_atoi(tmp));
}

void AnalysePlugin::saveSettings() {
   DBG0("saveSettings()");
   _findDlg.getSearchHistory(mSearchHistory);
   _findDlg.getCommentHistory(mCommentHistory);
   _findDlg.getDefaultOptions(mDefaultOptions);
   ::WritePrivateProfileString(SECTIONNAME, KEYNAME, (funcItem[SHOWFINDDLG]._init2Check?TEXT("1"):TEXT("0")), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYLASTFILENAME, _findDlg.getFileName(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYSEARCHHISTORY, mSearchHistory.c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYCOMMENTHISTORY, mCommentHistory.c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYDEFAULTOPTIONS, mDefaultOptions.c_str(), iniFilePath);
   TCHAR tmp[10];
   generic_itoa(_configDlg.getUseBookmark(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYUSEBOOKMARK, tmp, iniFilePath);
   generic_itoa(_configDlg.getOnAutoUpdate(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYONAUTOUPDATE, tmp, iniFilePath);
   generic_itoa((int)_configDlg.getOnEnterAction(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYONENTERACTION, tmp, iniFilePath);
   const TCHAR* cp = _configDlg.getFontText().c_str();
   ::WritePrivateProfileString(SECTIONNAME, KEYFONTNAME, cp, iniFilePath);
   generic_itoa(_configDlg.getFontSize(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYFONTSIZE, tmp, iniFilePath);
}

void AnalysePlugin::showMargin(int witchMarge, bool willBeShown) {
   int pixelWidth;
   if (witchMarge == ScintillaSearchView::_SC_MARGE_LINENUMBER) {
      int chWidth = (int)execute(scnSecondHandle, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"8");
      // The 4 here allows for spacing: 1 pixel on left and 3 on right.
      pixelWidth = (willBeShown)?(4 + ScintillaSearchView::_MARGE_LINENUMBER_NB_CHIFFRE * chWidth):0;
   } else {
      pixelWidth = willBeShown?14:0;
   }
   execute(scnSecondHandle, SCI_SETMARGINWIDTHN, witchMarge, pixelWidth);
}

void AnalysePlugin::displaySectionCentered(int posStart, int posEnd, bool isDownwards)
{
   // to make sure the found result is visible
   //When searching up, the beginning of the (possible multiline) result is important, when scrolling down the end
   int testPos = (isDownwards)?posEnd:posStart;
   execute(scnActiveHandle, SCI_SETCURRENTPOS, testPos);
   int currentlineNumberDoc = (int)execute(scnActiveHandle, SCI_LINEFROMPOSITION, testPos);
   int currentlineNumberVis = (int)execute(scnActiveHandle, SCI_VISIBLEFROMDOCLINE, currentlineNumberDoc);
   execute(scnActiveHandle, SCI_ENSUREVISIBLE, currentlineNumberDoc);	// make sure target line is unfolded

   int firstVisibleLineVis =	(int)execute(scnActiveHandle, SCI_GETFIRSTVISIBLELINE);
   int linesVisible =			(int)execute(scnActiveHandle, SCI_LINESONSCREEN) - 1;	//-1 for the scrollbar
   int lastVisibleLineVis =	(int)linesVisible + firstVisibleLineVis;

   //if out of view vertically, scroll line into (center of) view
   int linesToScroll = 0;
   if (currentlineNumberVis < firstVisibleLineVis)
   {
      linesToScroll = currentlineNumberVis - firstVisibleLineVis;
      //use center
      linesToScroll -= linesVisible/2;		
   }
   else if (currentlineNumberVis > lastVisibleLineVis)
   {
      linesToScroll = currentlineNumberVis - lastVisibleLineVis;
      //use center
      linesToScroll += linesVisible/2;
   }
	execute(scnActiveHandle, SCI_LINESCROLL, 0, linesToScroll);

   //Make sure the caret is visible, scroll horizontally (this will also fix wrapping problems)
   execute(scnActiveHandle, SCI_GOTOPOS, posStart);
   execute(scnActiveHandle, SCI_GOTOPOS, posEnd);
   //execute(scnActiveHandle, SCI_SETSEL, start, posEnd);	
   //execute(scnActiveHandle, SCI_SETCURRENTPOS, posEnd);
   execute(scnActiveHandle, SCI_SETANCHOR, posStart);	
}

void AnalysePlugin::setSearchFileName(const generic_string& file) {
   if(_findDlg.getFileName() != file) {
      _findDlg.setFileName(file.c_str());
      _findResult.clear();
   }
}

void AnalysePlugin::removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult) 
{
   _findResult.removeUnusedResultLines(pattId, oldResult, newResult);
}

void AnalysePlugin::clearResult() 
{
   _findResult.clear();
}

void AnalysePlugin::moveResult(tPatId oldPattId, tPatId newPattId)
{  // this function is expected to be called from find dialog but may be called
   // from somewhere too; therefore we inform both windows about this action
   DBG2("moveResult(old, new) %f %f", oldPattId, newPattId);
   _findDlg.moveResult(oldPattId, newPattId);
   _findResult.moveResult(oldPattId, newPattId);
   updateStyles();
   _findResult.updateDockingDlg();
}

bool AnalysePlugin::bCheckLastFileNameSame(generic_string& file) {
   //static TCHAR lastFileName[MAX_PATH] = "";
   TCHAR newFilename[MAX_PATH] = TEXT("");
   TCHAR newDirname[MAX_PATH] = TEXT("");
   execute(nppHandle, NPPM_GETFILENAME, COUNTCHAR(newFilename), (LPARAM)newFilename);
   execute(nppHandle, NPPM_GETCURRENTDIRECTORY, COUNTCHAR(newDirname), (LPARAM)newDirname);
   file = newDirname;
   if(file.size()>0) {
     file += TEXT("\\");
   }
   file += newFilename;
   if(mLastSearchedFileName.compare(file)==0) 
   {
      return true;
   }
   return false;
}

BOOL AnalysePlugin::doSearch(tclResultList& resultList)
{
   DBG0("doSearch() started");
   BOOL bRes = TRUE;
   // set all styles actually used
   _findResult.setPatternStyles(_findDlg.getPatternList());
   // make sure result is shown when we are active
   if(isVisible()) {
      _findResult.display();
   }
   _findResult.setCodePage(execute(scnActiveHandle, SCI_GETCODEPAGE));
   // check whether searchwindow is the same as before
   bool bReSearch = false;
   generic_string currentfile;
   if (!bCheckLastFileNameSame(currentfile)){
      // not same file in editor. Remove content from results
      mLastSearchedFileName = currentfile;
      bReSearch = true;
   }
   // check if we have the correct linenumcolumnsize
   int iNumLines = (int)execute(scnActiveHandle, SCI_GETLINECOUNT, 0, (LPARAM)0);
   // easy way of int(log10(iNumLines))
   int iLineNumColSize = (iNumLines<10)?1:
                         (iNumLines<100)?2:
                         (iNumLines<1000)?3:
                         (iNumLines<10000)?4:
                         (iNumLines<100000)?5:
                         (iNumLines<1000000)?6:
                         (iNumLines<10000000)?7:
                         (iNumLines<100000000)?8:
                         (iNumLines<1000000000)?9:10;
   if(_findResult.getLineNumColSize() != iLineNumColSize){
      // when chaning the column size we have to refill the text
      _findResult.setLineNumColSize(iLineNumColSize);
      bReSearch = true;
      DBG0("doSearch(): re search because iLineNumColSize changed");
   }
   // if all are not dirty then we redo the search
   if(resultList.getIsDirty()==false) {
      bReSearch = true;
      DBG0("doSearch(): re search because all are same");
   }
   // did on option flag research, then ... 
   if (bReSearch) {
      _findResult.clear();
      _findDlg.setAllDirty();
   }
   // create the please wait message box
   _FindProcessCancelled = false;

   // activate progress controls
   _findDlg.setPleaseWaitRange(0, resultList.size());
   _findDlg.activatePleaseWait();

   // for all patterns in the list test if result is dirty
   tclResultList::iterator iResult = resultList.begin();
   int iPatIndex = 1;
   for (; iResult != resultList.end(); ++iResult, ++iPatIndex) 
   {
      tclResult & result = iResult.refResult();
      // if result is dirty, start the search of the given pattern
      if(result.getIsDirty()==false) {
         continue; // next pattern
      }
      // find the pattern
      unsigned u=0;
      tclResult oldResult = result;
      result.clear();
      const tclPattern& pattern = resultList.getPattern(iResult.getPatId());
      // update please wait controls
      _findDlg.setPleaseWaitProgress(iPatIndex);

      if(u = doFindPattern(pattern, result)){
         DBG1("doSearch() %d items found. Update result window.", u);
         _findResult.reserve(u);
         _findResult.removeUnusedResultLines(iResult.getPatId(), oldResult, result);
         tclResult::tlvPosInfo::const_iterator it = result.getPositions().begin();
         int erasedLen = 0;
         int lcount = (int)execute(scnActiveHandle, SCI_GETLINECOUNT);
         for (;it!=result.getPositions().end();++it) {
            if(it->line >= lcount) {
               DBG4("doSearch() ERROR line is out of range! possible %d, line %d, start %d, end %d.",
                  lcount, it->line, it->start, it->end);
               continue;
            }
            int resultLine = _findResult.insertPosInfo(iResult.getPatId(), it->line, *it);
            int lend = (int)execute(scnActiveHandle, SCI_GETLINEENDPOSITION, it->line);
            int lstart = (int)execute(scnActiveHandle, SCI_POSITIONFROMLINE, it->line);
            int lineLength = lend - lstart; // formerly nbChar
            if(!_findResult.getLineAvail(it->line)) {
               if (_line==0 || ((int)_maxNbCharAllocated < lineLength))	//line longer than buffer, resize buffer
               {
                  _maxNbCharAllocated = lineLength;
                  delete [] _line;
                  _line = new char[_maxNbCharAllocated + 3];

               }
               execute(scnActiveHandle, SCI_GETLINE, it->line, (LPARAM)_line);
               _line[lineLength] = 0x0D;
               _line[lineLength+1] = 0x0A;
               _line[lineLength+2] = '\0';
               _findResult.setLineText(it->line, _line);
            } else {
               // line is already in search result
               DBG0("doSearch() line is already in search result");
            }
         }
      } else {
         _findResult.removeUnusedResultLines(iResult.getPatId(), oldResult, result);
      }
      if (_FindProcessCancelled) {
         DBG1("doSearch(_FindProcessCancelled) cancelled at pattern %d", iPatIndex );
         break;
      }
      if(_findDlg.getPleaseWaitCanceled()) {
         DBG1("doSearch(APN_MSG_CANCEL_FIND) cancelled at pattern %d", iPatIndex );
         break;
      }

      _findResult.updateDockingDlg();

      // we could store the the search range to the result too so that we  
      //    later can check whether update of search is required when text
      //    becomes appended
   } // for patterns
   _findDlg.activatePleaseWait(false);
   return bRes;
}

teOnEnterAction AnalysePlugin::getOnEnterAction() const {
   return _configDlg.getOnEnterAction();
}
int AnalysePlugin::getUseBookmark() const {
   return _configDlg.getUseBookmark();
}

const generic_string& AnalysePlugin::getResultFontName() const {
   return _configDlg.getFontText();
}

unsigned AnalysePlugin::getResultFontSize() const {
   return _configDlg.getFontSize();
}

void AnalysePlugin::beNotified(SCNotification *notification)
{
   if(notification==0) return;

   //DBG2("beNotified() 0x%04x \t %d", notification->nmhdr.code, notification->nmhdr.code);

   switch (notification->nmhdr.code) 
   {
   case NPPN_FILEBEFORELOAD:
      {
         DBG0("beNotified() NPPN_FILEBEFORELOAD _bIgnoreBufferModify = true");
         _bIgnoreBufferModify = true;
         break;
      }
   case SCN_SAVEPOINTLEFT:DBG0("beNotified() SCN_SAVEPOINTLEFT");break;
   case SCN_SAVEPOINTREACHED:DBG0("beNotified() SCN_SAVEPOINTREACHED");break;
   case NPPN_FILEBEFOREOPEN:DBG0("beNotified() NPPN_FILEBEFOREOPEN");break;
   case NPPN_FILEOPENED:DBG0("beNotified() NPPN_FILEOPENED");break;
   case NPPN_BUFFERACTIVATED:DBG0("beNotified() NPPN_BUFFERACTIVATED");break;
   case SCN_UPDATEUI:
      {
         if(_bIgnoreBufferModify) {
            DBG0("beNotified() SCN_UPDATEUI _bIgnoreBufferModify = false");
            _bIgnoreBufferModify = false;
         }
         break;
      }
#if 1 
      // diese nachricht ist hier nicht angekommen trotz
      //::SendMessage(hwnd, SCI_SETLEXER, SCLEX_CONTAINER , 0 );
   case SCN_STYLENEEDED:
      {
         DBG0("SCN_STYLENEEDED");
         // make sure the text is drawn with our style
         teNppWindows currentEdit;
         ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
         HWND hCurrentEditView = getCurrentHScintilla(currentEdit); // the enum is synchronized for 0 and 1	
         int startPos = (int)::SendMessage(hCurrentEditView, SCI_GETENDSTYLED, 0, 0);
         int lineNumber = (int)::SendMessage(hCurrentEditView, SCI_LINEFROMPOSITION, startPos, 0);
         startPos = (int)::SendMessage(hCurrentEditView, SCI_POSITIONFROMLINE, lineNumber, 0);
         doStyleFormating(hCurrentEditView, startPos, notification->position);

      }
      break;
#endif // 0
   case SCN_MODIFIED:
      {
         if((notification->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))!= 0) {
            if (notification->length < 100) {
            DBG4("AnalysePlugin: SCN_MODIFIED(text) linesAdded %d, position %d, length %d, text '%s'",
                  notification->linesAdded,
                  notification->position,
                  notification->length,
                  notification->text);
            } else {
            DBG3("AnalysePlugin: SCN_MODIFIED(text) linesAdded %d, position %d, length %d, text >100 chars",
                  notification->linesAdded,
                  notification->position,
                  notification->length);
            }
            if (!_bIgnoreBufferModify && _findDlg.isVisible() && _configDlg.getOnAutoUpdate()) {
               generic_string currentfile;
               if(bCheckLastFileNameSame(currentfile)){
                  // set the modification flag which will become activated after timer has elapsed
                  DBG0("AnalysePlugin: SCN_MODIFIED(text) setting SetModified()");
                  _findDlg.SetModified();
               } else {
                  char name[MAX_PATH];
#ifdef UNICODE
                  wcstombs(name, currentfile.c_str(),MAX_PATH);
#else
                  strcpy(name, currentfile.c_str());
#endif
                  DBG1("AnalysePlugin: SCN_MODIFIED(text) for different file %s", name);
               }
            }
         }
         break;
      }
   case NPPN_READONLYCHANGED:
      {
         DBG0("AnalysePlugin: NPPN_READONLYCHANGED");
         break;
      }

   case NPPN_TBMODIFICATION:
      {
         DBG0("NPPN_TBMODIFICATION");
         g_TBSearchInFiles.hToolbarBmp = (HBITMAP)::LoadImage(_findDlg.getHinst(), MAKEINTRESOURCE(IDB_TB_ANALYSE), IMAGE_BITMAP, 0, 0, (LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS));
         ::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON, (WPARAM)funcItem[SHOWFINDDLG]._cmdID, (LPARAM)&g_TBSearchInFiles);
         break;
      }

   case NPPN_READY:
      {
         DBG0("NPPN_READY");
         _nppReady = true;
         loadSettings();

			::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[SHOWFINDDLG]._cmdID, (LPARAM)gbPluginVisible);
         showFindDlg();
//         _findResult.updateWindowData(_configDlg.getFontText(), _configDlg.getFontSize());
         ::SetFocus(nppData._scintillaMainHandle);
         break;
      }
   case NPPN_SHUTDOWN:
      {
         DBG0("NPPN_SHUTDOWN");
         saveSettings();
         break;
      }
   default:
      break;
   } // switch (notification->nmhdr.code)
}

generic_string AnalysePlugin::convertExtendedToString(const generic_string& query) {	
	int i = 0, j = 0;
   int length = (int)query.size();
	int charLeft = length;
	bool isGood = true;
	TCHAR current;
   generic_string result;
   result.reserve(length);
	while(i < length) {	//because the backslash escape quences always reduce the size of the generic_string, no overflow checks have to be made for target, assuming parameters are correct
		current = query[i];
		charLeft--;
		if (current == '\\' && charLeft) {	//possible escape sequence
			i++;
			charLeft--;
			current = query[i];
			switch(current) {
				case 'r':
					//result[j] = '\r';
					result += '\r';
					break;
				case 'n':
					result += '\n';
					break;
				case '0':
					result += (TCHAR)'\0';
					break;
				case 't':
					result += '\t';
					break;
				case '\\':
					result += '\\';
					break;
				case 'b':
				case 'd':
				case 'o':
				case 'x':
				case 'u': {
					int size = 0, base = 0;
					if (current == 'b') {			//11111111
						size = 8, base = 2;
					} else if (current == 'o') {	//377
						size = 3, base = 8;
					} else if (current == 'd') {	//255
						size = 3, base = 10;
					} else if (current == 'x') {	//0xFF
						size = 2, base = 16;
					} else if (current == 'u') {	//0xCDCD
						size = 4, base = 16;
					}
					if (charLeft >= size) {
						int res = 0;
                  if (readBase(query,(i+1), &res, base, size)) {
							result += res;
							i+=size;
							break;
						}
					}
					//not enough chars to make parameter, use default method as fallback
					}
				default: {	//unknown sequence, treat as regular text
					result += '\\';
					j++;
					result += (int)current;
					isGood = false;
					break;
				}
			}
		} else {
			result += query[i];
		}
		i++;
		j++;
	}
	//result[j] = 0;
	//return j;
   return result;
}

bool AnalysePlugin::readBase(const generic_string& str, int curPos, int * value, int base, int size) {
	int i = 0, temp = 0;
	*value = 0;
	TCHAR max = '0' + (TCHAR)base - 1;
	TCHAR current;
	while(i < size) {
		current = str[i];
		if (current >= 'A') 
		{
			current &= 0xdf;
			current -= ('A' - '0' - 10);
		}
		else if (current > '9')
			return false;

		if (current >= '0' && current <= max) {
			temp *= base;
			temp += (current - '0');
		} else {
			return false;
		}
		i++;
	}
	*value = temp;
	return true;
}


int AnalysePlugin::doFindPattern(const tclPattern& pattern, tclResult& result)
{
   DBGW1("doFindPattern() %s", pattern.getSearchText().c_str());
   if(pattern.getDoSearch() == false) {
      DBG0("doFindPattern() don't search: mDoSearch==false.");
      result.clear();
      result.setDirty(false); // once through we mark the list as ready
      return 0;
   }
   // when finding an entry add it to the result and set the result to not dirty 
   // initial range definition
   int startRange = 0; // from very begin
   int endRange = (int)execute(scnActiveHandle, SCI_GETLENGTH);
   if (endRange < 1) {
      DBG0("doFindPattern() don't search: document is empty.");
      // nothing to do because that means the document is empty
      return 0; 
   } 
   // flags for the search 
   int flags =0;
   generic_string text;
   if(pattern.getSearchType()== tclPattern::regex) {
      flags |= (SCFIND_REGEXP|SCFIND_POSIX);
      text = pattern.getSearchText(); // text to be searched
   } else if(pattern.getSearchType()== tclPattern::escaped) {
      text = convertExtendedToString(pattern.getSearchText());
   } else {
      text = pattern.getSearchText(); // text to be searched
   }
   
   flags |= pattern.getIsMatchCase()?SCFIND_MATCHCASE:0;
   flags |= pattern.getIsWholeWord()?SCFIND_WHOLEWORD:0;

   //Initial range for searching
   execute(scnActiveHandle, SCI_SETTARGETSTART, startRange);
   execute(scnActiveHandle, SCI_SETTARGETEND, endRange);
   execute(scnActiveHandle, SCI_SETSEARCHFLAGS, flags);
   DBG2("doFindPattern() initial tstart %d, tend %d.", startRange, endRange);


   int targetStart = 0; // position of actual finding
   int targetEnd = 0;   // position of actual finding
   int nbProcessed = 0; // number of findings

   if(text.length()==0) {
      // empty string is found "every where" so we return directly with 0 
      DBG0("doFindPattern() don't search: empty search string.");
      result.setDirty(false); // once through we mark the list as ready
      return nbProcessed;
   }
#ifdef UNICODE
	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
	unsigned int cp = (unsigned int)execute(scnActiveHandle, SCI_GETCODEPAGE); 
	const char *text2FindA = wmc->wchar2char(text.c_str(), cp);
	size_t text2FindALen = strlen(text2FindA);
	targetStart = execute(scnActiveHandle, SCI_SEARCHINTARGET, 
      (WPARAM)text2FindALen, 
      (LPARAM)text2FindA);
#else
   targetStart = (int)execute(scnActiveHandle, SCI_SEARCHINTARGET, 
      (WPARAM)text.size(), 
      (LPARAM)text.c_str());
#endif
   while (targetStart >= 0) // something has been found
   {	
      if(_findDlg.getPleaseWaitCanceled()) {
         // please wait dialog indicates stopping
         DBG1("doFindPattern() cancelled! Return with %d results",nbProcessed);
         _FindProcessCancelled = true;
         return nbProcessed;
      }

      targetStart = (int)execute(scnActiveHandle, SCI_GETTARGETSTART);
      targetEnd = (int)execute(scnActiveHandle, SCI_GETTARGETEND);
      if (targetEnd > endRange) {	
         // we found a result but outside our range, therefore we do not process it
         // in fact that should not happen, because we set the range before
         break;
      }
      int foundTextLen = targetEnd - targetStart;

      int lineNumberStart = (int)execute(scnActiveHandle, SCI_LINEFROMPOSITION, targetStart);
      int lineNumberEnd = (int)execute(scnActiveHandle, SCI_LINEFROMPOSITION, targetEnd);
      int lineCount = lineNumberEnd - lineNumberStart;
      int thisLineIndex = 0;
      while (thisLineIndex <= lineCount) {
         DBG3("doFindPattern() found: start %d end %d line %d.", targetStart, targetEnd, lineNumberStart+thisLineIndex);
         result.push_back(targetStart, targetEnd, lineNumberStart+thisLineIndex/*, pLine*/);
         ++thisLineIndex;
      }
      startRange = targetStart + foundTextLen ;	//search from result onwards
      execute(scnActiveHandle, SCI_SETTARGETSTART, startRange);
      // end needs to be set because search did use it to signal found selection
      execute(scnActiveHandle, SCI_SETTARGETEND, endRange);
      //DBG2("doFindPattern() tstart %d, tend %d.", startRange, endRange);
      nbProcessed++;
      // do next search
#ifdef UNICODE
	   targetStart = execute(scnActiveHandle, SCI_SEARCHINTARGET, 
         (WPARAM)text2FindALen, 
         (LPARAM)text2FindA);
#else
      targetStart = (int)execute(scnActiveHandle, SCI_SEARCHINTARGET, 
         (WPARAM)text.size(), 
         (LPARAM)text.c_str());
#endif
   } // while
   if(targetStart == -2) {
      _findDlg.activatePleaseWait(false);
      generic_string serr = TEXT("Error in pattern [") + pattern.getSearchText() + TEXT("]");
      ::MessageBox(getCurrentHScintilla(scnActiveHandle), TEXT("Invalid regular expression") ,serr.c_str() , MB_ICONERROR | MB_OK);
   }
   result.setDirty(false); // once through we mark the list as ready
   if(nbProcessed == 0) {
      DBG0("doFindPattern() didn't find anything.");
   } else {
      DBG1("doFindPattern() found %d items.", nbProcessed);
   }
   return nbProcessed;
}

void AnalysePlugin::doStyleFormating(HWND hCurrentEditView, int startPos, int endPos) 
{
   // get the result list, including all positions 
   // get the pattern list including all the styles
   // apply to all positions in the given area the correct pattern
   ::SendMessage(hCurrentEditView, SCI_STARTSTYLING, 0, MY_STYLE_MASK ); //(int pos, int mask)
   ::SendMessage(hCurrentEditView, SCI_SETSTYLING, 100, 10);//foundTextLen, fs

}

void AnalysePlugin::toggleShowFindDlg () 
{
   //MessageBox(NULL, (funcItem[SHOWFINDDLG]._init2Check?"true":"false"), "toggle from", 0);
   funcItem[SHOWFINDDLG]._init2Check = !funcItem[SHOWFINDDLG]._init2Check;
   showFindDlg();
}

void AnalysePlugin::showHelpDialog() {
   execute(nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)funcItem[SHOWHELPDLG]._cmdID, (LPARAM)true);
   _helpDlg.doDialog(funcItem[SHOWHELPDLG]._cmdID);
}

void AnalysePlugin::showConfigDialog() {
#ifdef CONFIG_DIALOG
   execute(nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)funcItem[SHOWCNFGDLG]._cmdID, (LPARAM)true);
   _configDlg.doDialog(funcItem[SHOWCNFGDLG]._cmdID);
#endif
}

void AnalysePlugin::setInfo(NppData notpadPlusData) 
{
   nppData = notpadPlusData;
   _findDlg.init(_hModule, nppData);
   _configDlg.init(_hModule, nppData);
   _configDlg.setParent(this, &_findDlg, &_findResult);
   _findResult.init(_hModule, nppData._nppHandle);
   _helpDlg.init(_hModule, nppData);
}

HWND AnalysePlugin::getCurrentHScintilla(teNppWindows which) const
{  
   if(nppData._nppHandle==0) {
      return 0;	
   }
   int activView=0;
   switch(which) 
   {
   case scnMainHandle:
      return nppData._scintillaMainHandle;
   case scnSecondHandle:
      return nppData._scintillaSecondHandle;
   case nppHandle:
      return nppData._nppHandle;
   case scnActiveHandle:
      ::SendMessage(nppData._nppHandle, 
                    NPPM_GETCURRENTSCINTILLA,
                    (WPARAM)0,
                    (LPARAM)&activView);
      return (activView==0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
   default :
      return 0;
   } // switch
}

void AnalysePlugin::showFindDlg ()
{
   bool bVisible = funcItem[SHOWFINDDLG]._init2Check;

   if(_findDlg.isCreated() || bVisible) {
      //MessageBox(NULL, (bVisible?"true":"false"), "bVisible", 0);
      tTbData	data = {0};
      RECT rect={0,0,0,0};
      if (!_findDlg.isCreated())
      {
         _findDlg.create(&data);
         // use actual selected file as basis for loading config file
         if (mSearchPatternFileName.size()>0) {
            unsigned pos = (unsigned)mSearchPatternFileName.find_last_of('\\');
            if ( pos == 0 ) {
               pos = (unsigned)mSearchPatternFileName.find_last_of('/');
            }
            if( pos > 0 ) {
               generic_string s = mSearchPatternFileName.substr(0, pos+1); // we want to have the slash too
               s += TEXT("*.xml");
               _findDlg.setFileName(s.c_str());
            }else {
               _findDlg.setFileName(mSearchPatternFileName.c_str());
            }
         }
      
         ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
         
         //_findDlg.getWindowRect(rect);
         //_findDlg.updateDockingDlg
         //rect.right = 900;
         //::MoveWindow(_findDlg.getHSelf(), 0, 0, 100, 100, true);
      }
      if (!_findResult.isCreated())
      {
         ZeroMemory(&data, sizeof(data));
         _findResult.create(&data);
         //::GetWindowRect(_findDlg.getHSelf(), &rect);
         //::MoveWindow(_findDlg.getHSelf(), 0, 0, 100, 100, true);
         ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
         DBG0("showFindDlg() result window initEdit()");
         // set the diag 
         _findDlg.setSearchHistory(mSearchHistory.c_str());
         _findDlg.setCommentHistory(mCommentHistory.c_str());
         // let finddialog decode the string from the ini file
         _findDlg.setDefaultOptions(mDefaultOptions.c_str());
         // afterwards transfer the data to the config dialog
         _configDlg.setDialogData(_findDlg.getDefaultPattern());
         _findResult.initEdit(_findDlg.getDefaultPattern()); // here findresult first time exists
         _findResult.updateWindowData(_configDlg.getFontText(), _configDlg.getFontSize());
      }

      // Always show, unless npp is not ready yet
      if (_nppReady) {
         _findResult.display(bVisible);
         _findDlg.display(bVisible);
      }
      funcItem[SHOWFINDDLG]._init2Check = isVisible();
   }
}

void AnalysePlugin::visibleChanged(bool isVisible) 
{
   execute(nppHandle, NPPM_SETMENUITEMCHECK, funcItem[SHOWFINDDLG]._cmdID, (LPARAM)isVisible);
}

LRESULT AnalysePlugin::messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
   // Here you can process the Npp Messages 
   // I will make the messages accessible little by little, according to the need of plugin development.
   // Please let me know if you need to access to some messages :
   // http://sourceforge.net/forum/forum.php?forum_id=482781
   //
   /*
   if (Message == WM_MOVE)
   {
   ::MessageBox(NULL, "move", "", MB_OK);
   }
   */
   return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
#endif //UNICODE
