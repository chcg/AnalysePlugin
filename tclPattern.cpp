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
/**
tclPattern implements information stored to execute one search on
*/

#include "stdafx.h"
#include "precompiledHeaders.h"
#include "tclPattern.h"
#include <windows.h>
// defined in windows
//#define RGB(r,g,b) ((unsigned long)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

const char*  tclPattern::transSearchType[max_searchType] = {
   "normal",
   "escaped",
   "regex"
};
const char*  tclPattern::transSelectionType[max_selectionType] = {
   "text",
   "line"
};
const char*  tclPattern::transColorName[max_color] = {
   "black",             //RGB(0,       0,    0)},
   "red",               //RGB(0xFF,    0,    0)},
   "darkRed",           //RGB(0x80,    0,    0)},
   "deepPurple",        //RGB(0x87, 0x13, 0x97)},
   "darkBlue",          //RGB(0,       0, 0x80)},
   "darkGreen",         //RGB(0,    0x80,    0)},

   "darkGrey",          //RGB(64,     64,   64)},
   "liteRed",           //RGB(0xFF, 0x40, 0x40)},
   "brown",             //RGB(128,    64,    0)},
   "purple",            //RGB(0x80, 0x00, 0xFF)},
   "blue",              //RGB(0,       0, 0xFF)},
   "blueGreen",         //RGB(0,    0x80, 0x80)},

   "grey",              //RGB(128,   128,  128)},
   "orange",            //RGB(0xFF, 0x80, 0x00)},
   "beige",             //RGB(192,   128,   64)   RGB(0xFD, 0xF8, 0xE3)},
   "pink",              //RGB(0xFF, 0x00, 0xFF)}  RGB(0xE7, 0xD8, 0xE9)},
   "liteBlue",          //6CA8E0 RGB(0xA6, 0xCA, 0xF0)}, 
   "green",             //RGB(0,    0xBE,    0)},

   "liteGrey",          //RGB(192,   192,  192)},
   "darkYellow",        //RGB(0xFF, 0xC0,    0)},
   "liteBeige",         //RGB(0xDB, 0xB7, 0x93)   RGB(0xFE, 0xFC, 0xF5)},
   "litePink",          //RGB(FF, 99, CC)   extLiteBlueRGB(0xF9, 0xF4, 0xFF)   RGB(0xF2, 0xF4, 0xFF)},
   "cyan",              //RGB(0,    0xFF, 0xFF)},
   "liteGreen",         //RGB(0,    0xFF,    0)},

   "white",             //RGB(0xFF, 0xFF, 0xFF)},
   "yellow",            //RGB(0xFF, 0xFF,    0)},
   "offWhite",          //RGB(0xFF, 0xFB, 0xF0)},
   "veryLitePurple",     //RGB(0xE7, 0xD8, 0xE9)   RGB(192,   128,   64)},
   "veryLiteBlue",      //RGB(0xC4, 0xF9, 0xFD)},
   "veryLiteGrey"       //RGB(224,   224,  224)},
};
const unsigned long tclPattern::transColorNum[max_color] = {
   RGB(0,       0,    0),//"black",             
   RGB(0xFF,    0,    0),//"red",               
   RGB(0x80,    0,    0),//"darkRed",           
   RGB(0x87, 0x13, 0x97),//"deepPurple",        
   RGB(0,       0, 0x80),//"darkBlue",          
   RGB(0,    0x80,    0),//"darkGreen",         

   RGB(64,     64,   64),//"darkGrey",          
   RGB(0xFF, 0x60, 0x60),//"liteRed",           
   RGB(128,    64,    0),//"brown",             
   RGB(0x80, 0x00, 0xFF),//"purple",            
   RGB(0,       0, 0xFF),//"blue",              
   RGB(0,    0x80, 0x80),//"blueGreen",         
   
   RGB(128,   128,  128),//"grey",              
   RGB(0xFF, 0x80, 0x00),//"orange",            
   RGB(192,   128,   64),//"beige"
   RGB(0xFF, 0x00, 0xFF),//pink,    
   RGB(0x6C, 0xA8, 0xE0),//"liteBlue",        RGB(0xA6, 0xCA, 0xF0)
   RGB(0,    0xBE,    0),//"green",             

   RGB(192,   192,  192),//"liteGrey",          
   RGB(0xFF, 0xC0,    0),//"darkYellow",        
   RGB(0xDB, 0xB7, 0x93),//"liteBeige",             
   RGB(0xFF, 0x99, 0xDD),//"litePink" RGB(0xF9, 0xF4, 0xFF),//"extLiteBlue",   
   RGB(0,    0xFF, 0xFF),//"cyan",              
   RGB(0,    0xFF,    0),//"liteGreen",         

   RGB(0xFF, 0xFF, 0xFF),//"white",             
   RGB(0xFF, 0xFF,    0),//"yellow",            
   RGB(0xFF, 0xFB, 0xF0),//"offWhite",          
   RGB(0xEF, 0xD8, 0xE9),//"veryLitePurple",         
   RGB(0xC4, 0xF9, 0xFD),//"veryLiteBlue",      
   RGB(224,   224,  224) //"veryLiteGrey",      
};

