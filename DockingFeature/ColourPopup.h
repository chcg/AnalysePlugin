/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2020 Matthias H. mattesh(at)gmx.net
partly copied from the NotePad++ project from 
Don HO don.h(at)free.fr 

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

#pragma once

#include "resource.h"
#include "Window.h"

#define COLOURPOPUP_USER   (WM_USER + 6000)
#define WM_PICKUP_COLOR (COLOURPOPUP_USER + 1)
#define WM_PICKUP_CANCEL (COLOURPOPUP_USER + 2)

class ColourPopup : public Window
{
public :
   ColourPopup() : Window(), /*isColourChooserLaunched(false),*/ _colour(0), _pacCustomColors(0){};
   ColourPopup(COLORREF defaultColor) : Window(), /*isColourChooserLaunched(false),*/ _colour(defaultColor), _pacCustomColors(0){};
   virtual ~ColourPopup(){}

	bool isCreated() const
	{
      return (_hSelf != NULL);
   }

   void create(int dialogID);
    
   void doDialog(POINT p) ;

	virtual void destroy()
	{
      ::DestroyWindow(_hSelf);
	}

	void setColour(COLORREF c)
	{
		_colour = c;
	}
   void setCustomColors(COLORREF* p) {
      _pacCustomColors = p;
   }

   COLORREF getSelColour(){
      return _colour;
   };

private :
   RECT _rc;
   COLORREF _colour;
	//bool isColourChooserLaunched = false;
   COLORREF * _pacCustomColors;
   static INT_PTR CALLBACK dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
   INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam);
};



