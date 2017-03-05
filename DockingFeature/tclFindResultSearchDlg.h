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

#ifndef TCLFINDRESULTSEARCHDLG_H
#define TCLFINDRESULTSEARCHDLG_H

#include "StaticDialog.h"
//#include "resource.h"
//#include "tclComboBoxCtrl.h"
//#include "ScintillaSearchView.h"
//#include "tclPattern.h"
//#include <string>
#include "tclPatternList.h"

class tclFindResultSearchDlg : public StaticDialog
{
public :
   tclFindResultSearchDlg() 
      : StaticDialog()
      ,_pSearchResultView(0)
      ,_bSearchDown(true)
      ,_bDoWrap(true)
      ,mhlvPatterns(0)
   {}

   void init(HINSTANCE hInst, HWND hPere, ScintillaSearchView* pSearchResultView);

   virtual void create(int dialogID, bool isRTL = false);

   /** called to show the dialog */
   void doDialog(bool isRTL = false);
   
   void setSearchPatterns(const tclPatternList& patterns) ;

   virtual void display(bool toShow = true) ;

   void updatePatternList() ;
   
   int doFindText(int start, int end/*, bool bDownWards*/) ;

   void doFindFirst() ;

   void markFoundText(int targetStart, int targetEnd) ;

   void doFindNext(bool bSearchDown, bool bDoWrap) ;

   void doCount() ;

   void setdefaultPattern(const tclPattern& p) ;

protected :

   virtual INT_PTR CALLBACK run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam);

   tclComboBoxCtrl _CmbSearchText;
   tclComboBoxCtrl _CmbSearchType;
   tclComboBoxCtrl _CmbSearchDir;
   ScintillaSearchView* _pSearchResultView;
   bool _bSearchDown;
   bool _bDoWrap;
   HWND mhlvPatterns;
   tclPatternList mPatterns; // used for the find window
   tclPattern mDefPat;
   POINT mpPatt; // original posisition
   POINT mpPattDist; // right / bottom boundary for patterns
};

#endif //TCLFINDRESULTSEARCHDLG_H
