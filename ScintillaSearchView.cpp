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

#include <windows.h>
#include <ShellAPI.h>
#include <string.h>
#include "ScintillaSearchView.h"
#include "chardefines.h"
#include "Common.h"
//#include "resource.h"

#define MDBG_COMP "SSView:" 
#include "myDebug.h"
#include <Shlwapi.h>

long ScintillaSearchView::NppVersion = 0;

#define RTF_HEADER_BEGIN "{\\rtf1\\ansi\\ansicpg\\lang1024\\noproof1252\\uc1 \\deff0{\\fonttbl{\\f0\\fnil\\fcharset0\\fprq1 Courier New;}}\n{\\colortbl"
// inbetween color table
#define RTF_HEADER_END "}\n\\fs20\\sa0\\sl0\n" // ensure line spacing 1-line
#define RTF_CRLF "\\par " // eol
#define RTF_FOOTER "\\par }" // end doc
#define RTF_COLNUM "\\cf" // fg color
#define RTF_BGCOLNUM "\\highlight" // bg color

// available style id -> sub sequent 0-based index in color table
// as defined in counter definition is tclFindResultDlg::transStyleId
const int ScintillaSearchView::transStylePosTab[MY_STYLE_MASK+1] = {
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
   110,111,112,113,114,115,116,117,118,119,
   120,121,122,123,124,125,126,127,128,129,
   130,131,132,133,134,135,136,137,138,139,
   140,141,142,143,144,145,146,147,148,149,
   150,151,152,153,154,155,156,157,158,159,
   160,161,162,163,164,165,166,167,168,169,
   170,171,172,173,174,175,176,177,178,179,
   180,181,182,183,184,185,186,187,188,189,
   190,191,192,193,194,195,196,197,198,199,
   200,201,202,203,204,205,206,207,208,209,
   210,211,212,213,214,215,216,217,218,219,
   220,221,222,223,224,225,226,227,228,229,
   230,231,232,233,234,235,236,237,238,239,
   240,241,242,243,244,245,246,247
};
int ScintillaSearchView::transStylePos(unsigned char stid) const {
   int i = transStylePosTab[stid];
   return i;
}
void ScintillaSearchView::init(HINSTANCE hInst, HWND hPere)
{
#pragma warning(disable:4312 4311)
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
		throw std::runtime_error("ScintillaEditView::init : CreateWindowEx() function return null");
    }

    _pScintillaFunc = (SCINTILLA_FUNC)::SendMessage(_hSelf, SCI_GETDIRECTFUNCTION, 0, 0);
    _pScintillaPtr = (SCINTILLA_PTR)::SendMessage(_hSelf, SCI_GETDIRECTPOINTER, 0, 0);

//    _userDefineDlg.init(_hInst, _hParent, this);

	if (!_pScintillaFunc)
	{
		throw std::runtime_error("ScintillaEditView::init : SCI_GETDIRECTFUNCTION message failed");
	}

	if (!_pScintillaPtr)
	{
		throw std::runtime_error("ScintillaEditView::init : SCI_GETDIRECTPOINTER message failed");
	}

	execute(SCI_SETMARGINMASKN, _SC_MARGE_FOLDER, SC_MASK_FOLDERS);
	showMargin(_SC_MARGE_FOLDER, true);

	execute(SCI_SETMARGINMASKN, _SC_MARGE_SYMBOL, (1<<MARK_BOOKMARK) | (1<<MARK_HIDELINESBEGIN) | (1<<MARK_HIDELINESEND) | (1<<MARK_HIDELINESUNDERLINE));

	execute(SCI_MARKERSETALPHA, MARK_BOOKMARK, 70);

	execute(SCI_MARKERDEFINE, MARK_HIDELINESUNDERLINE, SC_MARK_UNDERLINE);
	execute(SCI_MARKERSETBACK, MARK_HIDELINESUNDERLINE, 0x77CC77);

	if (NppParameters::getInstance()._dpiManager.scaleX(100) >= 150)
	{
		execute(SCI_RGBAIMAGESETWIDTH, 18);
		execute(SCI_RGBAIMAGESETHEIGHT, 18);
		execute(SCI_MARKERDEFINERGBAIMAGE, MARK_BOOKMARK, reinterpret_cast<LPARAM>(bookmark18));
		execute(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESBEGIN, reinterpret_cast<LPARAM>(hidelines_begin18));
		execute(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESEND, reinterpret_cast<LPARAM>(hidelines_end18));
	}
	else
	{
		execute(SCI_RGBAIMAGESETWIDTH, 14);
		execute(SCI_RGBAIMAGESETHEIGHT, 14);
		execute(SCI_MARKERDEFINERGBAIMAGE, MARK_BOOKMARK, reinterpret_cast<LPARAM>(bookmark14));
		execute(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESBEGIN, reinterpret_cast<LPARAM>(hidelines_begin14));
		execute(SCI_MARKERDEFINERGBAIMAGE, MARK_HIDELINESEND, reinterpret_cast<LPARAM>(hidelines_end14));
	}

    execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_FOLDER, true);
    execute(SCI_SETMARGINSENSITIVEN, _SC_MARGE_SYMBOL, true);

    execute(SCI_SETFOLDFLAGS, 16);
	execute(SCI_SETSCROLLWIDTHTRACKING, true);
	execute(SCI_SETSCROLLWIDTH, 1);	//default empty document: override default width of 2000

	// smart hilighting
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_SMART, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_INC, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_TAGMATCH, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_TAGATTR, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT1, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT2, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT3, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT4, INDIC_ROUNDBOX);
	execute(SCI_INDICSETSTYLE, SCE_UNIVERSAL_FOUND_STYLE_EXT5, INDIC_ROUNDBOX);

	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_SMART, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_INC, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_TAGMATCH, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_TAGATTR, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT1, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT2, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT3, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT4, 100);
	execute(SCI_INDICSETALPHA, SCE_UNIVERSAL_FOUND_STYLE_EXT5, 100);

	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_SMART, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_INC, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_TAGMATCH, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_TAGATTR, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT1, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT2, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT3, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT4, true);
	execute(SCI_INDICSETUNDER, SCE_UNIVERSAL_FOUND_STYLE_EXT5, true);
    
