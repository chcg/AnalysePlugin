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
#include "stdafx.h"
#include "precompiledHeaders.h"


#include <windows.h>
#include <ShellAPI.h>
#include <string.h>
#include <windows.h>
#include "ScintillaSearchView.h"
#include "myDebug.h"

#ifdef UNICODE
#include "Common.h"
#endif

#define RTF_HEADER "{\\rtf1\\ansi\\ansicpg\\lang1024\\noproof1252\\uc1 \\deff0{\\fonttbl{\\f0\\fnil\\fcharset0\\fprq1 Courier New;}}\n{\\colortbl" // end
#define RTF_CRLF "\\par " // end
#define RTF_FOOTER "\\par }" // end
#define RTF_COLNUM "\\cf" // end

// initialize the static variable
HINSTANCE ScintillaSearchView::_hLib = ::LoadLibrary(TEXT("SciLexer.DLL"));
int ScintillaSearchView::_refCount = 0;
//UserDefineDialog ScintillaSearchView::_userDefineDlg;

const int ScintillaSearchView::_SC_MARGE_LINENUMBER = 0;
const int ScintillaSearchView::_SC_MARGE_SYBOLE = 1;
const int ScintillaSearchView::_SC_MARGE_FOLDER = 2;

const int ScintillaSearchView::_MARGE_LINENUMBER_NB_CHIFFRE = 5;

WNDPROC ScintillaSearchView::_scintillaDefaultProc = NULL;
/*
SC_MARKNUM_*     | Arrow               Plus/minus           Circle tree                 Box tree 
-------------------------------------------------------------------------------------------------------------
FOLDEROPEN       | SC_MARK_ARROWDOWN   SC_MARK_MINUS     SC_MARK_CIRCLEMINUS            SC_MARK_BOXMINUS 
FOLDER           | SC_MARK_ARROW       SC_MARK_PLUS      SC_MARK_CIRCLEPLUS             SC_MARK_BOXPLUS 
FOLDERSUB        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_VLINE                  SC_MARK_VLINE 
FOLDERTAIL       | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_LCORNERCURVE           SC_MARK_LCORNER 
FOLDEREND        | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEPLUSCONNECTED    SC_MARK_BOXPLUSCONNECTED 
FOLDEROPENMID    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_CIRCLEMINUSCONNECTED   SC_MARK_BOXMINUSCONNECTED 
FOLDERMIDTAIL    | SC_MARK_EMPTY       SC_MARK_EMPTY     SC_MARK_TCORNERCURVE           SC_MARK_TCORNER 
*/

const int ScintillaSearchView::_markersArray[][NB_FOLDER_STATE] = {
  {SC_MARKNUM_FOLDEROPEN, SC_MARKNUM_FOLDER, SC_MARKNUM_FOLDERSUB, SC_MARKNUM_FOLDERTAIL, SC_MARKNUM_FOLDEREND,        SC_MARKNUM_FOLDEROPENMID,     SC_MARKNUM_FOLDERMIDTAIL},
  {SC_MARK_MINUS,         SC_MARK_PLUS,      SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_ARROWDOWN,     SC_MARK_ARROW,     SC_MARK_EMPTY,        SC_MARK_EMPTY,         SC_MARK_EMPTY,               SC_MARK_EMPTY,                SC_MARK_EMPTY},
  {SC_MARK_CIRCLEMINUS,   SC_MARK_CIRCLEPLUS,SC_MARK_VLINE,        SC_MARK_LCORNERCURVE,  SC_MARK_CIRCLEPLUSCONNECTED, SC_MARK_CIRCLEMINUSCONNECTED, SC_MARK_TCORNERCURVE},
  {SC_MARK_BOXMINUS,      SC_MARK_BOXPLUS,   SC_MARK_VLINE,        SC_MARK_LCORNER,       SC_MARK_BOXPLUSCONNECTED,    SC_MARK_BOXMINUSCONNECTED,    SC_MARK_TCORNER}
};

