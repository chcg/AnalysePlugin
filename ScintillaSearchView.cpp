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

#include <windows.h>
#include <ShellAPI.h>
#include <string.h>
#include "ScintillaSearchView.h"
//#include "resource.h"

#define MDBG_COMP "SSView:" 
#include "myDebug.h"


#define RTF_HEADER "{\\rtf1\\ansi\\ansicpg\\lang1024\\noproof1252\\uc1 \\deff0{\\fonttbl{\\f0\\fnil\\fcharset0\\fprq1 Courier New;}}\n{\\colortbl" // end
#define RTF_CRLF "\\par " // end
#define RTF_FOOTER "\\par }" // end
#define RTF_COLNUM "\\cf" // end

// style number to rtf color position code
const int ScintillaSearchView::transStylePos[MY_STYLE_MASK+1] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 
   10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
   20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
   30, 31,  0,  0,  0,  0,  0,  0,  0,  0,
           32, 33, 34, 35, 36, 37, 38, 39,
   40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
   50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
   60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
   70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
   90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
  100,101,102,103,104,105,106,107,108,109,
  110,111,112,113,114,115,116,117,118,119
};

void ScintillaSearchView::init(HINSTANCE hInst, HWND hPere)
{
#pragma warning(disable:4312 4311)
    if (!_hLib)
    {
        MessageBox( NULL, TEXT("Can not load the dynamic library _hLib==0"), TEXT("ScintillaSearchView ERROR : "), MB_OK | MB_ICONSTOP);
        throw int(106900);
    }

    Window::init(hInst, hPere);
   _hSelf = ::CreateWindowEx(
                    WS_EX_CLIENTEDGE,\
                    TEXT("Scintilla"),\
                    TEXT("FindResults"),\
                    WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_EX_RTLREADING,\
                    0, 0, 100, 100,\
                    _hParent,\
                    NULL,\
                    _hInst,\
                    NULL);

    if (!_hSelf)
    {
//		systemMessage(TEXT("System Error"));
        MessageBox( NULL, TEXT("System Error no handle"), TEXT("ScintillaSearchView ERROR : "), MB_OK | MB_ICONSTOP);
        throw int(106901);
    }

    _pScintillaFunc = (SCINTILLA_FUNC)::SendMessage(_hSelf, SCI_GETDIRECTFUNCTION, 0, 0);
    _pScintillaPtr = (SCINTILLA_PTR)::SendMessage(_hSelf, SCI_GETDIRECTPOINTER, 0, 0);

//    _userDefineDlg.init(_hInst, _hParent, this);

    if (!_pScintillaFunc || !_pScintillaPtr)
    {
        //systemMessage(TEXT("System Err"));
        MessageBox( NULL, TEXT("System Error no scintilla pointer"), TEXT("ScintillaSearchView ERROR : "), MB_OK | MB_ICONSTOP);
        throw int(106902);
    }

    //execute(SCI_SETMARGINMASKN, _SC_MARGE_FOLDER, SC_MASK_FOLDERS);
    // showMargin(_SC_MARGE_FOLDER, true);
//new 5.3 begin
 //   execute(SCI_SETMARGINMASKN, _SC_MARGE_SYBOLE, (1<<MARK_BOOKMARK) | (1<<MARK_HIDELINESBEGIN) | (1<<MARK_HIDELINESEND));

    //execute(SCI_SETMARGINMASKN, _SC_MARGE_MODIFMARKER, (1<<MARK_LINEMODIFIEDUNSAVED)|(1<<MARK_LINEMODIFIEDSAVED));
    //execute(SCI_SETMARGINTYPEN, _SC_MARGE_MODIFMARKER, SC_MARGIN_BACK);
    //showMargin(_SC_MARGE_MODIFMARKER, true);

    //execute(SCI_MARKERDEFINE, MARK_LINEMODIFIEDSAVED, SCI_MARKERDEFINE);
    //execute(SCI_MARKERDEFINE, MARK_LINEMODIFIEDUNSAVED, SCI_MARKERDEFINE);

    //execute(SCI_MARKERSETALPHA, MARK_BOOKMARK, 70);
    //execute(SCI_MARKERDEFINEPIXMAP, MARK_BOOKMARK, (LPARAM)bookmark_xpm);
    //execute(SCI_MARKERDEFINEPIXMAP, MARK_HIDELINESBEGIN, (LPARAM)acTop_xpm);
    //execute(SCI_MARKERDEFINEPIXMAP, MARK_HIDELINESEND, (LPARAM)acBottom_xpm);
// new 5.3 end
    //execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_FOLDER, true);
    //execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_SYBOLE, true);

   execute(SCI_SETMARGINWIDTHN, _SC_MARGE_SYBOLE, 0);
   execute(SCI_SETMARGINWIDTHN, _SC_MARGE_FOLDER, 0);

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("0"));

    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.html"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.comment"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.preprocessor"), reinterpret_cast<LPARAM>("1"));
    execute(SCI_SETFOLDFLAGS, 16);
    execute(SCI_SETSCROLLWIDTHTRACKING, true);
    execute(SCI_SETSCROLLWIDTH, 1);	//default empty document: override default width of 2000

    // smart hilighting
    execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_SMART, INDIC_ROUNDBOX);
    execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE, INDIC_ROUNDBOX);
    execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_INC, INDIC_ROUNDBOX);
    execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_TAGMATCH, INDIC_ROUNDBOX);
    execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_TAGATTR, INDIC_ROUNDBOX);
    

   execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_SMART, 100);
    execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE, 100);
    execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_INC, 100);
    execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_TAGMATCH, 100);
    execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_TAGATTR, 100);

   execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_SMART, true);
    execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE, true);
    execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_INC, true);
    execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_TAGMATCH, true);
    execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_TAGATTR, true);
    
   //switch off the context menu off scintilla as we have our own
   execute(SCI_USEPOPUP, 0);
