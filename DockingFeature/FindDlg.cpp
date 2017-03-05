/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2016 Matthias H. mattesh(at)gmx.net
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
//#include "stdafx.h"
#include "resource.h"

#include "FindDlg.h"
#include <vector>
#include "ContextMenu.h"
#include "MyPlugin.h"
#include "menuCmdID.h"
#include "UniConversion.h"
#include "tclPattern.h"
#include "FindConfigDoc.h"
#include "SciLexer.h"
#include "chardefines.h"
#include <commctrl.h>// For ListView control APIs
#include <commdlg.h>// For fileopen dialog.
#define MDBG_COMP "FDmain:" 
#include "myDebug.h"

// vars for deferred research
#define IDT_ANALYSEPLG_TIMER 9882
int FindDlg::_ModifiedMode = 0;
HWND FindDlg::_ModifiedHwnd = 0;

void FindDlg::setFileName(const generic_string& str) {
   _FileName = str;
}

void FindDlg::setConfigFileName(const generic_string str) {
   for(size_t i = 0; i < _lastConfigFiles.size(); ++i) {
      if (_lastConfigFiles[i] == str) {
         _lastConfigFiles.erase(_lastConfigFiles.begin() + i);
         --i; // reduce because one less in the list
      }
   }
   _lastConfigFiles.insert(_lastConfigFiles.begin(), str);
   ::GetFileTitle(str.c_str(), _ConfigFileName, COUNTCHAR(_ConfigFileName));
   while (_lastConfigFiles.size() > _maxConfigFiles) {
      _lastConfigFiles.pop_back();
   }
   //_ConfigFileName[COUNTCHAR(_ConfigFileName)-1]=0;
   ::SendMessage(_hParent, NPPM_DMMUPDATEDISPINFO, 0, (LPARAM)_hSelf);
}

bool FindDlg::loadConfigFile(const TCHAR* file, bool bAppend, bool bLoadNew) {
   FindConfigDoc doc(file); 
   if(doc.readPatternList(mResultList, bAppend, bLoadNew)) {
      refillTable();
      _pParent->updateSearchPatterns();
      return true;
   }
   return false;
}
bool FindDlg::saveConfigFile(const TCHAR* file) {
      FindConfigDoc doc(file);
      return doc.writePatternList(mResultList);
}
void FindDlg::setNumOfCfgFiles(unsigned u) {
   if (u > 0) {
      _maxConfigFiles = u;
      while (u < _lastConfigFiles.size()) {
         _lastConfigFiles.pop_back();
      }
   }
}

void FindDlg::setNumOfCfgFilesStr(const generic_string& str) {
   unsigned u = generic_atoi(str.c_str());
   if (u > 0) {
      _maxConfigFiles = u;
      while (u < _lastConfigFiles.size()) {
         _lastConfigFiles.pop_back();
      }
   }
}