//Array with all the names of all languages
LanguageName ScintillaSearchView::langNames[L_EXTERNAL+1] = {
{TEXT("normal"),		TEXT("Normal text"),				TEXT("Normal text file"),										L_TEXT,			SCLEX_NULL},					
{TEXT("php"),			TEXT("PHP"),						TEXT("PHP Hypertext Preprocessor file"),						L_PHP,			SCLEX_HTML},
{TEXT("c"),			TEXT("C"),						TEXT("C source file"),										L_C,			SCLEX_CPP},
{TEXT("cpp"),			TEXT("C++"),						TEXT("C++ source file"),										L_CPP,			SCLEX_CPP},
{TEXT("cs"),			TEXT("C#"),						TEXT("C# source file"),										L_CS,			},
{TEXT("objc"),		TEXT("Objective-C"),				TEXT("Objective-C source file"),								L_OBJC,			},
{TEXT("java"),		TEXT("Java"),						TEXT("Java source file"),										L_JAVA,			SCLEX_CPP},
{TEXT("rc"),			TEXT("RC"),						TEXT("Windows Resource file"),								L_RC,			SCLEX_CPP},
{TEXT("html"),		TEXT("HTML"),						TEXT("Hyper Text Markup Language file"),						L_HTML,			SCLEX_HTML},
{TEXT("xml"),			TEXT("XML"),						TEXT("eXtensible Markup Language file"),						L_XML,			SCLEX_XML},
{TEXT("makefile"),	TEXT("Makefile"),					TEXT("Makefile"),												L_MAKEFILE,		SCLEX_MAKEFILE},
{TEXT("pascal"),		TEXT("Pascal"),					TEXT("Pascal source file"),									L_PASCAL,		SCLEX_PASCAL},
{TEXT("batch"),		TEXT("Batch"),					TEXT("Batch file"),											L_BATCH,		SCLEX_BATCH},
{TEXT("ini"),			TEXT("ini"),						TEXT("MS ini file"),											L_INI,			SCLEX_PROPERTIES},
{TEXT("nfo"),			TEXT("NFO"),						TEXT("MSDOS Style/ASCII Art"),								L_ASCII,			SCLEX_NULL},
{TEXT("udf"),			TEXT("udf"),						TEXT("User Define File"),										L_USER,			SCLEX_USER},
{TEXT("asp"),			TEXT("ASP"),						TEXT("Active Server Pages script file"),						L_ASP,			SCLEX_HTML},
{TEXT("sql"),			TEXT("SQL"),						TEXT("Structured Query Language file"),						L_SQL,			SCLEX_SQL},
{TEXT("vb"),			TEXT("VB"),						TEXT("Visual Basic file"),									L_VB,			SCLEX_VB},
{TEXT("javascript"),	TEXT("JavaScript"),				TEXT("JavaScript file"),										L_JS,			SCLEX_CPP},
{TEXT("css"),			TEXT("CSS"),						TEXT("Cascade Style Sheets File"),							L_CSS,			SCLEX_CSS},
{TEXT("perl"),		TEXT("Perl"),						TEXT("Perl source file"),										L_PERL,			SCLEX_PERL},
{TEXT("python"),		TEXT("Python"),					TEXT("Python file"),											L_PYTHON,		SCLEX_PYTHON},
{TEXT("lua"),			TEXT("Lua"),						TEXT("Lua source File"),										L_LUA,			SCLEX_LUA},
{TEXT("tex"),			TEXT("TeX"),						TEXT("TeX file"),												L_TEX,			SCLEX_TEX},
{TEXT("fortran"),		TEXT("Fortran"),					TEXT("Fortran source file"),									L_FORTRAN,		SCLEX_FORTRAN},
{TEXT("bash"),		TEXT("Shell"),					TEXT("Unix script file"),										L_BASH,			SCLEX_BASH},
{TEXT("actionscript"), TEXT("Flash Action"),				TEXT("Flash Action script file"),								L_FLASH,		SCLEX_OBJC},		//WARNING, was "flash"
{TEXT("nsis"),		TEXT("NSIS"),						TEXT("Nullsoft Scriptable Install System script file"),		L_NSIS,			SCLEX_NSIS},
{TEXT("tcl"),			TEXT("TCL"),						TEXT("Tool Command Language file"),							L_TCL,			SCLEX_TCL},
{TEXT("lisp"),		TEXT("Lisp"),						TEXT("List Processing language file"),						L_LISP,			SCLEX_LISP},
{TEXT("scheme"),		TEXT("Scheme"),					TEXT("Scheme file"),											L_SCHEME,		SCLEX_LISP},
{TEXT("asm"),			TEXT("Assembly"),					TEXT("Assembly language source file"),						L_ASM,			SCLEX_ASM},
{TEXT("diff"),		TEXT("Diff"),						TEXT("Diff file"),											L_DIFF,			SCLEX_DIFF},
{TEXT("props"),		TEXT("Properties file"),			TEXT("Properties file"),										L_PROPS,		SCLEX_PROPERTIES},
{TEXT("postscript"),	TEXT("Postscript"),				TEXT("Postscript file"),										L_PS,			SCLEX_PS},
{TEXT("ruby"),		TEXT("Ruby"),						TEXT("Ruby file"),											L_RUBY,			SCLEX_RUBY},
{TEXT("smalltalk"),	TEXT("Smalltalk"),				TEXT("Smalltalk file"),										L_SMALLTALK,	SCLEX_SMALLTALK},
{TEXT("vhdl"),		TEXT("VHDL"),						TEXT("VHSIC Hardware Description Language file"),				L_VHDL,			SCLEX_VHDL},
{TEXT("kix"),			TEXT("KiXtart"),					TEXT("KiXtart file"),											L_KIX,			SCLEX_KIX},
{TEXT("autoit"),		TEXT("AutoIt"),					TEXT("AutoIt"),												L_AU3,			SCLEX_AU3},
{TEXT("caml"),		TEXT("CAML"),						TEXT("Categorical Abstract Machine Language"),				L_CAML,			SCLEX_CAML},
{TEXT("ada"),			TEXT("Ada"),						TEXT("Ada file"),												L_ADA,			SCLEX_ADA},
{TEXT("verilog"),		TEXT("Verilog"),					TEXT("Verilog file"),											L_VERILOG,		SCLEX_VERILOG},
{TEXT("matlab"),		TEXT("MATLAB"),					TEXT("MATrix LABoratory"),									L_MATLAB,		SCLEX_MATLAB},
{TEXT("haskell"),		TEXT("Haskell"),					TEXT("Haskell"),												L_HASKELL,		SCLEX_HASKELL},
{TEXT("inno"),		TEXT("Inno"),						TEXT("Inno Setup script"),									L_INNO,			SCLEX_INNOSETUP},
{TEXT("searchResult"), TEXT("Internal Search"),			TEXT("Internal Search"),										L_SEARCHRESULT,	SCLEX_SEARCHRESULT},
{TEXT("cmake"),		TEXT("CMAKEFILE"),				TEXT("CMAKEFILE"),											L_CMAKE,		SCLEX_CMAKE},
{TEXT("yaml"),		TEXT("YAML"),						TEXT("YAML Ain't Markup Language"),							L_YAML,			SCLEX_YAML},
{TEXT("ext"),			TEXT("External"),					TEXT("External"),												L_EXTERNAL,		SCLEX_NULL}
};

