/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011 Matthias H. mattesh(at)gmx.net

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

#include "stdafx.h"
#include "precompiledHeaders.h"
#include "tclComboBoxCtrl.h"
using namespace std;



void tclComboBoxCtrl::init(HWND hwnd){
   mhMyCtrl = hwnd;
}

tclComboBoxCtrl::tclComboBoxCtrl()
   :mhMyCtrl(0), bMustDie9x(false)
{
   OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if( !(GetVersionEx ((OSVERSIONINFO *) &osvi)) ){
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx ( (OSVERSIONINFO *) &osvi); 
	}
   if(osvi.dwMajorVersion != 0) {
      bMustDie9x =  (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
   }
}

tclComboBoxCtrl::~tclComboBoxCtrl(){
   mhMyCtrl =0;
}

void tclComboBoxCtrl::addText2Combo(const char * txt2add, bool isUTF8)
{   
	if (!mhMyCtrl) return;
	if ((txt2add==0)) return; //  || (*txt2add==0)

	char text[MAX_CHAR_CELL];
	WCHAR textW[MAX_CHAR_CELL];

   int count = (int)::SendMessage(mhMyCtrl, CB_GETCOUNT, 0, 0);
	bool hasFound = false;
	int i = 0;

   WCHAR wchars2Add[MAX_CHAR_CELL];
   if (isUTF8){
	   ::MultiByteToWideChar(CP_UTF8, 0, txt2add, -1, wchars2Add, MAX_CHAR_CELL - 1);
   }
   // avoid adding the same string twice
   for ( ; i < count ; i++)
	{      
      if (isUTF8)
	   {
	      if ( !bMustDie9x )
	      {  // WINNT
	         ::SendMessageW(mhMyCtrl, CB_GETLBTEXT, i, (LPARAM)textW);
	      }
	      else
	      {  // WIN32S
	         ::SendMessageA(mhMyCtrl, CB_GETLBTEXT, i, (LPARAM)text);
	         ::MultiByteToWideChar(CP_ACP, 0, text, -1, textW, MAX_CHAR_CELL - 1);
	      }
	      if (!wcscmp(wchars2Add, textW))
	      {
	         hasFound = true;
	         break;
	      }
	   }
	   else
	   {
	      ::SendMessageA(mhMyCtrl, CB_GETLBTEXT, i, (LPARAM)text);
	      if (!strcmp(txt2add, text))
	      {
	         hasFound = true;
	         break;
	      }
	   }
	}

   if (!hasFound)
	{
      if (!isUTF8)
	      i = (int)::SendMessageA(mhMyCtrl, CB_ADDSTRING, 0, (LPARAM)txt2add);
	   else
	   {
	      if ( !bMustDie9x )
	      {
	         i = (int)::SendMessageW(mhMyCtrl, CB_ADDSTRING, 0, (LPARAM)wchars2Add);
	      }
	      else
	      {
	         ::WideCharToMultiByte(CP_ACP, 0, wchars2Add, -1, text, MAX_CHAR_CELL - 1, NULL, NULL);
	         i = (int)::SendMessageA(mhMyCtrl, CB_ADDSTRING, 0, (LPARAM)text);
	      }
	   }
	}
   // set the text in the edit control
   ::SendMessage(mhMyCtrl, CB_SETCURSEL, i, 0);
}

std::string tclComboBoxCtrl::getComboTextList(bool isUTF8) const {
	char text[MAX_CHAR_CELL];
	WCHAR textW[MAX_CHAR_CELL];
   std::string s;

   int count = (int)::SendMessage(mhMyCtrl, CB_GETCOUNT, 0, 0);
	int i = 0;

   //WCHAR wchars2Add[MAX_CHAR_CELL];
   //if (isUTF8){
	  // ::MultiByteToWideChar(CP_UTF8, 0, txt2add, -1, wchars2Add, MAX_CHAR_CELL - 1);
   //}
   // avoid adding the same string twice
   for ( ; i < count ; i++)
	{      
      if (isUTF8)
	   {
	      if ( !bMustDie9x )
	      {  // WINNT
	         ::SendMessageW(mhMyCtrl, CB_GETLBTEXT, i, (LPARAM)textW);
            bool b = false;
            ::WideCharToMultiByte(CP_ACP, 0, textW, -1, text, MAX_CHAR_CELL - 1, " ", (LPBOOL)&b);
	      }
	      else
	      {  // WIN32S
	         ::SendMessageA(mhMyCtrl, CB_GETLBTEXT, i, (LPARAM)text);
	         //::MultiByteToWideChar(CP_ACP, 0, text, -1, textW, MAX_CHAR_CELL - 1);
	      }
	   }
	   else
	   {
	      ::SendMessageA(mhMyCtrl, CB_GETLBTEXT, i, (LPARAM)text);
	   }
      std::string s1(text);
      const char* pc = s1.c_str();
      while((pc = strchr(pc, '|'))!=0) {
         // double delimiter as escaping
         int pos = (int)(pc-s1.c_str());
         s1.insert(pos, "|");
         pc = s1.c_str() + pos+2;
      }

      if(s.length()>0) {
         // if not the first put the delimiter
         s += "|";
      }
      // add string
      s += s1;
	}
   return s;
}
std::string tclComboBoxCtrl::getTextFromCombo(bool isUnicode) const
{   
	char str[MAX_CHAR_CELL];
   memset(str, 0, sizeof(str));
	if (isUnicode)
	{
      WCHAR wchars[MAX_CHAR_CELL];
	   if ( !bMustDie9x )
	   {  // WINNT
	      ::SendMessageW(mhMyCtrl, WM_GETTEXT, MAX_CHAR_CELL - 1, (LPARAM)wchars);
	   }
	   else
	   {  //WIN32S
	      char achars[MAX_CHAR_CELL];
	      ::SendMessageA(mhMyCtrl, WM_GETTEXT, MAX_CHAR_CELL - 1, (LPARAM)achars);
	      ::MultiByteToWideChar(CP_ACP, 0, achars, -1, wchars, MAX_CHAR_CELL - 1);
	   }
	   ::WideCharToMultiByte(CP_UTF8, 0, wchars, -1, str, MAX_CHAR_CELL - 1, NULL, NULL);
	}
   else
   {
      ::SendMessageA(mhMyCtrl, WM_GETTEXT, MAX_CHAR_CELL - 1, (LPARAM)str);
	}
   return string(str);
}

void tclComboBoxCtrl::addInitialText2Combo(const std::set<std::string>& argv, bool isUTF8) {
   if(argv.size() == 0)
      return;
   std::set<std::string>::const_iterator i = argv.begin(),
      ie = argv.end();
   for(; i != ie ; ++i) {
      if((i->c_str()) && (*(i->c_str()))) {
         addText2Combo(i->c_str(), isUTF8);
      }
   }
   // clear the text in the edit control
   ::SendMessage(mhMyCtrl, CB_SETCURSEL, -1, 0);
}

void tclComboBoxCtrl::addInitialText2Combo(int argc, const char** argv, bool isUTF8){
   if((argc == 0)||(argv ==0))
      return;
   for(int i = 0; i < argc; ++i) {
      if((argv[i]) && (*argv[i])) {
         addText2Combo(argv[i], isUTF8);
      }
   }
   // clear the text in the edit control
   ::SendMessage(mhMyCtrl, CB_SETCURSEL, -1, 0);
}
