/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (c) 2022 Matthias H. mattesh(at)gmx.net
partly copied from the NotePad++ project from 
Don HO don.h(at)free.fr 

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

#include "MyPluginInterface.h"
#include "Notepad_plus_msgs.h"
#include "version.h"
#include <shlobj.h>

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
// TODO #include "NppDarkMode.h"
#include "myDebug.h"

// special context menu in finder window 
// we assume that the menu is not required any more after 
// using scintillas second window as output
#define FINDDLG_SCINTILLAFINFERCOLLAPSE   (WM_USER + 8) 
#define FINDDLG_SCINTILLAFINFERUNCOLLAPSE (WM_USER + 9)
#define FINDDLG_SCINTILLAFINFERCOPY       (WM_USER + 16)
#define FINDDLG_SCINTILLAFINFERSELECTALL  (WM_USER + 17)
#define NUM_CUSTOM_COLORS 16
// allow longer paths as old winnt coding 
#define AP_MAX_PATH 1024

#define vstr(a) __vstr(a)
#define __vstr(a) #a

typedef void *WindowID;

// list of functions provided to notepad++
enum tePluginFuncId {
   SHOWFINDDLG,
   SEP1,
   ADDSELTOPATT,
   RUNSEARCH,
   SEP2,
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
      , _bPluginVisible(false)
      , _bResultVisible(true)
      , _markedLine((unsigned int)-1)
      , _Npp(0)
      , _hModule(0)
      , _line(0)
      , _maxNbCharAllocated(0)
      ,_FindProcessCancelled(false)
      ,_bIgnoreBufferModify(false)
//      , mResultFontSize(0)
      , _nppBookmarkId(MARK_BOOKMARK_OLD)
   
  {
      memset(&_nppData, 0, sizeof(_nppData));
      memset(_iniFilePath, 0, sizeof(_iniFilePath));
      memset(_szPluginFileName, 0, sizeof(_szPluginFileName));
      _findDlg.setParent(this);
      _findResult.setParent(this);

      _VersionString = TEXT("Analyse Plugin ");
      _VersionString += TEXT(vstr(VER_FILEVERSION_MAYOR));
      _VersionString += TEXT(".");
      _VersionString += TEXT(vstr(VER_FILEVERSION_MINOR));
      _VersionString += TEXT(" Rev. ");
      _VersionString += TEXT(SVNREVINFO);
      _VersionString += TEXT(" ");
      _VersionString += TEXT(SVNDATE);
      _VersionString += TEXT(" ");
      _VersionString += TEXT(SVNMODIFIED);
      _VersionString += TEXT(" ");
      _VersionString += TEXT(SVNMIXED);
      _VersionString += (sizeof(void *) == 8) ? TEXT(" (64bit)") : TEXT(" (32bit)");
      _helpDlg.setVersion(_VersionString);
      for (int i = 0; i < NUM_CUSTOM_COLORS; ++i) {
         _acrCustClr[i] = RGB(0xff, 0xff, 0xff);
      }
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
   void createFindDlg();
   void toggleShowFindDlg();
   void showHelpDialog();
   void showConfigDialog();
   void addSelectionToPatterns();
   void runSearch();

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
      return _funcItem;
   }

   // message handler access
   HWND getCurrentHScintilla(teNppWindows which) const;

   /** apply styles for the different patterns
   * this is done for findresults and for main window
   */ 
   //void doStyleFormating(HWND hCurrentView, int startPos, int endPos);
   
   /**
   * returns the name of the file being used for analysis
   */
   virtual generic_string getSearchFileName() const {
      return _LastSearchedFileName;
   }

   /**
   * set the search file being analysed
   */
   virtual void setSearchFileName(const generic_string& file); 
   virtual void setDisplayLineNo(bool bOn);

