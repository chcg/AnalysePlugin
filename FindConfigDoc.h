/* -------------------------------------
This file is part of AnalysePlugin for NotePad++ 
Copyright (C)2011-2017 Matthias H. mattesh(at)gmx.net
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
#pragma once

#include "tclPatternList.h"
#include "chardefines.h"


class TiXmlDocument;
class TiXmlDocumentA;

/**
* FindConfigDoc encapsulates the physical format of the search pattern configuration
* file.  It provides read and write to the document and takes the patternlist
* as input or output
*/
class FindConfigDoc
{
public:
   
   /**
    * constructor gets the name of the xml_file to be read. 
    */
   FindConfigDoc(const TCHAR * filename);
   virtual ~FindConfigDoc(void);

   /**
    * read the list from the file and return true if all ok. 
    */
   bool readPatternList(tclPatternList& pl, bool bAppend=true, bool bLoadNew=true);
   bool writePatternList(tclPatternList& pl);

protected:
   TiXmlDocument* mDoc;
private:

   /** @link dependency */
   /*# tclPatternList lnktclPatternList; */
};