//unsigned long tclPattern::transColorNum16[16] = {
//    RGB(0,       0,    0),//"black",             
//    RGB(0xFF,    0,    0),//"red",               
//    RGB(0,    0xBE,    0),//"green",             
//    RGB(0,       0, 0xFF),//"blue",              
//    RGB(128,   128,  128),//"grey",              
//    RGB(0xFF, 0xFF, 0xFF),//"white",             
//    RGB(128,    64,    0),//"brown",             
//    RGB(0xFF, 0xFF,    0),//"yellow",            
//    RGB(0,    0xFF, 0xFF),//"cyan",              
//    RGB(0xFF, 0x80, 0x00),//"orange",            
//    RGB(0x80, 0x00, 0xFF),//"purple",            
//    RGB(0x80,    0,    0),//"darkRed",           
//    RGB(0xFF, 0x40, 0x40),//"liteRed",           
//    RGB(0,    0x80,    0),//"darkGreen",         
//    RGB(0,    0x80, 0x80),//"blueGreen",         
//    RGB(0,    0xFF,    0)//"liteGreen",         
//}; 

const char*  tclPattern::transBool[2] = {
   "false",
   "true" // == 1
};

// ########################################################

tclPattern::tclPattern()
:mDoSearch(true)
,mSearchType(normal)
,mWholeWord(false)
,mMatchCase(false)
,mBold(false)
,mItalic(false)
,mUnderlined(false)
,mColor(black)
,mBgColor(white)
,mHideText(false)
,mDoReplace(false)
,mSelectionType(line)
{}

tclPattern::~tclPattern(){}

std::string tclPattern::getSearchText() const{
   return mSearchText;
}
std::string tclPattern::getComment() const{
   return mComment;
}
std::string tclPattern::getReplaceText() const{
   return mReplaceText;
}
void tclPattern::setSearchText(const std::string& thisSearchText){
   mSearchText = thisSearchText;
}
void tclPattern::setComment(const std::string& thisComment){
   mComment = thisComment;
}
void tclPattern::setReplaceText(const std::string& thisReplaceText){
   mReplaceText = thisReplaceText;
}
std::string tclPattern::getSearchTypeStr() const {
   return std::string(transSearchType[mSearchType]);
}
void tclPattern::setSearchTypeStr(const std::string& type) {
   for(int i=0; i<max_searchType;i++) {
      if(type == transSearchType[i]) {
         mSearchType =(teSearchType)i;
         break;
      }
   }
}
void tclPattern::setSearchType(int type) {
   mSearchType =(teSearchType)type;
}
std::string tclPattern::getBoldStr() const {
   return transBool[mBold];
}
void tclPattern::setBold(bool isBold) {
   mBold =isBold;
}
std::string tclPattern::getItalicStr() const {
   return transBool[mItalic];
}
void tclPattern::setItalic(bool isItalic) {
   mItalic =isItalic;
}
std::string tclPattern::getUnderlinedStr() const {
   return transBool[mUnderlined];
}
void tclPattern::setUnderlined(bool isUnderlined) {
   mUnderlined =isUnderlined;
}
std::string tclPattern::getColorStr()const{
   return  std::string(transColorName[mColor]);
}

std::string tclPattern::getBgColorStr()const{
   return  std::string(transColorName[mBgColor]);
}

std::string tclPattern::convColor2Str(teColor col) {
   return  std::string(transColorName[col]);
}

unsigned long tclPattern::convColorStr2Num(const std::string& color) {
   for(int i=0; i<max_color;i++) {
      if(color == transColorName[i]) {
         return transColorNum[i];
      }
   }
   return transColorNum[0];
}

tclPattern::teColor tclPattern::convColorNum2Enum(unsigned long color) {
   for(int i=0; i<max_color;i++) {
      if(color == transColorNum[i]) {
         return (teColor)i;
      }
   }
   return max_color;
}

