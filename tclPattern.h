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
tclPattern stores the infos to execute one search
*/

#ifndef TCLPATTERN_H
#define TCLPATTERN_H
#include <string>

/**
* A Pattern is a text and additionaly stores the configuration information for it.
* The text has two visualisation forms 1. the text as shown in find config. 
* 2. the text as a result of unescaping special chars 
*/
class tclPattern {
public:
   enum teSearchType {
      normal,
      escaped,
      regex,
      max_searchType
   };

   enum teSelectionType {
      text,
      line,
      max_selectionType
   };

   enum teColor 
   {
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


protected:
   // following tables are used to translate enum into text and back
   static const TCHAR*  transSearchType[max_searchType];
   static const TCHAR*  transSelectionType[max_selectionType];
   static const TCHAR*  transColorName[max_color];
   static const TCHAR*  transBool[2];
   //static unsigned long transColorNum16[16];
   static const unsigned long transColorNum[max_color];

public:
   
   tclPattern();

   tclPattern(const tclPattern& right) {
      mDoSearch = right.mDoSearch;
      mSearchText = right.mSearchText;
      mReplaceText = right.mReplaceText;
      mSearchType = right.mSearchType;
      mWholeWord = right.mWholeWord;
      mMatchCase = right.mMatchCase;
      mBold = right.mBold;
      mItalic = right.mItalic;
      mUnderlined = right.mUnderlined;
      mColor = right.mColor;
      mBgColor = right.mBgColor;
      mHideText = right.mHideText;
      mDoReplace = right.mDoReplace;
      mSelectionType = right.mSelectionType;
      mComment = right.mComment;
   }

   virtual ~tclPattern();
   
   tclPattern& operator = (const tclPattern& right){
      if(&right == this) {
         return *this;
      }
      mDoSearch = right.mDoSearch;
      mSearchText = right.mSearchText;
      mReplaceText = right.mReplaceText;
      mSearchType = right.mSearchType;
      mWholeWord = right.mWholeWord;
      mMatchCase = right.mMatchCase;
      mBold = right.mBold;
      mItalic = right.mItalic;
      mUnderlined = right.mUnderlined;
      mColor = right.mColor;
      mBgColor = right.mBgColor;
      mHideText = right.mHideText;
      mDoReplace = right.mDoReplace;
      mSelectionType = right.mSelectionType;
      mComment = right.mComment;
      return *this;
   }

   bool operator == (const tclPattern& right) const {
      if(&right == this) {
         return true;
      }
      bool bRet =((mDoSearch == right.mDoSearch) &&
                  (mSearchText == right.mSearchText) &&
                  (mReplaceText == right.mReplaceText) &&
                  (mSearchType == right.mSearchType) &&
                  (mWholeWord == right.mWholeWord) &&
                  (mMatchCase == right.mMatchCase) &&
                  (mBold == right.mBold) &&
                  (mItalic == right.mItalic) &&
                  (mUnderlined == right.mUnderlined) &&
                  (mColor == right.mColor) &&
                  (mBgColor == right.mBgColor) &&
                  (mHideText == right.mHideText) &&
                  (mDoReplace == right.mDoReplace) &&
                  (mSelectionType == right.mSelectionType) &&
                  (mComment == right.mComment));
      return bRet;
   }

   bool isSearchEqual(const tclPattern& right) const {
      if(&right == this) {
         return true;
      }
      bool bRet =((mDoSearch == right.mDoSearch) &&
                  (mSearchText == right.mSearchText) &&
                  (mSearchType == right.mSearchType) &&
                  (mWholeWord == right.mWholeWord) &&
                  (mMatchCase == right.mMatchCase) &&
                  (mSearchText == right.mSearchText) &&
                  (mDoReplace == right.mDoReplace));
     return bRet;
   }

   bool getDoSearch() const{
      return mDoSearch;
   }
   generic_string tclPattern::getDoSearchStr() const {
      return transBool[mDoSearch];
   }

   void setDoSearch(bool bActive) {
      mDoSearch = bActive;
   }
   void setDoSearchStr(const generic_string& val){
      mDoSearch = convBool(val);
   }

   generic_string getSearchText() const;
   generic_string getComment() const;

   generic_string getReplaceText() const;

   /** used for the search algorithm */
   generic_string getSearchTextConverted() const {
      if(mSearchType==escaped) {
         return convertExtendedToString();
      } else {
         return mSearchText;
      }
   }

   void setSearchText(const generic_string& thisSearchText);
   void setComment(const generic_string& thisComment);

   void setReplaceText(const generic_string& thisReplaceText);

   generic_string getSearchTypeStr() const ;

   teSearchType getSearchType() const{
      return mSearchType;
   }
   
   int getDefSearchTypeListSize() const {
      return max_searchType;
   }

