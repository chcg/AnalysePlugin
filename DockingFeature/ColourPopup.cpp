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
//#include "stdafx.h"
#include "ColourPopup.h"
#include "tclPattern.h"
#include <commdlg.h>

#define COLPOP_ITEM_LINES   6
#define COLPOP_ITEM_COLUMNS 5

void ColourPopup::create(int dialogID) 
{
   _hSelf = ::CreateDialogParam(_hInst, MAKEINTRESOURCE(dialogID), _hParent, dlgProc, reinterpret_cast<LPARAM>(this));

   if (!_hSelf)
   {
      throw std::runtime_error("ColourPopup::create : CreateDialogParam() function return null");
   }
   Window::getClientRect(_rc);
   display();
}


void ColourPopup::doDialog(POINT p)
{
   if (!isCreated()){
     create(IDD_COLOUR_POPUP);
   }
   int px, py;
   int cx = _rc.right - _rc.left;
   int cy = _rc.bottom - _rc.top;
   if (p.x >= 0) {
	   px = (p.x > cx) ? (p.x - cx) : 0;
   } else {
	   px = p.x - cx;
   }
   if (p.y >= 0) {
	   py = (p.y > cy) ? (p.y - cy) : 0;
   } else {
	   py = p.y - cy;
   }
   ::SetWindowPos(_hSelf, HWND_TOP, px, py, cx, cy, SWP_SHOWWINDOW);
}

INT_PTR CALLBACK ColourPopup::dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
   switch (message) 
   {
   case WM_MEASUREITEM:
      {
         RECT rc;
         LPMEASUREITEMSTRUCT lpmis = reinterpret_cast<LPMEASUREITEMSTRUCT>(lParam);
         ::GetWindowRect(::GetDlgItem(hwnd, lpmis->CtlID), &rc);
         lpmis->itemHeight = (rc.bottom-rc.top)/COLPOP_ITEM_LINES; 
         lpmis->itemWidth = (rc.right-rc.left)/COLPOP_ITEM_COLUMNS;
         return TRUE;
      }

   case WM_INITDIALOG :
      {
         ColourPopup *pColourPopup = reinterpret_cast<ColourPopup *>(lParam);
         pColourPopup->_hSelf = hwnd;
         ::SetWindowLongPtr(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(lParam));
         pColourPopup->run_dlgProc(message, wParam, lParam);
         return TRUE;
      }

   default :
      {
         ColourPopup *pColourPopup = reinterpret_cast<ColourPopup *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
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
         for (nColor = 0 ; nColor < size ; nColor++)
         {
            ::SendDlgItemMessage(_hSelf, IDC_COLOUR_LIST, LB_ADDSTRING, nColor, (LPARAM) "");
            ::SendDlgItemMessage(_hSelf, IDC_COLOUR_LIST, LB_SETITEMDATA , nColor, (LPARAM) tclPattern::getDefColorNum(nColor));
         }
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
            [[fallthrough]];
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
            // Initialize CHOOSECOLOR 
            ::ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = _hParent;

            cc.lpCustColors = (LPDWORD)_pacCustomColors;
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
               auto i = ::SendMessage(reinterpret_cast<HWND>(lParam), LB_GETCURSEL, 0L, 0L);
               _colour = static_cast<COLORREF>(::SendMessage(reinterpret_cast<HWND>(lParam), LB_GETITEMDATA, i, 0L));

               ::SendMessage(_hParent, WM_PICKUP_COLOR, _colour, 0);
               return TRUE;
            }
            break;
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