//TODO evtl doch benutzen	_pParameter = NppParameters::getInstance();
    
    _codepage = ::GetACP();
    _oemCodepage = ::GetOEMCP();


    ::SetWindowLongPtr(_hSelf, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    _callWindowProc = CallWindowProc;
    _scintillaDefaultProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(_hSelf, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ScintillaSearchView::scintillaStatic_Proc)));

    //Get the startup document and make a buffer for it so it can be accessed like a file
    //attachDefaultDoc();
#pragma warning(default:4312 4311)
}


void ScintillaSearchView::setRtfColorTable(const char* pColortbl) {
   _RtfFooter = RTF_FOOTER;
   _RtfHeader = RTF_HEADER;
   _RtfHeader += pColortbl;
   _RtfHeader += "}\n\\fs20\n";
}

bool ScintillaSearchView::doRichTextCopy() {
   /*
   as in the control symbols \\, \{, and \}
   as CRLF \par
   {\rtf1\ansi\ansicpg\lang1024\noproof1252\uc1 \deff0 
      {\colortbl;  
         \red0\green0\blue255;      
         \red255\green255\blue255;      
         \red0\green0\blue0;      
         \red0\green128\blue0;   
      }   
      \cf1 void \cf0 ScintillaSearchView::doRichTextCopy() \{\par
   }   
   */
   int iEnd = (int)execute(SCI_GETSELECTIONEND);
   int iBegin = (int)execute(SCI_GETSELECTIONSTART);
   TextRange tr;
   tr.chrg.cpMin = (long)iBegin;
   tr.chrg.cpMax = (long)iEnd;
   assert(iEnd>=iBegin);
   int iSelTextLength = iEnd-iBegin;
   if (iSelTextLength <= 0) {
      DBG0("doRichTextCopy() ERROR no text selected!");
      return false; // no text selected
   }
   tr.lpstrText = new char[(iSelTextLength+1)*2]; // // 1 for zero word at end and each char as short
   (void)execute(SCI_GETSTYLEDTEXT, 0, (LPARAM)&tr);
   int iColCount = countColorChanges(tr);
   int iEscapeCount = countEscapeChars(tr);
   int iParCount = countLinefeeds(tr);

   //expand size of allocated text by iColCount*expected length per COL + header and footer
   int iClipLength = (iColCount*6)                 // color control words
                     +(iParCount*5)                // count of crlf
                     +iSelTextLength               // the text
                     +iEscapeCount                 // \ { } escaping
                     +(int)_RtfHeader.length()     // rtf header including color table
                     +(int)_RtfFooter.length();    // rtf footer
        
   UINT uClipFormatId = RegisterClipboardFormat(TEXT("Rich Text Format")); 
   if(uClipFormatId == 0) {
      DBG0("doRichTextCopy() ERROR enumerate clipboard format RTF");
      return false; 
   }
   
   if (!::OpenClipboard(_hSelf)){
        ::CloseClipboard(); // try a second time
      ::Sleep(100);
      if (!::OpenClipboard(_hSelf)){
         DBG0("doRichTextCopy() ERROR could not open clipboard");
         return false; 
      }
   }

    // ::EmptyClipboard(); CF_TEXT is already in
    
    HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, iClipLength);
    
    if (hglbCopy == NULL) { 
      DBG0("doRichTextCopy() ERROR could not GlobalAlloc");
        ::CloseClipboard(); 
        return false; 
    } 

    // Lock the handle and copy the text to the buffer. 
    char* pStr = (char*)::GlobalLock(hglbCopy);
   if(pStr == 0) {
      DBG0("doRichTextCopy() ERROR could not GlobalLock");
      return false;
   }
   bool bOk = prepareRtfClip(pStr, iClipLength, tr.lpstrText, iSelTextLength);
    //strcpy(pStr, str2cpy);

   //FILE* f = fopen("c:\\temp\\notepad_text.rtf", "wt");
   //fwrite(pStr, 1, strlen(pStr), f);
   //fclose(f);
    
   ::GlobalUnlock(hglbCopy); 

    // Place the handle on the clipboard. 
   if (bOk) {
      ::SetClipboardData(uClipFormatId, hglbCopy);
   } else {
      GlobalFree(hglbCopy);
   }
    ::CloseClipboard();
    return true;
}