void FindDlg::doSearch() {
   DBG0("FindDlg::doSearch()");
   // make sure last edited config values are tsored into the patterns
   if(mTableView.getRowCount() < 1) {
      ::SendMessage(getHSelf(), WM_COMMAND, IDC_BUT_UPD, (LPARAM)0);
   } else {
      switch(_pParent->getOnEnterAction()) 
      {
      case enOnEntUpdate: 
         if (!isPatternEqual()) {
            ::SendMessage(getHSelf(), WM_COMMAND, IDC_BUT_UPD, (LPARAM)0);
         }
         break;
      case enOnEntAdd:
         if (!isSearchTextEqual()) {
            ::SendMessage(getHSelf(), WM_COMMAND, IDC_BUT_ADD, (LPARAM)0);
         } else if (!isPatternEqual()) {
            ::SendMessage(getHSelf(), WM_COMMAND, IDC_BUT_UPD, (LPARAM)0);
         }
         break;
      default: // enOnEntNoAction
         {
            if(mTableView.getRowCount() == 1) {
               ::SendMessage(getHSelf(), WM_COMMAND, IDC_BUT_UPD, (LPARAM)0);
            } else {
               // to avoid errors in clicking the search button w/o update
               // we reset the dialog with what ever has been selected
               doCopyLineToDialog();
            }
         }
      };
   }
   // start search
   _pParent->doSearch(mResultList);
   // set the modification notification for this window if not already on
   int mode = (int)_pParent->execute(scnActiveHandle,SCI_GETMODEVENTMASK);
   if ((mode & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) != (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) {
      DBG1("FindDlg: editor notification mode was not ok: mode=%d add insert text and delete text", mode);
      mode |= (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT);
      _pParent->execute(scnActiveHandle,SCI_SETMODEVENTMASK, mode);
   }
}

INT_PTR CALLBACK FindDlg::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) {
   static bool bDoTrace = false; // make break point and set to true to activate trace
   if(bDoTrace) {
      DBG3("FindDlg::run_dlgProc(0x%08x, 0x%08x, 0x%08x)", message, wParam, lParam);
   }
   // TODO check if this need to be added here
   if (message == WM_COMMAND) {
      if (wParam == IDC_SHOW_ADDCTXDLG) {
         int i = 0;
      }
   }

   switch (message) 
   {
   case IDC_DO_CHECK_CONF: 
      {
         tclPattern* p = (tclPattern*)lParam;
         if(p) {
            mDefPat = *p;
         }
         // check if we are on a selected line and if not take over the default values
         if(!getIsRowSelected()) {
            setDialogData(mDefPat);
            _pFgColour->setColour(mDefPat.getColorNum());
            _pFgColour->redraw();
            _pBgColour->setColour(mDefPat.getBgColorNum());
            _pBgColour->redraw();
         }
         // specialty to shortcut transfer...
         setNumOfCfgFiles((unsigned int)wParam);
         break;
      }
   case WM_COMMAND : 
      {
         switch (wParam)
         {
         case IDC_DO_DISABLE_ALL:
            {
               setAllDoSearch(false);               
               return TRUE;
            }
         case IDC_DO_ENABLE_ALL:
            {
               setAllDoSearch(true);               
               return TRUE;
            }
         case IDC_DO_TOGGLE_SEARCH:
            {
               doToggleToSearch();
               return TRUE;
            }
         case IDC_DO_CLOSE : // Close
            {
               DBG0("IDC_DO_CLOSE");
               display(false);
               return FALSE;  // message shall go to all further clients too
            }
         case IDC_DO_RESEARCH :
            {
               DBG0("IDC_DO_RESEARCH");
               setAllDirty();
               doSearch();
               // finally set focus to editor window
               ::SetFocus(_pParent->getCurrentHScintilla(scnActiveHandle));
               return TRUE;
            }
         case IDC_DO_SEARCH :
            {
               DBG0("IDC_DO_SEARCH");
               doSearch();
               // finally set focus to editor window
               ::SetFocus(_pParent->getCurrentHScintilla(scnActiveHandle));
               return TRUE;
         }
         case IDC_BUT_LOAD:
            {
               DBG0("IDC_BUT_LOAD");
               // load the file from open dialog
               // instantiate xml doc
               HWND hButton = ::GetDlgItem(_hSelf, IDC_BUT_LOAD);
               POINT pt = getTopPoint(hButton);
               ::ClientToScreen(_hSelf, &pt);
               ContextMenu contextmenu;
               std::vector<MenuItemUnit> tmp;
               tmp.push_back(MenuItemUnit(IDC_CTXCFG_LOADNEW, TEXT("Load file...")));
               tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
               tmp.push_back(MenuItemUnit(IDC_CTXCFG_LOADBEG, TEXT("Prepend file...")));
               tmp.push_back(MenuItemUnit(IDC_CTXCFG_LOADEND, TEXT("Append file...")));
               if(_lastConfigFiles.size()) {
                  tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
                  for(int i = 0; i < (int)_lastConfigFiles.size(); ++i) {
                     TCHAR name[MAX_PATH];
                     ::GetFileTitle(_lastConfigFiles[i].c_str(), name, COUNTCHAR(name));
                     tmp.push_back(MenuItemUnit(IDC_CTXCFG_LOADX_0 + i, name));
                  }
                  tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
                  tmp.push_back(MenuItemUnit(IDC_CTXCFG_LOADRESET, TEXT("Reset this list")));
               }
               contextmenu.create(_hSelf, tmp);
               contextmenu.display(pt);
               return TRUE; 
            }
         // case IDC_CTXCFG_LOADX_0 .. IDC_CTXCFG_LOADX_E is handled in default case!
         case IDC_CTXCFG_LOADRESET:
            {
               _lastConfigFiles.clear();
               return TRUE;
            }
         case IDC_CTXCFG_LOADNEW:
            {
               if(doLoadConfigFile(true, true)) {
                  refillTable();
                  _pParent->updateSearchPatterns();
               } 
               return TRUE;
            }
         case IDC_CTXCFG_LOADBEG:
            {
               if(doLoadConfigFile(false, false)) {
                  refillTable();
                  _pParent->updateSearchPatterns();
               } 
               return TRUE;
            }
         case IDC_CTXCFG_LOADEND:
            {
               if(doLoadConfigFile(true, false)) {
                  refillTable();
                  _pParent->updateSearchPatterns();
               } 
               return TRUE;
            }
         case IDC_BUT_SAVE:
            {
               DBG0("IDC_BUT_SAVE");
               doStoreConfigFile();
               return TRUE;
            }
         case IDC_BUT_CLEAR:
            {
               DBG0("IDC_BUT_CLEAR");
               _pParent->clearResult();
               mResultList.clear();
               mTableView.refillTable(mResultList);
               setDialogData(getDefaultPattern());
               _pParent->updateSearchPatterns();
               return TRUE;
            }
         case IDC_BUT_ADD:
            {
               DBG0("IDC_BUT_ADD");
               int selectedRow = mTableView.getSelectedRow();
               int i = mTableView.insertAfterRow();
               if (i != -1) {
                  mResultList.insertAfter(mResultList.getPatternId(selectedRow), mDefPat);
                  mTableView.setSelectedRow(i); 
                  doCopyDialogToLine();
               } else {
                  // don't know why but table don't want to add rows
                  return TRUE;  // no further processing of message required
               }
               //TODO find a way to move the highligted row too and remove this workaround
               selectedRow = mTableView.getSelectedRow(); // index moves with add
               mTableView.setSelectedRow(selectedRow); // set selection back to that being marked before
               doCopyLineToDialog();
               return TRUE;
            }
         case IDC_BUT_UPD:
            {
               DBG0("IDC_BUT_UPD");
               int i = -2;
               if((mTableView.getSelectedRow()==-1)&&(mTableView.getRowCount()==0)) {
                  i = mTableView.insertRow(); // ist immer 0
                  if (i != -1) {
                     mResultList.insert(mResultList.getPatternId(i), mDefPat);
                     mTableView.setSelectedRow(i); 
                     ++i; // position where visible selection is
                  } else {
                     // -1 is error
                     return TRUE; // don't know why but table don't want to add rows
                  }
               }
               doCopyDialogToLine();

               if(!mResultList.getIsDirty())
               {  // if only color was changed this is possible
                  // as soon as the search list has changed the index may not fit anymore...
                  _pParent->updateStyles(); 
               }
               return TRUE;
            }
         case IDC_BUT_DEL:
            {
               DBG0("IDC_BUT_DEL");
               int i =mTableView.getSelectedRow();
               if (i >=0) {
                  mTableView.removeRow(i);
                  // TODO make function in mResultList
                  tclResult oldResult(mResultList.refResult(mResultList.getPatternId(i)));
                  tclResult newResult;
                  _pParent->removeUnusedResultLines(mResultList.getPatternId(i), oldResult, newResult);
                  mResultList.remove(mResultList.getPatternId(i));
                  _pParent->updateSearchPatterns();
               }
               if(mTableView.getRowCount() == 0) {
                  mResultList.clear();
                  _pParent->clearResult();
                  _pParent->updateStyles(); // reset also pattern styles
               } else if (mTableView.getRowCount() > 0){
                  // we have to make sure that old dialog content does 
                  // not override the remaining line
                  doCopyLineToDialog();
               }
               int selectedRow = mTableView.getSelectedRow(); // index moves with add
               mTableView.setSelectedRow(selectedRow);
               return TRUE;
            }
         case IDC_BUT_MOVE_UP:
            {
               int iOldRow = mTableView.getSelectedRow();
               if((iOldRow != -1)&&                // row selected
                  (iOldRow != 0)&&                 // not the first
                  (mTableView.getRowCount()>1))    // more then one
               {
                  tPatId oldId = mResultList.getPatternId(iOldRow);
                  tPatId beforeId = mResultList.getPatternId(iOldRow-1);
                  tPatId newId = mResultList.insert(beforeId, mResultList.getPattern(oldId));
                  // now oldId pattern search result needs to be moved 
                  // into newId without re-searching if not dirty 
                  if(!mResultList.refResult(oldId).getIsDirty())
                  { // move result into new pattern
                     _pParent->moveResult(oldId, newId);
                  } else {
                     // remove just the old line and add the new
                     tclResult oldResult(mResultList.refResult(oldId));
                     tclResult newResult;
                     _pParent->removeUnusedResultLines(oldId, oldResult, newResult);
                     mResultList.remove(oldId);
                  }
                  mTableView.refillTable(mResultList);
                  mTableView.setSelectedRow(iOldRow-1);
                  _pParent->updateSearchPatterns();
               }
               return TRUE;
            }
         case IDC_BUT_MOVE_DOWN:
            {
               int iOldRow = mTableView.getSelectedRow();
               int iCount = mTableView.getRowCount();
               if((iOldRow != -1)&&                // row selected
                  (iOldRow != iCount-1)&&          // not the last
                  (iCount>1))    // more then one
               {
                  tPatId oldId = mResultList.getPatternId(iOldRow);
                  tPatId afterId = mResultList.getPatternId(iOldRow+1);
                  tPatId newId = mResultList.insertAfter(afterId, mResultList.getPattern(oldId));
                  // now oldId pattern search result needs to be moved 
                  // into newId without re-searching if not dirty 
                  if(!mResultList.refResult(oldId).getIsDirty())
                  { // move result into new pattern
                     _pParent->moveResult(oldId, newId);
                  } else {
                     // remove just the old line and add the new
                     tclResult oldResult(mResultList.refResult(oldId));
                     tclResult newResult;
                     _pParent->removeUnusedResultLines(oldId, oldResult, newResult);
                     mResultList.remove(oldId);
                  }
                  mTableView.refillTable(mResultList);
                  mTableView.setSelectedRow(iOldRow+1);
                  _pParent->updateSearchPatterns();
               }
               return TRUE;
            }
         default :
            {
               if(wParam >= IDC_CTXCFG_LOADX_0 && wParam < IDC_CTXCFG_LOADX_E) {
                  // context menu to load special config files
                  WPARAM i = wParam - IDC_CTXCFG_LOADX_0;
                  if (i < _lastConfigFiles.size()) {
                     setConfigFileName(_lastConfigFiles[i]); // will by this remove this instance and add it at pos 0
                     loadConfigFile(_lastConfigFiles[0].c_str()); // so we take it from there
                  }
                  return TRUE;
               }
               // color picker messages
               switch (HIWORD(wParam))
               {
               case CPN_COLOURPICKED:	
                  {
                     if ((HWND)lParam == _pFgColour->getHSelf())
                     {  
						DBGDEF(unsigned long u = _pFgColour->getColour();)
                        DBG2("run_dlgProc() COLOURPICKED 0x%X", u, tclPattern::convColor2Str(tclPattern::convColorNum2Enum(u)));
                        //updateColour(C_FOREGROUND);
                        //notifyDataModified();
                        //int tabColourIndex;
                        //if ((tabColourIndex = whichTabColourIndex()) != -1)
                        //{
                        //	//::SendMessage(_hParent, WM_UPDATETABBARCOLOUR, tabColourIndex, _pFgColour->getColour());
                        //	TabBarPlus::setColour(_pFgColour->getColour(), (TabBarPlus::tabColourIndex)tabColourIndex);
                        //	return TRUE;
                        //}
                        //apply();
                        return TRUE;
                     }
                     else if ((HWND)lParam == _pBgColour->getHSelf())
                     {
						DBGDEF(unsigned long u = _pBgColour->getColour();)
                        DBG2("run_dlgProc() COLOURPICKED 0x%X", u, tclPattern::convColor2Str(tclPattern::convColorNum2Enum(u)));
                        //	updateColour(C_BACKGROUND);
                        //	notifyDataModified();
                        //	int tabColourIndex;
                        //	if ((tabColourIndex = whichTabColourIndex()) != -1)
                        //	{
                        //		tabColourIndex = (int)tabColourIndex == TabBarPlus::inactiveText? TabBarPlus::inactiveBg : tabColourIndex;
                        //		TabBarPlus::setColour(_pBgColour->getColour(), (TabBarPlus::tabColourIndex)tabColourIndex);
                        return TRUE;
                     }

                     //	apply();
                     //	return TRUE;
                     //}
                     else 
                     {
                        return FALSE;
                     }
                  }
               } // switch (HIWORD(wParam))
               break;
            }
         } // switch (wParam)
         break;
      } // case WM_COMMAND
   case WM_SIZE :
      {
         RECT rcDlg, rcText, rcComment;
         getClientRect(rcDlg);
         HWND hTable = ::GetDlgItem(_hSelf, IDC_LIST_CONF);
         HWND hText = ::GetDlgItem(_hSelf,IDC_CMB_SEARCH_TEXT);
         HWND hComment = ::GetDlgItem(_hSelf,IDC_CMB_COMMENT);
         ::GetClientRect(hText, &rcText);
         ::GetClientRect(hComment, &rcComment);
         POINT pTable = getTopPoint(hTable);
         POINT pText = getTopPoint(hText);
         POINT pComment = getTopPoint(hComment);
         int dWidth = rcDlg.right-rcDlg.left;
         int dTableHeight = rcDlg.bottom-rcDlg.top-pTable.y;
         if(dWidth>=0) {
            if(dTableHeight>=0){
               ::MoveWindow(hTable, pTable.x, pTable.y, dWidth, dTableHeight, TRUE);
            }
            ::MoveWindow(hText, pText.x, pText.y, dWidth, rcText.bottom, TRUE);
            ::MoveWindow(hComment, pComment.x, pComment.y, dWidth, rcComment.bottom, TRUE);
            redraw();
         }
         break;
      }
   case WM_NOTIFY:
      {
         return notify((SCNotification *)lParam);
      }
   case WM_SHOWWINDOW:
      {
         _pParent->visibleChanged(wParam?true:false);
         break;
      }
   case WM_CLOSE:
      {
         display(false);
         break;
      }
   case WM_DESTROY:
      {
         _pFgColour->destroy();
         _pBgColour->destroy();
         delete _pFgColour;
         delete _pBgColour;
         break;
      }
   case WM_DRAWITEM:
      {
		 DBGDEF(DRAWITEMSTRUCT *pdis = (DRAWITEMSTRUCT *)lParam;)
         //if(IDC_LIST_CONF == pdis->CtlID) {
            DBG1("WM_DRAWITEM: paint request for item %d", pdis->itemID);
         //}
         break;
      }

   default :
      return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
   } // switch (message)
   return FALSE;
}