//	if ((NppParameters::getInstance()).getNppGUI()._writeTechnologyEngine == directWriteTechnology)
//		execute(SCI_SETTECHNOLOGY, SC_TECHNOLOGY_DIRECTWRITE);
   //switch off the context menu off scintilla as we have our own
   execute(SCI_USEPOPUP, 0);
//TODO evtl doch benutzen	_pParameter = NppParameters::getInstance();
    
    _codepage = ::GetACP();


    ::SetWindowLongPtr(_hSelf, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    _callWindowProc = CallWindowProc;
    _scintillaDefaultProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(_hSelf, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ScintillaSearchView::scintillaStatic_Proc)));

//	if (_defaultCharList.empty())
//	{
//		auto defaultCharListLen = execute(SCI_GETWORDCHARS);
//		char *defaultCharList = new char[defaultCharListLen + 1];
//		execute(SCI_GETWORDCHARS, 0, reinterpret_cast<LPARAM>(defaultCharList));
//		defaultCharList[defaultCharListLen] = '\0';
//		_defaultCharList = defaultCharList;
//		delete[] defaultCharList;
//	}
    //Get the startup document and make a buffer for it so it can be accessed like a file
    //attachDefaultDoc();
#pragma warning(default:4312 4311)
}

void ScintillaSearchView::startRtfColorTable(unsigned defColor) {
   _RtfColTbl = "";
   addRtfColor2Table(defColor);
}

void ScintillaSearchView::addRtfColor2Table(unsigned color) {
   char styleNum[4];
   _RtfColTbl += RTF_COLTAG_RED;
   _RtfColTbl += _itoa(RTF_COL_R(color), styleNum, 10);
   _RtfColTbl += RTF_COLTAG_GREEN;
   _RtfColTbl += _itoa(RTF_COL_G(color), styleNum, 10);
   _RtfColTbl += RTF_COLTAG_BLUE;
   _RtfColTbl += _itoa(RTF_COL_B(color), styleNum, 10);
   _RtfColTbl += RTF_COLTAG_END;
   _RtfColTbl += RTF_COLTAG_RED;
   _RtfColTbl += _itoa(RTF_COL_R(color), styleNum, 10);
   _RtfColTbl += RTF_COLTAG_GREEN;
   _RtfColTbl += _itoa(RTF_COL_G(color), styleNum, 10);
   _RtfColTbl += RTF_COLTAG_BLUE;
   _RtfColTbl += _itoa(RTF_COL_B(color), styleNum, 10);
   _RtfColTbl += RTF_COLTAG_END;
}

void ScintillaSearchView::finalizeRtfColorTable() {
   _RtfFooter = RTF_FOOTER;
   _RtfHeader = RTF_HEADER_BEGIN;
   _RtfHeader += _RtfColTbl;
   _RtfHeader += RTF_HEADER_END;
}

