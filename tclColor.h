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

#ifndef TCL_COLOUR_H
#define TCL_COLOUR_H

#include <windows.h>
#include <assert.h>
#include "Common.h"


typedef unsigned long tColor; // allows to store all RGB values between ffffff an 000000

#define COLOR_COUNT (tclColor::max_color - tclColor::min_color - 1) // -1 because min_color is no color

class tclColor {
public:
   enum teColor
   {
      min_color = 0x00ffffff, // min value is las valid RGB value. So we can detect the enum in a number
      black,
      red,
      darkRed,
      deepPurple,
      darkBlue,
      darkGreen,

      darkGrey,
      liteRed,
      brown,
      purple,
      blue,
      blueGreen,

      grey,
      orange,
      beige,
      pink,
      liteBlue,
      green,

      liteGrey,
      darkYellow,
      liteBeige,
      litePink,
      cyan,
      liteGreen,

      white,
      yellow,
      offWhite,
      veryLitePurple,
      veryLiteBlue,
      veryLiteGrey,
      max_color
   };

   static int getDefColorListSize() {
      return COLOR_COUNT;
   }

   static generic_string getColStr(tColor c) {
      generic_string s;
      if (c > min_color) {
         tColor c2 = c - (min_color + 1);
         if (c2 < COLOR_COUNT) {
            s = generic_string(transColorName[c2]);
            return s;
         }
         else {
            assert("value c is bigger as allowed in enum teColor");
            return transColorName[0];
         }
      }
      else {
         for (int i = 0; i < COLOR_COUNT; i++) {
            if (c == transColorNum[i]) {
               s = transColorName[i];
               return s;
            }
         }
         TCHAR col[8];
         c += 0x01000000; // to preserve all zeros
         generic_itoa(c, col, 16);
         col[0] = TEXT('#'); // set string trigger 
         s = generic_string(col);
         return s;
      }
   }

   static unsigned long convColorStr2Rgb(const generic_string& color) {
      unsigned long ul;
      if (color.length() == 7 && color[0] == TEXT('#')) {
         // convert RGB 1 to 1
         ul = generic_strtol((color.c_str() + 1), NULL, 16);
         return ul;
      }
      for (int i = 0; i < COLOR_COUNT; i++) {
         if (color == transColorName[i]) {
            ul = transColorNum[i];
            return ul;
         }
      }
      assert("string color not found in defined list");   
      ul = transColorNum[0];
      return ul;
   }

   static tColor convColorNum2Enum(unsigned long color) {
      tColor c;
      for (int i = 0; i<COLOR_COUNT; i++) {
         if (color == transColorNum[i]) {
            c = (min_color + i + 1);
            return c;
         }
      }
      // send 
      return color;
   }
   static void setColStr(tColor& val, const generic_string& color) {
      val = convColorStr2Rgb(color);
   }
   static tColor getDefColorNum(int e) {
      if (e < COLOR_COUNT) {
         return transColorNum[e];
      }
      else {
         assert("wrong color number given");
         return transColorNum[0];
      }
   }
   static COLORREF getColRgb(tColor c) {
      if (c > min_color) {
         return transColorNum[c - (tColor)min_color - 1];
      }
      else {
         return c;
      }
   }
private:
   static const TCHAR*  transColorName[COLOR_COUNT];
   static const unsigned long transColorNum[COLOR_COUNT];

};

#endif // TCL_COLOUR_H