void FindDlg::moveResult(tPatId oldPattId, tPatId newPattId) {
   mResultList.moveResult(oldPattId, newPattId);
}

void FindDlg::refillTable() {
   _pParent->clearResult();
   mTableView.refillTable(mResultList);
   mTableView.setSelectedRow(0);
   // update view to table
   doCopyLineToDialog();
   updateDockingDlg();
}

bool FindDlg::doLoadConfigFile(bool bAppend, bool bLoadNew) {
   OPENFILENAME ofn;       // common dialog box structure
   // Initialize OPENFILENAME
   TCHAR szFile[MAX_PATH]=TEXT("");       // buffer for file name
   if (_lastConfigFiles.size()) {
      (void)generic_strncpy(szFile, _lastConfigFiles[0].c_str(), COUNTCHAR(szFile));
      szFile[COUNTCHAR(szFile)-1]=0;
   }
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = _hSelf;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = COUNTCHAR(szFile);
   ofn.lpstrFilter = TEXT("All\0*.*\0XML Files\0*.xml\0");
   ofn.nFilterIndex = 2;
   ofn.lpstrFileTitle = TEXT("Open Analyse Config File");
   ofn.nMaxFileTitle = 0;//strlen("Open Analyse Config File")+1;
   ofn.lpstrInitialDir = szFile;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   if (GetOpenFileName(&ofn)==TRUE) 
   {   
      // happens because of pointer in ofn      strncpy(szFile, ofn.lpstrFile, MAX_PATH);
       setConfigFileName(ofn.lpstrFile);
      return loadConfigFile(ofn.lpstrFile, bAppend, bLoadNew);
   }
   return false;
}

