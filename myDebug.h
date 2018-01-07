/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2018 Matthias H. mattesh(at)gmx.net

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
//"myDebug.h"
#ifndef MYDEBUG_H
#define MYDEBUG_H
#if defined (_DEBUG) && !defined (DISABLE_DEBUG)
#define DBGDEF(par) par // use this macro to specify code not to be available in release mode
#define DBGNDEF(par) // use this macro to specify code not to be available in debug mode

// use MDBG_COMP to define a compnent of file identifier which becomes prepended to all output
#ifndef MDBG_COMP
#define MDBG_COMP ""
#endif

#include <crtdbg.h>
#ifdef UNICODE
#define DBG0 DBGW0      
#define DBG1 DBGW1      
#define DBG2 DBGW2      
#define DBG3 DBGW3      
#define DBG4 DBGW4      
#else
#define DBG0 DBGA0      
#define DBG1 DBGA1      
#define DBG2 DBGA2      
#define DBG3 DBGA3      
#define DBG4 DBGA4      
#endif

#define DBGA0(msg)                               _RPT1(_CRT_WARN, MDBG_COMP##msg##"\t"##__FILE__##"(%d)\n", __LINE__)
#define DBGA1(msg, arg1)                         _RPT2(_CRT_WARN, MDBG_COMP##msg##"\t"##__FILE__##"(%d)\n", arg1, __LINE__)
#define DBGA2(msg, arg1, arg2)                   _RPT3(_CRT_WARN, MDBG_COMP##msg##"\t"##__FILE__##"(%d)\n", arg1, arg2, __LINE__)
#define DBGA3(msg, arg1, arg2, arg3)             _RPT4(_CRT_WARN, MDBG_COMP##msg##"\t"##__FILE__##"(%d)\n", arg1, arg2, arg3, __LINE__)
#define DBGA4(msg, arg1, arg2, arg3, arg4)       _RPT5(_CRT_WARN, MDBG_COMP##msg##"\t"##__FILE__##"(%d)\n", arg1, arg2, arg3, arg4, __LINE__)
//#define DBG5(msg, arg1, arg2, arg3, arg4, arg5) _RPT6(_CRT_WARN, msg##__FILE__##"("##__LINE__##")\n", arg1, arg2, arg3, arg4, arg5)
#define DBGW0(msg)                               _RPTW1(_CRT_WARN, TEXT(MDBG_COMP)##TEXT(##msg##)TEXT("\t")##TEXT(__FILE__)##TEXT("(%d)\n"), __LINE__)
#define DBGW1(msg, arg1)                         _RPTW2(_CRT_WARN, TEXT(MDBG_COMP)##TEXT(##msg##)TEXT("\t")##TEXT(__FILE__)##TEXT("(%d)\n"), arg1, __LINE__)
#define DBGW2(msg, arg1, arg2)                   _RPTW3(_CRT_WARN, TEXT(MDBG_COMP)##TEXT(##msg##)TEXT("\t")##TEXT(__FILE__)##TEXT("(%d)\n"), arg1, arg2, __LINE__)
#define DBGW3(msg, arg1, arg2, arg3)             _RPTW4(_CRT_WARN, TEXT(MDBG_COMP)##TEXT(##msg##)TEXT("\t")##TEXT(__FILE__)##TEXT("(%d)\n"), arg1, arg2, arg3, __LINE__)
#define DBGW4(msg, arg1, arg2, arg3, arg4)       _RPTW5(_CRT_WARN, TEXT(MDBG_COMP)##TEXT(##msg##)TEXT("\t")##TEXT(__FILE__)##TEXT("(%d)\n"), arg1, arg2, arg3, arg4, __LINE__)
#else
#define DBGDEF(par)
#define DBGNDEF(par) par
#define DBG0(msg)                               
#define DBG1(msg, arg1)                         
#define DBG2(msg, arg1, arg2)                   
#define DBG3(msg, arg1, arg2, arg3)             
#define DBG4(msg, arg1, arg2, arg3, arg4)       
//#define DBG5(msg, arg1, arg2, arg3, arg4, arg5) 
#define DBGW0(msg)                               
#define DBGW1(msg, arg1)                         
#define DBGW2(msg, arg1, arg2)                   
#define DBGW3(msg, arg1, arg2, arg3)             
#define DBGW4(msg, arg1, arg2, arg3, arg4)       
//#define DBGW5(msg, arg1, arg2, arg3, arg4, arg5) 
#define DBGA0(msg)                         
#define DBGA1(msg, arg1)                   
#define DBGA2(msg, arg1, arg2)             
#define DBGA3(msg, arg1, arg2, arg3)       
#define DBGA4(msg, arg1, arg2, arg3, arg4) 
#endif
#endif