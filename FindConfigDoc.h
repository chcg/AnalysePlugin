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
#pragma once

#include "tclPatternList.h"
#include "tclResultList.h"
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
   bool getError(generic_string& msg) const;

   /**
    * read the list from the file and return true if all ok. 
    */
   bool readPatternList(tclPatternList& pl, bool bAppend=true, bool bLoadNew=true);
   bool writePatternList(tclPatternList& pl);
   bool writePatternHitsList(tclResultList& rl);

protected:
   TiXmlDocument* mDoc;
private:

   /** @link dependency */
   /*# tclPatternList lnktclPatternList; */
};