bool FindDlg::doStoreConfigFile(){
   OPENFILENAME ofn;       // common dialog box structure
   TCHAR szFile[MAX_PATH]=TEXT("");       // buffer for file name
   (void)generic_strncpy(szFile, getszConfigFileName(), COUNTCHAR(szFile));
   szFile[COUNTCHAR(szFile)-1]=0;
   
   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = _hSelf;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = COUNTCHAR(szFile);
   ofn.lpstrFilter = TEXT("All\0*.*\0XML Files\0*.xml\0");
   ofn.nFilterIndex = 2;
   ofn.lpstrFileTitle = TEXT("Save Analyse Config File");
   ofn.nMaxFileTitle = 0;// strlen("Open Analyse Config File")+1;
   ofn.lpstrInitialDir = szFile;
   ofn.Flags = OFN_OVERWRITEPROMPT; // OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   if (GetSaveFileName(&ofn)==TRUE) 
   {   
      int l = (int)generic_strlen(ofn.lpstrFile);
      if(l > 3 && l < ((int)ofn.nMaxFile-5)) {
         if(!(((ofn.lpstrFile[l-1]^0x20)=='L') &&
            ((ofn.lpstrFile[l-2]^0x20)=='M') &&
            ((ofn.lpstrFile[l-3]^0x20)=='X') &&
            ((ofn.lpstrFile[l-4]     )=='.')))
         {
            // extension is missing
            if (generic_strlen(ofn.lpstrFile) < MAX_PATH - 4){
               // doit only if place enough
               generic_strncpy(&ofn.lpstrFile[l], TEXT(".xml"),5); // bcause of \0
            } else {
               DBG1("doStoreConfigFile() path was too long couldn't add ext! %s", ofn.lpstrFile);
            }
         }
      }
      setConfigFileName(ofn.lpstrFile);
      return saveConfigFile(ofn.lpstrFile);
   }
   return false;
}

BOOL FindDlg::notify(SCNotification *notification) {
   BOOL ret = 0; // true if message processed
   if(notification == 0) return ret;

   switch (notification->nmhdr.code) 
   {
   case NM_RCLICK:
      {
         LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) notification;
         DBG1("NM_RCLICK row %d", pItem->iItem);
         POINT pt = pItem->ptAction;
         ::ClientToScreen(mTableView.getListViewHandle(), &pt);
         ContextMenu contextmenu;
         std::vector<MenuItemUnit> tmp;
         if(pItem->iItem >= 0) {
            tmp.push_back(MenuItemUnit(IDC_DO_TOGGLE_SEARCH, TEXT("Toggle this")));
            tmp.push_back(MenuItemUnit(IDC_BUT_DEL, TEXT("Delete this")));
            tmp.push_back(MenuItemUnit(IDC_BUT_MOVE_UP, TEXT("Move this up")));
            tmp.push_back(MenuItemUnit(IDC_BUT_MOVE_DOWN, TEXT("Move this down")));
         }
         tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
         tmp.push_back(MenuItemUnit(IDC_DO_ENABLE_ALL, TEXT("Enable All")));
         tmp.push_back(MenuItemUnit(IDC_DO_DISABLE_ALL, TEXT("Disable All")));
         tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
         tmp.push_back(MenuItemUnit(IDC_BUT_CLEAR, TEXT("Clear All")));

         contextmenu.create(_hSelf, tmp);
         contextmenu.display(pt);
         ret = false; // we want rclick to still continue with selection
         break;
      }
   case NM_CLICK:
      {
         LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) notification;
         DBG1("NM_CLICK row %d", pItem->iItem);
         if (pItem->iItem != -1) {
            doCopyLineToDialog(); 
            ret = true;
         }
         break;
      }
   case NM_DBLCLK:
      {
         doToggleToSearch();
         ret = true;
         break;
      }
      //case LVN_ITEMACTIVATE:
      //   {
      //      DBG0("LVN_ITEMACTIVATE");
      //      // listview item double clicked.
      //      // copy line to dialog
      //      doCopyLineToDialog(); 
      //      ret = true;
      //      break;
      //   }
   //case LVN_ITEMCHANGING:
   //   {
   //      LPNMLISTVIEW pnmv = (LPNMLISTVIEW) notification;
   //      DBG2("LVN_ITEMCHANGING item %d subitem %d", pnmv->iItem, pnmv->iSubItem);  
   //      break;
   //   }
   case NM_CUSTOMDRAW:
      {// NMCUSTOMDRAW 
         LPNMLVCUSTOMDRAW pItem = (LPNMLVCUSTOMDRAW) notification;
         if( notification->nmhdr.hwndFrom == mTableView.getListViewHandle()) {
            //|| pItem->nmcd.hdr.hwndFrom == g_hList) {
               //DBG1("NM_CUSTOMDRAW Table reached %s",((pItem->nmcd.hdr.hwndFrom == g_hList)?"Test":"List"));
            const char* cp;
            const char* cp1;

            switch(pItem->nmcd.uItemState) {
               case CDIS_CHECKED:cp="CDIS_CHECKED";break;
               case CDIS_DEFAULT:cp="CDIS_DEFAULT";break;
               case CDIS_DISABLED:cp="CDIS_DISABLED";break;
               case CDIS_FOCUS:cp="CDIS_FOCUS";break;
               case CDIS_GRAYED:cp="CDIS_GRAYED";break;
               case CDIS_HOT:cp="CDIS_HOT";break;
               case CDIS_INDETERMINATE:cp="CDIS_INDETERMINATE";break;
               case CDIS_MARKED:cp="CDIS_MARKED";break;
               case CDIS_SELECTED:cp="CDIS_SELECTED";break;
               //case CDIS_SHOWKEYBOARDCUES:cp="CDIS_SHOWKEYBOARDCUES";break;
               default:cp="default";break;
            };

            switch(pItem->nmcd.dwDrawStage) {
               case CDDS_PREPAINT  :cp1="CDDS_PREPAINT  "; 
                  ret = (CDRF_NOTIFYITEMDRAW); 
                  break;
               case CDDS_POSTPAINT :cp1="CDDS_POSTPAINT "; break;
               case CDDS_PREERASE  :cp1="CDDS_PREERASE  "; break;
               case CDDS_POSTERASE :cp1="CDDS_POSTERASE "; break;
               case CDDS_ITEMPREPAINT :cp1="CDDS_ITEMPREPAINT ";
               {
                  //SelectObject(pItem->nmcd.hdc,
                  //             GetFontForItem(pItem->nmcd.dwItemSpec,
                  //                            pItem->nmcd.lItemlParam) );
                  //pItem->clrText = GetColorForItem(pItem->nmcd.dwItemSpec,
                  //                                 pItem->nmcd.lItemlParam);
                  //DBG3("CDDS_ITEMPREPAINT clrText (%x,%x,%x)", GetRValue(pItem->clrText),
                  //   GetGValue(pItem->clrText), GetBValue(pItem->clrText));
                  //pItem->clrTextBk = GetBkColorForItem(pItem->nmcd.dwItemSpec,
                  //                                     pItem->nmcd.lItemlParam);
                  int iPattern = (int)pItem->nmcd.dwItemSpec;
                  const tclPattern& pat = mResultList.getPattern(mResultList.getPatternId(iPattern));
                  pItem->clrText = pat.getColorNum();  
                  pItem->clrTextBk = pat.getBgColorNum();
                  ret = CDRF_NEWFONT; 
                  break; 
               }
               case CDDS_ITEMPOSTPAINT:cp1="CDDS_ITEMPOSTPAINT"; break;
               case CDDS_ITEMPREERASE :cp1="CDDS_ITEMPREERASE "; break;
               case CDDS_ITEMPOSTERASE:cp1="CDDS_ITEMPOSTERASE"; break;
               default:cp1="default"; break; 
            };
            DBGA3("NM_CUSTOMDRAW for item %d : %s | %s", (int)pItem->nmcd.dwItemSpec, cp1, cp);
         } else { // if handle correct
            //DBG0("NM_CUSTOMDRAW for different element");
         }
         //CDRF_NOTIFYITEMDRAW; // CDRF_DODEFAULT;
         break;
      } // case NM_CUSTOMDRAW
   default :
      break;
   } // switch