void tclPattern::setColorStr(const std::string& color) {
   for(int i=0; i<max_color;i++) {
      if(color ==transColorName[i]) {
         mColor =(teColor)i;
         break;
      }
   }
}

void tclPattern::setBgColorStr(const std::string& color) {
   for(int i=0; i<max_color;i++) {
      if(color ==transColorName[i]) {
         mBgColor =(teColor)i;
         break;
      }
   }
}
void tclPattern::setColor(teColor color) {
   mColor = color;
}
void tclPattern::setBgColor(teColor color) {
   mBgColor = color;
}
std::string tclPattern::getHideTextStr()const{
   return  std::string(transBool[mHideText]);
}
std::string tclPattern::getIsReplaceTextStr()const{
   return  std::string(transBool[mDoReplace]);
}
void tclPattern::setHideText(bool isHideText) {
   mHideText = isHideText;
}
void tclPattern::setIsReplaceText(bool isReplaceText) {
   mDoReplace = isReplaceText;
}
std::string tclPattern::getSelectionTypeStr()const{
   return  std::string(transSelectionType[mSelectionType]);
}

bool tclPattern::convBool(const std::string& val) const {
   int i=0;
   for(; i < 2 ;i++) { // 0 false 1 true
      if(val ==transBool[i]) {
         return i?true:false;
      }
   }
   return false;
}
void tclPattern::setSelectionType(int selectionType) {
   mSelectionType =(teSelectionType)selectionType;
}
void tclPattern::setSelectionTypeStr(const std::string& type){
   for(int i=0; i<max_selectionType;i++) {
      if(type ==transSelectionType[i]) {
         mSelectionType =(teSelectionType)i;
         break;
      }
   }
}

std::string tclPattern::convertExtendedToString() const
{	// IN query OUT result IN length
   //query may equal to result, since it always gets smaller
   const char* query = mSearchText.c_str();
   int length = (int)mSearchText.size();
   std::string result;
   result.reserve(length);
   int i = 0, j = 0;
   int charLeft = length;
   bool isGood = true;
   char current;
   while(i < length) {	//because the backslash escape quences always reduce the size of the string, no overflow checks have to be made for target, assuming parameters are correct
      current = query[i];
      charLeft--;
      if (current == '\\' && charLeft) {	//possible escape sequence
         i++;
         charLeft--;
         current = query[i];
         switch(current) {
            case 'r':
               result += '\r';
               break;
            case 'n':
               result += '\n';
               break;
            case '0':
               result += '\0';
               break;
            case 't':
               result += '\t';
               break;
            case '\\':
               result += '\\';
               break;
            case 'b':
            case 'd':
            case 'o':
            case 'x': {
               int size = 0, base = 0;
               if (current == 'b') {			//11111111
                  size = 8, base = 2;
               } else if (current == 'o') {	//377
                  size = 3, base = 8;
               } else if (current == 'd') {	//255
                  size = 3, base = 10;
               } else if (current == 'x') {	//0xFF
                  size = 2, base = 16;
               }
               if (charLeft >= size) {
                  int res = 0;
                  if (readBase(query+(i+1), &res, base, size)) {
                     result += (char)res;
                     i+=size;
                     break;
                  }
               }
               //not enough chars to make parameter, use default method as fallback
                      }
            default: {	//unknown sequence, treat as regular text
               result += '\\';
               j++;
               result += current;
               isGood = false;
               break;
                     }
         }
      } else {
         result += query[i];
      }
      i++;
      j++;
   }
   result += '\0';
   return result;
}

bool tclPattern::readBase(const char * string, int * value, int base, int size) const {
   int i = 0, temp = 0;
   *value = 0;
   char max = '0' + base - 1;
   char current;
   while(i < size) {
      current = string[i];
      if (current >= '0' && current <= max) {
         temp *= base;
         temp += (current - '0');
      } else {
         return false;
      }
      i++;
   }
   *value = temp;
   return true;
}
const char** tclPattern::getDefSelTypeList() const {
   return transSelectionType;
}
const char** tclPattern::getDefColorList() {
   return transColorName;
}
const unsigned long* tclPattern::getDefColorNumList() {
   return transColorNum;
}
unsigned long tclPattern::getDefColorNum(int e) {
   return transColorNum[e];
}
const char** tclPattern::getDefSearchTypeList() const {
   return transSearchType;
}
unsigned long tclPattern::getColorNum() const {
   return transColorNum[mColor];
}
unsigned long tclPattern::getBgColorNum() const {
   return transColorNum[mBgColor];
}

