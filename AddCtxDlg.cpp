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
//#include "stdafx.h"
#include "precompiledHeaders.h"

#include "AddCtxDlg.h"
#include "PluginInterface.h"
#include "resource.h"
#include <windows.h>
#define MDBG_COMP "AddCtxDlg:" 
#include "myDebug.h"

#define COMMAND_SNIPPET TEXT("<Item FolderName=\"AnalysePlugin\" PluginEntryName=\"AnalysePlugin\" PluginCommandItemName=\"Add selection as patterns\" />\r\n\
<Item FolderName=\"AnalysePlugin\" PluginEntryName=\"AnalysePlugin\" PluginCommandItemName=\"Search now\" />\r\n")

void AddCtxDlg::init(HINSTANCE hInst, NppData nppData)
{
   _nppData = nppData;
   Window::init(hInst, nppData._nppHandle);
}

void AddCtxDlg::doDialog()
{
   if (!isCreated()) {
        create(IDD_ADDCTX_DIALOG);
   }
   goToCenter();
}


BOOL CALLBACK AddCtxDlg::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
   switch (Message) 
   {
   case WM_INITDIALOG :
      {
         ::SendDlgItemMessage(_hSelf, IDC_EDT_COPYTEXT, WM_SETTEXT, 0,  (LPARAM)COMMAND_SNIPPET);
         
         resizeWindow(); 
         return TRUE;
      }
   case WM_CLOSE :
      {
         break;
      }
   case WM_COMMAND : 
      {
         switch (wParam)
         {
         case IDOK :
         case IDCANCEL :
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

void AddCtxDlg::resizeWindow() {
   RECT rcDlg, rcText;
   getClientRect(rcDlg);
   HWND hText = ::GetDlgItem(_hSelf,IDC_EDT_COPYTEXT);
   ::GetClientRect(hText, &rcText);
   POINT pText = getTopPoint(hText);
   int dWidth = rcDlg.right-rcDlg.left-20; // for border
   int dTextHeight = rcDlg.bottom-rcDlg.top-pText.y-10; // for border
   if(dWidth>=0) {
      if(dTextHeight>=0){
         ::MoveWindow(hText, pText.x, pText.y, dWidth, dTextHeight, TRUE);
      }
      redraw();
   }
}

BOOL CALLBACK AddCtxDlg::run_dlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   return run_dlgProc(Message, wParam, lParam);
}