#ifdef _DEBUG_OFF
   const char* code;
   char num[10];
   //NMLVSCROLL nm;
   switch (notification->nmhdr.code) {
      case NM_OUTOFMEMORY     : code="NM_OUTOFMEMORY    "; break;
      case NM_CLICK           : code="NM_CLICK          "; break;
      case NM_DBLCLK          : code="NM_DBLCLK         "; break;
      case NM_RETURN          : code="NM_RETURN         "; break;
      case NM_RCLICK          : code="NM_RCLICK         "; break;
      case NM_RDBLCLK         : code="NM_RDBLCLK        "; break;
      case NM_SETFOCUS        : code="NM_SETFOCUS       "; break;
      case NM_KILLFOCUS       : code="NM_KILLFOCUS      "; break;
      case NM_CUSTOMDRAW      : code="NM_CUSTOMDRAW     "; break;
      case NM_HOVER           : code="NM_HOVER          "; break;
      case NM_NCHITTEST       : code="NM_NCHITTEST      "; break;
      case NM_KEYDOWN         : code="NM_KEYDOWN        "; break;
      case NM_RELEASEDCAPTURE : code="NM_RELEASEDCAPTURE"; break;
      case NM_SETCURSOR       : code="NM_SETCURSOR      "; break;
      case NM_CHAR            : code="NM_CHAR           "; break;
      case NM_TOOLTIPSCREATED : code="NM_TOOLTIPSCREATED"; break;
      case NM_LDOWN           : code="NM_LDOWN          "; break;
      case NM_RDOWN           : code="NM_RDOWN          "; break;
      case NM_THEMECHANGED    : code="NM_THEMECHANGED   "; break;
      case LVN_ITEMCHANGING   : code="LVN_ITEMCHANGING  "; break;
      case LVN_ITEMCHANGED    : code="LVN_ITEMCHANGED   "; break;   
      case LVN_INSERTITEM     : code="LVN_INSERTITEM    "; break;   
      case LVN_DELETEITEM     : code="LVN_DELETEITEM    "; break;   
      case LVN_DELETEALLITEMS : code="LVN_DELETEALLITEMS"; break;   
      case LVN_BEGINLABELEDITA: code="LVN_BEGINLABELEDITA"; break;  
      case LVN_BEGINLABELEDITW: code="LVN_BEGINLABELEDITW"; break;  
      case LVN_ENDLABELEDITA  : code="LVN_ENDLABELEDITA "; break;   
      case LVN_ENDLABELEDITW  : code="LVN_ENDLABELEDITW "; break;   
      case LVN_COLUMNCLICK    : code="LVN_COLUMNCLICK   "; break;   
      case LVN_BEGINDRAG      : code="LVN_BEGINDRAG     "; break;   
      case LVN_BEGINRDRAG     : code="LVN_BEGINRDRAG    "; break;   
      case LVN_ODCACHEHINT    : code="LVN_ODCACHEHINT   "; break; 
      case LVN_ODFINDITEMA    : code="LVN_ODFINDITEMA   "; break; 
      case LVN_ODFINDITEMW    : code="LVN_ODFINDITEMW   "; break; 
      case LVN_ITEMACTIVATE   : code="LVN_ITEMACTIVATE  "; break; 
      case LVN_ODSTATECHANGED : code="LVN_ODSTATECHANGED"; break; 
      case LVN_HOTTRACK       : code="LVN_HOTTRACK      "; break;
      case LVN_GETDISPINFOA   : code="LVN_GETDISPINFOA  "; break;
      case LVN_GETDISPINFOW   : code="LVN_GETDISPINFOW  "; break;
      case LVN_SETDISPINFOA   : code="LVN_SETDISPINFOA  "; break;
      case LVN_SETDISPINFOW   : code="LVN_SETDISPINFOW  "; break;
      case LVN_KEYDOWN        : code="LVN_KEYDOWN       "; break;
      case LVN_MARQUEEBEGIN   : code="LVN_MARQUEEBEGIN  "; break;
      case LVN_GETINFOTIPA    : code="LVN_GETINFOTIPA   "; break;
      case LVN_GETINFOTIPW    : code="LVN_GETINFOTIPW   "; break;
      case LVN_BEGINSCROLL    : code="LVN_BEGINSCROLL   "; break;
      case LVN_ENDSCROLL      : code="LVN_ENDSCROLL     "; break;
      
      default                : code=itoa(notification->nmhdr.code, num, 16); break;
   };
   DBG2("notify() %s returns 0x%x", code, ret);
#endif
   return ret;
}

