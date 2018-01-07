/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2012-2018 Matthias H. mattesh(at)gmx.net
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
#include "PleaseWaitDlg.h"
#include "resource.h"
#include <commctrl.h>

// to get correct id for NPPM_INTERNAL_CANCEL_FIND_IN_FILES
//#include "../PowerEditor/src/resource.h"
//#define NOTEPADPLUS_USER_INTERNAL     (WM_USER + 0000)
//#define NPPM_INTERNAL_CANCEL_FIND_IN_FILES		(NOTEPADPLUS_USER_INTERNAL + 24)
#define APN_MSG_CANCEL_FIND WM_USER + 24

DWORD WINAPI AsyncPleaseWaitFunc(LPVOID phWnd) {
   ::Sleep(1000); // wait a bit before showing the popup 
   MessageBox((HWND)NULL, 
      TEXT("Finding patterns ...\nPress OK to cancel."), 
      TEXT("Analyse Plugin"), MB_OK);
   ::PostMessage(*(HWND*)phWnd, APN_MSG_CANCEL_FIND, 0, 0);
   return 0;
}

PleaseWaitDlg::PleaseWaitDlg(HWND hSelf): _hSelf(hSelf), _hProgress(0), _hCancelThread(0) {
     _hProgress = ::GetDlgItem(_hSelf, IDC_PROGRESS_SEARCH);
}

PleaseWaitDlg::~PleaseWaitDlg(){
   if(_hCancelThread) {
      TerminateThread(_hCancelThread, 0);
      _hCancelThread=0;
   }
}

bool PleaseWaitDlg::getCanceled() {
   MSG msg;
   if(PeekMessage(&msg, _hSelf, APN_MSG_CANCEL_FIND, APN_MSG_CANCEL_FIND, PM_REMOVE)) {
      return true;
   } else {
      return false;
   }
}

void PleaseWaitDlg::activate(bool isEnable) {
   if(isEnable) {
      _hCancelThread = ::CreateThread(NULL, 0, AsyncPleaseWaitFunc, &_hSelf, 0, NULL);
      ::Sleep(10); // give thread the chance to be there at first
   } else {
      if(_hCancelThread) {
         TerminateThread(_hCancelThread, 0);
         _hCancelThread=0;
      }
   }

   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_ADD ), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_UPD ), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_DEL ), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_LOAD), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_SAVE), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_MOVE_UP), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_MOVE_DOWN), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_BUT_CLEAR), isEnable?SW_HIDE:SW_SHOW);
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_DO_SEARCH), isEnable?SW_HIDE:SW_SHOW);

   ::ShowWindow(::GetDlgItem(_hSelf, IDC_STATIC_PLSWAIT), isEnable?SW_SHOW:SW_HIDE);
   ::UpdateWindow(::GetDlgItem(_hSelf, IDC_STATIC_PLSWAIT));
   ::ShowWindow(::GetDlgItem(_hSelf, IDC_PROGRESS_SEARCH), isEnable?SW_SHOW:SW_HIDE);
   ::UpdateWindow(::GetDlgItem(_hSelf, IDC_PROGRESS_SEARCH));
}

void PleaseWaitDlg::setProgressRange(int nMinRange, int nMaxRange){
   ::SendMessage(_hProgress, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (nMinRange, nMaxRange)); 
}
void PleaseWaitDlg::setProgressPos(int nPos) {
   ::SendMessage(_hProgress, PBM_SETPOS, (WPARAM) nPos , (LPARAM) 0); 
}
void PleaseWaitDlg::setProgressStepSize(int nStepSize) {
   ::SendMessage(_hProgress, PBM_SETSTEP, (WPARAM) nStepSize , (LPARAM) 0); 
}
void PleaseWaitDlg::incProgressStep(int nStepCount) {
   for(int i = 0; i<nStepCount;++i) {
      ::SendMessage(_hProgress, PBM_STEPIT, (WPARAM) 0 , (LPARAM) 0); 
   }
}
