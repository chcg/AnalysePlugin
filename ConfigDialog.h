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
#ifndef CONFIG_DIALOG_H
#define CONFIG_DEFINE_H

#include "StaticDialog.h"
#include "PluginInterface.h"
#include "URLCtrl.h"
#include <string>
#include <set>
#include "chardefines.h"
#include "MyPlugin.h"
#include "tclComboBoxCtrl.h"
#include "tclPattern.h"
#include "ColourPicker2.h"

typedef std::set<generic_string> tlsString;

enum teOnEnterAction {
   enOnEntNoAction,
   enOnEntUpdate,
   enOnEntAdd,
   max_onEnterAction
};

class ConfigDialog : public StaticDialog
{

public:
   ConfigDialog() 
      : StaticDialog() 
      , _pFindDlg(0)
      , _pResultDlg(0)
      , _pParent(0)
      , _cmdId(0)
      , mbOkPressed(false) 
      ,_pFgColour(0)
      ,_pBgColour(0)
      ,mOnAutoUpdate(1)
      ,mUseBookmark(1)
   {}
      //, mFontSize(8) {}

   void init(HINSTANCE hInst, NppData nppData);
   
   void setFontList(HWND hWnd);

   void setOnAutoUpdate(int iOn) {
      mOnAutoUpdate = iOn;
   }

   int getOnAutoUpdate() const {
      return mOnAutoUpdate;
   }

   void setUseBookmark(int iOn) {
      mUseBookmark = iOn;
   }

   int getUseBookmark() const {
      return mUseBookmark;
   }

   void setOnEnterAction(teOnEnterAction i) {
      mOnEnterAction = i;
   }

   teOnEnterAction getOnEnterAction() const {
      return mOnEnterAction ;
   }

   generic_string getOnEnterActionStr() const ;
   void setOnEnterActionStr(const generic_string& action) ;

   void setFontText(const generic_string& str);
   const generic_string& getFontText() const;
   void setFontSize(unsigned s);
   void setFontSizeStr(const generic_string& s);
   unsigned getFontSize() const;
   generic_string getFontSizeStr() const;
   

   void setDialogData(const tclPattern& p);
   // returns true if pattern was set.
   bool getDialogData(tclPattern& p) const;

   void doDialog(int FuncCmdId=0);

   virtual void destroy() {
      // remove data
   }

   void setParent(MyPlugin* parent, Window* pFindDlg, Window* pResultDlg) {
      _pParent = parent;
      _pFindDlg = pFindDlg;
      _pResultDlg = pResultDlg;
   }

   // used to retrive the different font names from windows
	static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *, int, LPARAM lParam) ;

protected :
   virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
   virtual BOOL CALLBACK run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

   /* Handles */
   NppData			_nppData;
   Window*			_pFindDlg; // used to inform find dialog about update
   Window*			_pResultDlg; // used to inform result dialog about update
   MyPlugin* _pParent;
   int _cmdId;

   tclComboBoxCtrl mCmbSearchText;
   tclComboBoxCtrl mCmbSearchType;
   tclComboBoxCtrl mCmbSelType;
#ifdef RESULT_COLORING
   tclComboBoxCtrl mCmbColor;
#endif
   tclComboBoxCtrl mCmbOnEnterAction;
   tclPattern mDefPat;
   static const TCHAR*  transOnEnterAction[max_onEnterAction];
   teOnEnterAction mOnEnterAction;
   bool mbOkPressed;
   tlsString mlsFontList;
   /** defines the name of the font to be used for the result */
   tclComboBoxCtrl mCmbFontName;
   /** defines the text size in the result window **/
   tclComboBoxCtrl mCmbFontSize;
   generic_string mResultFontName;
   unsigned mResultFontSize;
   ColourPicker2* _pFgColour;
   ColourPicker2* _pBgColour;
   int mOnAutoUpdate;
   int mUseBookmark;
};



#endif // CONFIG_DEFINE_H


