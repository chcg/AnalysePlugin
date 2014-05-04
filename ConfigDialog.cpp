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
#include "menuCmdID.h"

#include "ConfigDialog.h"
#include "PluginInterface.h"
#include "resource.h"
#include <windows.h>
#include "tclPattern.h"

#define MDBG_COMP "CfgDlg:" 
#include "myDebug.h"

#define FONTSIZELIST_COUNT 11
const TCHAR * FONTSIZELIST[] = {TEXT("20"), TEXT("18"), TEXT("16"), TEXT("14"), TEXT("12"), TEXT("11"), TEXT("10"), TEXT("9"), TEXT("8"), TEXT("7"), TEXT("6") };
#define LISTLENGTHVALS_COUNT 2
const TCHAR * LISTLENGTHVALS[] = {TEXT("4"), TEXT("10")}; 

using namespace std;

void ConfigDialog::init(HINSTANCE hInst, NppData nppData)
{
   _nppData = nppData;
   Window::init(hInst, nppData._nppHandle);
   _addCtxDlg.init(hInst, nppData);
   setFontList(nppData._nppHandle);
}

void ConfigDialog::setFontList(HWND hWnd)
{
	//---------------//
	// Sys font list //
	//---------------//

	LOGFONT lf;
	mlsFontList.clear();

	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfFaceName[0]='\0';
	lf.lfPitchAndFamily = 0;
	HDC hDC = ::GetDC(hWnd);

	::EnumFontFamiliesEx(hDC, 
						&lf, 
						(FONTENUMPROC) EnumFontFamExProc, 
						(LPARAM) &mlsFontList, 0);
}

int ConfigDialog::EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *, int, LPARAM lParam) {
	tlsString* pStringSet = reinterpret_cast<tlsString*>(lParam);
	//We can add the font
	//Add the face name and not the full name, we do not care about any styles
   TCHAR* pName = (TCHAR*)lpelfe->elfLogFont.lfFaceName;
   
   generic_string str(pName);
   pStringSet->insert(str);

	return 1; // I want to get all fonts
};

void ConfigDialog::doDialog(int FuncCmdId)
{
   if (!isCreated()) {
      create(IDD_ANALYSE_CONF_DLG);
      mCmbOnEnterAction.init(::GetDlgItem(_hSelf, IDC_CMB_ONENTERACT));
      mCmbSearchType.init(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_TYPE));
//      mCmbSearchText.init(::GetDlgItem(_hSelf, IDC_CMB_SEARCH_TEXT));
      mCmbSelType.init(::GetDlgItem(_hSelf, IDC_CMB_SELECTION));
#ifdef RESULT_COLORING
      mCmbColor.init(::GetDlgItem(_hSelf, IDC_CMB_COLOR));
#endif
      mCmbFontName.init(::GetDlgItem(_hSelf, IDC_CMB_FONTNAME));
      mCmbFontSize.init(::GetDlgItem(_hSelf, IDC_CMB_FONTSIZE));
      mCmbNumOfCfgFiles.init(::GetDlgItem(_hSelf, IDC_CMB_NUMOFCFGFILES));
      // now set defaults
      mCmbOnEnterAction.addInitialText2Combo(max_onEnterAction, transOnEnterAction, false);
      mCmbSearchType.addInitialText2Combo(mDefPat.getDefSearchTypeListSize(), mDefPat.getDefSearchTypeList(), false);
      mCmbSelType.addInitialText2Combo(mDefPat.getDefSelTypeListSize(), mDefPat.getDefSelTypeList(), false);
#ifdef RESULT_COLORING
      mCmbColor.addInitialText2Combo(mDefPat.getDefColorListSize(), mDefPat.getDefColorList(), false);
#endif
      mCmbFontName.addInitialText2Combo(mlsFontList, false);
      mCmbFontSize.addInitialText2Combo(FONTSIZELIST_COUNT, FONTSIZELIST, false);
      mCmbNumOfCfgFiles.addInitialText2Combo(LISTLENGTHVALS_COUNT, LISTLENGTHVALS, false);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_USEBOOKMARK, BM_SETCHECK, getUseBookmark()?BST_CHECKED:BST_UNCHECKED, 0);
      // now get the stuff from configuration in the dialog 
      ::SendDlgItemMessage(_hSelf, IDC_CHK_AUTOUPDT, BM_SETCHECK, getOnAutoUpdate()?BST_CHECKED:BST_UNCHECKED, 0);
      mCmbOnEnterAction.addText2Combo(getOnEnterActionStr().c_str(), false);
      mCmbSearchType.addText2Combo(mDefPat.getSearchTypeStr().c_str(), false);
      mCmbSelType.addText2Combo(mDefPat.getSelectionTypeStr().c_str(), false);
//      mCmbSearchText.addText2Combo(mDefPat.getSearchText().c_str(), false);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_SETCHECK, mDefPat.getDoSearch()?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_HIDE, BM_SETCHECK, mDefPat.getIsHideText()?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_SETCHECK, mDefPat.getIsWholeWord()?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_SETCHECK, mDefPat.getIsMatchCase()?BST_CHECKED:BST_UNCHECKED, 0);
      ::SendDlgItemMessage(_hSelf, IDC_CHK_DISPLINENO, BM_SETCHECK, getDisplayLineNo()?BST_CHECKED:BST_UNCHECKED, 0);