//const int MASK_RED   = 0xFF0000;
//const int MASK_GREEN = 0x00FF00;
//const int MASK_BLUE  = 0x0000FF;

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

	//Use either Unicode or ANSI setwindowlong, depending on environment
	if (::IsWindowUnicode(_hSelf)) 
	{
		::SetWindowLongPtrW(_hSelf, GWL_USERDATA, reinterpret_cast<LONG>(this));
		_callWindowProc = CallWindowProcW;
		_scintillaDefaultProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtrW(_hSelf, GWL_WNDPROC, reinterpret_cast<LONG>(scintillaStatic_Proc)));
	}
	else 
	{
		::SetWindowLongPtrA(_hSelf, GWL_USERDATA, reinterpret_cast<LONG>(this));
		_callWindowProc = CallWindowProcA;
		_scintillaDefaultProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtrA(_hSelf, GWL_WNDPROC, reinterpret_cast<LONG>(scintillaStatic_Proc)));
	}

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
         itoa(style, styleNum, 10);
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
               return FALSE; // ready with processing
            case FNDRESDLG_SCINTILLAFINFER_SELECTALL:
               execute(SCI_SELECTALL);
               break;
            case FNDRESDLG_SCINTILLAFINFER_SEARCH:
               // deferre to parent window
               ::SendMessage(_hParent, WM_COMMAND, FNDRESDLG_SCINTILLAFINFER_SEARCH, (LPARAM)0);
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
//			ContextMenu(pt);

         ContextMenu scintillaContextmenu;
         std::vector<MenuItemUnit> tmp;
         //example tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
         tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_COPY, TEXT("Copy")));
         tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SELECTALL, TEXT("Select All")));
         tmp.push_back(MenuItemUnit(0, TEXT("Separator")));
         tmp.push_back(MenuItemUnit(FNDRESDLG_SCINTILLAFINFER_SEARCH, TEXT("Find... [Ctrl+F]")));
         scintillaContextmenu.create(_hSelf, tmp);
         scintillaContextmenu.display(pt);
         
     //    bool writable = !(bool)execute(SCI_GETREADONLY);
		   //popup.CreatePopUp();
		   //AddToPopUp(TEXT("Undo"), idcmdUndo, writable && pdoc->CanUndo());
		   //AddToPopUp(TEXT("Redo"), idcmdRedo, writable && pdoc->CanRedo());
		   //AddToPopUp(TEXT(""));
		   //AddToPopUp(TEXT("Cut"), idcmdCut, writable && currentPos != anchor);
		   //AddToPopUp(TEXT("Copy"), idcmdCopy, currentPos != anchor);
		   //AddToPopUp(TEXT("Paste"), idcmdPaste, writable && WndProc(SCI_CANPASTE, 0, 0));
		   //AddToPopUp(TEXT("Delete"), idcmdDelete, writable && currentPos != anchor);
		   //AddToPopUp(TEXT(""));
		   //AddToPopUp(TEXT("Select All"), idcmdSelectAll);
		   //popup.Show(pt, wMain);
			return 0;
      }
      case WM_KEYDOWN: 
      {
         DBG2("WM_KEYDOWN key %x flags %x.", wParam, lParam);
         if((::GetKeyState(VK_CONTROL) & 0x80000000) != 0) {
            if (wParam=='C') {
               // ctrl-c
               ::SendMessage(_hParent, WM_COMMAND, FNDRESDLG_SCINTILLAFINFER_COPY,(LPARAM)0);
               return false;
            } 
         }
         break; // let base win work
         
      }
		case WM_CHAR :
		{
			//if (execute(SCI_SELECTIONISRECTANGLE) && !(::GetKeyState(VK_LCONTROL) & 0x80000000))
			//{
			//	if (wParam != VK_ESCAPE)
			//	{
			//		execute(SCI_BEGINUNDOACTION);

			//		ColumnModeInfo colInfos = getColumnModeSelectInfo();
			//		columnReplace(colInfos, (char*)wParam);

			//		execute(SCI_ENDUNDOACTION);
			//		execute(SCI_SETCURRENTPOS,colInfos[colInfos.size()-1].second);
			//	}
			//	else
			//	{
			//		int pos = (int)execute(SCI_GETSELECTIONSTART);
			//		execute(SCI_SETSEL, pos, pos);
			//	}
			//	return TRUE;
			//} 
			break;
		}

		case WM_MOUSEHWHEEL :
		{
			::CallWindowProc(_scintillaDefaultProc, hwnd, WM_HSCROLL, ((short)HIWORD(wParam) > 0)?SB_LINERIGHT:SB_LINELEFT, NULL);
			break;
		}

		case WM_MOUSEWHEEL :
		{
			if (LOWORD(wParam) & MK_RBUTTON)
			{
				::SendMessage(_hParent, Message, wParam, lParam);
				return TRUE;
			}

			//Have to perform the scroll first, because the first/last line do not get updated untill after the scroll has been parsed
			LRESULT scrollResult = ::CallWindowProc(_scintillaDefaultProc, hwnd, Message, wParam, lParam);
			return scrollResult;
			break;
		}

		case WM_VSCROLL :
		{
			break;
		}
	}
	return _callWindowProc(_scintillaDefaultProc, hwnd, Message, wParam, lParam);
}

void ScintillaSearchView::setSpecialIndicator(Style & styleToSet)
{
	execute(SCI_INDICSETFORE, styleToSet._styleID, styleToSet._bgColor);
}


void ScintillaSearchView::setSpecialStyle(Style & styleToSet)
{
	int styleID = styleToSet._styleID;
	if ( styleToSet._colorStyle & COLORSTYLE_FOREGROUND )
	    execute(SCI_STYLESETFORE, styleID, styleToSet._fgColor);

    if ( styleToSet._colorStyle & COLORSTYLE_BACKGROUND )
	    execute(SCI_STYLESETBACK, styleID, styleToSet._bgColor);
    
    if ((!styleToSet._fontName)||(lstrcmp(styleToSet._fontName, TEXT(""))))
	{
#ifdef UNICODE
		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
		const char * fontNameA = wmc->wchar2char(styleToSet._fontName, CP_ACP);
		execute(SCI_STYLESETFONT, (WPARAM)styleID, (LPARAM)fontNameA);
#else
		execute(SCI_STYLESETFONT, (WPARAM)styleID, (LPARAM)styleToSet._fontName);
#endif
	}
	int fontStyle = styleToSet._fontStyle;
    if (fontStyle != -1)
    {
        execute(SCI_STYLESETBOLD,		(WPARAM)styleID, fontStyle & FONTSTYLE_BOLD);
        execute(SCI_STYLESETITALIC,		(WPARAM)styleID, fontStyle & FONTSTYLE_ITALIC);
        execute(SCI_STYLESETUNDERLINE,	(WPARAM)styleID, fontStyle & FONTSTYLE_UNDERLINE);
    }

	if (styleToSet._fontSize > 0)
		execute(SCI_STYLESETSIZE, styleID, styleToSet._fontSize);
}

