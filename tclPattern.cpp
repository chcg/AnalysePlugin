/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2020 Matthias H. mattesh(at)gmx.net

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

//#include "stdafx.h"
#include "tclPattern.h"
#include <windows.h>
#include <stdio.h>

const TCHAR*  tclPattern::transSearchType[max_searchType] = {
   TEXT("normal"),
   TEXT("escaped"),
   TEXT("regex"),
   TEXT("rgx_multiline")
};
const TCHAR*  tclPattern::transSelectionType[max_selectionType] = {
   TEXT("text"),
   TEXT("line")
};

const TCHAR*  tclPattern::transBool[2] = {
   TEXT("false"),
   TEXT("true") // == 1
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
,mColor(tclColor::black)
,mBgColor(tclColor::white)
,mHideText(false)
,mDoReplace(false)
,mSelectionType(line)
{}

tclPattern::~tclPattern(){}

generic_string tclPattern::getSearchText() const{
   return mSearchText;
}
generic_string tclPattern::getComment() const{
   return mComment;
}
generic_string tclPattern::getReplaceText() const{
   return mReplaceText;
}
void tclPattern::setSearchText(const generic_string& thisSearchText){
   mSearchText = thisSearchText;
}
void tclPattern::setComment(const generic_string& thisComment){
   mComment = thisComment;
}
void tclPattern::setReplaceText(const generic_string& thisReplaceText){
   mReplaceText = thisReplaceText;
}
generic_string tclPattern::getSearchTypeStr() const {
   return generic_string(transSearchType[mSearchType]);
}
void tclPattern::setSearchTypeStr(const generic_string& type) {
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
generic_string tclPattern::getBoldStr() const {
   return transBool[mBold];
}
void tclPattern::setBold(bool isBold) {
   mBold =isBold;
}
generic_string tclPattern::getItalicStr() const {
   return transBool[mItalic];
}
void tclPattern::setItalic(bool isItalic) {
   mItalic =isItalic;
}
generic_string tclPattern::getUnderlinedStr() const {
   return transBool[mUnderlined];
}
void tclPattern::setUnderlined(bool isUnderlined) {
   mUnderlined =isUnderlined;
}
generic_string tclPattern::getColorStr()const{
   return tclColor::getColStr(mColor);
}
generic_string tclPattern::getBgColorStr()const{
   return tclColor::getColStr(mBgColor);
}

unsigned long tclPattern::convColorStr2Rgb(const generic_string& color) {
   return tclColor::convColorStr2Rgb(color);
}

tColor tclPattern::convColorNum2Enum(unsigned long color) {
   return tclColor::convColorNum2Enum(color);
}

void tclPattern::setColorStr(const generic_string& color) {
   tclColor::setColStr(mColor, color);
}

void tclPattern::setBgColorStr(const generic_string& color) {
   tclColor::setColStr(mBgColor, color);
}
void tclPattern::setColor(tColor color) {
   mColor = color;
}
void tclPattern::setBgColor(tColor color) {
   mBgColor = color;
}

tColor tclPattern::getDefColorNum(int e) {
   return tclColor::getDefColorNum(e);
}
tColor tclPattern::getColorNum() const {
   return tclColor::getColRgb(mColor);
}

tColor tclPattern::getBgColorNum() const {
   return tclColor::getColRgb(mBgColor);
}

generic_string tclPattern::getHideTextStr()const{
   return  generic_string(transBool[mHideText]);
}
generic_string tclPattern::getIsReplaceTextStr()const{
   return  generic_string(transBool[mDoReplace]);
}
void tclPattern::setHideText(bool isHideText) {
   mHideText = isHideText;
}
void tclPattern::setIsReplaceText(bool isReplaceText) {
   mDoReplace = isReplaceText;
}
generic_string tclPattern::getSelectionTypeStr()const{
   return  generic_string(transSelectionType[mSelectionType]);
}

bool tclPattern::convBool(const generic_string& val) const {
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
void tclPattern::setSelectionTypeStr(const generic_string& type){
   for(int i=0; i<max_selectionType;i++) {
      if(type ==transSelectionType[i]) {
         mSelectionType =(teSelectionType)i;
         break;
      }
   }
}

generic_string tclPattern::convertExtendedToString() const
{	// IN query OUT result IN length
   //query may equal to result, since it always gets smaller
   const TCHAR* query = mSearchText.c_str();
   int length = (int)mSearchText.size();
   generic_string result;
   result.reserve(length);
   int i = 0, j = 0;
   int charLeft = length;
   bool isGood = true;
   TCHAR current;
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
               result += (TCHAR)'\0';
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
   result += (TCHAR)'\0';
   return result;
}

bool tclPattern::readBase(const TCHAR* string, int* value, int base, int size) const {
   int i = 0, temp = 0;
   *value = 0;
   TCHAR mymax = '0' + (TCHAR)(base - 1);
   TCHAR current;
   while(i < size) {
      current = string[i];
	  if (current >= '0' && current <= mymax) {
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
const TCHAR** tclPattern::getDefSelTypeList() const {
   return transSelectionType;
}

const TCHAR** tclPattern::getDefSearchTypeList() const {
   return transSearchType;
}