int ScintillaSearchView::countColorChanges(const TextRange& rtr){
   int iCount=0;
   short* pwChar= (short*)rtr.lpstrText;
   register char prevStyle = -1;
   register char style = -1;
   short* pwEnd = (short*)rtr.lpstrText + rtr.chrg.cpMax - rtr.chrg.cpMin;
   for(; pwChar < pwEnd; ++pwChar) {
      if(prevStyle !=(style = ((*pwChar)>>8))) {
         ++iCount;
         prevStyle = style;
      }
   }
   return iCount;
}


int ScintillaSearchView::countLinefeeds(const TextRange& rtr){
   int iCount=0;
   short* pwChar= (short*)rtr.lpstrText;
   short* pwEnd = (short*)rtr.lpstrText + rtr.chrg.cpMax - rtr.chrg.cpMin;
   for(; pwChar< pwEnd; ++pwChar) {
      switch((*pwChar)&0xff) {
         case 0x0a:
            ++iCount;
            break;
         default:
            break;
      };
   }
   return iCount;
}

int ScintillaSearchView::countEscapeChars(const TextRange& rtr){
   int iCount=0;
   short* pwChar= (short*)rtr.lpstrText;
   short* pwEnd = (short*)rtr.lpstrText + rtr.chrg.cpMax - rtr.chrg.cpMin;
   for(; pwChar< pwEnd; ++pwChar) {
      switch((*pwChar)&0xff) {
         case '\\':
         case '{':
         case '}':
            ++iCount;
            break;
         default:
            break;
      };
   }
   return iCount;
}

bool ScintillaSearchView::prepareRtfClip(char *pGlobalText, int iClipLength, char* lpSelText, int iSelTextLength){
   char* pDest = pGlobalText;
   memset(pDest, 0, iClipLength);
   // RTF Header
   iClipLength -= (int)_RtfHeader.length();
   if(iClipLength <= 0) { assert(0); return false; }
   strncpy(pDest, _RtfHeader.c_str(), _RtfHeader.length());
   pDest += _RtfHeader.length();
   // text
   char styleNum[4];
   char style=-1;
   char prevStyle=-1;
   short* pChar = (short*)lpSelText;
   short* pEnd = (short*)lpSelText + iSelTextLength;
   while(pChar < pEnd) {
      // style
      if ((style = ((*pChar)>>8))!=prevStyle) {
         // add style
         iClipLength -= (int)strlen(RTF_COLNUM);
         if(iClipLength <= 0) { assert(0); return false; }
         strcpy(pDest, RTF_COLNUM);
         pDest += strlen(RTF_COLNUM);
         _itoa(transStylePos[style], styleNum, 10);
         iClipLength -= (int)strlen(styleNum);
         if(iClipLength <= 0) { assert(0); return false; }
         strcpy(pDest, styleNum);
         pDest += strlen(styleNum);
         --iClipLength;
         if(iClipLength <= 0) { assert(0); return false; }
         strcpy(pDest++, " ");
         prevStyle = style;
      }
      // text
      switch((*pChar)&0xff) {
         case '\\':
         case '{':
         case '}':
            --iClipLength;
            if(iClipLength <= 0) { assert(0); return false; }
            strcpy(pDest++, "\\"); 
            break;
         case 0x0d:
         case 0x0a:
            // line feed
            if((((*++pChar)&0xff)==0x0a)||((((*++pChar)&0xff)==0x0a))) {
               // removed second char
               ++pChar;
            }
            iClipLength -= (int)strlen(RTF_CRLF);
            if(iClipLength <= 0) { assert(0); return false; }
            strcpy(pDest, RTF_CRLF);
            pDest += (int)strlen(RTF_CRLF);
            continue; // because char is ready
            break;
         case 0:
            ++pChar;
            continue; // zero is not valid
            break;
         default:
            break;
      };
      // copy the char
      --iClipLength;
      if(iClipLength <= 0) { assert(0); return false; }
      *pDest++ = ((*pChar)&0xff);
      ++pChar;
   }
   iClipLength -= (int)_RtfFooter.length();
   if(iClipLength <= 0) { assert(0); return false; }
   strcpy(pDest, _RtfFooter.c_str());
   return true;
}

