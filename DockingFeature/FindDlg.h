/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2018 Matthias H. mattesh(at)gmx.net
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
#ifndef FIND_DLG_H
#define FIND_DLG_H

#include "DockingDlgInterface.h"
#include "PluginInterface.h"

#include "resource.h"
#include <string.h>
#include "ColourPicker2.h"


#include <string>
#include <vector>
#include "Scintilla.h"
#include "MyPlugin.h"
#include "FindConfigDoc.h"
#include "tclResultList.h"
// #include "ScintillaEditView.h"

#include "tclComboBoxCtrl.h"
#include "ConfigDialog.h"
class tclPattern;
#include "tclResult.h"
#include "tclTableview.h"
#include "PleaseWaitDlg.h"

//const int MARK_BOOKMARK = 1;
//const int MARK_HIDELINESBEGIN = 2;
//const int MARK_HIDELINESEND = 3;

#define MAX_CHAR_HISTORY 2000


#if 0
struct TargetRange {
   int targetStart;
   int targetEnd;
};

enum SearchType { 
   FindNormal,    // text as is
   FindExtended,  // back slash escaped
   FindRegex      // regular expression
};


struct FindOption {
   FindOption()
      :_isWholeWord(true), _isMatchCase(true), _searchType(FindNormal),
      _isWrapAround(true)/*, _whichDirection(DIR_DOWN), _isIncremental(false)*/{}
   bool _isWholeWord;
   bool _isMatchCase;
   bool _isWrapAround;
   //bool _whichDirection;
   //bool _isIncremental;
   SearchType _searchType;
};