void FindDlg::setSearchHistory(const TCHAR* hist) {
   setCmbHistory(mCmbSearchText, hist, 2);
   //mCmbSearchText.addText2Combo(TEXT(""), false); 
   mCmbSearchText.clearSelection();
}
//void FindDlg::setSearchHistory(const char* hist, int charSize) {
//   setCmbHistory(mCmbSearchText, hist, charSize);
//}
void FindDlg::setCommentHistory(const TCHAR* hist) {
   setCmbHistory(mCmbComment, hist, 2);
   //mCmbComment.addText2Combo(TEXT(""), false); // add empty line to put first comment to empty as default
   mCmbComment.clearSelection();
}
//void FindDlg::setCommentHistory(const char* hist, int charSize) {
//   setCmbHistory(mCmbComment, hist, charSize);
//   mCmbComment.addText2Combo(TEXT(""), false); // add empty line to put first comment to empty as default
//}
void FindDlg::setCmbHistory(tclComboBoxCtrl& thisCmb, const TCHAR* hist, int charSize) {
   if(hist==0) return;
   TCHAR szPart[MAX_CHAR_HISTORY];
   if(charSize == 1) {
      assert(0); // not supported any more
      //(void)strncpy(szPart, hist, sizeof(szPart)-1);
   } else if(charSize == 2) {
      // hist is in fact TCHAR
      TCHAR* wHist = (TCHAR*)hist;
      int j = (int)generic_strlen(wHist)+1; 
      j = (j>MAX_CHAR_HISTORY)?MAX_CHAR_HISTORY:j;
      for(int i=0; i<j; ++i) {szPart[i] = (TCHAR)wHist[i];}
   } else {
      return;
   }
   szPart[_countof(szPart)-1] =0;
   TCHAR* pcEnd=szPart+generic_strlen(szPart);
   TCHAR* pc=szPart;
   while(pc<pcEnd) {
      if(*pc=='|'){
         if(*(pc+1)=='|') {
            TCHAR* pcEsc = pc;
            do {
               *pcEsc = *(pcEsc+1);
               ++pcEsc;
            }while(*pcEsc);
            --pcEnd;
           // ++pc;
         } else {
            *pc = 1;
         }
      }
      ++pc;
   }
   TCHAR* szToken = generic_strtok(szPart, TEXT("\x01"));
   while(szToken) {
      thisCmb.addText2Combo(szToken, false, false);
      szToken = generic_strtok(NULL, TEXT("\x01")); // next token
   }
}

void FindDlg::getSearchHistory(generic_string& str) const {
   str = mCmbSearchText.getComboTextList(false);
}

void FindDlg::getCommentHistory(generic_string& str) const {
   str = mCmbComment.getComboTextList(false);
}

generic_string FindDlg::getSearchHistory() const {
   return mCmbSearchText.getComboTextList(false);
}

generic_string FindDlg::getCommentHistory() const {
   return mCmbComment.getComboTextList(false);
}

//void FindDlg::getCommentHistory(std::wstring& str) const {
//   generic_string s = mCmbComment.getComboTextList(false);
//   int i = (int)s.length();
//   str.resize(i, 0);
//   for(; i>=0; --i) {
//      str[i] = (TCHAR)(s[i]&0xff);
//   }
//}

//void FindDlg::getSearchHistory(std::wstring& str) const {
//   generic_string s = mCmbSearchText.getComboTextList(false);
//   int i = (int)s.length();
//   str.resize(i, 0);
//   for(; i>=0; --i) {
//      str[i] = (TCHAR)(s[i]&0xff);
//   }
//}

//void FindDlg::setDefaultOptions(const wchar_t* options) {
//   // deferr into the other func to implement decoder only once
//   setDefaultOptions((const char*)options, 2);
//}

void FindDlg::setDefaultOptions(const TCHAR* options, int charSize) {
   if(options==0) return;
   TCHAR szPart[MAX_CHAR_HISTORY];
   if(charSize == 1) {
      assert(0); // not supported anymore
      //(void)strncpy(szPart, options, sizeof(szPart)-1);
   } else if(charSize == 2) {
      // this is in fact TCHAR
      const TCHAR* wOptions = options;
      int j = (int)generic_strlen(wOptions)+1; 
      j = (j>MAX_CHAR_HISTORY)?MAX_CHAR_HISTORY:j;
      for(int i=0; i<j; ++i) {
         szPart[i] = wOptions[i];
      }
   } else return;
   szPart[_countof(szPart)-1] =0;
   TCHAR* szToken = generic_strtok(szPart, TEXT(","));
   int num = 0;
   while(szToken) {
      switch(num) {
         case 0: 
            mDefPat.setSearchTypeStr(szToken); break;
         case 1: 
            mDefPat.setMatchCaseStr(szToken); break;
         case 2:
            mDefPat.setWholeWordStr(szToken); break;
         case 3:
            mDefPat.setDoSearchStr(szToken); break;
         case 4:
            mDefPat.setHideTextStr(szToken); break;
         case 5:
            mDefPat.setColorStr(szToken); break;
         case 6:
            mDefPat.setSelectionTypeStr(szToken); break;
         case 7:
            mDefPat.setBgColorStr(szToken); break;
         default:;
      };
      szToken = generic_strtok(NULL, TEXT(",")); // next token
      ++num;
   } // while
   setDialogData(mDefPat);
}

void FindDlg::getDefaultOptions(generic_string& str) {
   str = getDefaultOptions();
}

//void FindDlg::getDefaultOptions(std::wstring& str) {
//   generic_string s = getDefaultOptions();
//   str.resize(s.length(), 0);
//   for(int i = (int)s.length(); i>=0; --i) {
//      str[i] = (TCHAR)(unsigned)(s[i]&0xff);
//   }
//}

generic_string FindDlg::getDefaultOptions() const {
   generic_string s(mDefPat.getSearchTypeStr());
   s += ',';
   s += mDefPat.getMatchCaseStr();
   s += ',';
   s += mDefPat.getWholeWordStr();
   s += ',';
   s += mDefPat.getDoSearchStr();
   s += ',';
   s += mDefPat.getHideTextStr();
   s += ',';
   s += mDefPat.getColorStr();
   s += ',';
   s += mDefPat.getSelectionTypeStr();
   s += ',';
   s += mDefPat.getBgColorStr();
   return s;
}

void FindDlg::doToggleToSearch() {
   if(mTableView.getSelectedRow()>=0) {
      bool bDoSearch = !(mTableView.getDoSearchStr() == TEXT("X"));
      ::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_SETCHECK, bDoSearch?BST_CHECKED:BST_UNCHECKED, 0);
      doCopyDialogToLine();
   }
}