   const TCHAR** getDefSearchTypeList() const ;

   void setSearchTypeStr(const generic_string& type) ;

   void setSearchType(int type) ;
   generic_string getBoldStr() const;

   bool getIsBold() const{
      return mBold;
   }

   void setBold(bool isBold) ;

   void setBoldStr(const generic_string& val){
      mBold = convBool(val);
   }

   generic_string getWholeWordStr() const {
      return transBool[mWholeWord];
   }

   bool getIsWholeWord() const{
      return mWholeWord;
   }

   void setWholeWord(bool isWholeWord) {
      mWholeWord =isWholeWord;
   }

   void setWholeWordStr(const generic_string& val){
      mWholeWord = convBool(val);
   }

   generic_string getMatchCaseStr() const {
      return transBool[mMatchCase];
   }

   bool getIsMatchCase() const{
      return mMatchCase;
   }
   bool getIsHideText() const{
      return mHideText;
   }
   bool getIsReplaceText() const{
      return mDoReplace;
   }
   bool getIsUnderlined() const{
      return mUnderlined;
   }
   bool getIsItalic() const{
      return mItalic;
   }

   void setMatchCase(bool isMatchCase) {
      mMatchCase =isMatchCase;
   }

   void setMatchCaseStr(const generic_string& val){
      mMatchCase = convBool(val);
   }

   generic_string getItalicStr() const;
   void setItalic(bool isItalic) ;

   void setItalicStr(const generic_string& val){
      mItalic = convBool(val);
   }

   generic_string getUnderlinedStr() const;

   void setUnderlined(bool isUnderlined) ;

   void setUnderlinedStr(const generic_string& val){
      mUnderlined = convBool(val);
   }

   generic_string getColorStr()const;
   generic_string getBgColorStr()const;
   
   teColor getColor() const {
      return mColor;
   }
   teColor getBgColor() const {
      return mBgColor;
   }

   unsigned long getColorNum() const ;
   unsigned long getBgColorNum() const ;

   void setColorStr(const generic_string& color) ;
   void setBgColorStr(const generic_string& color) ;

   void setColor(teColor color) ;
   void setBgColor(teColor color) ;

   static int getDefColorListSize() {
      return max_color;
   }

   static unsigned long convColorStr2Num(const generic_string& color);
   static teColor convColorNum2Enum(unsigned long color);
   static generic_string convColor2Str(teColor col);

   static const TCHAR** getDefColorList() ;
   static const unsigned long* getDefColorNumList() ;
   static unsigned long getDefColorNum(int e) ;

   generic_string getHideTextStr()const;
   generic_string getIsReplaceTextStr()const;

   void setHideText(bool isHideText) ;
   void setIsReplaceText(bool isReplaceText);

   void setIsReplaceTextStr(const generic_string& val) {
      mDoReplace = convBool(val);
   }

   void setHideTextStr(const generic_string& val) {
      mHideText = convBool(val);
   }

   generic_string getSelectionTypeStr()const;

   teSelectionType getSelectionType()const{
      return mSelectionType;
   }

   int getDefSelTypeListSize() const {
      return max_selectionType;
   }

   const TCHAR** getDefSelTypeList() const ;

   void setSelectionType(int selectionType) ;

   void setSelectionTypeStr(const generic_string& type);

protected:
   /** function is original copy from NPP project to be in syncwith their options */
   generic_string convertExtendedToString() const;
   /** function is original copy from NPP project to be in syncwith their options */
   bool readBase(const TCHAR* string, int* value, int base, int size) const;
   /** used to translate 0 = false and 1 = true */
   bool convBool(const generic_string& val) const;

protected:
   /** set to true in case that the pattern shall be regarded in the search **/
   bool mDoSearch;
   /** the search pattern as entered by the user */
   generic_string mSearchText;
   /** the replacement text as entered by the user */
   generic_string mReplaceText;
   /** kind of treatment for the search pattern same as in find dialog of NPP */
   teSearchType mSearchType;
   /** true if the found text shall be fitting to a whole word */
   bool mWholeWord;
   /** true if the found text shall be searched case sensitive */
   bool mMatchCase;
   /** true if the found text shall be shown in bold */
   bool mBold;
   /** true if the found text shall be shown in italic */
   bool mItalic;
   /** true if the found text shall be shown underlined  */
   bool mUnderlined;
   /** color in which the found text shall be shown */
   teColor mColor;
   teColor mBgColor;
   /** true if the found text shall be madi invisible */
   bool mHideText;
   bool mDoReplace;
   /** defines if the found text or the whole in which the text was found shall be 
   highlighted with aforesaid atributes */
   teSelectionType mSelectionType;
   /** the comment as entered by the user */
   generic_string mComment;
};
#endif //TCLPATTERN_H
