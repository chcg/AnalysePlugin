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

#include <windows.h>
#include "StaticDialog.h"
#include "resource.h"
#include "tclComboBoxCtrl.h"
#include "ScintillaSearchView.h"
#include "tclPattern.h"
#include <string>
#include "tclFindResultSearchDlg.h"

#define MDBG_COMP "FnResDlg:" 
#include "myDebug.h"


void tclFindResultSearchDlg::init(HINSTANCE hInst, HWND hPere, ScintillaSearchView* pSearchResultView) 
{
   Window::init(hInst, hPere);
   if (!pSearchResultView){
      MessageBox( NULL, TEXT("System Error no scintilla pointer"), TEXT("tclFindResultSearchDlg ERROR : "), MB_OK | MB_ICONSTOP);
      throw int(9900);
   }
   _pSearchResultView = pSearchResultView;
}

void tclFindResultSearchDlg::create(int dialogID, bool isRTL) 
{
   StaticDialog::create(dialogID, isRTL);
   mhlvPatterns = ::GetDlgItem(_hSelf, IDC_LST_PATTERNS);
   ListView_SetExtendedListViewStyle(mhlvPatterns,
      LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
   LVCOLUMN lvc = {0};
   lvc.mask = LVCF_TEXT | LVCF_WIDTH;
   lvc.pszText = TEXT("Patterns");
   lvc.cx = 175;
   ListView_InsertColumn(mhlvPatterns, 0, &lvc);
};

/** called to show the dialog */
void tclFindResultSearchDlg::doDialog(bool isRTL) 
{
   if (!isCreated()) {
      create(IDD_FIND_RES_DLG_SEARCH, isRTL);
   }
   display();
}

void tclFindResultSearchDlg::setSearchPatterns(const tclPatternList& patterns) {
   mPatterns = patterns;
   updatePatternList();
}

void tclFindResultSearchDlg::display(bool toShow) 
{
   if (toShow){
      ::SetFocus(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_TEXT));

      _CmbSearchText.init(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_TEXT));
      _CmbSearchType.init(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_TYPE));
      _CmbSearchDir.init(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_DIR));

      // fill combos
      _CmbSearchType.addInitialText2Combo(mDefPat.getDefSearchTypeListSize(), mDefPat.getDefSearchTypeList(), false);
      // set to default values
      _CmbSearchType.addText2Combo(mDefPat.getSearchTypeStr().c_str(), false); 
      //_CmbSearchText.addText2Combo(mDefPat.getSearchText().c_str(), false); 
      ::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_SETCHECK, mDefPat.getIsWholeWord()?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_SETCHECK, mDefPat.getIsMatchCase()?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_RADIO_DIRDOWN , BM_SETCHECK, _bSearchDown?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_WRAP , BM_SETCHECK, _bDoWrap?BST_CHECKED:BST_UNCHECKED, 0);
      updatePatternList();
   }
   Window::display(toShow);
}

void tclFindResultSearchDlg::updatePatternList() {
   // add one first line
   ListView_DeleteAllItems(mhlvPatterns);
   LVITEM lvi;
   ZeroMemory(&lvi, sizeof(lvi));
   lvi.mask = LVIF_TEXT;
   lvi.iItem = 0;
   tclPatternList::const_iterator i = mPatterns.begin();
   for (;i != mPatterns.end(); ++i) {
      generic_string s = i.getPattern().getSearchText().c_str();
      lvi.pszText = (TCHAR*)s.c_str();
      lvi.iItem = ListView_GetItemCount(mhlvPatterns);
      ListView_InsertItem(mhlvPatterns, &lvi);
   }
}

int tclFindResultSearchDlg::doFindText(int start, int end/*, bool bDownWards*/) {
   if(_pSearchResultView==0) {
      DBG0("doFindText() ERROR no searchresult window");
      return -1;
   }
   tclPattern p;
   p.setWholeWord(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_GETCHECK, 0, 0));
   p.setMatchCase(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_GETCHECK, 0, 0));
   p.setSearchText(_CmbSearchText.getTextFromCombo(false));
   p.setSearchTypeStr(_CmbSearchType.getTextFromCombo(false));
   if(p.getSearchText().length()==0) {
      // empty string is found "every where" so we return directly with 0 
      DBG0("doFindText() don't search: empty search string.");
      ::MessageBox(_hSelf, TEXT("Search string empty!"), TEXT("Find in Result"), MB_OK);
      return -1;
   }
   // add text to history
   _CmbSearchText.addText2Combo(_CmbSearchText.getTextFromCombo(false).c_str(), false);
   // flags for the search 
   int flags =0;
   if(p.getSearchType()== tclPattern::regex) {
      flags |= (SCFIND_REGEXP|SCFIND_POSIX);
   }
   flags |= p.getIsMatchCase()?SCFIND_MATCHCASE:0;
   flags |= p.getIsWholeWord()?SCFIND_WHOLEWORD:0;
   _pSearchResultView->execute(SCI_SETSEARCHFLAGS, flags);
   // set search area
   _pSearchResultView->execute(SCI_SETTARGETSTART, start);
   _pSearchResultView->execute(SCI_SETTARGETEND, end);

   int targetStart=-1;
