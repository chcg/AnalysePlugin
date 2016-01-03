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

#define MY_STYLE_MASK 0x7f  // 7 bits
#define MY_STYLE_BITS 7    

// see ~\scintilla\src\ScintillaBase.h
#define FNDRESDLG_SCINTILLAFINFER_COPY (13) 
#define FNDRESDLG_SCINTILLAFINFER_SELECTALL (16)

#define FNDRESDLG_BASE (WM_USER + 0100)
#define FNDRESDLG_SCINTILLAFINFER_SEARCH   (FNDRESDLG_BASE + 1)
#define FNDRESDLG_SCINTILLAFINFER_SAVEFILE (FNDRESDLG_BASE + 2)
#define FNDRESDLG_SCINTILLAFINFER_SAVE_CLR (FNDRESDLG_BASE + 3)

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
   bool doRichTextCopy();
   std::vector<MenuItemUnit> getContextMenu() const;
   void updateLineNumberWidth(bool lineNumbersShown);

protected:
   static const int transStylePos[MY_STYLE_MASK+1];

   int countColorChanges(const TextRange& rtr);
   int countLinefeeds(const TextRange& rtr);
   int countEscapeChars(const TextRange& rtr);
   bool prepareRtfClip(char *pGlobalText, int iClipLength, char* lpSelText, int iSelTextLength);
   
   std::string _RtfHeader;
   std::string _RtfFooter;

   // override to do specialised things and finally call parent directly 
   virtual LRESULT scintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


    int _oemCodepage;



};

#endif //SCINTILLA_SEARCH_VIEW_H
