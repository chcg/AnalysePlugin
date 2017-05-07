/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2017 Matthias H. mattesh(at)gmx.net

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
#include "FindConfigDoc.h"


#include "tinyxml.h"

#include "tclPattern.h"

#define FNDDOC_XMLNS TEXT("xmlns:xsi")
#define FNDDOC_XMLNS_VALUE TEXT("http://www.w3.org/2001/XMLSchema-instance") 
#define FNDDOC_XSD_LOCATION TEXT("xsi:noNamespaceSchemaLocation")
#define FNDDOC_XSD_LOCATION_VALUE TEXT("./AnalyseDoc.xsd")
#define FNDDOC_ANALYSE_DOC TEXT("AnalyseDoc")
#define FNDDOC_HEADLINE TEXT("Headline")  // on even w/o FEATURE_HEADLINE to be able to read the doc
#define FNDDOC_SEARCH_TEXT TEXT("SearchText")
#define FNDDOC_SEARCH_TYPE TEXT("searchType")
#define FNDDOC_DO_SEARCH TEXT("doSearch")
#define FNDDOC_MATCHCASE TEXT("matchCase") 
#define FNDDOC_WHOLEWORD TEXT("wholeWord") 
#define FNDDOC_SELECT TEXT("select")
#define FNDDOC_HIDE TEXT("hide")
#define FNDDOC_BOLD TEXT("bold")
#define FNDDOC_ITALIC TEXT("italic")
#define FNDDOC_UNDERLINED TEXT("underlined")
#define FNDDOC_COLOR TEXT("color")
#define FNDDOC_BGCOLOR TEXT("bgColor")
#define FNDDOC_COMMENT TEXT("comment")

FindConfigDoc::FindConfigDoc(const TCHAR * filename)
   : mDoc(0)
{
   mDoc = new TiXmlDocument(filename);
   if(mDoc) {
      // avoid removal of multiple tabs and spaces in searchstring, as they may be intentional
      mDoc->SetCondenseWhiteSpace(false);
      mDoc->LoadFile();
   }
}

FindConfigDoc::~FindConfigDoc(void)
{
   if(mDoc) {
      delete mDoc;
      mDoc = 0;
   }
}
bool FindConfigDoc::readPatternList(tclPatternList& pl, bool bAppend, bool bLoadNew){
   bool bRes=false;
   if(mDoc) {

      TiXmlNode* node = mDoc->FirstChild(FNDDOC_ANALYSE_DOC);
      if(node) {
         TiXmlNode* node2 = node->FirstChild(FNDDOC_HEADLINE);
         if(node2) {
            TiXmlElement* e3 = node2->ToElement();
            node2 = e3->FirstChild();
            if(node2){
#ifdef FEATURE_HEADLINE
               mHeadline = node2->Value();
#endif
            }
         }
         // continue with patterns
         if(bAppend) {
            node = node->FirstChild(FNDDOC_SEARCH_TEXT);
         } else {
            node = node->LastChild(FNDDOC_SEARCH_TEXT);
         }
      }
      if (bLoadNew) {
         pl.clear(); 
      }
      bRes = true;
      while(node) {
         TiXmlElement* elem = node->ToElement();
         const TCHAR* pc=0;
         if(elem && elem->FirstChild()) {
            tclPattern p;
            generic_string text(elem->FirstChild()->Value());
            p.setSearchText(text);
            
            pc = elem->Attribute(FNDDOC_DO_SEARCH);
            if(pc!=0 && *pc !=0) {
               p.setDoSearchStr(pc);
            }
            pc = elem->Attribute(FNDDOC_SEARCH_TYPE);
            if(pc!=0 && *pc !=0) {
               p.setSearchTypeStr(pc);
            }
            pc = elem->Attribute(FNDDOC_MATCHCASE);
            if(pc!=0 && *pc!=0) {
               p.setMatchCaseStr(pc);
            }
            pc = elem->Attribute(FNDDOC_WHOLEWORD);
            if(pc!=0 && *pc!=0) {
               p.setWholeWordStr(pc);
            }
            pc = elem->Attribute(FNDDOC_SELECT);
            if(pc!=0 && *pc!=0) {
               p.setSelectionTypeStr(pc);
            }
            pc = elem->Attribute(FNDDOC_HIDE);
            if(pc!=0 && *pc!=0) {
               p.setHideTextStr(pc);
            }
            pc = elem->Attribute(FNDDOC_BOLD);
            if(pc!=0 && *pc!=0) {
               p.setBoldStr(pc);
            }
            pc = elem->Attribute(FNDDOC_ITALIC);
            if(pc!=0 && *pc!=0) {
               p.setItalicStr(pc);
            }
            pc = elem->Attribute(FNDDOC_UNDERLINED);
            if(pc!=0 && *pc!=0) {
               p.setUnderlinedStr(pc);
            }
            pc = elem->Attribute(FNDDOC_COLOR);
            if(pc!=0 && *pc!=0) {
               p.setColorStr(pc);
            }
            pc = elem->Attribute(FNDDOC_BGCOLOR);
            if(pc!=0 && *pc!=0) {
               p.setBgColorStr(pc);
            }
            pc = elem->Attribute(FNDDOC_COMMENT);
            if(pc!=0 && *pc!=0) {
               p.setComment(pc);
            }
            if(bAppend) {
               pl.push_back(p);
            } else {
               pl.insert(pl.begin().getPatId(), p);
            }
         } // search string available
         if (bAppend) {
            node = elem->NextSibling(FNDDOC_SEARCH_TEXT);
         } else {
            node = elem->PreviousSibling(FNDDOC_SEARCH_TEXT);
         }
      } // while
   } // mDoc != 0
   return bRes;
}

