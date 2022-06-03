/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C) 2022 Matthias H. mattesh(at)gmx.net

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
#ifndef CHARDEFINES_H
#define CHARDEFINES_H

#define COUNTCHAR(ar) COUNT(ar,TCHAR)
#define COUNT(ar,ty) (sizeof(ar)/sizeof(ty))

// others are defined in Common.h
#ifdef UNICODE
   #define generic_strlen wcslen
#else // UNICODE
   #define generic_strlen strlen
#endif // UNICODE

#endif // CHARDEFINES_H