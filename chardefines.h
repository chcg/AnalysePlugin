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
#ifndef CHARDEFINES_H
#define CHARDEFINES_H

#define COUNTCHAR(ar) COUNT(ar,TCHAR)
#define COUNT(ar,ty) (sizeof(ar)/sizeof(ty))

#ifdef UNICODE
   #define generic_strlen wcslen
   #define generic_itoa _itow
#else // UNICODE
   #define generic_strlen strlen
   #define generic_itoa itoa
#endif // UNICODE

// already defined Common.h?
#ifndef NppMainEntry
   #ifdef UNICODE
	   #define NppMainEntry wWinMain
	   #define generic_strtol wcstol
	   #define generic_strncpy wcsncpy
	   #define generic_stricmp wcsicmp
	   #define generic_strncmp wcsncmp
	   #define generic_strnicmp wcsnicmp
	   #define generic_strncat wcsncat
	   #define generic_strchr wcschr
	   #define generic_atoi _wtoi
	   #define generic_atof _wtof
	   #define generic_strtok wcstok
	   #define generic_strftime wcsftime
	   #define generic_fprintf fwprintf
	   #define generic_sscanf swscanf
	   #define generic_fopen _wfopen
	   #define generic_fgets fgetws
	   #define generic_stat _wstat
	   #define generic_string wstring
	   #define COPYDATA_FILENAMES COPYDATA_FILENAMESW
   #else // UNICODE
	   #define NppMainEntry WinMain
	   #define generic_strtol strtol
	   #define generic_strncpy strncpy
	   #define generic_stricmp stricmp
	   #define generic_strncmp strncmp
	   #define generic_strnicmp strnicmp
	   #define generic_strncat strncat
	   #define generic_strchr strchr
	   #define generic_atoi atoi
	   #define generic_atof atof
	   #define generic_strtok strtok
	   #define generic_strftime strftime
	   #define generic_fprintf fprintf
	   #define generic_sscanf sscanf
	   #define generic_fopen fopen
	   #define generic_fgets fgets
	   #define generic_stat _stat
	   #define generic_string string
	   #define COPYDATA_FILENAMES COPYDATA_FILENAMESA
   #endif// UNICODE
#endif // NppMainEntry
#endif // CHARDEFINES_H