std::vector<MenuItemUnit> ScintillaSearchView::getContextMenu() const {
   std::vector<MenuItemUnit> tmp;
   //example tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_COPY, TEXT("Copy")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SELECTALL, TEXT("Select All")));
   tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SEARCH, TEXT("Find... [Ctrl+F]")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SAVEFILE, TEXT("Save to file...")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SAVE_CLR, TEXT("Reset save file")));
   return tmp;
}

LRESULT ScintillaSearchView::scintillaNew_Proc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) 
{
    switch (Message)
    {
   case WM_COMMAND:
      {
         switch (wParam) 
         {
            case FNDRESDLG_SCINTILLAFINFER_COPY:
               execute(SCI_COPY);
               doRichTextCopy();
               return 0; // ready with processing
            case FNDRESDLG_SCINTILLAFINFER_SELECTALL:
               execute(SCI_SELECTALL);
               break;
            case FNDRESDLG_SCINTILLAFINFER_SEARCH:
            case FNDRESDLG_SCINTILLAFINFER_SAVEFILE:
            case FNDRESDLG_SCINTILLAFINFER_SAVE_CLR:
               // deferre to parent window
               ::SendMessage(_hParent, WM_COMMAND, wParam, (LPARAM)0);
               break;
            default:
               break;
         };
         break;
      }
    case WM_CONTEXTMENU:
      {
            //Point pt = Point::FromLong(lParam);
         POINT pt = {static_cast<short>(LOWORD(lParam)), 
                     static_cast<short>(HIWORD(lParam))};
            if ((pt.x == -1) && (pt.y == -1)) {
                // Caused by keyboard so display menu near caret
            //pt = LocationFromPosition((int)execute(SCI_GETCURRENTPOS));
                int pos = (int)execute(SCI_GETCURRENTPOS);
            pt.x = (int)execute(SCI_POINTXFROMPOSITION,0,(LPARAM)pos);
            pt.y = (int)execute(SCI_POINTYFROMPOSITION,0,(LPARAM)pos);
            POINT spt = {pt.x, pt.y};
            ::ClientToScreen(_hSelf, &spt);
                pt = spt;
            }

         ContextMenu scintillaContextmenu;
         scintillaContextmenu.create(_hSelf, getContextMenu());
         scintillaContextmenu.display(pt);
         
            return 0;
      }
      case WM_KEYDOWN: 
      {
         DBG2("WM_KEYDOWN key %x flags %x.", wParam, lParam);
         if((::GetKeyState(VK_CONTROL) & 0x80000000) != 0) {
            if (wParam=='C') {
               // ctrl-c
               ::SendMessage(_hParent, WM_COMMAND, FNDRESDLG_SCINTILLAFINFER_COPY,(LPARAM)0);
               return 0;
            } 
         }
         break; // let base win work
         
      }
      case EM_SETSEL:
      {
         DBG0("SetSelection");
         // we stop the selection call after set focus to keep current selection alive
         return 1;
      }

   }
   return ScintillaEditView::scintillaNew_Proc(hwnd, Message, wParam, lParam);
}




void ScintillaSearchView::updateLineNumberWidth(bool lineNumbersShown) 
{
   if (lineNumbersShown)
   {
      int linesVisible = (int) execute(SCI_LINESONSCREEN);
      if (linesVisible)
      {
         int iNumLines = (int) execute(SCI_GETLINECOUNT);
         int iLineNumColSize = (iNumLines<10)?1:
            (iNumLines<100)?2:
            (iNumLines<1000)?3:
            (iNumLines<10000)?4:
            (iNumLines<100000)?5:
            (iNumLines<1000000)?6:
            (iNumLines<10000000)?7:
            (iNumLines<100000000)?8:
            (iNumLines<1000000000)?9:10;

         int pixelWidth = int(4 + (iLineNumColSize * execute(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"8")));
         DBG2("updateLineNumberWidth() iLineNumColSize=%d, pixelWidth=%d", iLineNumColSize, pixelWidth);
         execute(SCI_SETMARGINWIDTHN, _SC_MARGE_LINENUMBER, pixelWidth);
      }
   } else {
      execute(SCI_SETMARGINWIDTHN, _SC_MARGE_LINENUMBER, 0);
   }
}

