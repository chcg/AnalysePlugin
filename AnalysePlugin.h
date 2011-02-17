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

#include "MyPluginInterface.h"
#include "Notepad_plus_msgs.h"
#include "versioninfo.inc"
#include <shlobj.h>
#include "myDebug.h"
// #include "GoToLineDlg.h"

// outscourced demo functions
//#include "CDemoFunction.h"
//#include "CFinder.h"
#include "DockingFeature/FindDlg.h"
#include "tclFindResultDoc.h"
#include "tclFindResultDlg.h"
#include <string.h>
#include "MyPlugin.h"
#include "HelpDialog.h"
#include "ScintillaSearchView.h"

// special context menu in finder window 
// we assume that the menu is not required any more after 
// using scintillas second window as output
#define FINDDLG_SCINTILLAFINFERCOLLAPSE   (WM_USER + 8) 
#define FINDDLG_SCINTILLAFINFERUNCOLLAPSE (WM_USER + 9)
#define FINDDLG_SCINTILLAFINFERCOPY       (WM_USER + 16)
#define FINDDLG_SCINTILLAFINFERSELECTALL  (WM_USER + 17)

typedef void *WindowID;

// list of functions provided to notepad++
enum tePluginFuncId {
   SHOWFINDDLG,
#ifdef CONFIG_DIALOG
   SHOWCNFGDLG,
#endif
   SHOWHELPDLG,
   LAST_PLUGINFUNCID // this one is used as max index of function ids
};


/**
 * this is the plugin interface to notepad 
 */
class AnalysePlugin : public MyPlugin
{
public:

   AnalysePlugin()
      : _nppReady(false)
      , gbPluginVisible(false)
      , _markedLine(-1)
      , _Npp(0)
      , _hModule(0)
      , _line(0)
      , _maxNbCharAllocated(0)
//      , mResultFontSize(0)
  {
      memset(&nppData, 0, sizeof(nppData));
      memset(iniFilePath, 0, sizeof(iniFilePath));
      memset(_szPluginFileName, 0, sizeof(_szPluginFileName));
      _findDlg.setParent(this);
      _findResult.setParent(this);

      mVersionString = TEXT("Analyse Plugin V1.6 ");
      mVersionString += TEXT("Rev. ");
      mVersionString += SVNRevInfo;
      mVersionString += TEXT(" ");
      mVersionString += SVNDate;
      mVersionString += TEXT(" ");
      mVersionString += SVNModified;
      mVersionString += TEXT(" ");
      mVersionString += SVNMixed;
      _helpDlg.setVersion(mVersionString);
   }

   virtual ~AnalysePlugin() {
      if (_line) {
         delete [] _line;
         _line = 0;
         _maxNbCharAllocated =0;
      }
   }
   /** 
   * Analyse plugin dialog
   */
   void showFindDlg();
   void toggleShowFindDlg();
   void showHelpDialog();
   void showConfigDialog();

   bool isVisible() const {
      return _findDlg.isVisible();
   }

   // sets the handle access to main_edit, second_edit and npp_main
   void setInfo(NppData notpadPlusData); 

   // returns the name of this plugin
   const TCHAR * getName(){
      return PLUGIN_NAME;
   }

   // returns the pointer to all plugin functions
   FuncItem * getFuncsArray(int * nbF){
      *nbF = LAST_PLUGINFUNCID;
      return funcItem;
   }

   // message handler access
   HWND getCurrentHScintilla(teNppWindows which) const;

   /** apply styles for the different patterns
   * this is done for findresults and for main window
   */ 
   void doStyleFormating(HWND hCurrentView, int startPos, int endPos);
   
   /**
   * returns the name of the file being used for analysis
   */
   virtual /*std::*/ generic_string getSearchFileName() const {
     // return /*std::*/ generic_string(_findDlg.getFileName());
      return /*std::*/ mLastSearchedFileName;
   }

   /**
   * set the search file being analysed
   */
   virtual void setSearchFileName(const /*std::*/ generic_string& file); 