//void ScintillaSearchView::setStyle(Style styleToSet)
//{
//	//GlobalOverride & go = _pParameter->getGlobalOverrideStyle();
//	GlobalOverride  go; // default CTOR has default values nothing enabled
//	//go.enableBg = true;
//
//	if (go.isEnable())
//	{
//		StyleArray & stylers = _pParameter->getMiscStylerArray();
//		int i = stylers.getStylerIndexByName(TEXT("Global override"));
//		if (i != -1)
//		{
//			Style & style = stylers.getStyler(i);
//
//			if (go.enableFg) {
//				if (style._colorStyle & COLORSTYLE_FOREGROUND) {
//					styleToSet._colorStyle |= COLORSTYLE_FOREGROUND;
//					styleToSet._fgColor = style._fgColor;
//				} else {
//					if (styleToSet._styleID == STYLE_DEFAULT) {	//if global is set to transparent, use default style color
//						styleToSet._colorStyle |= COLORSTYLE_FOREGROUND;
//					} else {
//						styleToSet._colorStyle &= ~COLORSTYLE_FOREGROUND;
//					}
//				}
//			}
//			if (go.enableBg) {
//				if (style._colorStyle & COLORSTYLE_BACKGROUND) {
//					styleToSet._colorStyle |= COLORSTYLE_BACKGROUND;
//					styleToSet._bgColor = style._bgColor;
//				} else {
//					if (styleToSet._styleID == STYLE_DEFAULT) {	//if global is set to transparent, use default style color
//						styleToSet._colorStyle |= COLORSTYLE_BACKGROUND;
//					} else {
//						styleToSet._colorStyle &= ~COLORSTYLE_BACKGROUND;
//					}
//				}
//			}
//			if (go.enableFont && style._fontName && style._fontName[0])
//				styleToSet._fontName = style._fontName;
//			if (go.enableFontSize && (style._fontSize > 0))
//				styleToSet._fontSize = style._fontSize;
//
//			if (style._fontStyle != -1)
//			{	
//				if (go.enableBold)
//				{
//					if (style._fontStyle & FONTSTYLE_BOLD)
//						styleToSet._fontStyle |= FONTSTYLE_BOLD;
//					else
//						styleToSet._fontStyle &= ~FONTSTYLE_BOLD;
//				}
//				if (go.enableItalic)
//				{
//					if (style._fontStyle & FONTSTYLE_ITALIC)
//						styleToSet._fontStyle |= FONTSTYLE_ITALIC;
//					else 
//						styleToSet._fontStyle &= ~FONTSTYLE_ITALIC;
//				}
//				if (go.enableUnderLine)
//				{
//					if (style._fontStyle & FONTSTYLE_UNDERLINE)
//						styleToSet._fontStyle |= FONTSTYLE_UNDERLINE;
//					else
//						styleToSet._fontStyle &= ~FONTSTYLE_UNDERLINE;
//				}
//			}
//		}
//	}
//	setSpecialStyle(styleToSet);
//}


//void ScintillaSearchView::setKeywords(LangType langType, const char *keywords, int index)
//{
//	std::string wordList;
//	wordList = (keywords)?keywords:"";
//	execute(SCI_SETKEYWORDS, index, (LPARAM)getCompleteKeywordList(wordList, langType, index));
//}

void ScintillaSearchView::setLexer(int lexerID, LangType langType, int whichList)
{
	execute(SCI_SETLEXER, lexerID);
}

void ScintillaSearchView::restyleBuffer() {
	//int end = execute(SCI_GETENDSTYLED);	//style up to the last styled byte.
	//if (end == 0)
	//	return;
	execute(SCI_CLEARDOCUMENTSTYLE);
	execute(SCI_COLOURISE, 0, -1);
//	_currentBuffer->setNeedsLexing(false);
}

void ScintillaSearchView::styleChange() {
//	defineDocType(_currentBuffer->getLangType());
	restyleBuffer();
}


void ScintillaSearchView::collapse(int level2Collapse, bool mode)
{
	// The following code is needed :
	execute(SCI_COLOURISE, 0, -1);
	// according to the Scitilla document :
	//    This requests the current lexer or the container (if the lexer is set to SCLEX_CONTAINER)
	//    to style the document between startPos and endPos. If endPos is -1, the document is styled from startPos to the end.
	//    If the "fold" property is set to "1" and your lexer or container supports folding, fold levels are also set.
	//    This message causes a redraw.

	int maxLine = (int)execute(SCI_GETLINECOUNT);

	for (int line = 0; line < maxLine; line++) 
	{
		int level = (int)execute(SCI_GETFOLDLEVEL, line);
		if (level & SC_FOLDLEVELHEADERFLAG) 
		{
			level -= SC_FOLDLEVELBASE;
			if (level2Collapse == (level & SC_FOLDLEVELNUMBERMASK))
				if ((execute(SCI_GETFOLDEXPANDED, line) != 0) != mode)
					execute(SCI_TOGGLEFOLD, line);
		}
	}

	runMarkers(true, 0, true, false);
}