#ifdef UNICODE
   WcharMbcsConvertor *wmc = &WcharMbcsConvertor::getInstance();
   unsigned int cp = (unsigned int)_pSearchResultView->execute(SCI_GETCODEPAGE); 
   const char *text2FindA = wmc->wchar2char(p.getSearchText().c_str(), cp);
   size_t text2FindALen = strlen(text2FindA);
   targetStart = (int)_pSearchResultView->execute(SCI_SEARCHINTARGET, 
      (WPARAM)text2FindALen, 
      (LPARAM)text2FindA);
#else
   targetStart = (int)_pSearchResultView->execute(SCI_SEARCHINTARGET, 
      (WPARAM)p.getSearchText().size(), 
      (LPARAM)p.getSearchText().c_str());
#endif
   return targetStart;
}

void tclFindResultSearchDlg::doFindFirst() {
   DBG0("doFindFirst()");
   if(_pSearchResultView==0) {
      DBG0("doFindFirst() ERROR no searchresult window");
      return;
   }
   int startRange = 0; // from very begin
   int endRange = (int)_pSearchResultView->execute(SCI_GETLENGTH);
   if (endRange < 1) {
      DBG0("doFindFirst() don't search: document is empty.");
      ::MessageBox(_hSelf, TEXT("Result window empty!"), TEXT("Find in Result"), MB_OK);
      // nothing to do because that means the document is empty
      return; 
   } 
   //Initial range for searching
   DBG2("doFindFirst() initial tstart %d, tend %d.", startRange, endRange);

   // search the pattern
   startRange = doFindText(startRange, endRange);
   if(startRange != -1) {
      int targetEnd = (int)_pSearchResultView->execute(SCI_GETTARGETEND);
      markFoundText(startRange, targetEnd);
      DBGDEF(int lineNumber = (int)_pSearchResultView->execute(SCI_LINEFROMPOSITION, startRange);)
      DBGDEF(int foundTextLen = targetEnd - startRange;)
      DBG2("doFindFirst() text found in line %d with %d chars length", lineNumber, foundTextLen);
   } else {
      ::MessageBox(_hSelf, TEXT("Can' find the text!"), TEXT("Find in Result"), MB_OK);
   }
}

void tclFindResultSearchDlg::markFoundText(int targetStart, int targetEnd) {
   DBG0("markFoundText()");
   if (targetEnd < targetStart) {
      int tmp = targetStart;
      targetStart = targetEnd;
      targetEnd = tmp;
   }
   _pSearchResultView->execute(SCI_SETSEL, targetEnd, targetStart);
   int currentlineNumberDoc = (int)_pSearchResultView->execute(SCI_LINEFROMPOSITION, targetStart);
   int currentVisibleCenter =	(int)_pSearchResultView->execute(SCI_GETFIRSTVISIBLELINE);
   int linesVisible =			(int)_pSearchResultView->execute(SCI_LINESONSCREEN) - 1;	//-1 for the scrollbar
   currentVisibleCenter += linesVisible/2;
   int linesToScroll = currentlineNumberDoc - currentVisibleCenter;
   if (linesToScroll > 2 || linesToScroll < -2) {
      _pSearchResultView->execute(SCI_LINESCROLL, 0, linesToScroll);
   }
   SCNotification scn;
   memset(&scn, 0, sizeof(scn));
   scn.nmhdr.code = SCN_DOUBLECLICK;
   ::SendMessage(_pSearchResultView->getHParent(), WM_NOTIFY, 0, (LPARAM)&scn);
   ::SetFocus(_hSelf);
}

