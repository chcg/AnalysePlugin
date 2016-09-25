/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2016 Matthias H. mattesh(at)gmx.net

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
/**
tclComboBoxCtrl implements the WINAPI handling of a combobox
*/

//#include "stdafx.h"
#include "tclComboBoxCtrl.h"

#define MDBG_COMP "tclComboBoxCtrl:" 
#include "myDebug.h"

using namespace std;

void tclComboBoxCtrl::init(HWND hwnd){
   mhMyCtrl = hwnd;
}

tclComboBoxCtrl::tclComboBoxCtrl()
   :mhMyCtrl(0)
{
}

tclComboBoxCtrl::~tclComboBoxCtrl(){
   mhMyCtrl =0;
}

void tclComboBoxCtrl::addText2Combo(const TCHAR * txt2add, bool isUTF8, bool lastAsFirst, bool addAlways)
{   
   if (!mhMyCtrl) return;
   if ((txt2add==0)) return; 
   int i = 0;
   if(*txt2add==0) {
      DBG1("addText2Combo() adding zero length text! in %d", ::GetDlgCtrlID(mhMyCtrl));
   }
   // add even if empty!  if (!lstrcmp(txt2add, TEXT(""))) return;
   i = ::SendMessage(mhMyCtrl, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)txt2add);
   if (addAlways) {
      if (i != CB_ERR) { // found
         ::SendMessage(mhMyCtrl, CB_DELETESTRING, i, 0);
      }
      i = ::SendMessage(mhMyCtrl, CB_INSERTSTRING, ((lastAsFirst)?0:-1), (LPARAM)txt2add);
   } else {
      if (i != CB_ERR) { // found
         ::SendMessage(mhMyCtrl, CB_SELECTSTRING, i, 0);
      } else {
         i = ::SendMessage(mhMyCtrl, CB_INSERTSTRING, ((lastAsFirst)?0:-1), (LPARAM)txt2add);
      }
   }
   // set the text in the edit control
   ::SendMessage(mhMyCtrl, CB_SETCURSEL, i, 0);
}

void tclComboBoxCtrl::clearSelection() 
{   
   if (!mhMyCtrl) return;
   (void)::SendMessage(mhMyCtrl, CB_SETCURSEL, -1, 0);
}

generic_string tclComboBoxCtrl::getComboTextList(bool isUTF8) const {
   TCHAR text[MAX_CHAR_CELL];
   generic_string s;

   int count = (int)::SendMessage(mhMyCtrl, CB_GETCOUNT, 0, 0);
   int i = 0;

   // avoid adding the same string twice
   for ( ; i < count ; i++)
   {      
      ::SendMessage(mhMyCtrl, CB_GETLBTEXT, i, (LPARAM)text);
      generic_string s1(text);
      const TCHAR* pc = s1.c_str();
      while((pc = generic_strchr(pc, '|'))!=0) {
         // double delimiter as escaping
         int pos = (int)(pc-s1.c_str());
         s1.insert(pos, TEXT("|"));
         pc = s1.c_str() + pos+2;
      }

      if(s.length()>0 && s1.length()>0) {
         // if not the first put the delimiter
         s += TEXT("|");
      }
      // add string
      s += s1;
   }
   return s;
}

generic_string tclComboBoxCtrl::getTextFromCombo(bool isUnicode) const
{   
   TCHAR str[MAX_CHAR_CELL];
   ::SendMessage(mhMyCtrl, WM_GETTEXT, MAX_CHAR_CELL - 1, (LPARAM)str);
   return /*std::*/generic_string(str);
}

void tclComboBoxCtrl::addInitialText2Combo(const std::set<generic_string>& argv, bool isUTF8) {
   if(argv.size() == 0)
      return;
   ::SendMessage(mhMyCtrl, CB_RESETCONTENT, 0, 0);
   std::set<generic_string>::const_iterator i = argv.begin(),
      ie = argv.end();
   for(; i != ie ; ++i) {
      if((i->c_str()) && (*(i->c_str()))) {
         addText2Combo(i->c_str(), isUTF8, false);
      }
   }
   // clear the text in the edit control
   ::SendMessage(mhMyCtrl, CB_SETCURSEL, -1, 0);
}

void tclComboBoxCtrl::addInitialText2Combo(int argc, const TCHAR** argv, bool isUTF8){
   if((argc == 0)||(argv ==0))
      return;
   ::SendMessage(mhMyCtrl, CB_RESETCONTENT, 0, 0);
   int count = ::SendMessage(mhMyCtrl, CB_GETCOUNT, 0, 0);

   for(int ri = 0; ri < argc ; ++ri) {
      if((argv[ri]) && (*argv[ri])) {
         addText2Combo(argv[ri], isUTF8, false);
      }
   }
   // clear the text in the edit control
   ::SendMessage(mhMyCtrl, CB_SETCURSEL, -1, 0);
}