void ScintillaSearchView::foldCurrentPos(bool mode)
{
	// The following code is needed :
	execute(SCI_COLOURISE, 0, -1);
	// according to the Scitilla document :
	//    This requests the current lexer or the container (if the lexer is set to SCLEX_CONTAINER)
	//    to style the document between startPos and endPos. If endPos is -1, the document is styled from startPos to the end.
	//    If the "fold" property is set to "1" and your lexer or container supports folding, fold levels are also set.
	//    This message causes a redraw.

	int currentLine = this->getCurrentLineNumber();

	int headerLine;
	int level = (int)execute(SCI_GETFOLDLEVEL, currentLine);
		
	if (level & SC_FOLDLEVELHEADERFLAG)
		headerLine = currentLine;
	else
	{
		headerLine = (int)execute(SCI_GETFOLDPARENT, currentLine);
		if (headerLine == -1)
			return;
	}
	if ((execute(SCI_GETFOLDEXPANDED, headerLine) != 0) != mode)
		execute(SCI_TOGGLEFOLD, headerLine);

}

void ScintillaSearchView::foldAll(bool mode)
{
	// The following code is needed :
	execute(SCI_COLOURISE, 0, -1);
	// according to the Scitilla document :
	//    This requests the current lexer or the container (if the lexer is set to SCLEX_CONTAINER)
	//    to style the document between startPos and endPos. If endPos is -1, the document is styled from startPos to the end.
	//    If the "fold" property is set to "1" and your lexer or container supports folding, fold levels are also set.
	//    This message causes a redraw.

	int maxLine = (int)execute(SCI_GETLINECOUNT);

	for (int line = 0; line < maxLine; line++) 
	{
		int level = (int)execute(SCI_GETFOLDLEVEL, line);
		if (level & SC_FOLDLEVELHEADERFLAG) 
			if ((execute(SCI_GETFOLDEXPANDED, line) != 0) != mode)
				execute(SCI_TOGGLEFOLD, line);
	}
}

void ScintillaSearchView::getText(char *dest, int start, int end, unsigned msg) const
{
	TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = dest;
	execute(msg, 0, reinterpret_cast<LPARAM>(&tr));
}

void ScintillaSearchView::getGenericText(TCHAR *dest, int start, int end, unsigned msg) const
{
#ifdef UNICODE
	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
	char *destA = new char[end - start + 1];
	getText(destA, start, end, msg);
	unsigned cp = (unsigned)execute(SCI_GETCODEPAGE); 
	const TCHAR *destW = wmc->char2wchar(destA, cp);
	lstrcpy(dest, destW);
	delete [] destA;
#else
	getText(dest, start, end, msg);
#endif
}

void ScintillaSearchView::marginClick(int position, int modifiers)
{
	int lineClick = int(execute(SCI_LINEFROMPOSITION, position, 0));
	int levelClick = int(execute(SCI_GETFOLDLEVEL, lineClick, 0));
	if (levelClick & SC_FOLDLEVELHEADERFLAG)
    {
		if (modifiers & SCMOD_SHIFT)
        {
			// Ensure all children visible
			execute(SCI_SETFOLDEXPANDED, lineClick, 1);
			expand(lineClick, true, true, 100, levelClick);
		}
        else if (modifiers & SCMOD_CTRL) 
        {
			if (execute(SCI_GETFOLDEXPANDED, lineClick, 0)) 
            {
				// Contract this line and all children
				execute(SCI_SETFOLDEXPANDED, lineClick, 0);
				expand(lineClick, false, true, 0, levelClick);
			} 
            else 
            {
				// Expand this line and all children
				execute(SCI_SETFOLDEXPANDED, lineClick, 1);
				expand(lineClick, true, true, 100, levelClick);
			}
		} 
        else 
        {
			// Toggle this line
			execute(SCI_TOGGLEFOLD, lineClick, 0);
			runMarkers(true, lineClick, true, false);
		}
	}
}

void ScintillaSearchView::expand(int &line, bool doExpand, bool force, int visLevels, int level)
{
	int lineMaxSubord = int(execute(SCI_GETLASTCHILD, line, level & SC_FOLDLEVELNUMBERMASK));
	line++;
	while (line <= lineMaxSubord)
    {
		if (force) 
        {
			if (visLevels > 0)
				execute(SCI_SHOWLINES, line, line);
			else
				execute(SCI_HIDELINES, line, line);
		} 
        else 
        {
			if (doExpand)
				execute(SCI_SHOWLINES, line, line);
		}
		int levelLine = level;
		if (levelLine == -1)
			levelLine = int(execute(SCI_GETFOLDLEVEL, line, 0));
		if (levelLine & SC_FOLDLEVELHEADERFLAG)
        {
			if (force) 
            {
				if (visLevels > 1)
					execute(SCI_SETFOLDEXPANDED, line, 1);
				else
					execute(SCI_SETFOLDEXPANDED, line, 0);
				expand(line, doExpand, force, visLevels - 1);
			} 
            else
            {
				if (doExpand)
                {
					if (!execute(SCI_GETFOLDEXPANDED, line, 0))
						execute(SCI_SETFOLDEXPANDED, line, 1);

					expand(line, true, force, visLevels - 1);
				} 
                else 
                {
					expand(line, false, force, visLevels - 1);
				}
			}
		}
        else
        {
			line++;
		}
	}

	runMarkers(true, 0, true, false);
}

void ScintillaSearchView::performGlobalStyles() 
{
	StyleArray & stylers = _pParameter->getMiscStylerArray();

	int i = stylers.getStylerIndexByName(TEXT("Current line background colour"));
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		execute(SCI_SETCARETLINEBACK, style._bgColor);
	}