void FindDlg::doCopyLineToDialog() {
   if(mTableView.getSelectedRow()>=0) {
      bool bDoSearch = (mTableView.getDoSearchStr() == TEXT("X"));
      bool bHide = (mTableView.getHideStr() == TEXT("X"));
      bool bWord = (mTableView.getWholeWordStr() == TEXT("X"));
      bool bCase = (mTableView.getMatchCaseStr() == TEXT("X"));
      ::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_SETCHECK, bDoSearch?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_SETCHECK, bWord?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_SETCHECK, bCase?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_HIDE, BM_SETCHECK, bHide?BST_CHECKED:BST_UNCHECKED, 0);
      mCmbSearchText.addText2Combo(mTableView.getSearchTextStr().c_str(), false);
      mCmbComment.addText2Combo(mTableView.getCommentStr().c_str(), false);
      mCmbSearchType.addText2Combo(mTableView.getSearchTypeStr().c_str(), false, false, false);
      mCmbSelType.addText2Combo(mTableView.getSelectStr().c_str(), false, false, false);
#ifdef RESULT_STYLING
      bool bBold = (mTableView.getBoldStr() == TEXT("X"));
      bool bUnder = (mTableView.getUnderlinedStr() == TEXT("X"));
      bool bItalic = (mTableView.getItalicStr() == TEXT("X"));
      ::SendDlgItemMessage(_hSelf, IDC_CHK_BOLD, BM_SETCHECK, bBold?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_UNDERLINED, BM_SETCHECK, bUnder?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_ITALIC, BM_SETCHECK, bItalic?BST_CHECKED:BST_UNCHECKED, 0);
#endif
#ifdef RESULT_COLORING
//      mCmbColor.addText2Combo(mTableView.getColorStr().c_str(), false);
      _pFgColour->setColour(tclPattern::convColorStr2Num(mTableView.getColorStr()));
      _pFgColour->redraw();
      _pBgColour->setColour(tclPattern::convColorStr2Num(mTableView.getBgColorStr()));
      _pBgColour->redraw();
#endif
   }
}

void FindDlg::doCopyDialogToLine() {
   if(mTableView.getSelectedRow()>=0) {
      tclPattern p(mDefPat);
      // we expect that the line to be updated is already selected...
      p.setDoSearch(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_GETCHECK, 0, 0));
      p.setWholeWord(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_GETCHECK, 0, 0));
      p.setMatchCase(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_GETCHECK, 0, 0));
      p.setSearchText(mCmbSearchText.getTextFromCombo(false));
      p.setComment(mCmbComment.getTextFromCombo(false));
      p.setSearchTypeStr(mCmbSearchType.getTextFromCombo(false));
      p.setHideText(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_HIDE, BM_GETCHECK, 0, 0));
      p.setSelectionTypeStr(mCmbSelType.getTextFromCombo(false));
#ifdef RESULT_STYLING
      p.setBold(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_BOLD, BM_GETCHECK, 0, 0));
      p.setUnderlined(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_UNDERLINED, BM_GETCHECK, 0, 0));
      p.setItalic(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_ITALIC, BM_GETCHECK, 0, 0));
#endif
#ifdef RESULT_COLORING
      //p.setColorStr(mCmbColor.getTextFromCombo(false));
      p.setColor(p.convColorNum2Enum(_pFgColour->getColour()));
      p.setBgColor(p.convColorNum2Enum(_pBgColour->getColour()));
#endif
      mTableView.setRowItems(p);
      mResultList.setPattern(mResultList.getPatternId(mTableView.getSelectedRow()), p);
      // inform parent to let other know the status
      _pParent->updateSearchPatterns();
      // insert text to text history
      mCmbSearchText.addText2Combo(mCmbSearchText.getTextFromCombo(false).c_str(), false);
      mCmbComment.addText2Combo(mCmbComment.getTextFromCombo(false).c_str(), false);
   }
}

bool FindDlg::isSearchTextEqual() {
   return (mCmbSearchText.getTextFromCombo(false) == mTableView.getSearchTextStr());
}

bool FindDlg::isPatternEqual() {
   if(mTableView.getSelectedRow()>=0) {
      tclPattern p(mDefPat);

      p.setDoSearch(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_GETCHECK, 0, 0));
      p.setWholeWord(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_GETCHECK, 0, 0));
      p.setMatchCase(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_GETCHECK, 0, 0));
      p.setSearchText(mCmbSearchText.getTextFromCombo(false));
      p.setSearchTypeStr(mCmbSearchType.getTextFromCombo(false));
      p.setHideText(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_HIDE, BM_GETCHECK, 0, 0));
      p.setSelectionTypeStr(mCmbSelType.getTextFromCombo(false));
      p.setComment(mCmbComment.getTextFromCombo(false));
#ifdef RESULT_STYLING
      p.setBold(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_BOLD, BM_GETCHECK, 0, 0));
      p.setUnderlined(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_UNDERLINED, BM_GETCHECK, 0, 0));
      p.setItalic(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_ITALIC, BM_GETCHECK, 0, 0));
#endif
#ifdef RESULT_COLORING
      //p.setColorStr(mCmbColor.getTextFromCombo(false));
      p.setColor(p.convColorNum2Enum(_pFgColour->getColour()));
      p.setBgColor(p.convColorNum2Enum(_pBgColour->getColour()));
#endif

      bool b = true;
      b &= (p.getDoSearch() == (mTableView.getDoSearchStr() == TEXT("X")));
      b &= (p.getIsHideText() == (mTableView.getHideStr() == TEXT("X")));
      b &= (p.getIsWholeWord() == (mTableView.getWholeWordStr() == TEXT("X")));
      b &= (p.getIsMatchCase() == (mTableView.getMatchCaseStr() == TEXT("X")));
      b &= (p.getSearchText() == mTableView.getSearchTextStr());
      b &= (p.getComment() == mTableView.getCommentStr());
      b &= (p.getSearchTypeStr() == mTableView.getSearchTypeStr());
      b &= (p.getSelectionTypeStr() == mTableView.getSelectStr());
      b &= (p.getColorStr() == mTableView.getColorStr());
      b &= (p.getBgColorStr() == mTableView.getBgColorStr());
      return b;
   } else {
      return false;
   }
}

void FindDlg::addTextPattern(const TCHAR* pzsText) {
   mCmbSearchText.addText2Combo(pzsText, false);
   ::SendMessage(getHSelf(), WM_COMMAND, IDC_BUT_ADD, (LPARAM)0);
}

//void FindDlg::init(HINSTANCE hInst, HWND parent){
void FindDlg::init(HINSTANCE hInst, NppData nppData){
   DockingDlgInterface::init(hInst, nppData._nppHandle);
}

void FindDlg::create(tTbData * data, bool isRTL){
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
   data->pszAddInfo = _ConfigFileName;

   // supported features by plugin
   data->uMask	= (DWS_DF_CONT_RIGHT | DWS_PARAMSALL);
   //RECT rect;
   //::GetClientRect(_hSelf, &rect);
   //data->rcFloat = rect;
   //data->rcFloat.right += 300;
   // TODO find a solution for being taler as default
   // store for live time
//   _data = data;
   HWND hList = ::GetDlgItem(_hSelf, IDC_LIST_CONF);
   mTableView.setListViewHandle(hList);
   mTableView.create();
   mCmbSearchText.init(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_TEXT));
   mCmbComment.init(::GetDlgItem(_hSelf, IDC_CMB_COMMENT));
   mCmbSearchType.init(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_TYPE));
   mCmbSelType.init(::GetDlgItem(_hSelf, IDC_CMB_SELECTION));