bool FindConfigDoc::writePatternList(tclPatternList& pl){
   bool bRes = false;
   if(mDoc==0) {
      return bRes;
   }
   mDoc->Clear();
   TiXmlNode* n = mDoc->InsertEndChild(TiXmlDeclaration(TEXT("1.0"), TEXT("UTF-8"), TEXT(""))); // <?xml version="1.0" encoding="UTF-8"?>
   n = mDoc->InsertEndChild(TiXmlElement(FNDDOC_ANALYSE_DOC));
   if(n) {
      TiXmlElement* e = n->ToElement(); // must work because we added e just before
      e->SetAttribute(FNDDOC_XMLNS, FNDDOC_XMLNS_VALUE);
      e->SetAttribute(FNDDOC_XSD_LOCATION, FNDDOC_XSD_LOCATION_VALUE);
      TiXmlNode* n2 = 0;
#ifdef FEATURE_HEADLINE
      n2 = e->InsertEndChild(TiXmlElement(FNDDOC_HEADLINE));
      if(n2) {
         n2->ToElement()->InsertEndChild(TiXmlText(mHeadline.c_str()));
      }
#endif
      tclPattern defP;
      for(unsigned i = 0; i < pl.size();++i) {
         const tclPattern& rp = pl.getPattern(pl.getPatternId(i));
         n2 = e->InsertEndChild(TiXmlElement(FNDDOC_SEARCH_TEXT));
         if(n2) {
            TiXmlElement* e2 = n2->ToElement();
            e2->InsertEndChild(TiXmlText(rp.getSearchText().c_str()));
            // save attributes only if different from default value
            if (rp.getDoSearch() != defP.getDoSearch()) {
               e2->SetAttribute(FNDDOC_DO_SEARCH, rp.getDoSearchStr().c_str());
            }
            if (rp.getSearchType() != defP.getSearchType()) {
               e2->SetAttribute(FNDDOC_SEARCH_TYPE, rp.getSearchTypeStr().c_str());
            }
            if (rp.getSearchType() != defP.getSearchType()) {
               e2->SetAttribute(FNDDOC_SEARCH_TYPE, rp.getSearchTypeStr().c_str());
            }
            if (rp.getIsMatchCase() != defP.getIsMatchCase()) {
               e2->SetAttribute(FNDDOC_MATCHCASE, rp.getMatchCaseStr().c_str());
            }
            if (rp.getIsWholeWord() != defP.getIsWholeWord()) {
               e2->SetAttribute(FNDDOC_WHOLEWORD, rp.getWholeWordStr().c_str());
            }
            if (rp.getSelectionType() != defP.getSelectionType()) {
               e2->SetAttribute(FNDDOC_SELECT, rp.getSelectionTypeStr().c_str());
            }
            if (rp.getIsHideText() != defP.getIsHideText()) {
               e2->SetAttribute(FNDDOC_HIDE, rp.getHideTextStr().c_str());
            }
            if (rp.getIsBold() != defP.getIsBold()) {
               e2->SetAttribute(FNDDOC_BOLD, rp.getBoldStr().c_str());
            }
            if (rp.getIsItalic() != defP.getIsItalic()) {
               e2->SetAttribute(FNDDOC_ITALIC, rp.getItalicStr().c_str());
            }
            if (rp.getIsUnderlined() != defP.getIsUnderlined()) {
               e2->SetAttribute(FNDDOC_UNDERLINED, rp.getUnderlinedStr().c_str());
            }
            if (rp.getColor() != defP.getColor()) {
               e2->SetAttribute(FNDDOC_COLOR, rp.getColorStr().c_str());
            }
            if (rp.getBgColor() != defP.getBgColor()) {
               e2->SetAttribute(FNDDOC_BGCOLOR, rp.getBgColorStr().c_str());
            }
            if (!rp.getComment().empty()) {
               e2->SetAttribute(FNDDOC_COMMENT, rp.getComment().c_str());
            }
         }
      } // for
      bRes = mDoc->SaveFile();
   } else {
      bRes = false;
   }
   return bRes; 
   }