bool ScintillaSearchView::doRichTextCopy(const TCHAR* filename) {
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
   int iEnd;
   int iBegin;
   if (!filename) {
      iEnd = (int)execute(SCI_GETSELECTIONEND);
      iBegin = (int)execute(SCI_GETSELECTIONSTART);
   }
   else {
      iBegin = 0;
      iEnd = (int)execute(SCI_GETLENGTH);
   }
   long mv = HIWORD(NppVersion);
   long lv = LOWORD(NppVersion);
   int p = sizeof(Sci_PositionCR);
   if (sizeof(Sci_PositionCR) == 8 && ( mv < 8 || (mv == 8 && lv < 300))) {
      generic_string serr = TEXT("You are using a 64-Bit version smaller as v8.3, but newer AnalysePlugin as v1.13.\n") 
                            TEXT("In this case Clipboard copy function cannot be used.\n") 
                            TEXT("Please update NPP to a newer version!");
      ::MessageBox(getHSelf(), serr.c_str(), TEXT("Incompatible NPP Version!"), MB_ICONERROR | MB_OK);
      return false;
   }
   Sci_TextRange tr;
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
   int iClipLength = (iColCount*20)+2              // fg and bg color control words with three digets index + CRLF
                     +(iParCount*5)                // count of crlf
                     +iSelTextLength               // the text
                     +iEscapeCount                 // \ { } escaping
                     +(int)_RtfHeader.length()     // rtf header including color table
                     +(int)_RtfFooter.length();    // rtf footer
      
   // first create local mem to prepare the buffer
   char* pClip = new char[iClipLength];
   int iClipLengthRtfText = iClipLength;
   bool bRtfBufferOk = prepareRtfClip(pClip, iClipLength, tr.lpstrText, iSelTextLength);
   delete[] tr.lpstrText;
   DBG1("doRichTextCopy() estimated size delta (must be positive) %d",(iClipLengthRtfText - iClipLength));

   if (filename) {
      writeFileContent(filename, pClip);
      DBG1("doRichTextCopy() Wrote text to file as RTF %s", filename);
   }
   else {
      UINT uClipFormatId = RegisterClipboardFormat(TEXT("Rich Text Format"));
      if (uClipFormatId == 0) {
         DBG0("doRichTextCopy() ERROR enumerate clipboard format RTF");
         return false;
      }

      if (!::OpenClipboard(_hSelf)) {
         ::CloseClipboard(); // try a second time
         ::Sleep(100);
         if (!::OpenClipboard(_hSelf)) {
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
      if (pStr == 0) {
         DBG0("doRichTextCopy() ERROR could not GlobalLock");
         return false;
      }
      //strcpy(pStr, str2cpy);
      (void)memcpy(pStr, pClip, iClipLength);
      //FI* f = fopen("c:\\temp\\notepad_text.rtf", "wt");
      //fwrite(pStr, 1, strlen(pStr), f);
      //fclose(f);

      ::GlobalUnlock(hglbCopy);

      // Place the handle on the clipboard. 
      if (bRtfBufferOk) {
         ::SetClipboardData(uClipFormatId, hglbCopy);
      }
      else {
         GlobalFree(hglbCopy);
      }
      ::CloseClipboard();
   }
   delete[] pClip;
   return true;
}

int ScintillaSearchView::countColorChanges(const Sci_TextRange& rtr){
   int iCount=0;
   short* pwChar= (short*)rtr.lpstrText;
   char prevStyle = -1;
   char style = -1;
   short* pwEnd = (short*)rtr.lpstrText + rtr.chrg.cpMax - rtr.chrg.cpMin;
   for(; pwChar < pwEnd; ++pwChar) {
      if(prevStyle !=(style = ((*pwChar)>>8))) {
         ++iCount;
         prevStyle = style;
      }
   }
   return iCount;
}


int ScintillaSearchView::countLinefeeds(const Sci_TextRange& rtr){
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

int ScintillaSearchView::countEscapeChars(const Sci_TextRange& rtr){
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

bool ScintillaSearchView::prepareRtfClip(char *pGlobalText, int& iClipLength, char* lpSelText, int iSelTextLength){
   char* pDest = pGlobalText;
   memset(pDest, 0, iClipLength);
   // RTF Header
   iClipLength -= (int)_RtfHeader.length();
   if(iClipLength <= 0) { assert(iClipLength > 0); return false; }
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
         if(iClipLength <= 0) { assert(iClipLength > 0); return false; }
         strcpy(pDest, RTF_COLNUM);
         pDest += strlen(RTF_COLNUM);
         _i64toa((transStylePos(style) * 2), styleNum, 10); // every first of two is fgColor
         iClipLength -= (int)strlen(styleNum);
         if (iClipLength <= 0) { assert(iClipLength > 0); return false; }
         strcpy(pDest, styleNum);
         pDest += strlen(styleNum); 
         iClipLength -= (int)strlen(RTF_BGCOLNUM);
         if (iClipLength <= 0) { assert(iClipLength > 0); return false; }
         strcpy(pDest, RTF_BGCOLNUM);
         pDest += strlen(RTF_BGCOLNUM);
         _i64toa((transStylePos(style) * 2) + 1, styleNum, 10); // every second color is bgColor
         iClipLength -= (int)strlen(styleNum);
         if(iClipLength <= 0) { assert(iClipLength > 0); return false; }
         strcpy(pDest, styleNum);
         pDest += strlen(styleNum);
         --iClipLength;
         if(iClipLength <= 0) { assert(iClipLength > 0); return false; }
         strcpy(pDest++, " ");
         prevStyle = style;
      }
      // text
      switch((*pChar)&0xff) {
         case '\\':
         case '{':
         case '}':
            --iClipLength;
            if(iClipLength <= 0) { assert(iClipLength > 0); return false; }
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
            if(iClipLength <= 0) { assert(iClipLength > 0); return false; }
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
      if(iClipLength <= 0) { assert(iClipLength > 0); return false; }
      *pDest++ = ((*pChar)&0xff);
      ++pChar;
   }
   iClipLength -= (int)_RtfFooter.length();
   if(iClipLength < 0) { assert(iClipLength > 0); return false; }
   strcpy(pDest, _RtfFooter.c_str());
   iClipLength = (int)strlen(pGlobalText)+1; // expected to be same or shorter as at begin
   return true;
}

void ScintillaSearchView::doSaveRichtext() {
   OPENFILENAME ofn;       // common dialog box structure
   TCHAR szFile[MAX_PATH] = TEXT("");       // buffer for file name
   generic_strncpy(szFile, _lastRtfFilename.c_str(), MAX_PATH);
   // Initialize OPENFILENAME
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = _hSelf;
   ofn.lpstrFile = szFile;
   ofn.nMaxFile = COUNTCHAR(szFile);
   ofn.lpstrFilter = TEXT("Richtext\0*.rtf\0");
   ofn.nFilterIndex = 2;
   ofn.lpstrFileTitle = TEXT("Save Analyse Search Result once as Richtext File");
   ofn.nMaxFileTitle = 0;// strlen("Open Analyse Config File")+1;
   ofn.lpstrInitialDir = szFile;
   ofn.Flags = OFN_OVERWRITEPROMPT; // OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

   if (GetSaveFileName(&ofn) == TRUE)
   {
      generic_string dot = PathFindExtension(szFile);
      if (dot == TEXT("") && ((generic_strlen(szFile)+4) < MAX_PATH)){
         generic_strncat(szFile, TEXT(".rtf"), 5);
      }
      _lastRtfFilename = szFile;
      doRichTextCopy(szFile);
   }
}

void ScintillaSearchView::setWrapMode(bool bOn) {
   if (bOn) {
      execute(SCI_SETWRAPMODE, SC_WRAP_WORD);
   }
   else {
      execute(SCI_SETWRAPMODE, SC_WRAP_NONE);
   }
}

bool ScintillaSearchView::getWrapMode() const {
   return (execute(SCI_GETWRAPMODE) != SC_WRAP_NONE);
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
   tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SAVE_RTF, TEXT("Save once as Richtext...")));
   tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_WRAP_MODE, TEXT("Word Wrap")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_SHOW_LINE_NUMBERS, TEXT("Show line numbers")));
   tmp.push_back(MenuItemUnit(FNDRESDLG_SHOW_OPTIONS, TEXT("Options...")));
   
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
            case FNDRESDLG_SHOW_LINE_NUMBERS:
            case FNDRESDLG_SHOW_OPTIONS:
               // deferre to parent window
               ::SendMessage(_hParent, WM_COMMAND, wParam, (LPARAM)0);
               break;
            case FNDRESDLG_SCINTILLAFINFER_SAVE_RTF:
               doSaveRichtext();
               break;
            case FNDRESDLG_WRAP_MODE:
               setWrapMode(!getWrapMode());
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
            LPARAM lp = MAKELONG(pt.x, pt.y);
            ::SendMessage(_hParent, WM_COMMAND, FNDRESDLG_SHOW_CONTEXTMENU, lp);
        
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