void tclFindResultSearchDlg::doFindNext(bool bSearchDown, bool bDoWrap) {
   DBG0("doFindNext()");
   if(_pSearchResultView==0) {
      DBG0("doFindNext() ERROR no searchresult window");
      return;
   }
   int anchor = (int)_pSearchResultView->execute(SCI_GETANCHOR);
   int length = (int)_pSearchResultView->execute(SCI_GETLENGTH);
   if (length < 1) {
      DBG0("doFindNext() don't search: document is empty.");
      ::MessageBox(_hSelf, TEXT("Result window empty!"), TEXT("Find in Result"), MB_OK);
      // nothing to do because that means the document is empty
      return; 
   } 
   int startRange = bSearchDown ? anchor : anchor-1; // up has to search backw. so start before last char
   int endRange = bSearchDown ? length:0;
   //Initial range for searching
   DBG2("doFindNext() with tstart %d, tend %d.", startRange, endRange);
   // search the pattern
   startRange = doFindText(startRange, endRange);
   if((startRange == -1) && bDoWrap) {
      if(bSearchDown) {
         startRange = 0;
      } else {
         startRange = length-1;
      }
      startRange = doFindText(startRange, endRange);
   }
   if(startRange != -1) {
      int targetEnd = (int)_pSearchResultView->execute(SCI_GETTARGETEND);
      markFoundText(startRange, targetEnd);
      DBGDEF(int lineNumber = (int)_pSearchResultView->execute(SCI_LINEFROMPOSITION, startRange);)
      DBGDEF(int foundTextLen = targetEnd - startRange;)
      DBG2("doFindNext() text found in line %d with %d chars length", lineNumber, foundTextLen);
   } else {
      ::MessageBox(_hSelf, TEXT("Can' find the text!"), TEXT("Find in Result"), MB_OK);
   } 
}

void tclFindResultSearchDlg::doCount() {
   DBG0("doCount()");
   int iCount=0;
   if(_pSearchResultView==0) {
      DBG0("doCount() ERROR no searchresult window");
      return;
   }
   int startRange = 0; // from very begin
   int endRange = (int)_pSearchResultView->execute(SCI_GETLENGTH);
   if (endRange < 1) {
      DBG0("doCount() don't search: document is empty.");
      // nothing to do because that means the document is empty
   } else {
      while (startRange >= 0) {
         startRange = doFindText(startRange, endRange);
         if (startRange >= 0){
            startRange = (int)_pSearchResultView->execute(SCI_GETTARGETEND);
            iCount++;
         }
      } // while
   }
   TCHAR* text = TEXT(" instances found.");
   TCHAR msg[100];
   _itow(iCount, msg, 10);
   wcsncpy(msg+generic_strlen(msg), text, 100-wcslen(text));
   ::MessageBox(_hSelf, msg, TEXT("Count Instances"), MB_OK);
}

void tclFindResultSearchDlg::setdefaultPattern(const tclPattern& p) {
   mDefPat = p;
}

