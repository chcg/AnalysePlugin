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
//#include "stdafx.h"
#include "precompiledHeaders.h"
#include "ColourPopup.h"
#include "tclPattern.h"
#include <commdlg.h>

//DWORD colourItems[] = {
//	RGB(  0,   0,   0),	RGB( 64,   0,   0),	RGB(128,   0,   0),	RGB(128,  64,  64),	RGB(255,   0,   0),	RGB(255, 128, 128),
//	RGB(255, 255, 128),	RGB(255, 255,   0),	RGB(255, 128,  64),	RGB(255, 128,   0),	RGB(128,  64,   0),	RGB(128, 128,   0),
//	RGB(128, 128,  64),	RGB(  0,  64,   0),	RGB(  0, 128,   0),	RGB(  0, 255,   0),	RGB(128, 255,   0),	RGB(128, 255, 128),
//	RGB(  0, 255, 128),	RGB(  0, 255,  64),	RGB(  0, 128, 128),	RGB(  0, 128,  64),	RGB(  0,  64,  64),	RGB(128, 128, 128),
//	RGB( 64, 128, 128),	RGB(  0,   0, 128),	RGB(  0,   0, 255),	RGB(  0,  64, 128),	RGB(  0, 255, 255), RGB(128, 255, 255),
//	RGB(  0, 128, 255),	RGB(  0, 128, 192),	RGB(128, 128, 255),	RGB(  0,   0, 160),	RGB(  0,   0,  64),	RGB(192, 192, 192),
//	RGB( 64,   0,  64),	RGB( 64,   0,  64),	RGB(128,   0, 128),	RGB(128,   0,  64),	RGB(128, 128, 192),	RGB(255, 128, 192),
//	RGB(255, 128, 255),	RGB(255,   0, 255), RGB(255,   0, 128),	RGB(128,   0, 255), RGB( 64,   0, 128),	RGB(255, 255, 255),
//};

#define COLPOP_ITEM_LINES   6
#define COLPOP_ITEM_COLUMNS 5

void ColourPopup::create(int dialogID) 
{
   _hSelf = ::CreateDialogParam(_hInst, MAKEINTRESOURCE(dialogID), _hParent,  (DLGPROC)dlgProc, (LPARAM)this);

   if (!_hSelf)
   {
      throw std::runtime_error("ColourPopup::create : CreateDialogParam() function return null");
   }
   Window::getClientRect(_rc);
   display();
}


void ColourPopup::doDialog(POINT p) {
   if (!isCreated()){
      create(IDD_COLOUR_POPUP);
   }
   int cx = _rc.right - _rc.left;
   int cy = _rc.bottom - _rc.top;
   ::SetWindowPos(_hSelf, HWND_TOP, p.x-cx, p.y-cy, cx, cy, SWP_SHOWWINDOW);
}

INT_PTR CALLBACK ColourPopup::dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
   switch (message) 
   {
   case WM_MEASUREITEM:
      {
         RECT rc;
         LPMEASUREITEMSTRUCT lpmis =  (LPMEASUREITEMSTRUCT) lParam; 
         ::GetWindowRect(::GetDlgItem(hwnd, lpmis->CtlID), &rc);
         lpmis->itemHeight = (rc.bottom-rc.top)/COLPOP_ITEM_LINES; 
         lpmis->itemWidth = (rc.right-rc.left)/COLPOP_ITEM_COLUMNS;
         return TRUE;
      }

   case WM_INITDIALOG :
      {
         ColourPopup *pColourPopup = (ColourPopup *)(lParam);
         pColourPopup->_hSelf = hwnd;
#pragma warning (disable:4244)
         ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
#pragma warning (default:4244)
         pColourPopup->run_dlgProc(message, wParam, lParam);
         return TRUE;
      }

   default :
      {
#pragma warning (disable:4312)
         ColourPopup *pColourPopup = reinterpret_cast<ColourPopup *>(::GetWindowLongPtr(hwnd, GWL_USERDATA));
#pragma warning (default:4312)
         if (!pColourPopup)
            return FALSE;
         return pColourPopup->run_dlgProc(message, wParam, lParam);
      }
   }
}

