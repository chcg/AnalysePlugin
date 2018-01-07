/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2018 Matthias H. mattesh(at)gmx.net
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

#ifndef COLOUR_PICKER2_H
#define COLOUR_PICKER2_H
#include "ColourPopupResource.h"
class ColourPopup;
#include "Window.h"

//#define CP_CLASS_NAME "ColourPicker2Button"
#define CPN_COLOURPICKED (BN_CLICKED)

class ColourPicker2 : public Window
{
public :
	ColourPicker2() : Window(),  _currentColour(RGB(0xFF, 0x00, 0x00)), _pColourPopup(NULL), _isEnabled(true) {};
    ~ColourPicker2(){};
	virtual void init(HINSTANCE hInst, HWND parent);
	virtual void destroy();
    void setColour(COLORREF c) {
        _currentColour = c;
    };

	COLORREF getColour() const {return _currentColour;};

	bool isEnabled() {return _isEnabled;};
	void setEnabled(bool enabled) {_isEnabled = enabled;};

private :
	COLORREF _currentColour;
    WNDPROC _buttonDefaultProc;
	ColourPopup *_pColourPopup;
	bool _isEnabled;

    static LRESULT CALLBACK staticWinProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
       return (((ColourPicker2 *)(::GetWindowLongPtr(hwnd, GWLP_USERDATA)))->runProc(Message, wParam, lParam));
    };
	LRESULT runProc(UINT Message, WPARAM wParam, LPARAM lParam);
    void drawForeground(HDC hDC);
	void drawBackground(HDC hDC);
};

#endif // COLOUR_PICKER2_H