/*
	i = stylers.getStylerIndexByName(TEXT("Mark colour"));
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		execute(SCI_MARKERSETFORE, 1, style._fgColor);
		execute(SCI_MARKERSETBACK, 1, style._bgColor);
	}
*/
    COLORREF selectColorBack = grey;

	i = stylers.getStylerIndexByName(TEXT("Selected text colour"));
	if (i != -1)
    {
        Style & style = stylers.getStyler(i);
		selectColorBack = style._bgColor;
    }
	execute(SCI_SETSELBACK, 1, selectColorBack);

    COLORREF caretColor = black;
	i = stylers.getStylerIndexByID(SCI_SETCARETFORE);
	if (i != -1)
    {
        Style & style = stylers.getStyler(i);
        caretColor = style._fgColor;
    }
    execute(SCI_SETCARETFORE, caretColor);

	COLORREF edgeColor = liteGrey;
	i = stylers.getStylerIndexByName(TEXT("Edge colour"));
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		edgeColor = style._fgColor;
	}
	execute(SCI_SETEDGECOLOUR, edgeColor);

	COLORREF foldMarginColor = grey;
	COLORREF foldMarginHiColor = white;
	i = stylers.getStylerIndexByName(TEXT("Fold margin"));
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		foldMarginHiColor = style._fgColor;
		foldMarginColor = style._bgColor;
	}
	execute(SCI_SETFOLDMARGINCOLOUR, true, foldMarginColor);
	execute(SCI_SETFOLDMARGINHICOLOUR, true, foldMarginHiColor);

	COLORREF foldfgColor = white;
	COLORREF foldbgColor = grey;
	i = stylers.getStylerIndexByName(TEXT("Fold"));

	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		foldfgColor = style._bgColor;
		foldbgColor = style._fgColor;
	}
	for (int j = 0 ; j < NB_FOLDER_STATE ; j++)
        defineMarker(_markersArray[FOLDER_TYPE][j], _markersArray[_folderStyle][j], foldfgColor, foldbgColor);

#ifdef UNICODE
	COLORREF unsavedChangebgColor = liteRed;
	i = stylers.getStylerIndexByName(TEXT("Unsaved change marker"));
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		unsavedChangebgColor = style._bgColor;
	}
	execute(SCI_MARKERSETBACK, MARK_LINEMODIFIEDUNSAVED, unsavedChangebgColor);

	COLORREF savedChangebgColor = liteBlueGreen;
	i = stylers.getStylerIndexByName(TEXT("Saved change marker"));
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		savedChangebgColor = style._bgColor;
	}
	execute(SCI_MARKERSETBACK, MARK_LINEMODIFIEDSAVED, savedChangebgColor);
#endif

	COLORREF wsSymbolFgColor = black;
	i = stylers.getStylerIndexByName(TEXT("White space symbol"));
	if (i != -1)
	{
		Style & style = stylers.getStyler(i);
		wsSymbolFgColor = style._fgColor;
	}
	execute(SCI_SETWHITESPACEFORE, true, wsSymbolFgColor);
}

void ScintillaSearchView::setLineIndent(int line, int indent) const {
	if (indent < 0)
		return;
	CharacterRange crange = getSelection();
	int posBefore = (int)execute(SCI_GETLINEINDENTPOSITION, line);
	execute(SCI_SETLINEINDENTATION, line, indent);
	int posAfter = (int)execute(SCI_GETLINEINDENTPOSITION, line);
	int posDifference = posAfter - posBefore;
	if (posAfter > posBefore) {
		// Move selection on
		if (crange.cpMin >= posBefore) {
			crange.cpMin += posDifference;
		}
		if (crange.cpMax >= posBefore) {
			crange.cpMax += posDifference;
		}
	} else if (posAfter < posBefore) {
		// Move selection back
		if (crange.cpMin >= posAfter) {
			if (crange.cpMin >= posBefore)
				crange.cpMin += posDifference;
			else
				crange.cpMin = posAfter;
		}
		if (crange.cpMax >= posAfter) {
			if (crange.cpMax >= posBefore)
				crange.cpMax += posDifference;
			else
				crange.cpMax = posAfter;
		}
	}
	execute(SCI_SETSEL, crange.cpMin, crange.cpMax);
}



bool ScintillaSearchView::expandWordSelection()
{
	int caretPos = (int)execute(SCI_GETCURRENTPOS, 0, 0);
	int startPos = static_cast<int>(execute(SCI_WORDSTARTPOSITION, caretPos, true));
	int endPos = static_cast<int>(execute(SCI_WORDENDPOSITION, caretPos, true));
	if (startPos != endPos) {
		execute(SCI_SETSELECTIONSTART, startPos);
		execute(SCI_SETSELECTIONEND, endPos);
		return true;
	}
	return false;
}

TCHAR * int2str(TCHAR *str, int strLen, int number, int base, int nbChiffre, bool isZeroLeading) 
{
	if (nbChiffre >= strLen) return NULL;
	TCHAR f[64];
	TCHAR fStr[2] = TEXT("d");
	if (base == 16)
		fStr[0] = 'X';
	else if (base == 8)
		fStr[0] = 'o';
	else if (base == 2)
	{
		const unsigned int MASK_ULONG_BITFORT = 0x80000000;
		int nbBits = sizeof(unsigned int) * 8;
		int nbBit2Shift = (nbChiffre >= nbBits)?nbBits:(nbBits - nbChiffre);
		unsigned long mask = MASK_ULONG_BITFORT >> nbBit2Shift;
		int i = 0; 
		for (; mask > 0 ; i++)
		{
			str[i] = (mask & number)?'1':'0';
			mask >>= 1;
		}
		str[i] = '\0';
	}

	if (!isZeroLeading)
	{
		if (base == 2)
		{
			TCHAR *j = str;
			for ( ; *j != '\0' ; j++)
				if (*j == '1')
					break;
			lstrcpy(str, j);
		}
		else
		{
			wsprintf(f, TEXT("%%%s"), fStr);
			wsprintf(str, f, number);
		}
		int i = lstrlen(str);
		for ( ; i < nbChiffre ; i++)
			str[i] = ' ';
		str[i] = '\0';
	}
	else
	{
		if (base != 2)
		{
			wsprintf(f, TEXT("%%.%d%s"), nbChiffre, fStr);
			wsprintf(str, f, number);
		}
		// else already done.
	}
	return str;
}

