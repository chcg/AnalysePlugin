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
interface for my plugin knwon to the dock window
*/

#ifndef MYPLUGIN_H
#define MYPLUGIN_H

#include <windows.h>
#include <string>
#include "chardefines.h"

class tclResultList;
class tclResult;
//typedef int tiIndex; // index of the pattern applied
typedef double tPatId;// new id of the pattern applied
// typedef int tiLine;  // number of the line of the found entry start in main window
enum teOnEnterAction;

enum teNppWindows {
	scnMainHandle,
	scnSecondHandle,
	nppHandle,
   scnActiveHandle // used to take the one being active regardless if main or second
};

#define SCINTILLA_USER     (WM_USER + 2000)
#define WM_DOOPEN		   (SCINTILLA_USER + 8)

/**
 * call back interface to the plugin windows
 * all functions being available for sub classes.
 */
class MyPlugin {
public:

   /**
   * my replacement for direct access is to use scintillas second window
   */
   virtual LRESULT execute(teNppWindows window, UINT Msg, WPARAM wParam=0, LPARAM lParam=0) const  =0;
   
   /**
   * standard message send function
   */
   virtual LRESULT execute(HWND hwnd, UINT Msg, WPARAM wParam=0, LPARAM lParam=0) const  =0;

   /**
   * provides acces to the actual scintilla and main window handles 
   * which are set to the plugin during initialization
   */
   virtual HWND getCurrentHScintilla(teNppWindows which) const  =0;

   /**
   * reaction on (un)collapse all 
   * @param true = uncollapse
   */
   virtual void updateStyles() =0;

   /**
   * displaySectionCentered positions the text marking to the given position and 
   * moves this position into the visible area.
   */
   virtual void displaySectionCentered(int posStart, int posEnd, bool isDownwards = true) =0;

   /**
   * sets find result window into read only mode
   */
   virtual void setFinderReadOnly(bool isReadOnly)  =0;
   
   /**
   * remove one line in patternlist and all corresponding 
   * result lines in resultwindow
   */
   virtual void removeUnusedResultLines(tPatId pattId, const tclResult& oldResult, const tclResult& newResult)=0;
   
   /**
   * remove all search content
   */
   virtual void clearResult()=0;

   /**
   * move the results in result window and search result from old to new Id
   * this shall not cause a re-searching but a reodering of the paint order
   */
   virtual void moveResult(tPatId oldPattId, tPatId newPattId)=0;
   
   /**
   * activates the search on the given result list.
   * this will cause the plugin to check which patterns in the resultlist have to 
   * be updated and activates the repaint of the different windows.
   */
   virtual BOOL doSearch(tclResultList& resultList) =0;
   
   /**
   * returns the name of the file being used for analysis
   */
   virtual /*std::*/ generic_string getSearchFileName() const =0;

   /**
   * set the search file being analysed
   */
   virtual void setSearchFileName(const /*std::*/ generic_string& file) =0;

   /**
   * function is called as notification that the plugin has been switched off by
   * closing the dock window. The implementation shall make sure that menu and result
   * window are in sync with the config window.
   */
   virtual void visibleChanged(bool isVisible) =0;

   /**
   * function reprts how find dialog shall react on pressing enter
   */
   virtual teOnEnterAction getOnEnterAction() const =0;

   /**
   * function reports which font result dialog shall use
   */
   virtual std::string getResultFontName() const =0;
   virtual unsigned getResultFontSize() const =0;
};
#endif //MYPLUGIN_H
