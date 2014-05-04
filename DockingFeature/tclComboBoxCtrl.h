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

#ifndef TCLCOMBOBOXCTRL_H
#define TCLCOMBOBOXCTRL_H
#include <windows.h>
#include <string>
#include <set>
#include "common.h"


#define MAX_CHAR_CELL 1000 // max chars in a cell including \0

/**
 * class encapsulates combobox specific behaviour in terms of sending the right messages...
 * When the class ist initialized it provide constant access to the content on functional basis. 
 */
class tclComboBoxCtrl {
public:

   tclComboBoxCtrl();

   virtual ~tclComboBoxCtrl();

   void init(HWND hwnd);

   /**
    * inserts a list of strings into the combobox in one call
    * @param argc is the number of strings following
    * @param argv is the array of char * with argc size 
    */
   void addInitialText2Combo(int argc, const TCHAR** argv, bool isUTF8);

   /**
    *same as before but use a vectore to tansfer the initila text
    */
   void addInitialText2Combo(const std::set<generic_string>& argv, bool isUTF8);

   /**
    * adds one text to the combobox and set it into the selection
    * if this text is already in the list it will be selected only
    */
   void addText2Combo(const TCHAR* txt2add, bool isUTF8, bool lastAsFirst=true, bool addAlways=true);

   /**
    * cear the selection
    */
   void clearSelection();

   /**
    * retrieves the string actually stored in the editor field of the combobox 
    */
   generic_string getTextFromCombo(bool isUnicode) const ;

   /**
   * returns the number argc of the array of strings stored in 
   * argv 
   */
   generic_string getComboTextList(bool isUTF8) const ;

protected:

   /**
    * handle to the combobox to be controlled 
    */
   HWND mhMyCtrl;

   /** different behaviour for win32s */
   bool bMustDie9x;
};
#endif //TCLCOMBOBOXCTRL_H