#ifdef RESULT_COLORING
//      mCmbColor.addText2Combo(mDefPat.getColorStr().c_str(), false);
#endif
      const generic_string& s = _pParent->getResultFontName();
      if (!s.empty()) {
         mCmbFontName.addText2Combo(s.c_str(), false);
      }
      TCHAR tmp[10];
      generic_itoa(_pParent->getResultFontSize(),tmp, 10);
      mCmbFontSize.addText2Combo(tmp, false);
      generic_itoa(mNumOfCfgFiles,tmp, 10);
      mCmbNumOfCfgFiles.addText2Combo(tmp, false);
      DBG1("ConfigDialog::dDialog() %s ", mCmbNumOfCfgFiles.getComboTextList(false).c_str());
      _pFgColour = new ColourPicker2;
      _pBgColour = new ColourPicker2;
      _pFgColour->init(_hInst, _hSelf);
      _pBgColour->init(_hInst, _hSelf);
      _pFgColour->setColour(mDefPat.getColorNum());
      _pBgColour->setColour(mDefPat.getBgColorNum());

      POINT p1, p2;

      alignWith(::GetDlgItem(_hSelf, IDC_STATIC_COL_FG2), _pFgColour->getHSelf(), ALIGNPOS_RIGHT, p1);
      alignWith(::GetDlgItem(_hSelf, IDC_STATIC_COL_BG2), _pBgColour->getHSelf(), ALIGNPOS_RIGHT, p2);

      p1.y += 1;
      p2.y += 1;

      ::MoveWindow((HWND)_pFgColour->getHSelf(), p1.x, p1.y, 12, 12, TRUE);
      ::MoveWindow((HWND)_pBgColour->getHSelf(), p2.x, p2.y, 12, 12, TRUE);
      _pFgColour->display();
      _pBgColour->display();
   } // isCreated()

   ::EnableWindow(_pFgColour->getHSelf(), true);
   ::EnableWindow(_pBgColour->getHSelf(), true);
   mbOkPressed = false;
   _cmdId = FuncCmdId;
   // position dialog to the center of the screen
   goToCenter();
}

const TCHAR*  ConfigDialog::transOnEnterAction[max_onEnterAction] = {
   TEXT("just search"),
   TEXT("update line"),
   TEXT("add line")
};

generic_string ConfigDialog::getOnEnterActionStr() const {
   return generic_string(transOnEnterAction[mOnEnterAction]);
}

void ConfigDialog::setOnEnterActionStr(const generic_string& action) {
   for(int i=0; i<max_onEnterAction;i++) {
      if(action == transOnEnterAction[i]) {
         mOnEnterAction =(teOnEnterAction)i;
         break;
      }
   }
}


bool ConfigDialog::getDialogData(tclPattern& p) const{
   if (mbOkPressed) {
      p = mDefPat;
      return true;
   }else{
      return false;
   }
}

void ConfigDialog::setFontText(const generic_string& str) {
   mResultFontName = str;
}

const generic_string& ConfigDialog::getFontText() const {
   return mResultFontName;
}

void ConfigDialog::setFontSize(unsigned s) {
   if(s) {
      mResultFontSize = s;
   } else {
      DBG0("setFontSize() called with size 0 set it to 8");
      mResultFontSize = 8;
   }
}

void ConfigDialog::setFontSizeStr(const generic_string& s) {
   setFontSize(generic_atoi(s.c_str()));
}

unsigned ConfigDialog::getFontSize() const {
  return mResultFontSize;
}

generic_string ConfigDialog::getFontSizeStr() const {
   TCHAR cp[10];
   generic_itoa(mResultFontSize, cp, 10);
   return generic_string(cp);
}

const generic_string ConfigDialog::getNumOfCfgFilesStr() const {
   TCHAR cp[10];
   generic_itoa(mNumOfCfgFiles, cp, 10);
   return generic_string(cp);
}

void ConfigDialog::setNumOfCfgFilesStr(const generic_string& str) {
   mNumOfCfgFiles = generic_atoi(str.c_str());
}

void ConfigDialog::setDialogData(const tclPattern& p) {
   // store for returning
   mDefPat = p;

   // set to default values
   mCmbSearchType.addText2Combo(p.getSearchTypeStr().c_str(), false);
   mCmbSelType.addText2Combo(p.getSelectionTypeStr().c_str(), false);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_SETCHECK, p.getDoSearch()?BST_CHECKED:BST_UNCHECKED, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHK_HIDE, BM_SETCHECK, p.getIsHideText()?BST_CHECKED:BST_UNCHECKED, 0);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_SETCHECK, p.getIsWholeWord()?BST_CHECKED:BST_UNCHECKED, 0);
   ::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_SETCHECK, p.getIsMatchCase()?BST_CHECKED:BST_UNCHECKED, 0);
#ifdef RESULT_COLORING
   mCmbColor.addText2Combo(p.getColorStr().c_str(), false);
