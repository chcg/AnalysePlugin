/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2012 Matthias H. mattesh(at)gmx.net
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
#ifndef PLEASEWAITDLG_H
#define PLEASEWAITDLG_H

#include "windows.h"

class PleaseWaitDlg {
public:
   PleaseWaitDlg(HWND hSelf);

   bool getCanceled();
   void activate(bool isEnable);

   void setProgressRange(int nMinRange=0, int nMaxRange=100);
   void setProgressPos(int nPos);
   void setProgressStepSize(int nStepSize);
   void incProgressStep(int nStepCount=1);
   ~PleaseWaitDlg();

protected:
   HWND _hSelf;
   HWND _hProgress;
   HANDLE _hCancelThread;
};

#endif //PLEASEWAITDLG_H