ColumnModeInfo ScintillaSearchView::getColumnModeSelectInfo()
{
	ColumnModeInfo columnModeInfo;
	if (execute(SCI_SELECTIONISRECTANGLE))
	{
		int selStartAbsPos = (int)execute(SCI_GETSELECTIONSTART);
		int selEndAbsPos = (int)execute(SCI_GETSELECTIONEND);

		int startCol = (int)execute(SCI_GETCOLUMN, selStartAbsPos);
		int endCol = (int)execute(SCI_GETCOLUMN, selEndAbsPos);

		int startLine = (int)execute(SCI_LINEFROMPOSITION, selStartAbsPos);
		int endLine = (int)execute(SCI_LINEFROMPOSITION, selEndAbsPos);
		
		if (endCol < startCol)// another way of selection
		{
			int tmp = startCol;
			startCol = endCol;
			endCol = tmp;

			selStartAbsPos = (int)execute(SCI_FINDCOLUMN, startLine, startCol);
			selEndAbsPos = (int)execute(SCI_FINDCOLUMN, endLine, endCol);
		}

		bool zeroCharSelMode = true;
		for (int i = startLine ; i <= endLine ; i++)
		{		
			int absPosSelStartPerLine =  (int)execute(SCI_FINDCOLUMN, i, startCol);
			int absPosSelEndPerLine = (int)execute(SCI_FINDCOLUMN, i, endCol);

			if (absPosSelStartPerLine != absPosSelEndPerLine)
			{	
				zeroCharSelMode = false;
			}
			columnModeInfo.push_back(pair<int, int>(absPosSelStartPerLine, absPosSelEndPerLine));
		}

		if (!zeroCharSelMode)
		{
			for (int i = (int)columnModeInfo.size() - 1 ; i >= 0 ; i--)
			{
				ColumnModeInfo::iterator it = columnModeInfo.begin() + i;
				if (it->first == it->second)
					columnModeInfo.erase(it);
			}
		}
	}
	return columnModeInfo;
}

//void ScintillaSearchView::columnReplace(ColumnModeInfo & cmi, const TCHAR *str)
//{
//	//for (int i = (int)cmi.size() - 1 ; i >= 0 ; i--)
//	int totalDiff = 0;
//	for (size_t i = 0 ; i < cmi.size() ; i++)
//	{
//		int len2beReplace = cmi[i].second - cmi[i].first;
//		int diff = lstrlen(str) - len2beReplace;
//
//		cmi[i].first += totalDiff;
//		cmi[i].second += totalDiff;
//
//		execute(SCI_SETTARGETSTART, cmi[i].first);
//		execute(SCI_SETTARGETEND, cmi[i].second);
//#ifdef UNICODE
//		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
//		unsigned int cp = execute(SCI_GETCODEPAGE);
//		const char *strA = wmc->wchar2char(str, cp);
//		execute(SCI_REPLACETARGET, -1, (LPARAM)strA);
//#else
//		execute(SCI_REPLACETARGET, -1, (LPARAM)str);
//#endif
//		totalDiff += diff;
//		cmi[i].second += diff;
//	}
//}
//
//
//void ScintillaSearchView::columnReplace(ColumnModeInfo & cmi, int initial, int incr, unsigned char format)
//{
//	// 0000 00 00 : Dec BASE_10
//	// 0000 00 01 : Hex BASE_16
//	// 0000 00 10 : Oct BASE_08
//	// 0000 00 11 : Bin BASE_02
//
//	// 0000 01 00 : 0 leading
//
//	//Defined in ScintillaSearchView.h :
//	//const unsigned char MASK_FORMAT = 0x03;
//	//const unsigned char MASK_ZERO_LEADING = 0x04;
//
//	unsigned char f = format & MASK_FORMAT;
//	bool isZeroLeading = (MASK_ZERO_LEADING & format) != 0;
//	
//	int base = 10;
//	if (f == BASE_16)
//		base = 16;
//	else if (f == BASE_08)
//		base = 8;
//	else if (f == BASE_02)
//		base = 2;
//
//	int endNumber = initial + incr * ((int)cmi.size() - 1);
//	int nbEnd = getNbChiffre(endNumber, base);
//	int nbInit = getNbChiffre(initial, base);
//	int nb = max(nbInit, nbEnd);
//
//	const int stringSize = 512;
//	TCHAR str[stringSize];
//
//	int totalDiff = 0;
//	for (size_t i = 0 ; i < cmi.size() ; i++)
//	{
//		int len2beReplace = cmi[i].second - cmi[i].first;
//		int diff = nb - len2beReplace;
//
//		cmi[i].first += totalDiff;
//		cmi[i].second += totalDiff;
//
//		int2str(str, stringSize, initial, base, nb, isZeroLeading);
//		
//		execute(SCI_SETTARGETSTART, cmi[i].first);
//		execute(SCI_SETTARGETEND, cmi[i].second);
//#ifdef UNICODE
//		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
//		unsigned int cp = execute(SCI_GETCODEPAGE);
//		const char *strA = wmc->wchar2char(str, cp);
//		execute(SCI_REPLACETARGET, -1, (LPARAM)strA);
//#else
//		execute(SCI_REPLACETARGET, -1, (LPARAM)str);
//#endif
//		initial += incr;
//		totalDiff += diff;
//		cmi[i].second += diff;
//	}
//}
//
//
//void ScintillaSearchView::columnReplace(const ColumnModeInfo & cmi, const TCHAR ch)
//{
//	for (size_t i = 0 ; i < cmi.size() ; i++)
//	{
//		int len = cmi[i].second - cmi[i].first;
//		generic_string str(len, ch);
//		execute(SCI_SETTARGETSTART, cmi[i].first);
//		execute(SCI_SETTARGETEND, cmi[i].second);
//
//#ifdef UNICODE
//		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
//		unsigned int cp = execute(SCI_GETCODEPAGE);
//		const char *strA = wmc->wchar2char(str.c_str(), cp);
//		execute(SCI_REPLACETARGET, -1, (LPARAM)strA);
//#else
//		execute(SCI_REPLACETARGET, -1, (LPARAM)str.c_str());
//#endif
//	}
//}
//
void ScintillaSearchView::foldChanged(int line, int levelNow, int levelPrev)
{
	if (levelNow & SC_FOLDLEVELHEADERFLAG)		//line can be folded
	{
		if (!(levelPrev & SC_FOLDLEVELHEADERFLAG))	//but previously couldnt
		{
			// Adding a fold point.
			execute(SCI_SETFOLDEXPANDED, line, 1);
			expand(line, true, false, 0, levelPrev);
		}
	}
	else if (levelPrev & SC_FOLDLEVELHEADERFLAG)
	{
		if (!execute(SCI_GETFOLDEXPANDED, line))
		{
			// Removing the fold from one that has been contracted so should expand
			// otherwise lines are left invisible with no way to make them visible
			execute(SCI_SETFOLDEXPANDED, line, 1);
			expand(line, true, false, 0, levelPrev);
		}
	}
	else if (!(levelNow & SC_FOLDLEVELWHITEFLAG) &&
	        ((levelPrev & SC_FOLDLEVELNUMBERMASK) > (levelNow & SC_FOLDLEVELNUMBERMASK)))
	{
		// See if should still be hidden
		int parentLine = (int)execute(SCI_GETFOLDPARENT, line);
		if ((parentLine < 0) || (execute(SCI_GETFOLDEXPANDED, parentLine) && execute(SCI_GETLINEVISIBLE, parentLine)))
			execute(SCI_SHOWLINES, line, line);
	}
}