   virtual void removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult);
   
   virtual void clearResult();
   virtual void moveResult(tPatId oldPattId, tPatId newPattId);

   virtual BOOL doSearch(tclResultList& resultList);

   virtual void visibleChanged(bool isVisible);
   virtual teOnEnterAction getOnEnterAction() const;
   virtual std::string getResultFontName() const;
   virtual unsigned getResultFontSize() const;

   LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam);

   // scintilla notifications
   void beNotified(SCNotification *notification);

   // main entry to the dll
   BOOL dllmain(HANDLE hModule, DWORD  reasonForCall, LPVOID lpReserved);

   // configure checks for ini files and presets al internal setting from it
   // dllPath is the position of the dll being used to find the ini-file
   void loadSettings();

   void saveSettings();

   /**
   * reaction on (un)collapse all 
   * @param true = uncollapse
   */
   void updateStyles()
   {
      _findResult.setPatternStyles(_findDlg.getPatternList());
   }

   /**
   * my replacement for direct access is to use scintillas second window
   */
   LRESULT execute(teNppWindows window, UINT Msg, WPARAM wParam=0, LPARAM lParam=0) const {
      HWND hCurrentEditView = getCurrentHScintilla(window); // it's 0 for first = find text		
      return ::SendMessage(hCurrentEditView, Msg, wParam, lParam);
   }
   
   LRESULT execute(HWND hwnd, UINT Msg, WPARAM wParam=0, LPARAM lParam=0) const  {
      return ::SendMessage(hwnd, Msg, wParam, lParam);
   }

   void showMargin(int witchMarge, bool willBeShown = true);

   void displaySectionCentered(int posStart, int posEnd, bool isDownwards = true);

protected:

   std::string convertExtendedToString(const std::string& query);	
   bool readBase(const std::string& str, int curPos, int * value, int base, int size) ;
   int doFindPattern(const tclPattern& pattern, tclResult& result);

   /**
   * return the actually marked line
   */
   unsigned getCurrentMarkedLine() const {
      return _markedLine;
   }

   /**
   * set the actually marked line
   */
   void setCurrentMarkedLine(unsigned line) {
      _markedLine = line;
   }

   unsigned getCurrentViewLineNumber()const {
      LRESULT curpos = execute(scnSecondHandle, SCI_GETCURRENTPOS);
      return (unsigned)execute(scnSecondHandle, SCI_LINEFROMPOSITION, curpos);
	}

public:

   /**
   * sets find result window into read only mode
   */
   void setFinderReadOnly(bool isReadOnly) {
      execute(scnSecondHandle, SCI_SETREADONLY, isReadOnly);
   }

protected:

   static const TCHAR PLUGIN_NAME[];
   static const TCHAR ADDITIONALINFO[];
   static const TCHAR KEYNAME[];
   static const TCHAR KEYSEARCHHISTORY[];
   static const TCHAR KEYCOMMENTHISTORY[];
   static const TCHAR KEYDEFAULTOPTIONS[];
   static const TCHAR KEYONENTERACTION[];
   static const TCHAR KEYLASTFILENAME[];
   static const TCHAR KEYFONTNAME[];
   static const TCHAR KEYFONTSIZE[];
   static const TCHAR SECTIONNAME[];
   static const TCHAR LOCALCONFFILE[];
   static const TCHAR ANALYSE_INIFILE[];

   TCHAR iniFilePath[MAX_PATH];
   TCHAR _szPluginFileName[MAX_PATH];

   /** handles of notpepad++ */
   NppData nppData;

   /** my icon in toolbar */
   toolbarIcons g_TBSearchInFiles;

   /** ready true if we can start over the  search */
   bool _nppReady;

   /** function array */
   FuncItem funcItem[LAST_PLUGINFUNCID];


   /** dialog to configure analyse */
   FindDlg _findDlg;
   /** demo dialog */
   //GoToLineDlg _goToLineDlg;
   tclFindResultDlg _findResult;
   HelpDlg _helpDlg;
   ConfigDialog _configDlg;

   /** demofunctions */
//   CDemoFunction _demofunction;

   /** algorithmic part and cache of found line information */
   //CFinder _finder;
   //tclFindResultDoc mResultDoc;
   /*std::*/ generic_string mSearchPatternFileName;
   /*std::*/ generic_string mSearchHistory;
   /*std::*/ generic_string mCommentHistory;
   /*std::*/ generic_string mVersionString;
   /*std::*/ generic_string mDefaultOptions;
   generic_string mLastSearchedFileName; // used to find out if window has changed
   /** is true if the closing tag shall be triggered with adding end tag */

   bool gbPluginVisible;
   unsigned _markedLine;
   NppData* _Npp;
   HINSTANCE _hModule;
      
   char * _line;
   size_t _maxNbCharAllocated;

   // LexAnalyseResult mLex;

};