#endif
}

BOOL CALLBACK ConfigDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
   {
   case WM_INITDIALOG :
      {
         //  ::SendDlgItemMessage(_hSelf, IDC_EMAIL_LINK, WM_SETTEXT, 0, (LPARAM)EMAIL_LINK);
         return TRUE;
      }
   case WM_CLOSE :
      {
         ::SendMessage(_hParent, NPPM_SETMENUITEMCHECK, (WPARAM)_cmdId, (LPARAM)false);
         break;
      }

   case WM_COMMAND : 
      {
         switch (wParam)
         {
         case IDOK :
            {
               ::SendMessage(_hParent, NPPM_SETMENUITEMCHECK, (WPARAM)_cmdId, (LPARAM)false);
               display(FALSE);
               setOnAutoUpdate((BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_AUTOUPDT, BM_GETCHECK, 0, 0))?1:0);
               setUseBookmark((BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_USEBOOKMARK, BM_GETCHECK, 0, 0))?1:0);
               setDisplayLineNo((BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_DISPLINENO, BM_GETCHECK, 0, 0))?1:0);
               setOnEnterActionStr(mCmbOnEnterAction.getTextFromCombo(false));
               setFontText(mCmbFontName.getTextFromCombo(false));
               setFontSizeStr(mCmbFontSize.getTextFromCombo(false));
               setNumOfCfgFilesStr(mCmbNumOfCfgFiles.getTextFromCombo(false));
               mDefPat.setSearchTypeStr(mCmbSearchType.getTextFromCombo(false));
               mDefPat.setSelectionTypeStr(mCmbSelType.getTextFromCombo(false));
               mDefPat.setDoSearch(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_DO_SEARCH, BM_GETCHECK, 0, 0));
               mDefPat.setWholeWord(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_WHOLE_WORD, BM_GETCHECK, 0, 0));
               mDefPat.setMatchCase(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_MATCH_CASE, BM_GETCHECK, 0, 0));
               mDefPat.setHideText(BST_CHECKED==::SendDlgItemMessage(_hSelf, IDC_CHK_HIDE, BM_GETCHECK, 0, 0));
#ifdef RESULT_COLORING
               //            mDefPat.setColorStr(mCmbColor.getTextFromCombo(false));
#endif
               mDefPat.setColor(mDefPat.convColorNum2Enum(_pFgColour->getColour()));
               mDefPat.setBgColor(mDefPat.convColorNum2Enum(_pBgColour->getColour()));

               mbOkPressed = true;
               // inform find dialog that user has pressed ok; findDilog then descides if 
               // new default data needs to be copied into the find dialog
               if(_pFindDlg) {
                  WPARAM uNumOfCfgFiles = generic_atoi(getNumOfCfgFilesStr().c_str());
                  ::SendMessage(_pFindDlg->getHSelf(), IDC_DO_CHECK_CONF, (WPARAM)uNumOfCfgFiles, (LPARAM)&mDefPat);
               }
               if(_pResultDlg) {
                  ::SendMessage(_pResultDlg->getHSelf(), IDC_DO_CHECK_CONF, (WPARAM)0, (LPARAM)&mDefPat);
               }
               return TRUE;
            }
         case IDCANCEL :
            {
               ::SendMessage(_hParent, NPPM_SETMENUITEMCHECK, (WPARAM)_cmdId, (LPARAM)false);
               display(FALSE);
               mbOkPressed = false;
               return TRUE;
            }
         case IDC_BTN_ADDCONTEXT:
            {
               ::PostMessage(getHSelf(), WM_CLOSE, 0, (LPARAM)0);
               ::SendMessage(_hParent, NPPM_MENUCOMMAND, 0, IDM_SETTING_EDITCONTEXTMENU);
               _addCtxDlg.doDialog();
               return TRUE;
            }

         default :
            break;
         }
         break;
      }
   case WM_SIZE :
      {
         // RECT rcDlg, rcText, rcOk;
         // getClientRect(rcDlg);
         // HWND hOk = ::GetDlgItem(_hSelf, IDOK);
         // HWND hText = ::GetDlgItem(_hSelf,IDC_DIALOG_DESCRIPTION);
         // ::GetClientRect(hText, &rcText);
         // ::GetClientRect(hOk, &rcOk);
         // POINT pOk = getLeftTopPoint(hOk);
         // POINT pText = getLeftTopPoint(hText);
         // int dWidth = rcDlg.right-rcDlg.left-20; // for border
         // int dTextHeight = rcDlg.bottom-rcDlg.top-pText.y-10; // for border
         // if(dWidth>=0) {
         //    if(dTextHeight>=0){
         //       ::MoveWindow(hText, pText.x, pText.y, dWidth, dTextHeight, TRUE);
         //    }
         //    ::MoveWindow(hOk, pOk.x, pOk.y, dWidth, rcOk.bottom, TRUE);
         //redraw();
         // }
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
   default:;
   } // switch


	return FALSE;
}

BOOL CALLBACK ConfigDialog::run_dlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   return run_dlgProc(Message, wParam, lParam);
}

