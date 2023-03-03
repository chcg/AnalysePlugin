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
#ifndef SCINTILLA_SEARCH_VIEW_32_H
#define SCINTILLA_SEARCH_VIEW_32_H

// required for incompatible pointer size change in v8.3++
// we keep compatibility by holding 32 bit pointers as special types

typedef long Sci_PositionCR32;
// new intptr_t

struct Sci_CharacterRange32 {
	Sci_PositionCR32 cpMin;
	Sci_PositionCR32 cpMax;
};

struct Sci_TextRange32 {
	struct Sci_CharacterRange32 chrg;
	char* lpstrText;
};

#endif // SCINTILLA_SEARCH_VIEW_32_H