bool ScintillaSearchView::markerMarginClick(int lineNumber) {

	int state = (int)execute(SCI_MARKERGET, lineNumber);
	bool openPresent = ((state & (1 << MARK_HIDELINESBEGIN)) != 0);
	bool closePresent = ((state & (1 << MARK_HIDELINESEND)) != 0);

	if (!openPresent && !closePresent)
		return false;

	//Special func on buffer. First call show with location of opening marker. Then remove the marker manually
	if (openPresent) {
		//_currentBuffer->setHideLineChanged(false, lineNumber);
	}
	if (closePresent) {
		openPresent = false;
		for(lineNumber--; lineNumber >= 0 && !openPresent; lineNumber--) {
			state = (int)execute(SCI_MARKERGET, lineNumber);
			openPresent = ((state & (1 << MARK_HIDELINESBEGIN)) != 0);
		}
		if (openPresent) {
			//_currentBuffer->setHideLineChanged(false, lineNumber);
		}
	}

	return true;
}

void ScintillaSearchView::notifyMarkers(Buffer * buf, bool isHide, int location, bool del) {
	//if (buf != _currentBuffer)	//if not visible buffer dont do a thing
	//	return;
	runMarkers(isHide, location, false, del);
}
//Run through full document. When switching in or opening folding
//hide is false only when user click on margin
void ScintillaSearchView::runMarkers(bool doHide, int searchStart, bool endOfDoc, bool doDelete) {
	//Removes markers if opening
	/*
	AllLines = (start,ENDOFDOCUMENT)
	Hide:
		Run through all lines.
			Find open hiding marker:
				set hiding start
			Find closing:
				if (hiding):
					Hide lines between now and start
					if (endOfDoc = false)
						return
					else
						search for other hidden sections
		
	Show:
		Run through all lines
			Find open hiding marker
				set last start
			Find closing:
				Show from last start. Stop.
			Find closed folding header:
				Show from last start to folding header
				Skip to LASTCHILD
				Set last start to lastchild
	*/
	int maxLines = (int)execute(SCI_GETLINECOUNT);
	if (doHide) {
		int startHiding = searchStart;
		bool isInSection = false;
		for(int i = searchStart; i < maxLines; i++) {
			int state = (int)execute(SCI_MARKERGET, i);
			if ( ((state & (1 << MARK_HIDELINESEND)) != 0) ) {
				if (isInSection) {
					execute(SCI_HIDELINES, startHiding, i-1);
					if (!endOfDoc) {
						return;	//done, only single section requested
					}	//otherwise keep going
				}
				isInSection = false;
			}
			if ( ((state & (1 << MARK_HIDELINESBEGIN)) != 0) ) {
				isInSection = true;
				startHiding = i+1;
			}

		}
	} else {
		int startShowing = searchStart;
		bool isInSection = false;
		for(int i = searchStart; i < maxLines; i++) {
			int state = (int)execute(SCI_MARKERGET, i);
			if ( ((state & (1 << MARK_HIDELINESEND)) != 0) ) {
				if (doDelete)
					execute(SCI_MARKERDELETE, i, MARK_HIDELINESEND);
				 else if (isInSection) {
					if (startShowing >= i) {	//because of fold skipping, we passed the close tag. In that case we cant do anything
						if (!endOfDoc) {
							return;
						} else {
							continue;
						}
					}
					execute(SCI_SHOWLINES, startShowing, i-1);
					if (!endOfDoc) {
						return;	//done, only single section requested
					}	//otherwise keep going
					isInSection = false;
				}
			}
			if ( ((state & (1 << MARK_HIDELINESBEGIN)) != 0) ) {
				if (doDelete)
					execute(SCI_MARKERDELETE, i, MARK_HIDELINESBEGIN);
				else {
					isInSection = true;
					startShowing = i+1;
				}
			}

			int levelLine = (int)execute(SCI_GETFOLDLEVEL, i, 0);
			if (levelLine & SC_FOLDLEVELHEADERFLAG) {	//fold section. Dont show lines if fold is closed
				if (isInSection && execute(SCI_GETFOLDEXPANDED, i) == 0) {
					execute(SCI_SHOWLINES, startShowing, i);
					startShowing = (int)execute(SCI_GETLASTCHILD, i, (levelLine & SC_FOLDLEVELNUMBERMASK));
				}
			}
		}
	}
}