   virtual void removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult);
   
   virtual void clearResult(bool initial = false);
   virtual void moveResult(tPatId oldPattId, tPatId newPattId);

   virtual BOOL doSearch(tclResultList& resultList);
   virtual bool bCheckLastFileNameSame(generic_string& file);

   virtual void visibleChanged(bool isVisible);
   virtual teOnEnterAction getOnEnterAction() const;
   virtual const generic_string& getResultFontName() const;
   virtual unsigned getResultFontSize() const;
   virtual int getUseBookmark() const;
   virtual bool getResultWrapMode() const;
   virtual void setResultWrapMode(bool bOn);
   virtual int getDisplayLineNo() const;
   virtual bool getIsSyncScroll() const;
   virtual bool getDblClickJumps2EditView() const;
   virtual void showConfigDlg();
   virtual LRESULT getBookmarkId() const {
      return _nppBookmarkId;
   }

   LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam);

   // scintilla notifications
   void beNotified(SCNotification *notification);

   // main entry to the dll
   BOOL dllmain(HANDLE hModule, DWORD  reasonForCall, LPVOID lpReserved);
   BOOL FileExists(LPCTSTR szPath);

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
   
   // whenever the search patterns have been changed we want to update the find dialog
   void updateSearchPatterns()
   {
      _findResult.setSearchPatterns(_findDlg.getPatternList());
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

   void displaySectionCentered(int posStart, int posEnd, bool isDownwards = true);

   /**
   * sets find result window into read only mode
   */
   void setFinderReadOnly(bool isReadOnly) {
      execute(teNppWindows::scnSecondHandle, SCI_SETREADONLY, isReadOnly);
   }

   COLORREF* refCustomColors() {  
      // intentioally r/w because custom colors are modified by different instances of color dialog
      return static_cast<COLORREF*>(_acrCustClr);
   }
   
   int getPatternIndex(tPatId id) const;
   // this function provides either the line number or the order num if defined
   generic_string getPatternIdentification(tPatId id) const;
   generic_string getPatternSearchText(tPatId id) const;
   void setSelectedPattern(int index);


protected:

   void setCustomColorsStr(const TCHAR* options);
   generic_string getCustomColorsStr();
   generic_string convertExtendedToString(const generic_string& query);
   bool readBase(const generic_string& str, int curPos, int * value, int base, int size) ;
   int doFindPattern(const tclPattern& pattern, tclResult& result);

   /**
   * return the actually marked line
   */
   size_t getCurrentMarkedLine() const {
      return _markedLine;
   }

   /**
   * set the actually marked line
   */
   void setCurrentMarkedLine(size_t line) {
      _markedLine = line;
   }

   unsigned getCurrentViewLineNumber()const {
      LRESULT curpos = execute(teNppWindows::scnSecondHandle, SCI_GETCURRENTPOS);
      return (unsigned)execute(teNppWindows::scnSecondHandle, SCI_LINEFROMPOSITION, curpos);
    }


   static const TCHAR PLUGIN_NAME[];
   static const TCHAR ADDITIONALINFO[];
   static const TCHAR KEYNAME[];
   static const TCHAR KEYSHOWSEARCH[];
   static const TCHAR KEYSEARCHHISTORY[];
   static const TCHAR KEYCOMMENTHISTORY[];
   static const TCHAR KEYGROUPHISTORY[];
   static const TCHAR KEYDEFAULTOPTIONS[];
   static const TCHAR KEYUSEBOOKMARK[];
   static const TCHAR KEYRESWINWORDWRAP[];
   static const TCHAR KEYDISPLAYLINENO[];
   static const TCHAR KEYONAUTOUPDATE[];
   static const TCHAR KEYSYNCEDSCORLL[];
   static const TCHAR KEYDBLCLKUMP2EDIT[];
   static const TCHAR KEYONENTERACTION[];
   static const TCHAR KEYLASTFILENAME[];
   static const TCHAR KEYFINDFILECAPTION[];
   static const TCHAR KEYFONTNAME[];
   static const TCHAR KEYFONTSIZE[];
   static const TCHAR KEYMAXNUMOFCFGFILES[];
   static const TCHAR KEYLASTSRCHCFGFILE[];
   static const TCHAR KEYNUMOFLASTCFGFILES[];
   static const TCHAR KEYCONFIGLISTCOLUMNS[];
   static const TCHAR KEYCONFIGLISTCOLORDER[];
   static const TCHAR KEYCUSTOMCOLORS[];
   static const TCHAR KEYORDERNUMHIDECOLWIDTH[];
   static const TCHAR SECTIONNAME[];
   static const TCHAR LOCALCONFFILE[];
   static const TCHAR ANALYSE_INIFILE[];

   TCHAR _iniFilePath[AP_MAX_PATH];
   TCHAR _xmlFilePath[AP_MAX_PATH];
   TCHAR _szPluginFileName[AP_MAX_PATH];

   /** handles of notpepad++ */
   NppData _nppData;

   /** my icon in toolbar */
   toolbarIcons _TBIconsOld = { 0 };
   toolbarIconsWithDarkMode _TBIconsDrk = { 0 };

   /** ready true if we can start over the  search */
   bool _nppReady;

   /** function array */
   FuncItem _funcItem[LAST_PLUGINFUNCID];


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
   //generic_string mSearchPatternFileName;
   generic_string _SearchHistory;
   generic_string _CommentHistory;
   generic_string _GroupHistory;
   generic_string _VersionString;
   generic_string _DefaultOptions;
   generic_string _LastSearchedFileName; // used to find out if window has changed
   /** is true if the closing tag shall be triggered with adding end tag */

   bool _bPluginVisible;
   bool _bResultVisible;
   size_t _markedLine;
   NppData* _Npp;
   HINSTANCE _hModule;
      
   char * _line;
   size_t _maxNbCharAllocated;
   bool _FindProcessCancelled;
   bool _bIgnoreBufferModify;
   // LexAnalyseResult mLex;
   static COLORREF _acrCustClr[NUM_CUSTOM_COLORS];
//   HWND mCurScnHandle = NULL;
   LRESULT _nppBookmarkId;
// TODO   NppDarkMode::Options _darkModeOptions;			// actual runtime options
};



