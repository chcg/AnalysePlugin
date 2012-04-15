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
#ifndef HELP_DIALOG_H
#define HELP_DEFINE_H

#include "StaticDialog.h"
#include "PluginInterface.h"
#include "URLCtrl.h"
#include <string>
#include "chardefines.h"

#define PROGRAM_DESCRIPTION TEXT("")

#define DIALOG_DESCRIPTION TEXT("\r\n\
This Plugin will help you to search for more than one \
search pattern at a time. \r\n\
\r\n\
Add the different search patterns to the configuration dialog and press [search]. \
As result you will find all matching lines in the result window and in the main \
window matching lines are marked. \r\n\
All patterns can have a different search type e.g. regular expression or \
normal text same as the find dialog in NotePad++ (see manual in \"?\")\r\n\
Additionally you may want to blend out a sub set of a search \
pattern. For this you can check the \"hide\" option which results in hiding \
of that line or text from the search result. The text is still there and \
clipboard copy will still hold all text. To hint that a line was found once, \
the line number remains in the result window.\r\n\
You can also give the different searches colours to better distinguish which \
line belongs to which pattern. The colouring is done top line first and \
bottom line last. If a pattern is found twice, the lower pattern defines \
the colour. You can also give each line a comment e.g. what this pattern helps \
to find.\r\n\
The Anaylse Configuration Dialog handles the list of search patterns. In this \
dialog you can adjust the different search patterns stacked into with \
following buttons:\r\n\
---- Buttons ---- \r\n\
[Add]    \tPuts actual setting of the upper part into a new line in the table.\r\n\
         \tIf a line was marked in the table the line is added underneath.\r\n\
[Update] \tCopies the dialog data into the marked line of the table.\r\n\
[Delete] \tRemoves the line actually marked.\r\n\
[  ^  ]  \tMoves the selected pattern one step up in the list.\r\n\
[  v  ]  \tMoves the selected pattern one step down in the list.\r\n\
[Clear]  \tRemoves all patterns and search result at once.\r\n\
[Load]   \tAllows loading a list of search patterns from disk.\r\n\
[Save]   \tAllows saving a list of search patterns to disk.\r\n\
[Search] \tStarts the search of all previously changed patterns.\r\n\
         \tIf no pattern exists the actual settings are made to one line\r\n\
         \tof pattern. If only one line exists and actual settings are\r\n\
         \tdifferent, the line is updated before starting the search.\r\n\
\r\n\
---- Search Options ---- \r\n\
Search Type: \tSame as in find dialog; chose the type how to search.\r\n\
\t           \tHere an interesting feature is that \\r\\n can be used\r\n\
\t           \tto search for more than one line! Test it with normal\r\n\
\t           \tfind dialog in the editor and use the option \"escaped\".\r\n\
\t           \tAll types are the same as in normal NotePad++ find\r\n\
\t           \tsee also the help manual in menu \"?\" of NotePad++.\r\n\
Case:        \tSame as in find dialog; case sensitive or not.\r\n\
Whole Word:  \tSame as in find dialog; search for whole word or parts\r\n\
Do Search:   \tAllows to temporarily switching off this pattern \r\n\
\t           \t(remove from search result).\r\n\
Comment:     \tUseful for info what this pattern helps to find.\r\n\
\r\n\
---- Visualization Options ---- \r\n\
These options can be updated without activating a re-search\r\n\
Hide Text:   \tIf on, the found pattern is removed from result line.\r\n\
Colour FG/BG:\tThe result window text becomes coloured in \r\n\
\t           \tforeground and background herewith.\r\n\
\t           \tIf a result text is cached by several patterns, the \r\n\
\t           \tcolour of the last pattern in list is used.\r\n\
\t           \tThe maximum of maintainable pattern colors is 119!\r\n\
\t           \tAll further patterns will get the default color.\r\n\
Selection on:\tDecides if found text or the whole line shall be \r\n\
\t           \tcoloured / hidden.\r\n\
\r\n\
---- Mouse Events ---- \r\n\
 - Configuration table -\r\n\
Left mouse click   \tCopies the actual data of the clicked line into the \r\n\
                   \tconfiguration dialog for editing. click [update] to \r\n\
                   \tput the changes back to the marked line.\r\n\
Double click       \tToggles the current line to be searched or not.\r\n\
Right mouse click  \tOpens the context menu.\r\n\
\r\n\
 - Result window -\r\n\
Double click       \tMoves the main window to the corresponding line\r\n\
Right mouse click  \tOpens the context menu.\r\n\
The find dialog in this window is optimized for the search patterns.\r\n\
click on one of the currently defined patterns to search for this \r\n\
individually in the result window.\r\n\
\r\n\
---- Analyse Plugin Configuration ---- \r\n\
The Analyse Plugin has an additional dialog for setting the config. \
In this dialog you can adjust the default values used in the find dialog \
at the startup of NotePad++.\r\n\
In addition you can decide how the plugin shall react on pressing the \
search button (or Enter), when the search text has been modified. \
You may choose of either discard the modified text and 'just search'; in \
this case the modified search text will become replaced with the text of the \
actually selected line's search text. \r\n\
If you choose 'update line' than the will be copied into the marked line and \
searched for. \r\n\
If your choice is 'add line' the modified search text will be taken as an \
additional search pattern and is amended to the search list. In this setting \
changing other options will be treated like update line, as normally it does \
not make sense to search for the same text twice.\r\n")


#define EMAIL_LINK TEXT("mailto:mattesh@gmx.net")
#define NPP_PLUGINS_URL TEXT("http://analyseplugin.sourceforge.net/")
#define AUTHOR_NAME TEXT("Matthias Hessling")

class HelpDlg : public StaticDialog
{

public:
   HelpDlg(const generic_string& version=generic_string(TEXT("no version"))) 
      : StaticDialog(), mVersionString(version), _cmdId(0) {};

   void init(HINSTANCE hInst, NppData nppData)
   {
      _nppData = nppData;
      Window::init(hInst, nppData._nppHandle);
   };

   void doDialog(int FuncCmdId = 0);

   virtual void destroy() {
      _emailLink.destroy();
      _urlNppPlugins.destroy();
   }

   void setVersion(const generic_string& ver){
      mVersionString = ver;
   }

   void resizeWindow();

protected :
   virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
   virtual BOOL CALLBACK run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

   generic_string mVersionString;

   /* Handles */
   NppData			_nppData;
   HWND			_HSource;

   /* for eMail */
   URLCtrl			_emailLink;
   URLCtrl			_urlNppPlugins;
   int _cmdId;
};



#endif // HELP_DEFINE_H