INT_PTR CALLBACK tclFindResultSearchDlg::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
   //DBG1("run_dlgProc() message 0x%04x", Message);
   BOOL ret = 0; // true if message processed (2 incase of custom draw feature)
   switch (Message)
   {
   case WM_INITDIALOG :
      {
         // set button pos
         RECT rc;
         getClientRect(rc);
         mpPatt = getTopPoint(::GetDlgItem(_hSelf, IDC_LST_PATTERNS));
         RECT rcPatt;
         ::GetClientRect(::GetDlgItem(_hSelf, IDC_LST_PATTERNS), &rcPatt);
         mpPattDist.y = rc.bottom - rcPatt.bottom - mpPatt.y - 4; // 4 for border
         mpPattDist.x = rc.right - rcPatt.right- mpPatt.x - 4; // 4 for border
         return FALSE;
      }
   case WM_SIZE:
      {
         RECT rcDiag;
         getClientRect(rcDiag);

         HWND hList = ::GetDlgItem(_hSelf, IDC_LST_PATTERNS);
         if((rcDiag.right-mpPatt.x-mpPattDist.x) > 0) {
            ::MoveWindow(hList, mpPatt.x, mpPatt.y, rcDiag.right-mpPatt.x-mpPattDist.x, rcDiag.bottom-mpPatt.y-mpPattDist.y, FALSE);
         }
         // now repaint
         redraw();
      }
   case WM_COMMAND : 
      {
         switch (wParam)
         {
         case IDCANCEL :
            display(FALSE);
            return TRUE;

         case IDC_FINDFIRST:
            doFindFirst();
            return TRUE;
         case IDOK :
         case IDC_FINDNEXT:
            _bSearchDown = 0!=::SendDlgItemMessage(_hSelf, IDC_RADIO_DIRDOWN , BM_GETCHECK, 0, 0);
            _bDoWrap = 0!=::SendDlgItemMessage(_hSelf, IDC_CHK_WRAP , BM_GETCHECK, 0, 0);
            doFindNext(_bSearchDown, _bDoWrap);
            return TRUE;
         case IDC_COUNT:
            doCount();
            return TRUE;
         default :
            break;
         }
         break;
      }
   case WM_NOTIFY:
      {
         if (lParam) {
            SCNotification* pscn = (SCNotification *)lParam;
            switch(pscn->nmhdr.code)
            {
            case NM_CLICK:
               {
                  LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;
                  DBG1("NM_CLICK row %d", pItem->iItem);
                  // pattern in suchse übernehmen
                  tPatId i = mPatterns.getPatternId(pItem->iItem);
                  const tclPattern& rp = mPatterns.getPattern(i);
                  _CmbSearchText.addText2Combo(rp.getSearchText().c_str(), false);
                  _CmbSearchType.addText2Combo(rp.getSearchTypeStr().c_str(), false);
                  ::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_SETCHECK, rp.getIsWholeWord()?BST_CHECKED:BST_UNCHECKED, 0);
                  ::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_SETCHECK, rp.getIsMatchCase()?BST_CHECKED:BST_UNCHECKED, 0);

                  return TRUE;
               }
            case NM_CUSTOMDRAW:
            {// NMCUSTOMDRAW 
               LPNMLVCUSTOMDRAW pItem = (LPNMLVCUSTOMDRAW)pscn;
			   { // always if (pscn->nmhdr.hwndFrom == _hSelf) {
                  //|| pItem->nmcd.hdr.hwndFrom == g_hList) {
                  //DBG1("NM_CUSTOMDRAW Table reached %s",((pItem->nmcd.hdr.hwndFrom == g_hList)?"Test":"List"));
                  const char* cp;
                  const char* cp1;

                  switch (pItem->nmcd.uItemState) {
                  case CDIS_CHECKED:cp = "CDIS_CHECKED"; break;
                  case CDIS_DEFAULT:cp = "CDIS_DEFAULT"; break;
                  case CDIS_DISABLED:cp = "CDIS_DISABLED"; break;
                  case CDIS_FOCUS:cp = "CDIS_FOCUS"; break;
                  case CDIS_GRAYED:cp = "CDIS_GRAYED"; break;
                  case CDIS_HOT:cp = "CDIS_HOT"; break;
                  case CDIS_INDETERMINATE:cp = "CDIS_INDETERMINATE"; break;
                  case CDIS_MARKED:cp = "CDIS_MARKED"; break;
                  case CDIS_SELECTED:cp = "CDIS_SELECTED"; break;
                     //case CDIS_SHOWKEYBOARDCUES:cp="CDIS_SHOWKEYBOARDCUES";break;
                  default:cp = "default"; break;
                  };

                  switch (pItem->nmcd.dwDrawStage) {
                  case CDDS_PREPAINT:cp1 = "CDDS_PREPAINT  ";
                     return CDRF_NOTIFYITEMDRAW;
                  case CDDS_POSTPAINT:cp1 = "CDDS_POSTPAINT "; break;
                  case CDDS_PREERASE:cp1 = "CDDS_PREERASE  "; break;
                  case CDDS_POSTERASE:cp1 = "CDDS_POSTERASE "; break;
                  case CDDS_ITEMPREPAINT:cp1 = "CDDS_ITEMPREPAINT ";
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
                     const tclPattern& pat = mPatterns.getPattern(mPatterns.getPatternId(iPattern));
                     pItem->clrText = pat.getColorNum();
                     pItem->clrTextBk = pat.getBgColorNum();
                     ret = CDRF_NEWFONT;
                     break;
                  }
                  case CDDS_ITEMPOSTPAINT:cp1 = "CDDS_ITEMPOSTPAINT"; break;
                  case CDDS_ITEMPREERASE:cp1 = "CDDS_ITEMPREERASE "; break;
                  case CDDS_ITEMPOSTERASE:cp1 = "CDDS_ITEMPOSTERASE"; break;
                  default:cp1 = "default"; break;
                  };
                  DBGA3("NM_CUSTOMDRAW for item %d : %s | %s", (int)pItem->nmcd.dwItemSpec, cp1, cp);
               }
               //CDRF_NOTIFYITEMDRAW; // CDRF_DODEFAULT;
               break;
            } // case NM_CUSTOMDRAW
            default:
               break;
            } // switch(pscn->nmhdr.code)
         } // if
      } // WM_NOTIFY
   default:
      break;
   } // switch (Message)
   return FALSE;
} // run_dlgProc()