#ifdef RESULT_COLORING
   //mCmbColor.init(::GetDlgItem(_hSelf, IDC_CMB_COLOR));
   _pFgColour = new ColourPicker2;
   _pBgColour = new ColourPicker2;
   _pFgColour->init(_hInst, _hSelf);
   _pBgColour->init(_hInst, _hSelf);
   _pFgColour->setColour(mDefPat.getColorNum());
   _pBgColour->setColour(mDefPat.getBgColorNum());

   POINT p1, p2;

   alignWith(::GetDlgItem(_hSelf, IDC_STATIC_COL_FG), _pFgColour->getHSelf(), PosAlign::right, p1);
   alignWith(::GetDlgItem(_hSelf, IDC_STATIC_COL_BG), _pBgColour->getHSelf(), PosAlign::right, p2);

   p1.y += 1;
   p2.y += 1;
   DBG2("ColorPicker MoveWindow _pFgColour to: x:%d y:%d", p1.x, p1.y);
   ::MoveWindow((HWND)_pFgColour->getHSelf(), p1.x, p1.y, 12, 12, TRUE);
   DBG2("ColorPicker MoveWindow _pBgColour to: x:%d y:%d", p2.x, p2.y);
   ::MoveWindow((HWND)_pBgColour->getHSelf(), p2.x, p2.y, 12, 12, TRUE);
   _pFgColour->display();
   _pBgColour->display();
#endif
   setDialogData(mDefPat);

   _pPlsWait = new PleaseWaitDlg(_hSelf);

}
void FindDlg::setDialogData(const tclPattern& p) {
   // fill combos
   mCmbSearchType.addInitialText2Combo(p.getDefSearchTypeListSize(), p.getDefSearchTypeList(), false);
   mCmbSelType.addInitialText2Combo(p.getDefSelTypeListSize(), p.getDefSelTypeList(), false);
#ifdef RESULT_COLORING
//   mCmbColor.addInitialText2Combo(p.getDefColorListSize(), p.getDefColorList(), false);
#endif
   // set to default values
   mCmbSearchType.addText2Combo(p.getSearchTypeStr().c_str(), false, false, false);
   mCmbSelType.addText2Combo(p.getSelectionTypeStr().c_str(), false, false, false);
   // strings are by default empty and shall not get always a blank string
   //mCmbSearchText.addText2Combo(p.getSearchText().c_str(), false);
   //mCmbComment.addText2Combo(p.getComment().c_str(), false);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_SETCHECK, p.getDoSearch()?BST_CHECKED:BST_UNCHECKED, 0);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_HIDE, BM_SETCHECK, p.getIsHideText()?BST_CHECKED:BST_UNCHECKED, 0);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_SETCHECK, p.getIsWholeWord()?BST_CHECKED:BST_UNCHECKED, 0);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_SETCHECK, p.getIsMatchCase()?BST_CHECKED:BST_UNCHECKED, 0);
#ifdef RESULT_COLORING
   //mCmbColor.addText2Combo(p.getColorStr().c_str(), false);
    _pFgColour->setColour(p.getColorNum());
   _pFgColour->redraw();
    _pBgColour->setColour(p.getBgColorNum());
   _pBgColour->redraw();
#endif
#ifdef RESULT_STYLING
   ::SendDlgItemMessage(_hSelf, IDC_CHK_BOLD, BM_SETCHECK, p.getIsBold()?BST_CHECKED:BST_UNCHECKED, 0);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_UNDERLINED, BM_SETCHECK, p.getIsUnderlined()?BST_CHECKED:BST_UNCHECKED, 0);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_ITALIC, BM_SETCHECK, p.getIsItalic()?BST_CHECKED:BST_UNCHECKED, 0);
#endif
}

void FindDlg::display(bool toShow) const {
   if (toShow == isVisible()) return;
   DockingDlgInterface::display(toShow);
   ::EnableWindow(_pFgColour->getHSelf(), toShow);
   ::EnableWindow(_pBgColour->getHSelf(), toShow);
   _pParent->visibleChanged(toShow);
}

void FindDlg::setAllDoSearch(bool bOn) {
   
   tclPatternList::iterator iPatt = refPatternList().begin();
   for (; iPatt != refPatternList().end(); ++iPatt) {
      iPatt.refPattern().setDoSearch(bOn);
   }      
   mTableView.refillTable(refPatternList());
   doCopyLineToDialog();
   updateDockingDlg();
}

void FindDlg::SetModified(bool bModified) {
   if (bModified) {
      if ( mResultList.size() ) {
         if (_ModifiedMode == 0) {
            _ModifiedHwnd = getHSelf();
            if (_ModifiedHwnd){
               _ModifiedMode = 1;
               SetTimer(_ModifiedHwnd,IDT_ANALYSEPLG_TIMER, 500, (TIMERPROC) &FindDlg::MyTimerProc);
               DBG0("FindDlg::SetModified started");
            } else {
               DBG0("FindDlg::SetModified started");
            }
         } else if(_ModifiedMode == 1) {
            // set for wating
            DBG0("FindDlg::SetModified wait");
            _ModifiedMode = 2;
         } else {
            DBG0("FindDlg::SetModified nop");
         }
      } else {
         DBG0("FindDlg::SetModified while no search data");
      }
   } else {
      DBG0("FindDlg::SetModified kill timer");
      KillTimer(_ModifiedHwnd,IDT_ANALYSEPLG_TIMER);
   } 
}

void CALLBACK FindDlg::MyTimerProc( 
    HWND /*hwnd*/,        // handle to window for timer messages 
    UINT /*message*/,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD /*dwTime*/)     // current system time 
{ 
   DBG2("MyTimerProc() _ModifiedMode=%d, correct ID? %s",_ModifiedMode,(idTimer == IDT_ANALYSEPLG_TIMER)?"YES":"NO");
   if(idTimer == IDT_ANALYSEPLG_TIMER) {
      if ( _ModifiedMode == 1) {
         DBG0("MyTimerProc activate search");
         ::PostMessage(_ModifiedHwnd, WM_COMMAND, IDC_DO_RESEARCH, (LPARAM)0);
         _ModifiedMode = 0;
         KillTimer(_ModifiedHwnd,IDT_ANALYSEPLG_TIMER);
      } else if (_ModifiedMode == 2) {
         _ModifiedMode = 1;
      }
   }
} 

void FindDlg::activatePleaseWait(bool isEnable) {
   if(_pPlsWait) {
      _pPlsWait->activate(isEnable);
   }
}

void FindDlg::setPleaseWaitRange(int iMin, int iMax) {
   if(_pPlsWait) {
      _pPlsWait->setProgressRange(iMin, iMax);
   }
}

void FindDlg::setPleaseWaitProgress(int iCurr) {
   if(_pPlsWait) {
      _pPlsWait->setProgressPos(iCurr);
   }
}

bool FindDlg::getPleaseWaitCanceled() {
   if(_pPlsWait) {
      return _pPlsWait->getCanceled();
   }
   return false;
}
