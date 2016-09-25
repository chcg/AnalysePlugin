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
#ifndef ADDCTXDLG_H
#define ADDCTXDLG_H

#include "StaticDialog.h"
#include "PluginInterface.h"
#include "chardefines.h"

class AddCtxDlg : public StaticDialog
{

public:
   AddCtxDlg() 
      : StaticDialog() {};

   void init(HINSTANCE hInst, NppData nppData);

   void doDialog();

   virtual void destroy() 
   {
      // TODO
   }

   void resizeWindow();

protected :
   virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
   virtual BOOL CALLBACK run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

   /* Handles */
   NppData	_nppData;
};



#endif // ADDCTXDLG_H


