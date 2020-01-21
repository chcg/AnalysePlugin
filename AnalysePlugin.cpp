/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2020 Matthias H. mattesh(at)gmx.net
partly copied from the NotePad++ project from 
Don HO don.h(at)free.fr 

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
//#include "stdafx.h"
#include <windows.h>
#include "AnalysePlugin.h"
#include "tclFindResultDoc.h"
#include "chardefines.h"
#include "PleaseWaitDlg.h"
#define MDBG_COMP "APmain:" 
#include "myDebug.h"
#include "boostregexsearch.h"
// #include "ScintillaEditView.h"

AnalysePlugin g_plugin;

const TCHAR AnalysePlugin::PLUGIN_NAME[] = TEXT("AnalysePlugin");
const TCHAR AnalysePlugin::ADDITIONALINFO[] = TEXT("Analyse additional info");
const TCHAR AnalysePlugin::KEYNAME[] = TEXT("doAnalyse");
const TCHAR AnalysePlugin::KEYSHOWSEARCH[] = TEXT("showSearch");
const TCHAR AnalysePlugin::KEYSEARCHHISTORY[] = TEXT("searchHistory");
const TCHAR AnalysePlugin::KEYCOMMENTHISTORY[] = TEXT("commentHistory");
const TCHAR AnalysePlugin::KEYGROUPHISTORY[] = TEXT("groupHistory");
const TCHAR AnalysePlugin::KEYDEFAULTOPTIONS[] = TEXT("defaultOptions");
const TCHAR AnalysePlugin::KEYONAUTOUPDATE[] = TEXT("onAutoUpdate");
const TCHAR AnalysePlugin::KEYSYNCEDSCORLL[] = TEXT("syncedScrolling");
const TCHAR AnalysePlugin::KEYDBLCLKUMP2EDIT[] = TEXT("dblclkJumps2EditView");
const TCHAR AnalysePlugin::KEYUSEBOOKMARK[] = TEXT("useBookmark");
const TCHAR AnalysePlugin::KEYRESWINWORDWRAP[] = TEXT("resWinWordWrap");
const TCHAR AnalysePlugin::KEYDISPLAYLINENO[] = TEXT("displayLineNo");
const TCHAR AnalysePlugin::KEYONENTERACTION[] = TEXT("onEnterAction");
const TCHAR AnalysePlugin::KEYLASTFILENAME[] = TEXT("lastFile");
const TCHAR AnalysePlugin::KEYFONTNAME[] = TEXT("resultFontName");
const TCHAR AnalysePlugin::KEYFONTSIZE[] = TEXT("resultFontSize");
const TCHAR AnalysePlugin::KEYMAXNUMOFCFGFILES[] = TEXT("maxNumOfConfigFiles");
const TCHAR AnalysePlugin::KEYLASTSRCHCFGFILE[] = TEXT("lastSearchConfigFile");
const TCHAR AnalysePlugin::KEYNUMOFLASTCFGFILES[] = TEXT("numberOfLastConfigFiles");
const TCHAR AnalysePlugin::KEYCONFIGLISTCOLUMNS[] = TEXT("configListColumnWidths");
const TCHAR AnalysePlugin::KEYCONFIGLISTCOLORDER[] = TEXT("configListColumnOrder");
const TCHAR AnalysePlugin::KEYCUSTOMCOLORS[] = TEXT("customColors");
const TCHAR AnalysePlugin::SECTIONNAME[] = TEXT("Analyse Plugin");
const TCHAR AnalysePlugin::LOCALCONFFILE[] = TEXT("doLocalConf.xml");
const TCHAR AnalysePlugin::ANALYSE_INIFILE[] = TEXT("AnalysePlugin.ini");
COLORREF AnalysePlugin::_acrCustClr[NUM_CUSTOM_COLORS]; // array of custom colors 

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
void MenuAddSelectionToPatterns () {
   g_plugin.addSelectionToPatterns();
}
void MenuRunSearch() {
   g_plugin.runSearch();
}
void MenuShowHelpDialog () {
   g_plugin.showHelpDialog();
}
void MenuShowConfigDialog () {
   g_plugin.showConfigDialog();
}
void AnalysePlugin::showConfigDlg() {
   g_plugin.showConfigDialog();
}

BOOL AnalysePlugin::dllmain(HANDLE hModule, 
                            DWORD  reasonForCall, 
                            LPVOID /*lpReserved*/ )
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
         funcItem[ADDSELTOPATT]._pFunc = MenuAddSelectionToPatterns;
         funcItem[RUNSEARCH]._pFunc = MenuRunSearch;
#ifdef CONFIG_DIALOG
         funcItem[SHOWCNFGDLG]._pFunc = MenuShowConfigDialog;
#endif
         funcItem[SHOWHELPDLG]._pFunc = MenuShowHelpDialog;

         ::LoadString((HINSTANCE)_hModule, IDS_SHOW_ANALYSE_DIAG, funcItem[SHOWFINDDLG]._itemName, nbChar);
         ::LoadString((HINSTANCE)_hModule, IDS_ADDSELTOPATT, funcItem[ADDSELTOPATT]._itemName, nbChar);
         ::LoadString((HINSTANCE)_hModule, IDS_RUNSEARCH, funcItem[RUNSEARCH]._itemName, nbChar);
#ifdef CONFIG_DIALOG
         ::LoadString((HINSTANCE)_hModule, IDS_SHOW_ANALYSE_CONFIG, funcItem[SHOWCNFGDLG]._itemName, nbChar);
