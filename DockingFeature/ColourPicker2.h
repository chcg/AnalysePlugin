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

#pragma once

#include "Window.h"
#include "ColourPopupResource.h"
class ColourPopup;

#define CPN_COLOURPICKED (BN_CLICKED)

class ColourPicker2 : public Window
{
public :
   ColourPicker2() : Window(), _currentColour(RGB(0xFF, 0x00, 0x00)), _buttonDefaultProc(NULL), _pColourPopup(NULL), _isEnabled(true), _pCustomColors(0){};
    ~ColourPicker2(){};
	virtual void init(HINSTANCE hInst, HWND parent);
	virtual void destroy();
    void setColour(COLORREF c) {
        _currentColour = c;
    };

	COLORREF getColour() const {return _currentColour;};

	bool isEnabled() {return _isEnabled;};
	void setEnabled(bool enabled) {_isEnabled = enabled;};

   void setCustomColors(COLORREF* p);

private :
	COLORREF _currentColour;
    WNDPROC _buttonDefaultProc;
	ColourPopup *_pColourPopup;
	bool _isEnabled;
   COLORREF* _pCustomColors;

    static LRESULT CALLBACK staticWinProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
       return (((ColourPicker2 *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA)))->runProc(Message, wParam, lParam));
    };
	LRESULT runProc(UINT Message, WPARAM wParam, LPARAM lParam);
    void drawForeground(HDC hDC);
	void drawBackground(HDC hDC);
};

