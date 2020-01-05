/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2019 Matthias H. mattesh(at)gmx.net
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
#ifndef HELP_DIALOG_H
#define HELP_DEFINE_H

#include "StaticDialog.h"
#include "PluginInterface.h"
#include "URLCtrl.h"
#include <string>
#include "chardefines.h"

#define PROGRAM_DESCRIPTION TEXT("")


#define EMAIL_LINK TEXT("mailto:mattesh@gmx.net")
#define NPP_PLUGINS_URL TEXT("http://analyseplugin.sourceforge.net/")
#define AUTHOR_NAME TEXT("Matthias Hessling")

class HelpDlg : public StaticDialog
{

public:
   HelpDlg(const generic_string& version=generic_string(TEXT("no version"))) 
      : StaticDialog(), mVersionString(version), _cmdId(0) {};

   void init(HINSTANCE hInst, NppData nppData)
   {
      _nppData = nppData;
      Window::init(hInst, nppData._nppHandle);
   };

   void doDialog(int FuncCmdId = 0);

   virtual void destroy() {
      _emailLink.destroy();
      _urlNppPlugins.destroy();
   }

   void setVersion(const generic_string& ver){
      mVersionString = ver;
   }

   void setManText(const generic_string& man){
      mManual = man;
   }

   void setChangesText(const generic_string& changes){
      mChanges = changes;
   }

   void resizeWindow();

protected :
   virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
   virtual INT_PTR CALLBACK run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

   generic_string mVersionString;
   generic_string mManual;
   generic_string mChanges;

   /* Handles */
   NppData			_nppData;
   HWND			_HSource;

   /* for eMail */
   URLCtrl			_emailLink;
   URLCtrl			_urlNppPlugins;
   int _cmdId;
};



#endif // HELP_DEFINE_H


