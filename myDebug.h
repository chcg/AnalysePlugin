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
//"myDebug.h"
#ifndef MYDEBUG_H
#define MYDEBUG_H
#if 1
#include <crtdbg.h>
#define DBG0(msg)                               _RPT0(_CRT_WARN, msg##"\t"##__FILE__##"\n")
#define DBG1(msg, arg1)                         _RPT1(_CRT_WARN, msg##"\t"##__FILE__##"\n", arg1)
#define DBG2(msg, arg1, arg2)                   _RPT2(_CRT_WARN, msg##"\t"##__FILE__##"\n", arg1, arg2)
#define DBG3(msg, arg1, arg2, arg3)             _RPT3(_CRT_WARN, msg##"\t"##__FILE__##"\n", arg1, arg2, arg3)
#define DBG4(msg, arg1, arg2, arg3, arg4)       _RPT4(_CRT_WARN, msg##"\t"##__FILE__##"\n", arg1, arg2, arg3, arg4)
#define DBG5(msg, arg1, arg2, arg3, arg4, arg5) _RPT5(_CRT_WARN, msg##"\t"##__FILE__##"\n", arg1, arg2, arg3, arg4, arg5)
#else
#define DBG0(msg)                               
#define DBG1(msg, arg1)                         
#define DBG2(msg, arg1, arg2)                   
#define DBG3(msg, arg1, arg2, arg3)             
#define DBG4(msg, arg1, arg2, arg3, arg4)       
#define DBG5(msg, arg1, arg2, arg3, arg4, arg5) 
#endif
#endif