/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2020 Matthias H. mattesh(at)gmx.net
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

#ifndef SCINTILLA_SEARCH_VIEW_H
#define SCINTILLA_SEARCH_VIEW_H

#include <vector>
#include "ScintillaEditView.h"

#define RTF_COLTAG_RED "\n\\red" // red
#define RTF_COLTAG_GREEN "\\green" // green
#define RTF_COLTAG_BLUE "\\blue"  // blue
#define RTF_COLTAG_END ";" 
#define RTF_COL_R(r) ((BYTE)(r)) // mask char
#define RTF_COL_G(g) ((BYTE)(g>>8)) // mask short
#define RTF_COL_B(b) ((BYTE)(b>>16)) // mask third byte

#define MY_STYLE_MASK 0x7f  // 7 bits //TODO scintilla now always 8 style bits
#define MY_STYLE_BITS 7    

// see ~\scintilla\src\ScintillaBase.h
#define FNDRESDLG_SCINTILLAFINFER_COPY (13) 
#define FNDRESDLG_SCINTILLAFINFER_SELECTALL (16)

#define FNDRESDLG_BASE                     (WM_USER + 0x60)
#define FNDRESDLG_SCINTILLAFINFER_SEARCH   (FNDRESDLG_BASE + 1)
#define FNDRESDLG_SCINTILLAFINFER_SAVEFILE (FNDRESDLG_BASE + 2)
#define FNDRESDLG_SCINTILLAFINFER_SAVE_CLR (FNDRESDLG_BASE + 3)
#define FNDRESDLG_SCINTILLAFINFER_SAVE_RTF (FNDRESDLG_BASE + 4)
#define FNDRESDLG_WRAP_MODE                (FNDRESDLG_BASE + 5)
#define FNDRESDLG_SHOW_LINE_NUMBERS        (FNDRESDLG_BASE + 6)
#define FNDRESDLG_SHOW_OPTIONS             (FNDRESDLG_BASE + 7)
#define FNDRESDLG_SHOW_CONTEXTMENU         (FNDRESDLG_BASE + 8)
#define FNDRESDLG_ACTIVATE_PATTERN_LIST    (FNDRESDLG_BASE + 9)
#define FNDRESDLG_ACTIVATE_PATTERN_BASE    (FNDRESDLG_BASE + 0x0100)
#define FNDRESDLG_ACTIVATE_PATTERN_END     (FNDRESDLG_BASE + 0x01ff)

class ScintillaSearchView : public ScintillaEditView
{
    friend class Notepad_plus;

public:
    ScintillaSearchView()
        : ScintillaEditView()
    {
    };

    virtual ~ScintillaSearchView()
    {
    };

    virtual void init(HINSTANCE hInst, HWND hPere);

    

    



   void setRtfColorTable(const char* pColortbl);
   bool doRichTextCopy(const TCHAR* filename=NULL);
   void setWrapMode(bool bOn);
   bool getWrapMode() const;
   std::vector<MenuItemUnit> getContextMenu() const;
   void updateLineNumberWidth(bool lineNumbersShown);
   void setLineNumbersInResult(bool bOn) {
      _bLineNumbersInResult = bOn;
   }
   bool getLineNumbersInResult() const {
      return _bLineNumbersInResult;
   }
   void doSaveRichtext();
protected:
   static const int transStylePos[MY_STYLE_MASK+1];

   int countColorChanges(const Sci_TextRange& rtr);
   int countLinefeeds(const Sci_TextRange& rtr);
   int countEscapeChars(const Sci_TextRange& rtr);
   bool prepareRtfClip(char *pGlobalText, int& iClipLength, char* lpSelText, int iSelTextLength);

   std::string _RtfHeader;
   std::string _RtfFooter;

   // override to do specialised things and finally call parent directly 
   virtual LRESULT scintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


    int _oemCodepage;
    bool _bLineNumbersInResult = true;


};

#endif //SCINTILLA_SEARCH_VIEW_H
