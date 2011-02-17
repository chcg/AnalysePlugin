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
/* tclFindResultDlg.h 
This class implements the handling of the search result window 
it mainly contains the view based on scintilla and maintains parallel stored
search result string cache
*/

#ifndef TCLFINDRESULTDLG_H
#define TCLFINDRESULTDLG_H

#include "FindDlg.h"
#include "Scintilla.h"
#include "SciLexer.h"
#include "MyPlugin.h"
#include "ScintillaSearchView.h"
#include "tclFindResultDoc.h"
#include "myDebug.h"
#include "tclFindResultSearchDlg.h"

//off #define FEATURE_HEADLINE

class tclFindResultDlg : public DockingDlgInterface {

public:
   tclFindResultDlg();

   ~tclFindResultDlg();

   void setParent(MyPlugin* parent);

   void setLineNumColSize(int size); 

   int getLineNumColSize() const ;

   void initEdit(); 

   void reserve(unsigned count) {
      mFindResults.reserve(count);
   }

   /** remove all that line being referecend by result not no other pattern */
   void removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult); 
   

   void create(tTbData * data, bool isRTL = false);
   

#ifdef FEATURE_HEADLINE
   void addFileNameTitle(const TCHAR *fileName); 
#endif

   int insertPosInfo(tPatId patternId, tiLine iResultLine, tclPosInfo pos); 

   bool getLineAvail(tiLine foundLine) const ;

   const std::string& getLineText(int iResultLine); 

   void setLineText(int iFoundLine, const std::string& text); 
   
   void moveResult(tPatId oldPattId, tPatId newPattId);

   void updateWindowData(const std::string& fontName, unsigned fontSize);
   
   void clear(); 

   int getCurrentMarkedLine() const ;
   
   /**
   * set the marked line with
   */ 
   void setCurrentMarkedLine(int line); 
   
   /**
   function is called whenever the styles in the result window have to be changed
   */
   void setPatternStyles(const tclPatternList& list);
   	
   virtual void updateDockingDlg(void);

protected :
   // public version calls internal with correct start and end values
   void doStyle(int iFoundLine); 

   // open search dialog in result window
   void doFindResultSearchDlg();

   // message call back method
   virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);

   /** 
   this function sends the series of commands to 
   scintilla win with the corresponding style id 
   **/
   void setStyle(tPatId iPatternId, int iBeginPos, int iLength);
   void setDefaultStyle(int iBeginPos, int iLength);

   // callback from scintilla to colorize the search window
   void doStyle(int startResultLineNo, int startStyleNeeded, int endStyleNeeded); 

   bool notify(SCNotification *notification);
   
   void setFinderReadOnly(bool isReadOnly); 
   void setPatternFonts();

protected:
   MyPlugin* _pParent;

   //   ScintillaSearchView **_ppEditView;
   //	std::vector<FoundInfo> _foundInfos;
   tclFindResultDoc mFindResults;

   ScintillaSearchView _scintView;
   int _markedLine;  // -1 or the line actually marked in the main window
   // InWhat _mode;
   size_t _lineCounter; // incremented when a line has bee added to the edit window

   // count of chars reserved for line number
   int miLineNumColSize;
   // count of chars reserved for num header per line
   int miLineHeadSize;

   tclPatternList mPatStyleList;

   tclFindResultSearchDlg mFindResultSearchDlg;

   std::string mFontName;
   unsigned mFontSize;

};
#endif //TCLFINDRESULTDLG_H