INT_PTR CALLBACK ColourPopup::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{

   switch (message)
   {
   case WM_INITDIALOG:
      {
         int nColor;
         int size = tclPattern::getDefColorListSize();
         //for (nColor = 0 ; nColor < int(sizeof(colourItems)/sizeof(DWORD)) ; nColor++)
         for (nColor = 0 ; nColor < size ; nColor++)
         {
            ::SendDlgItemMessage(_hSelf, IDC_COLOUR_LIST, LB_ADDSTRING, nColor, (LPARAM) "");
            //::SendDlgItemMessage(_hSelf, IDC_COLOUR_LIST, LB_SETITEMDATA , nColor, (LPARAM) colourItems[nColor]);
            ::SendDlgItemMessage(_hSelf, IDC_COLOUR_LIST, LB_SETITEMDATA , nColor, (LPARAM) tclPattern::getDefColorNum(nColor));
            //if (g_bgColor == colourItems[nColor])
            //::SendDlgItemMessage(_hSelf, IDC_COLOUR_LIST, LB_SETCURSEL, nColor, 0);
         }
         //::SetCapture(_hSelf);
         return TRUE;
      }

   case WM_CTLCOLORLISTBOX:
      return (LRESULT) CreateSolidBrush(GetSysColor(COLOR_3DFACE));

   case WM_DRAWITEM:
      {
         HDC hdc;
         COLORREF	cr;
         HBRUSH		hbrush;

         DRAWITEMSTRUCT *pdis = (DRAWITEMSTRUCT *)lParam;
         hdc = pdis->hDC;
         RECT rc = pdis->rcItem;

         // Transparent.
         SetBkMode(hdc,TRANSPARENT);

         // NULL object
         if (pdis->itemID == UINT(-1)) return 0; 

         switch (pdis->itemAction)
         {
         case ODA_DRAWENTIRE:
            switch (pdis->CtlID)
            {
            case IDC_COLOUR_LIST:
               rc = pdis->rcItem;
               cr = (COLORREF) pdis->itemData;
               InflateRect(&rc, -3, -3);
               hbrush = CreateSolidBrush((COLORREF)cr);
               FillRect(hdc, &rc, hbrush);
               DeleteObject(hbrush);
               FrameRect(hdc, &rc, (HBRUSH) GetStockObject(GRAY_BRUSH));
               break;
            }
            // *** FALL THROUGH ***
         case ODA_SELECT:
            rc = pdis->rcItem;
            if (pdis->itemState & ODS_SELECTED)
            {
               rc.bottom --;
               rc.right --;
               // Draw the lighted side.
               HPEN hpen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
               HPEN holdPen = (HPEN)SelectObject(hdc, hpen);
               MoveToEx(hdc, rc.left, rc.bottom, NULL);
               LineTo(hdc, rc.left, rc.top);
               LineTo(hdc, rc.right, rc.top);
               SelectObject(hdc, holdPen);
               DeleteObject(hpen);
               // Draw the darkened side.
               hpen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
               holdPen = (HPEN)SelectObject(hdc, hpen);
               LineTo(hdc, rc.right, rc.bottom);
               LineTo(hdc, rc.left, rc.bottom);
               SelectObject(hdc, holdPen);
               DeleteObject(hpen);
            }
            else 
            {
               hbrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
               FrameRect(hdc, &rc, hbrush);
               DeleteObject(hbrush);
            }
            break;
         case ODA_FOCUS:
            rc = pdis->rcItem;
            InflateRect(&rc, -2, -2);
            DrawFocusRect(hdc, &rc);
            break;
         default:
            break;
         }
         return TRUE;
      }

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDOK :
         {
					//isColourChooserLaunched = true;
            CHOOSECOLOR cc;                 // common dialog box structure 
            static COLORREF acrCustClr[16] = {
               RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),\
               RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),\
               RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),\
               RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),RGB(0xFF,0xFF,0xFF),\
            }; // array of custom colors 

            // Initialize CHOOSECOLOR 
            ::ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = _hParent;

            cc.lpCustColors = (LPDWORD) acrCustClr;
            cc.rgbResult = _colour;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            display(false);

            if (ChooseColor(&cc)==TRUE) 
            {
               ::SendMessage(_hParent, WM_PICKUP_COLOR, cc.rgbResult, 0);
            }
            else
            {
               ::SendMessage(_hParent, WM_PICKUP_CANCEL, 0, 0);
            }

            return TRUE;
         }

      case IDC_COLOUR_LIST :
         {
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
               int i = (int)::SendMessage((HWND)lParam, LB_GETCURSEL, 0L, 0L);
               _colour = (COLORREF)::SendMessage((HWND)lParam, LB_GETITEMDATA, i, 0L);

               ::SendMessage(_hParent, WM_PICKUP_COLOR, _colour, 0);
               return TRUE;
            }
         }

      default :
         return FALSE;
      }

   case WM_ACTIVATE :
      {
         if (LOWORD(wParam) == WA_INACTIVE)
				//if (!isColourChooserLaunched)
               ::SendMessage(_hParent, WM_PICKUP_CANCEL, 0, 0);
         return TRUE;
      }

   }
   return FALSE;
}



