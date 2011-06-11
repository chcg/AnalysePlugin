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
#include "myDebug.h"

#include "HelpDialog.h"
#include "PluginInterface.h"
#include "resource.h"
#include <windows.h>

void HelpDlg::doDialog(int FuncCmdId)
{
    if (!isCreated())
        create(IDD_ANALYSE_HELP_DLG);
	_cmdId = FuncCmdId;

   goToCenter();
}


BOOL CALLBACK HelpDlg::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
   {
   case WM_INITDIALOG :
      {
         ::SendDlgItemMessage(_hSelf, IDC_EMAIL_LINK, WM_SETTEXT, 0, (LPARAM)EMAIL_LINK);
         _emailLink.init(_hInst, _hSelf);
         _emailLink.create(::GetDlgItem(_hSelf, IDC_EMAIL_LINK), EMAIL_LINK);

         ::SendDlgItemMessage(_hSelf, IDC_NPP_PLUGINS_URL, WM_SETTEXT, 0, (LPARAM)NPP_PLUGINS_URL);
         _urlNppPlugins.init(_hInst, _hSelf);
         _urlNppPlugins.create(::GetDlgItem(_hSelf, IDC_NPP_PLUGINS_URL), NPP_PLUGINS_URL);

         ::SendDlgItemMessage(_hSelf, IDC_VERSION_STRING, WM_SETTEXT, 0, (LPARAM)mVersionString.c_str());
         ::SendDlgItemMessage(_hSelf, IDC_AUTHOR_NAME, WM_SETTEXT, 0,  (LPARAM)AUTHOR_NAME);
         ::SendDlgItemMessage(_hSelf, IDC_DIALOG_DESCRIPTION, WM_SETTEXT, 0,  (LPARAM)DIALOG_DESCRIPTION);
         
         resizeWindow(); 
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
         case IDCANCEL :
            ::SendMessage(_hParent, NPPM_SETMENUITEMCHECK, (WPARAM)_cmdId, (LPARAM)false);
            display(FALSE);
            return TRUE;

         default :
            break;
         }
         break;
      }
   case WM_SIZE :
      {
         resizeWindow();
         break;
      } // case WM_SIZE
   }
	return FALSE;
}

void HelpDlg::resizeWindow() {
   RECT rcDlg, rcText, rcOk;
   getClientRect(rcDlg);
   HWND hOk = ::GetDlgItem(_hSelf, IDOK);
   HWND hText = ::GetDlgItem(_hSelf,IDC_DIALOG_DESCRIPTION);
   ::GetClientRect(hText, &rcText);
   ::GetClientRect(hOk, &rcOk);
   POINT pOk = getLeftTopPoint(hOk);
   POINT pText = getLeftTopPoint(hText);
   int dWidth = rcDlg.right-rcDlg.left-20; // for border
   int dTextHeight = rcDlg.bottom-rcDlg.top-pText.y-10; // for border
   if(dWidth>=0) {
      if(dTextHeight>=0){
         ::MoveWindow(hText, pText.x, pText.y, dWidth, dTextHeight, TRUE);
      }
      ::MoveWindow(hOk, pOk.x, pOk.y, dWidth, rcOk.bottom, TRUE);
      redraw();
   }
}
BOOL CALLBACK HelpDlg::run_dlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   return run_dlgProc(Message, wParam, lParam);
}

