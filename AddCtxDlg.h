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
#ifndef ADDCTXDLG_H
#define ADDCTXDLG_H

#include "StaticDialog.h"
#include "PluginInterface.h"
#include "chardefines.h"
/**
AddCtxDlg is used for showing the content that the user can copy into the NPP config.xml
*/
class AddCtxDlg : public StaticDialog
{

public:
   AddCtxDlg() 
      : StaticDialog() {};

   void init(HINSTANCE hInst, NppData nppData);

   void doDialog();

   virtual void destroy() {}

   void resizeWindow();

protected :
   virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
   virtual INT_PTR CALLBACK run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

   /* Handles */
   NppData	_nppData;
};



#endif // ADDCTXDLG_H