#endif
         ::LoadString((HINSTANCE)_hModule, IDS_SHOW_ANALYSE_HELP, funcItem[SHOWHELPDLG]._itemName, nbChar);
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

BOOL AnalysePlugin::FileExists(LPCTSTR szPath)
{
   DWORD dwAttrib = GetFileAttributes(szPath);

   return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
      !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
void AnalysePlugin::loadSettings() 
{
   DBG0("loadSettings()");
/*
check for NPP API path 
if not take old method
in case new path and old path are different and in new path ini is not there load once old path and save it in new path 
*/

   bool migration = false; // this feature will be removed after some versions
   TCHAR configBase[AP_MAX_PATH];
   configBase[0] = 0;
   TCHAR configBase2[AP_MAX_PATH];
   configBase2[0] = 0;
   TCHAR iniFilePath2[AP_MAX_PATH];
   iniFilePath2[0] = 0;
   TCHAR xmlFilePath2[AP_MAX_PATH];
   xmlFilePath2[0] = 0;
   if (execute(nppHandle, NPPM_GETPLUGINSCONFIGDIR)) {
      // new way loading since NPP version ...
      execute(nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)AP_MAX_PATH, (LPARAM)configBase2);
      generic_strncpy(iniFilePath2, configBase2, AP_MAX_PATH);
      PathAppend(iniFilePath2, TEXT("\\AnalysePlugin.ini"));
      generic_strncpy(xmlFilePath2, configBase2, AP_MAX_PATH);
      PathAppend(xmlFilePath2, TEXT("\\AnalysePlugin.xml"));
   }
   // old way of loading
   (NppParameters::getInstance()).load();
   bool isLocal = (NppParameters::getInstance()).isLocal();
   if (isLocal) {
      generic_string nppConfigPath = (NppParameters::getInstance()).getNppPath();
      generic_strncpy(configBase, nppConfigPath.c_str(), AP_MAX_PATH);
      PathAppend(configBase, TEXT("plugins\\Config"));
   }
   else
   {
      ITEMIDLIST* pidl;
      SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
      SHGetPathFromIDList(pidl, configBase);
      PathAppend(configBase, TEXT("NotePad++\\plugins\\Config"));
   }
   generic_strncpy(iniFilePath, configBase, AP_MAX_PATH);
   PathAppend(iniFilePath, TEXT("\\AnalysePlugin.ini"));
   generic_strncpy(xmlFilePath, configBase, AP_MAX_PATH);
   PathAppend(xmlFilePath, TEXT("\\AnalysePlugin.xml"));
   // migration support
   if (generic_strncmp(iniFilePath, iniFilePath2, AP_MAX_PATH) != 0) {
      if ((FileExists(iniFilePath)) && (!FileExists(iniFilePath2))) {
         // the paths are different, old file is there and in new place file is not there
         // use one time loading the data from old place
         if (CopyFile(iniFilePath, iniFilePath2, false)) {
            DeleteFile(iniFilePath);
         }
         if (generic_strncmp(xmlFilePath, xmlFilePath2, AP_MAX_PATH) != 0) {
            if ( (FileExists(xmlFilePath)) && (!FileExists(xmlFilePath2))) {
               if (CopyFile(xmlFilePath, xmlFilePath2, false)) {
                  DeleteFile(xmlFilePath);
               }
            }
            // we prefer the new path from API
            generic_strncpy(xmlFilePath, xmlFilePath2, AP_MAX_PATH);
         }
      }
      // we prefer the new path from API
      generic_strncpy(iniFilePath, iniFilePath2, AP_MAX_PATH);
   }

   TCHAR tmp[MAX_CHAR_CELL];
   gbPluginVisible = (0 != ::GetPrivateProfileInt(SECTIONNAME, KEYNAME, 0, iniFilePath));
   funcItem[SHOWFINDDLG]._init2Check = gbPluginVisible;  
   gbResultVisible = (0 != ::GetPrivateProfileInt(SECTIONNAME, KEYSHOWSEARCH, 1, iniFilePath));
   // the information is inverted here because ShowFindDialog() will invert it again
   ::GetPrivateProfileString(SECTIONNAME, KEYLASTFILENAME, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   setSearchFileName(tmp);
   ::GetPrivateProfileString(SECTIONNAME, KEYSEARCHHISTORY, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mSearchHistory = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYCOMMENTHISTORY, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mCommentHistory = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYGROUPHISTORY, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mGroupHistory = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYDEFAULTOPTIONS, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   mDefaultOptions = tmp;
   ::GetPrivateProfileString(SECTIONNAME, KEYUSEBOOKMARK, TEXT("1"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setUseBookmark(generic_atoi(tmp));
   _findResult.setUseBookmark(generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYRESWINWORDWRAP, TEXT("0"), tmp, COUNTCHAR(tmp), iniFilePath);
   _findResult.setWrapMode(generic_atoi(tmp) != 0);
   ::GetPrivateProfileString(SECTIONNAME, KEYDISPLAYLINENO, TEXT("1"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setDisplayLineNo(generic_atoi(tmp));
   _findResult.setDisplayLineNo(generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYONAUTOUPDATE, TEXT("0"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setOnAutoUpdate(generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYSYNCEDSCORLL, TEXT("1"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setIsSyncScroll(0 != generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYDBLCLKUMP2EDIT, TEXT("1"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setDblClickJumps2EditView(0 != generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYONENTERACTION, TEXT("0"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setOnEnterAction((teOnEnterAction)generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYFONTNAME, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setFontText(generic_string(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYFONTSIZE, TEXT("8"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setFontSize(generic_atoi(tmp));
   ::GetPrivateProfileString(SECTIONNAME, KEYCUSTOMCOLORS, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   if (generic_strlen(tmp) > 0) {
      setCustomColorsStr(tmp);
   }
   ::GetPrivateProfileString(SECTIONNAME, KEYMAXNUMOFCFGFILES, TEXT("4"), tmp, COUNTCHAR(tmp), iniFilePath);
   _configDlg.setNumOfCfgFilesStr(tmp);  
   _findDlg.setNumOfCfgFilesStr(tmp);
   int maxConfigFilesNum = generic_atoi(tmp);
   ::GetPrivateProfileString(SECTIONNAME, KEYNUMOFLASTCFGFILES, TEXT("0"), tmp, COUNTCHAR(tmp), iniFilePath);
   int i = generic_atoi(tmp);
   i = (i > maxConfigFilesNum) ? maxConfigFilesNum : i;
   for(--i; i > 0 ; --i) {
      TCHAR num[10];
      generic_string key = generic_string(KEYLASTSRCHCFGFILE) + generic_itoa(i, num, 10);
      ::GetPrivateProfileString(SECTIONNAME, key.c_str() , TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
      if(generic_strlen(tmp)){
         _findDlg.setConfigFileName(tmp);
      }
   }
   ::GetPrivateProfileString(SECTIONNAME, KEYCONFIGLISTCOLUMNS, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   _findDlg.setTableColumns(tmp);
   ::GetPrivateProfileString(SECTIONNAME, KEYCONFIGLISTCOLORDER, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   _findDlg.setTableColumnOrder(tmp);
   ::GetPrivateProfileString(SECTIONNAME, KEYLASTSRCHCFGFILE, TEXT(""), tmp, COUNTCHAR(tmp), iniFilePath);
   if(generic_strlen(tmp)){
      _findDlg.setConfigFileName(tmp);
   }

   // next is done after dialog is created
   //if(PathFileExists(xmlFilePath) == TRUE) {
   //   // load the patterns from last run
   //   _findDlg.loadConfigFile(xmlFilePath);
   //}

   HRSRC resourceHandle1 = ::FindResource(_hModule, MAKEINTRESOURCE(IDR_MANUAL), RT_HTML);
   HGLOBAL dataHandle1 = ::LoadResource(_hModule, resourceHandle1);
   const char* data1 = (const char*)::LockResource(dataHandle1);
   generic_string man = string2wstring(data1, CP_ACP);
   ::FreeResource(dataHandle1);
   HRSRC resourceHandle2 = ::FindResource(_hModule, MAKEINTRESOURCE(IDR_CHANGES), RT_HTML);
   HGLOBAL dataHandle2 = ::LoadResource(_hModule, resourceHandle2);
   const char* data2 = (const char*)::LockResource(dataHandle2);
   generic_string changes = string2wstring(data2, CP_ACP);
   ::FreeResource(dataHandle2);
   _helpDlg.setManText(man);
   _helpDlg.setChangesText(changes);
}

void AnalysePlugin::saveSettings() {
   DBG0("saveSettings()");
   _findDlg.getSearchHistory(mSearchHistory);
   _findDlg.getCommentHistory(mCommentHistory);
   _findDlg.getGroupHistory(mGroupHistory);
   _findDlg.getDefaultOptions(mDefaultOptions);
   ::WritePrivateProfileString(SECTIONNAME, KEYNAME, (_findDlg.isVisible()?TEXT("1"):TEXT("0")), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYSHOWSEARCH, (_findResult.isVisible()?TEXT("1"):TEXT("0")), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYLASTFILENAME, _findDlg.getFileName().c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYSEARCHHISTORY, mSearchHistory.c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYCOMMENTHISTORY, mCommentHistory.c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYGROUPHISTORY, mGroupHistory.c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYDEFAULTOPTIONS, mDefaultOptions.c_str(), iniFilePath);
   TCHAR tmp[10];
   generic_itoa(_configDlg.getUseBookmark(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYUSEBOOKMARK, tmp, iniFilePath);
   generic_itoa((_findResult.getWrapMode() ? 1 : 0), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYRESWINWORDWRAP, tmp, iniFilePath);
   generic_itoa(_configDlg.getDisplayLineNo(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYDISPLAYLINENO, tmp, iniFilePath);
   generic_itoa(_configDlg.getOnAutoUpdate(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYONAUTOUPDATE, tmp, iniFilePath);
   generic_itoa((_configDlg.getIsSyncScroll()?1:0), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYSYNCEDSCORLL, tmp, iniFilePath);
   generic_itoa((_configDlg.getDblClickJumps2EditView() ? 1 : 0), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYDBLCLKUMP2EDIT, tmp, iniFilePath);
   generic_itoa((int)_configDlg.getOnEnterAction(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYONENTERACTION, tmp, iniFilePath);
   const TCHAR* cp = _configDlg.getFontText().c_str();
   ::WritePrivateProfileString(SECTIONNAME, KEYFONTNAME, cp, iniFilePath);
   generic_itoa(_configDlg.getFontSize(), tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYFONTSIZE, tmp, iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYCUSTOMCOLORS, getCustomColorsStr().c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYMAXNUMOFCFGFILES, _configDlg.getNumOfCfgFilesStr().c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYCONFIGLISTCOLUMNS, _findDlg.getTableColumns().c_str(), iniFilePath);
   ::WritePrivateProfileString(SECTIONNAME, KEYCONFIGLISTCOLORDER, _findDlg.getTableColumnOrder().c_str(), iniFilePath);
   int maxFiles = static_cast<int>(_findDlg.getLastConfigFiles().size());
   generic_itoa(maxFiles, tmp, 10);
   ::WritePrivateProfileString(SECTIONNAME, KEYNUMOFLASTCFGFILES, tmp, iniFilePath);
   // write the first anyway 
   ::WritePrivateProfileString(SECTIONNAME, KEYLASTSRCHCFGFILE, ((maxFiles)?(_findDlg.getLastConfigFiles().at(0).c_str()):TEXT("")), iniFilePath);
   for(int i = 1; i < maxFiles; ++i) {
      TCHAR num[10];
      generic_string key = generic_string(KEYLASTSRCHCFGFILE) + generic_itoa(i, num, 10);
      ::WritePrivateProfileString(SECTIONNAME, key.c_str() , _findDlg.getLastConfigFiles().at(i).c_str(), iniFilePath);
   }
   if(xmlFilePath) {
      // store current settings in my self xml
      _findDlg.saveConfigFile(xmlFilePath);
   }
}

//void AnalysePlugin::showMargin(int witchMarge, bool willBeShown) {
//   int pixelWidth;
//   if (witchMarge == ScintillaSearchView::_SC_MARGE_LINENUMBER) {
//      int chWidth = (int)execute(scnSecondHandle, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"8");
//      // The 4 here allows for spacing: 1 pixel on left and 3 on right.
//      pixelWidth = (willBeShown)?(4 + 5 * chWidth):0; // Mattes ScintillaEditView::_MARGE_LINENUMBER_NB_CHIFFRE
//   } else {
//      pixelWidth = willBeShown?14:0;
//   }
//   execute(scnSecondHandle, SCI_SETMARGINWIDTHN, witchMarge, pixelWidth);
//}

void AnalysePlugin::displaySectionCentered(int posStart, int posEnd, bool isDownwards)
{
   // to make sure the found result is visible
   //When searching up, the beginning of the (possible multiline) result is important, when scrolling down the end
   int testPos = (isDownwards)?posEnd:posStart;
   execute(scnActiveHandle, SCI_SETCURRENTPOS, testPos);
   int currentlineNumberDoc = (int)execute(scnActiveHandle, SCI_LINEFROMPOSITION, testPos);
   int currentlineNumberVis = (int)execute(scnActiveHandle, SCI_VISIBLEFROMDOCLINE, currentlineNumberDoc);
   execute(scnActiveHandle, SCI_ENSUREVISIBLE, currentlineNumberDoc);   // make sure target line is unfolded

   int firstVisibleLineVis = (int)execute(scnActiveHandle, SCI_GETFIRSTVISIBLELINE);
   int linesVisible = (int)execute(scnActiveHandle, SCI_LINESONSCREEN) - 1;   //-1 for the scrollbar
   int lastVisibleLineVis = (int)linesVisible + firstVisibleLineVis;

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
   execute(scnActiveHandle, SCI_SETANCHOR, posStart);
}
int AnalysePlugin::getPatternIndex(tPatId id) const {
   return _findDlg.getPatternList().getPatternIndex(id);
}
generic_string AnalysePlugin::getPatternSearchText(tPatId id) const {
   return _findDlg.getPatternList().getPattern(id).getSearchText();
}
void AnalysePlugin::setSelectedPattern(int index) {
   _findDlg.setSelectedPattern(index);
}

void AnalysePlugin::setSearchFileName(const generic_string& file) {
   
   if(mLastSearchedFileName != file) {
      mLastSearchedFileName = file;
      _findDlg.setFileName(file);
      _findResult.setFileName(file);
      //don't do it here because window is not init() _findResult.clear();
   }
}
void AnalysePlugin::setDisplayLineNo(bool bOn) {
   _configDlg.setDisplayLineNo(bOn ? 1 : 0);
}

void AnalysePlugin::removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult) 
{
   _findResult.removeUnusedResultLines(pattId, oldResult, newResult);
}

void AnalysePlugin::clearResult(bool initial)
{
   setSearchFileName(TEXT(""));
   _findResult.clear(initial);
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
#ifdef FEATURE_RESVIEW_POS_KEEP_AT_SEARCH
   _findResult.saveCurrentViewPos();
#endif
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
      setSearchFileName(currentfile);
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
   unsigned commentWidth = resultList.getCommentWidth();

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
      tclResult oldResult = result;
      result.clear();
      const tclPattern& pattern = resultList.getPattern(iResult.getPatId());
      // update please wait controls
      _findDlg.setPleaseWaitProgress(iPatIndex);
       unsigned u = doFindPattern(pattern, result);

      if (u){
         DBG1("doSearch() %d items found. Update result window.", u);
         _findResult.reserve(u);
         _findResult.removeUnusedResultLines(iResult.getPatId(), oldResult, result);
         tclResult::tlvPosInfo::const_iterator it = result.getPositions().begin();
         //int erasedLen = 0;
         int lcount = (int)execute(scnActiveHandle, SCI_GETLINECOUNT);
         unsigned cp = (unsigned)execute(scnActiveHandle, SCI_GETCODEPAGE);
         WcharMbcsConvertor* wmc = &WcharMbcsConvertor::getInstance();
         std::string comment;
         if (wmc) {
            comment = wmc->wchar2char(pattern.getComment().c_str(), cp);
         }
         for (;it!=result.getPositions().end();++it) {
            if(it->line >= lcount) {
               DBG4("doSearch() ERROR line is out of range! possible %d, line %d, start %d, end %d.",
                  lcount, it->line, it->start, it->end);
               continue;
            }
            /*int resultLine =*/ _findResult.insertPosInfo(iResult.getPatId(), it->line, *it);
            int lend = (int)execute(scnActiveHandle, SCI_GETLINEENDPOSITION, it->line);
            int lstart = (int)execute(scnActiveHandle, SCI_POSITIONFROMLINE, it->line);
            int lineLength = lend - lstart; // formerly nbChar
            if(!_findResult.getLineAvail(it->line)) {
               if (_line==0 || ((int)_maxNbCharAllocated < lineLength))   //line longer than buffer, resize buffer
               {
                  _maxNbCharAllocated = lineLength;
                  delete [] _line;
                  _line = new char[_maxNbCharAllocated + 3];

               }
               execute(scnActiveHandle, SCI_GETLINE, it->line, (LPARAM)_line);
               for (int i = 0; i < lineLength; ++i) {
                  if (_line[i] == 0) { // ensure paradigma no zeros in strings
                     _line[i] = (char)0x20;
                  }
               }
               _line[lineLength] = 0x0D;
               _line[lineLength+1] = 0x0A;
               _line[lineLength+2] = '\0';
               _findResult.setLineText(it->line, _line, comment, commentWidth);
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
#ifdef FEATURE_RESVIEW_POS_KEEP_AT_SEARCH
   _findResult.restoreCurrentViewPos();
#endif
   // disable the jumping cursor when search runs
   //int iCurrFirstLineMain = (int)execute(scnActiveHandle, SCI_GETFIRSTVISIBLELINE);
   //int iThisLineToMove = _findResult.getNextFoundLine(iCurrFirstLineMain);
   //if (iThisLineToMove >= iCurrFirstLineMain) { // only move if search result line is valid
   //   _findResult.setCurrentMarkedLine(iThisLineToMove);
   //   _findResult.setCurrentViewPos(iThisLineToMove);
   //}
   _findDlg.activatePleaseWait(false);
//   mCurScnHandle = getCurrentHScintilla(scnActiveHandle);
// hier
//#define MARGIN_SCRIPT_FOLD_INDEX 1
//   ::SendMessage(mCurScnHandle, SCI_SETPROPERTY, (WPARAM)TEXT("fold"), (LPARAM)TEXT("1"));
//   ::SendMessage(mCurScnHandle, SCI_SETPROPERTY, (WPARAM)TEXT("fold.compact"), (LPARAM)TEXT("0"));
//   ::SendMessage(mCurScnHandle, SCI_SETMARGINWIDTHN, MARGIN_SCRIPT_FOLD_INDEX, 0);
//   ::SendMessage(mCurScnHandle, SCI_SETLEXER, SCLEX_CONTAINER, 0);
//   ::PostMessage(mCurScnHandle, SCI_COLOURISE, 0, -1);
   return bRes;
}

teOnEnterAction AnalysePlugin::getOnEnterAction() const {
   return _configDlg.getOnEnterAction();
}
int AnalysePlugin::getUseBookmark() const {
   return _configDlg.getUseBookmark();
}
bool AnalysePlugin::getResultWrapMode() const {
   return _findResult.getWrapMode();
}
void AnalysePlugin::setResultWrapMode(bool bOn) {
   _findResult.setWrapMode(bOn);
}

int AnalysePlugin::getDisplayLineNo() const {
   return _configDlg.getDisplayLineNo();
}
bool AnalysePlugin::getIsSyncScroll() const {
   return _configDlg.getIsSyncScroll();
}
bool AnalysePlugin::getDblClickJumps2EditView() const {
   return _configDlg.getDblClickJumps2EditView();
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
   case NPPN_BUFFERACTIVATED:DBG1("beNotified() NPPN_BUFFERACTIVATED BufferID = %p", notification->nmhdr.idFrom);break;
   case SCN_UPDATEUI:
      {
         if (((notification->updated & SC_UPDATE_V_SCROLL) != 0) && _configDlg.getIsSyncScroll() ) {
            generic_string currFile;
            if (bCheckLastFileNameSame(currFile)) {
               int currTopLine = (int)execute(scnActiveHandle, SCI_GETFIRSTVISIBLELINE);
               DBG1("beNotified() SCN_UPDATEUI: Scrolled to currTopLine=%d", currTopLine);
               _findResult.updateViewScrollState(currTopLine, true);
            }
         }
         if(_bIgnoreBufferModify) {
            DBG0("beNotified() SCN_UPDATEUI _bIgnoreBufferModify = false");
            _bIgnoreBufferModify = false;
         }
         break;
      }
#if 0 // SCN_STYLENEEDED
   case SCN_STYLENEEDED:
      {
         // make sure the text is drawn with our style
         if (mCurScnHandle == notification->nmhdr.hwndFrom) {
            DBG0("SCN_STYLENEEDED");

            WPARAM line_number = (WPARAM)execute(mCurScnHandle, SCI_LINEFROMPOSITION, execute(mCurScnHandle, SCI_GETENDSTYLED));
            int start_pos = execute(mCurScnHandle, SCI_POSITIONFROMLINE, line_number);
            int end_pos = notification->position;
            int line_length = execute(mCurScnHandle, SCI_LINELENGTH, line_number);
            // The SCI_STARTSTYLING here is important
            execute(mCurScnHandle, SCI_STARTSTYLING, start_pos, MY_STYLE_MASK);

            while (end_pos > start_pos) {
               if (line_length) {
                  char first_char = (char)execute(mCurScnHandle, SCI_GETCHARAT, (WPARAM)execute(mCurScnHandle, SCI_POSITIONFROMLINE, line_number));
                  switch (first_char)
                  {
                  case '#':
                     execute(mCurScnHandle, SCI_SETSTYLING, line_length, STYLE_DEFAULT);
                     execute(mCurScnHandle, SCI_SETFOLDLEVEL, line_number, 1);
                     break;

                  default:
                     execute(mCurScnHandle, SCI_SETSTYLING, line_length, STYLE_BRACELIGHT);
                     break;
                  } // switch
                  ++line_number;
                  start_pos = execute(mCurScnHandle, SCI_POSITIONFROMLINE, line_number);
                  line_length = execute(mCurScnHandle, SCI_LINELENGTH, line_number);
                  if (start_pos == -1 || start_pos > end_pos) {
                     start_pos = end_pos; // normal end
                  }
               } // if line_length
               else {
                  DBG2("SCN_STYLENEEDED line with length 0 start_pos %d line %d", start_pos, line_number);
                  break;
               }
            } // while end_pos >= start_pos
         } // if hwnd == mCurScnHandle
      } // case SCN_STYLENEEDED
      break;
      // hier
   case SCN_MARGINCLICK:
   {
      // make sure the text is drawn with our style
      if (mCurScnHandle == notification->nmhdr.hwndFrom) {
         DBG0("SCN_MARGINCLICK");
      }
   }
   break;
#endif // SCN_STYLENEEDED
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
                  DBG1("AnalysePlugin: SCN_MODIFIED(text) for different file %s", currentfile.c_str());
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
    while(i < length) {   //because the backslash escape quences always reduce the size of the generic_string, no overflow checks have to be made for target, assuming parameters are correct
        current = query[i];
        charLeft--;
        if (current == '\\' && charLeft) {   //possible escape sequence
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
                    if (current == 'b') {         //11111111
                        size = 8, base = 2;
                    } else if (current == 'o') {   //377
                        size = 3, base = 8;
                    } else if (current == 'd') {   //255
                        size = 3, base = 10;
                    } else if (current == 'x') {   //0xFF
                        size = 2, base = 16;
                    } else if (current == 'u') {   //0xCDCD
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
                default: {   //unknown sequence, treat as regular text
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

bool AnalysePlugin::readBase(const generic_string& str, int /*curPos*/, int * value, int base, int size) {
    int i = 0, temp = 0;
    *value = 0;
    TCHAR mx = '0' + (TCHAR)base - 1;
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

        if (current >= '0' && current <= mx) {
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
   } else if(pattern.getSearchType()== tclPattern::rgx_multiline) {
      flags |= (SCFIND_REGEXP|SCFIND_POSIX|SCFIND_REGEXP_DOTMATCHESNL);
      text = pattern.getSearchText(); // text to be searched
   } else if(pattern.getSearchType()== tclPattern::escaped) {
      text = convertExtendedToString(pattern.getSearchText());
   } else {
      text = pattern.getSearchText(); // text to be searched
   }
   
   flags |= pattern.getIsMatchCase()?SCFIND_MATCHCASE:0;
   flags |= pattern.getIsWholeWord()?SCFIND_WHOLEWORD:0;
   flags |= SCFIND_REGEXP_EMPTYMATCH_ALL | SCFIND_REGEXP_SKIPCRLFASONE;
#ifdef TAGGED_MARKUP
   bool bTagged = false;
   if((flags & SCFIND_REGEXP) != 0) 
   {
      // before doing expencive GETTAG we check if required
      const char* pcBraket = strstr(text2FindA, "(");
      if (pcBraket) {
         if (pcBraket > text2FindA) {
            // braket being excaped?
            bTagged = (*(--pcBraket) != '\\');
         } else {
            bTagged = true;
         }
      }
   }
#endif
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
    WcharMbcsConvertor *wmc = &WcharMbcsConvertor::getInstance();
    unsigned int cp = (unsigned int)execute(scnActiveHandle, SCI_GETCODEPAGE); 
    const char *text2FindA = wmc->wchar2char(text.c_str(), cp);
    size_t text2FindALen = strlen(text2FindA);
    targetStart = (int)execute(scnActiveHandle, SCI_SEARCHINTARGET, 
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
      if (targetStart == 0) {
         // might be an exception from scintilla let's check the errorcode
         int st = (int)execute(scnActiveHandle, SCI_GETSTATUS);
         if (st != 0) {
            _findDlg.activatePleaseWait(false);
            const TCHAR* cerr;
            switch (st) {
            case SC_STATUS_FAILURE: cerr = TEXT("SC_STATUS_FAILURE"); break; // 1
            case SC_STATUS_BADALLOC: cerr = TEXT("SC_STATUS_BADALLOC"); break; // 2
            case SC_STATUS_WARN_REGEX : cerr = TEXT("SC_STATUS_WARN_REGEX"); break; // 1001
            default: cerr = TEXT("UNKNOWN");
            };
            generic_string err = TEXT("While searching an error occured: ");
            err += cerr;
            MessageBox((HWND)NULL, err.c_str(), TEXT("Scintilla Error"), MB_OK);
            break;
         }
      }
      targetStart = (int)execute(scnActiveHandle, SCI_GETTARGETSTART);
      targetEnd = (int)execute(scnActiveHandle, SCI_GETTARGETEND);
      if (targetEnd > endRange) {   
         // we found a result but outside our range, therefore we do not process it
         // in fact that should not happen, because we set the range before
         break;
      }
      int foundTextLen = targetEnd - targetStart;
#ifdef TAGGED_MARKUP
      // TODO startRange = targetStart + foundTextLen ;   //search from result onwards
      //if((flags & SCFIND_REGEXP) != 0) 
      {
         // before doing expencive GETTAG we check if required
         //bool bTagged = false;
         //const char* pcBraket = strstr(text2FindA, "(");
         //if (pcBraket) {
         //   if (pcBraket > text2FindA) {
         //      // braket being excaped?
         //      bTagged = (*(--pcBraket) != '\\');
         //   } else {
         //      bTagged = true;
         //   }
         //}
         if(bTagged) {
            char* pacTag = new char[foundTextLen+1];
            char* pacText = 0; 
            int j = -1;
            int k = 0;
            for(int i = 1; j != 0 && i < 11; ++i) {
               j = (int)execute(scnActiveHandle, SCI_GETTAG, i, (LPARAM)pacTag);
               if(j != 0) {
                  DBGA3("doFindPattern() regex found tag %d return %d text '%s'", i, j, pacTag);
                  pacText = new char[foundTextLen+1];
                  pacText[0]=0;
                  if(k == 0) {
                     // do it once per searched tag
                     k = (int)execute(scnActiveHandle, SCI_GETTEXT, foundTextLen+1, (LPARAM)pacText);
                     DBGA2("doFindPattern() regex found text return %d text '%s'", k, pacText);
                     if (k == 0) {
                        DBG0("doFindPattern() ERROR selection is zero!");
                        k =  -1;
                     }
                  }
                  char* pc = strstr(pacText, pacTag);
                  int m = pc-pacText;
                  DBG1("doFindPattern() tag begin pos %d", m);
                  if (m > 0) {
                     targetStart += m;
                     targetEnd = targetStart + j;
                  }
               }
            }
            delete[] pacTag;
            delete[] pacText;
         }
      }
#endif // TAGGED_MARKUP
      int lineNumberStart = (int)execute(scnActiveHandle, SCI_LINEFROMPOSITION, targetStart);
      int lineNumberEnd = (int)execute(scnActiveHandle, SCI_LINEFROMPOSITION, targetEnd);
      int lineCount = lineNumberEnd - lineNumberStart;
      int thisLineIndex = 0;
      while (thisLineIndex <= lineCount) {
         DBG3("doFindPattern() found: start %d end %d line %d.", targetStart, targetEnd, lineNumberStart+thisLineIndex);
         result.push_back(targetStart, targetEnd, lineNumberStart+thisLineIndex/*, pLine*/);
         ++thisLineIndex;
      }
      startRange = targetStart + foundTextLen ;   //search from result onwards
      execute(scnActiveHandle, SCI_SETTARGETSTART, startRange);
      // end needs to be set because search did use it to signal found selection
      execute(scnActiveHandle, SCI_SETTARGETEND, endRange);
      //DBG2("doFindPattern() tstart %d, tend %d.", startRange, endRange);
      nbProcessed++;
      // do next search
#ifdef UNICODE
       targetStart = (int)execute(scnActiveHandle, SCI_SEARCHINTARGET, 
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

//void AnalysePlugin::doStyleFormating(HWND hCurrentEditView, int /*startPos*/, int /*endPos*/) 
//{
//   // get the result list, including all positions 
//   // get the pattern list including all the styles
//   // apply to all positions in the given area the correct pattern
//   ::SendMessage(hCurrentEditView, SCI_STARTSTYLING, 0, MY_STYLE_MASK ); //(int pos, int mask)
//   ::SendMessage(hCurrentEditView, SCI_SETSTYLING, 100, 10);//foundTextLen, fs
//
//}

void AnalysePlugin::toggleShowFindDlg () 
{
   funcItem[SHOWFINDDLG]._init2Check = !funcItem[SHOWFINDDLG]._init2Check;
   gbPluginVisible = funcItem[SHOWFINDDLG]._init2Check;
   gbResultVisible = gbPluginVisible;
   showFindDlg();
}

void AnalysePlugin::showHelpDialog() {
   execute(nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)funcItem[SHOWHELPDLG]._cmdID, (LPARAM)true);
   _helpDlg.doDialog(funcItem[SHOWHELPDLG]._cmdID);
}

void AnalysePlugin::showConfigDialog() {
#ifdef CONFIG_DIALOG
   execute(nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM)funcItem[SHOWCNFGDLG]._cmdID, (LPARAM)true);
   if(!_configDlg.isCreated() && !_configDlg.isConfigured()) {
      if (!_findDlg.isCreated()) {
         createFindDlg();
      }
      // let find dialog decode the ini string as single place
      _findDlg.setDefaultOptions(mDefaultOptions.c_str());
      _configDlg.setDefaultPattern(_findDlg.getDefaultPattern()); 
   }
   _configDlg.doDialog(funcItem[SHOWCNFGDLG]._cmdID);
#endif
}

#ifdef HOOK_INTO_WNDPROC
WNDPROC				wndProcNotepad = NULL;
LRESULT CALLBACK SubWndProcNotepad(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   return ::CallWindowProc(wndProcNotepad, hWnd, message, wParam, lParam);
}
#endif 

void AnalysePlugin::setInfo(NppData notpadPlusData) 
{
   nppData = notpadPlusData;
   _findDlg.init(_hModule, nppData);
   _configDlg.init(_hModule, nppData);
   _configDlg.setParent(this, &_findDlg, &_findResult);
   _findResult.init(_hModule, nppData._nppHandle);
   _helpDlg.init(_hModule, nppData);
#ifdef HOOK_INTO_WNDPROC
   wndProcNotepad = (WNDPROC)SetWindowLongPtr(nppData._nppHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SubWndProcNotepad)); // TODO check if change is correct
#endif
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

void AnalysePlugin::createFindDlg()
{
   tTbData   data = { 0 };
   _findDlg.create(&data);
   // set the diag 
   _findDlg.setSearchHistory(mSearchHistory.c_str());
   _findDlg.setCommentHistory(mCommentHistory.c_str());
   _findDlg.setGroupHistory(mGroupHistory.c_str());
   // let finddialog decode the string from the ini file
   _findDlg.setDefaultOptions(mDefaultOptions.c_str());
   if (PathFileExists(xmlFilePath) == TRUE) {
      // load the patterns from last run
      _findDlg.loadConfigFile(xmlFilePath);
   }
   ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
   // Always sync show status, unless npp is not ready yet
   if (_nppReady) {
      _findDlg.display(gbPluginVisible);
   }
}

void AnalysePlugin::showFindDlg ()
{
   if (_findDlg.isCreated() || gbPluginVisible) {
      //MessageBox(NULL, (bVisible?"true":"false"), "bVisible", 0);
      tTbData   data = { 0 };
      //RECT rect={0,0,0,0};
      if (!_findDlg.isCreated()) {
         createFindDlg();
      }
      if (!_findResult.isCreated())
      {
         ZeroMemory(&data, sizeof(data));
         _findResult.create(&data);
         ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
         DBG0("showFindDlg() result window initEdit()");
         _findResult.initEdit(_findDlg.getDefaultPattern()); // here findresult first time exists
         _findResult.updateWindowData(_configDlg.getFontText(), _configDlg.getFontSize());
      }

      // Always show, unless npp is not ready yet
      if (_nppReady) {
         _findResult.display(gbPluginVisible && gbResultVisible);
         _findDlg.display(gbPluginVisible);
      }
      // feed back in case it did'nt work;
      gbPluginVisible = isVisible();
      funcItem[SHOWFINDDLG]._init2Check = gbPluginVisible;

   }
}

void AnalysePlugin::visibleChanged(bool isVisible) 
{
   execute(nppHandle, NPPM_SETMENUITEMCHECK, funcItem[SHOWFINDDLG]._cmdID, (LPARAM)isVisible);
}

void AnalysePlugin::setCustomColorsStr(const TCHAR* colors) {
   if (colors == 0) return;
   int j = (int)generic_strlen(colors) + 1;
   j = (j>MAX_CHAR_HISTORY) ? MAX_CHAR_HISTORY : j;
   TCHAR szPart[MAX_CHAR_HISTORY];
   generic_strncpy(szPart, colors, j);
   szPart[j - 1] = 0;
   TCHAR* szToken = generic_strtok(szPart, TEXT(","));
   int num = 0;
   while (szToken && num < NUM_CUSTOM_COLORS) {
      tColor c;
      c = tclColor::convColorStr2Rgb(generic_string(szToken));
      _acrCustClr[num] = c;
      szToken = generic_strtok(NULL, TEXT(",")); // next token
      ++num;
   } // while
}

generic_string AnalysePlugin::getCustomColorsStr() {
   generic_string s;
   for (int i = 0; i < NUM_CUSTOM_COLORS; ++i) {
      if (i > 0) {
         s += TEXT(",");
      }
      TCHAR col[8];
      tColor c = _acrCustClr[i] + 0x01000000; // to preserve all zeros
      generic_itoa(c, col, 16);
      col[0] = TEXT('#'); // set string trigger 
      s += generic_string(col);
   }
   return s;
}

LRESULT AnalysePlugin::messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
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

void AnalysePlugin::addSelectionToPatterns() {
   int textLength = (int)execute(scnActiveHandle, SCI_GETSELTEXT);
   if (textLength == 0) {
      ::MessageBox(getCurrentHScintilla(scnActiveHandle), TEXT("Please select a text or text block to be taken as patterns."), TEXT("AnalysePlugin Selection to Patterns"), MB_OK);
      return;
   } 
   char* pc = new char[textLength];
   execute(scnActiveHandle, SCI_GETSELTEXT, 0, (LPARAM)pc);
    
   WcharMbcsConvertor *wmc = &WcharMbcsConvertor::getInstance();
    unsigned int cp = (unsigned int)execute(scnActiveHandle, SCI_GETCODEPAGE); 
#ifdef UNICODE
   WCHAR* selText = const_cast<WCHAR*>(wmc->char2wchar(pc, cp));
#else
   char * selText = pc;
#endif
   TCHAR* szToken = generic_strtok(selText, TEXT("\n"));
   while (szToken) {
      if (szToken[generic_strlen(szToken)-1] == TCHAR('\r')) {
         szToken[generic_strlen(szToken)-1] = 0;
      }
      if (generic_strlen(szToken)) {
         _findDlg.addTextPattern(szToken);
      }
      szToken = generic_strtok(NULL, TEXT("\n"));
   }
   delete[] pc;
}

void AnalysePlugin::runSearch() {
   if (!isVisible()) {
      toggleShowFindDlg();
   }
   _findDlg.doSearch();
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
#endif //UNICODE