//This class contains generic search functions as static functions for easy access
class Searching {
public:
   static int convertExtendedToString(const char * query, char * result, int length);
   static TargetRange t;
   static int buildSearchFlags(FindOption * option) {
      return	(option->_isWholeWord ? SCFIND_WHOLEWORD : 0) |
            (option->_isMatchCase ? SCFIND_MATCHCASE : 0) |
            (option->_searchType == FindRegex ? SCFIND_REGEXP|SCFIND_POSIX : 0);
   };

private:
   static bool readBase(const char * string, int * value, int base, int size){
      int i = 0, temp = 0;
      *value = 0;
      char max = '0' + base - 1;
      char current;
      while(i < size) {
         current = string[i];
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
};
#endif // 0

/**
 * FindDlg shows a docable window containing all configuration and search string
 * information it uses a configdoc instance to maintain search patterns to the disk.
 * It stores the configuration into the resultlist and activates the update of the
 * result list when requested.
 */
class FindDlg : public DockingDlgInterface
{
public :

   FindDlg()
      : DockingDlgInterface(IDD_FIND_DLG_CONF)
      ,_pParent(0)
      ,_lineCounter(0)
      ,_pFgColour(0)
      ,_pBgColour(0)
      ,_pPlsWait(0)
      ,_maxConfigFiles(4)
   {
      _ConfigFileName[0] = 0;
   }

   ~FindDlg()
   {
      _pParent = 0;
   }

//   void init(HINSTANCE hInst, HWND parent);
   void init(HINSTANCE hInst, NppData nppData);

   void create(tTbData * data, bool isRTL = false);

   void setDialogData(const tclPattern& p);

   void display(bool toShow = true) const ;

   void setAllDoSearch(bool bOn) ;

   void setAllDirty() 
   {
      // make sure found patterns become rechecked
      tclResultList::iterator iResult = mResultList.begin();
      for (; iResult != mResultList.end(); ++iResult) {
         iResult.refResult().setDirty();
      }
   }

   void setParent(MyPlugin* parent){
      _pParent = parent;
   }

   void setFileName(const generic_string& str);
   void setConfigFileName(const generic_string str); // intentionally a copy parameter

   bool loadConfigFile(const TCHAR* file, bool bAppend=true, bool bLoadNew=true);
   bool saveConfigFile(const TCHAR* file);

   void setNumOfCfgFiles(unsigned u);
   void setNumOfCfgFilesStr(const generic_string& str);

   void doSearch();

   const generic_string& getFileName() const{
      return _FileName;
   }
   const TCHAR* getszConfigFileName() const{
      return _ConfigFileName;
   }

   const tclPattern& getDefaultPattern() const {
      return mDefPat;
   }
   /**
   * callback function for receiving messages
   */
   virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

   /**
   * scintilla notification message
   */
   BOOL notify(SCNotification *notification);

   /**
    * store the patterns into a file 
    * returns true if file was saved
    */
   bool doStoreConfigFile();

   /**
    * implementation of the load button 
    */
   bool doLoadConfigFile(bool bAppend=true, bool bLoadNew=true);

   void setSearchHistory(const TCHAR* hist);
   //void setSearchHistory(const char* hist, int charSize=1);

   generic_string getSearchHistory() const;
   void getSearchHistory(generic_string& str) const ;
   //void getSearchHistory(std::wstring& str) const ;

   void setCommentHistory(const TCHAR* hist);
   //void setCommentHistory(const char* hist, int charSize=1);
   generic_string getCommentHistory() const;
   void getCommentHistory(generic_string& str) const ;
   //void getCommentHistory(std::wstring& str) const ;

   void setCmbHistory(tclComboBoxCtrl& thisCmb, const TCHAR* hist, int charSize);

   //void setDefaultOptions(const wchar_t* options);
   void setDefaultOptions(const TCHAR* options, int charSize=2);
   generic_string getDefaultOptions() const;
   void getDefaultOptions(generic_string& str) ;
   //void getDefaultOptions(std::wstring& str) ;
   void doToggleToSearch();
   /** called when the user double clicks a tabel row */
   void doCopyLineToDialog();
   /** and back */
   void doCopyDialogToLine();
   bool isSearchTextEqual();
   bool isPatternEqual();
   void addTextPattern(const TCHAR* pzsText);

   /** provide access to the pattern list for style informations 
   **/
   const tclPatternList& getPatternList() const {
      return mResultList;
   }
   tclPatternList& refPatternList() {
      return mResultList;
   }
   
   /**
   * same as moveResult() but for the patterns
   */
   void moveResult(tPatId oldPattId, tPatId newPattId);
   
   /**
   * to know if a row is selected is required for update sent to dialog from config diakog change
   */
   bool getIsRowSelected()const {
      return (-1 != mTableView.getSelectedRow());
   }

   // triggers that a research has to be done
   void SetModified(bool bModified=true);
   static void CALLBACK MyTimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime); 

   void activatePleaseWait(bool isEnable=true);
   void setPleaseWaitRange(int iMin, int iMax);
   void setPleaseWaitProgress(int iCurr);
   bool getPleaseWaitCanceled();

   const std::vector<generic_string>& getLastConfigFiles() const {
      return _lastConfigFiles;
   }

//ScintillaEditView **_ppEditView;    // access to the editor window
// find result cache
// actually marked line in result window
// last line found ??
// access to both scintilla windows

protected:
   /** this function is called to update the result for one search pattern 
   *  it stores the position infos into the result */
   int doFindPattern(tclPattern& pattern, tclResult& result);
   
   void refillTable();

   /** @link aggregation */
   MyPlugin* _pParent;
   size_t _lineCounter;

   /** @link dependency */
   /*# FindConfigDoc lnkFindConfigDoc; */
   #define MAX_HEADLINE 1000

   /** @link aggregation */
   tclResultList mResultList;
   tclTableview mTableView;
   tclComboBoxCtrl mCmbSearchText;
   tclComboBoxCtrl mCmbSearchType;
   tclComboBoxCtrl mCmbSelType;
   tclComboBoxCtrl mCmbComment;
//   tclComboBoxCtrl mCmbColor;
   generic_string _FileName;        // buffer for file name
   TCHAR _ConfigFileName[MAX_HEADLINE]; // buffer for config file name
   tclPattern mDefPat;
   //teOnEnterAction mOnEnterAction; // 0 do nothing 1 update 2 add

   ColourPicker2* _pFgColour;
   ColourPicker2* _pBgColour;

   static int _ModifiedMode;
   static HWND _ModifiedHwnd;
   PleaseWaitDlg* _pPlsWait;
   std::vector<generic_string> _lastConfigFiles;
   unsigned _maxConfigFiles;
};

#endif //FIND_DLG_H
