/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2019 Matthias H. mattesh(at)gmx.net

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
#ifndef NPP_DEFINES_H
#define NPP_DEFINES_H

// make sure exceptions are show at the place of occurence 
// so that we easier can follow the call stack in debug mode
#ifdef NDEBUG
#define NPP_TRY try
#define NPP_CATCH(ex) catch(ex)
#define NPP_CATCH_NOP(ex) catch(ex)
#define NPP_CATCH_ALL catch
#else
#define NPP_TRY
#define NPP_CATCH(ex) for(ex;false;)
#define NPP_CATCH_NOP(ex) ex; if(false)
#define NPP_CATCH_ALL(ex) if(false)
#endif

#endif // NPP_DEFINES_H