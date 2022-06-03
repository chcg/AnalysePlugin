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

#include "tclColor.h"
// defined in windows
//#define RGB(r,g,b) ((unsigned long)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

const TCHAR*  tclColor::transColorName[COLOR_COUNT] = {
   TEXT("black"),             //RGB(0,       0,    0)},
   TEXT("red"),               //RGB(0xFF,    0,    0)},
   TEXT("darkRed"),           //RGB(0x80,    0,    0)},
   TEXT("deepPurple"),        //RGB(0x87, 0x13, 0x97)},
   TEXT("darkBlue"),          //RGB(0,       0, 0x80)},
   TEXT("darkGreen"),         //RGB(0,    0x80,    0)},

   TEXT("darkGrey"),          //RGB(64,     64,   64)},
   TEXT("liteRed"),           //RGB(0xFF, 0x40, 0x40)},
   TEXT("brown"),             //RGB(128,    64,    0)},
   TEXT("purple"),            //RGB(0x80, 0x00, 0xFF)},
   TEXT("blue"),              //RGB(0,       0, 0xFF)},
   TEXT("blueGreen"),         //RGB(0,    0x80, 0x80)},

   TEXT("grey"),              //RGB(128,   128,  128)},
   TEXT("orange"),            //RGB(0xFF, 0x80, 0x00)},
   TEXT("beige"),             //RGB(192,   128,   64)   RGB(0xFD, 0xF8, 0xE3)},
   TEXT("pink"),              //RGB(0xFF, 0x00, 0xFF)}  RGB(0xE7, 0xD8, 0xE9)},
   TEXT("liteBlue"),          //6CA8E0 RGB(0xA6, 0xCA, 0xF0)}, 
   TEXT("green"),             //RGB(0,    0xBE,    0)},

   TEXT("liteGrey"),          //RGB(192,   192,  192)},
   TEXT("darkYellow"),        //RGB(0xFF, 0xC0,    0)},
   TEXT("liteBeige"),         //RGB(0xDB, 0xB7, 0x93)   RGB(0xFE, 0xFC, 0xF5)},
   TEXT("litePink"),          //RGB(FF, 99, CC)   extLiteBlueRGB(0xF9, 0xF4, 0xFF)   RGB(0xF2, 0xF4, 0xFF)},
   TEXT("cyan"),              //RGB(0,    0xFF, 0xFF)},
   TEXT("liteGreen"),         //RGB(0,    0xFF,    0)},

   TEXT("white"),             //RGB(0xFF, 0xFF, 0xFF)},
   TEXT("yellow"),            //RGB(0xFF, 0xFF,    0)},
   TEXT("offWhite"),          //RGB(0xFF, 0xFB, 0xF0)},
   TEXT("veryLitePurple"),     //RGB(0xE7, 0xD8, 0xE9)   RGB(192,   128,   64)},
   TEXT("veryLiteBlue"),      //RGB(0xC4, 0xF9, 0xFD)},
   TEXT("veryLiteGrey")       //RGB(224,   224,  224)},
};

const unsigned long tclColor::transColorNum[COLOR_COUNT] = {
   RGB(0, 0, 0),//"black",             
   RGB(0xFF, 0, 0),//"red",               
   RGB(0x80, 0, 0),//"darkRed",           
   RGB(0x87, 0x13, 0x97),//"deepPurple",        
   RGB(0, 0, 0x80),//"darkBlue",          
   RGB(0, 0x80, 0),//"darkGreen",         

   RGB(64, 64, 64),//"darkGrey",          
   RGB(0xFF, 0x60, 0x60),//"liteRed",           
   RGB(128, 64, 0),//"brown",             
   RGB(0x80, 0x00, 0xFF),//"purple",            
   RGB(0, 0, 0xFF),//"blue",              
   RGB(0, 0x80, 0x80),//"blueGreen",         

   RGB(128, 128, 128),//"grey",              
   RGB(0xFF, 0x80, 0x00),//"orange",            
   RGB(192, 128, 64),//"beige"
   RGB(0xFF, 0x00, 0xFF),//pink,    
   RGB(0x6C, 0xA8, 0xE0),//"liteBlue",        RGB(0xA6, 0xCA, 0xF0)
   RGB(0, 0xBE, 0),//"green",             

   RGB(192, 192, 192),//"liteGrey",          
   RGB(0xFF, 0xC0, 0),//"darkYellow",        
   RGB(0xDB, 0xB7, 0x93),//"liteBeige",             
   RGB(0xFF, 0x99, 0xDD),//"litePink" RGB(0xF9, 0xF4, 0xFF),//"extLiteBlue",   
   RGB(0, 0xFF, 0xFF),//"cyan",              
   RGB(0, 0xFF, 0),//"liteGreen",         

   RGB(0xFF, 0xFF, 0xFF),//"white",             
   RGB(0xFF, 0xFF, 0),//"yellow",            
   RGB(0xFF, 0xFB, 0xF0),//"offWhite",          
   RGB(0xEF, 0xD8, 0xE9),//"veryLitePurple",         
   RGB(0xC4, 0xF9, 0xFD),//"veryLiteBlue",      
   RGB(224, 224, 224) //"veryLiteGrey",      
};
