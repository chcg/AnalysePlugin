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

#define MY_STYLE_MASK 0xff  // 8 bits https://www.scintilla.org/ScintillaDoc.html#StyleDefinition
#define MY_STYLE_BITS 8    

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
   }

   virtual ~ScintillaSearchView()
   {
   }

   virtual void init(HINSTANCE hInst, HWND hPere);
   void startRtfColorTable(unsigned defColor, unsigned bgColor);
   void addRtfColor2Table(unsigned color, unsigned bgColor);
   void finalizeRtfColorTable();
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
   static long NppVersion; // required for incompatible pointer size change in v8.3++
   static unsigned WarnForOldNppVersionDone; // ensure warning comes only once

protected:
   static const int transStylePosTab[MY_STYLE_MASK+1];
   int transStylePos(unsigned char stid) const;
   int countColorChanges(const char* lpstrText, intptr_t cpMin, intptr_t cpMax);
   int countLinefeeds(const char* lpstrText, intptr_t cpMin, intptr_t cpMax);
   int countEscapeChars(const char* lpstrText, intptr_t cpMin, intptr_t cpMax);
   bool prepareRtfClip(char *pGlobalText, int& iClipLength, char* lpSelText, int iSelTextLength);

   std::string _RtfHeader;
   std::string _RtfFooter;
   std::string _RtfColTbl; // intermediate build up of color table

   // override to do specialised things and finally call parent directly 
   virtual LRESULT scintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);


    int _oemCodepage = 0;
    bool _bLineNumbersInResult = true;
    generic_string _lastRtfFilename;

};

#endif //SCINTILLA_SEARCH_VIEW_H
