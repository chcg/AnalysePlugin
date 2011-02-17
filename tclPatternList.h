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
class tclPatternList contains a vector of tclPatterns
*/

#ifndef TCLPATTERNLIST_H
#define TCLPATTERNLIST_H
#include "tclPattern.h"
#include "MyPlugin.h"
#include <map>
#include <set>

typedef std::map < tPatId , tclPattern > tlmPatternList;
typedef std::set < tPatId > tlsPatId;

/**
 * Is the list of Patterns.
 * You can iterate over the patterns and
 * query or set the configuration of each pattern. 
 */
class tclPatternList {
public:
   class const_iterator;
   class iterator;

   /**
    * The direct access to the members works directly on the vector function operator[] 
    */
   virtual const tclPattern& getPattern(tPatId i ) const ;
   
   /** index conversion to id */
   virtual tPatId getPatternId(unsigned index) const;

   /** id conversion to index */
   virtual unsigned getPatternIndex(tPatId id) const;
    
   /**
   * set the values in the pattern with index i
   * @return true if setting was successfull
   */
   virtual bool setPattern(tPatId i, const tclPattern& pattern);

   /**
    * adds the given pattern to the list at the end and returns the actual position (index 0..)
    */
   virtual tPatId push_back(const tclPattern& pattern);

   /**
    * insert the given pattern before the one with given index
    * and return the actual position. 
    */
   virtual tPatId insert(tPatId before, const tclPattern& pattern);
   virtual tPatId insertAfter(tPatId after, const tclPattern& pattern);

   /**
    * returns the count of patterns 
    */
   virtual unsigned size() const;

   /**
    * removes all entries from the list. 
    */
   virtual void clear();

   /**
    * remove the entry with given index 
    */
   virtual void remove(tPatId i);

   /**
   * copy old in new and clear old 
   */
   virtual void moveResult(tPatId oldPattId, tPatId newPattId);

   const_iterator begin() const {
      return mlmPattern.begin();
   }
   const_iterator end() const {
      return mlmPattern.end();
   }
   iterator begin() {
      return mlmPattern.begin();
   }
   iterator end() {
      return mlmPattern.end();
   }
   
   const_iterator find(tPatId i) const {
      return mlmPattern.find(i);
   }

   iterator find(tPatId i) {
      return mlmPattern.find(i);
   }

   /**
   * iterator for runing over the pattern list
   */
   class const_iterator {
   public:
      const_iterator() {}
      const_iterator(const const_iterator& right){
         mIt = right.mIt;
      }
      const_iterator(const tlmPatternList::const_iterator& right){
         // we use non const member but use only const access methods in const_iterator
         mIt = right;
      }
      const_iterator& operator++() {	// preincrement
         ++mIt;
         return (*this);
      }
      const_iterator operator++(int) {	// postincrement
         const_iterator _Tmp = *this;
         ++*this;
         return (_Tmp);
      }
      const_iterator& operator--() {	// predecrement
         --mIt;
         return (*this);
      }
      const_iterator operator--(int) {	// postdecrement
         const_iterator _Tmp = *this;
         --*this;
         return (_Tmp);
      }
      bool operator==(const const_iterator& right) const {	// test for iterator equality
         return (mIt == right.mIt);
      }
      bool operator!=(const const_iterator& right) const {	// test for iterator unequality
         return (mIt != right.mIt);
      }
      const tclPattern& getPattern() const {
         return mIt->second;
      }
      tPatId getPatId() const {
         return mIt->first;
      }
   protected:
      tlmPatternList::const_iterator mIt;
   };  // class const_iterator

   class iterator : public const_iterator {
   public:
      iterator(){}
      iterator(const iterator& right){
         mIt = right.mIt;
      }
      iterator(const tlmPatternList::iterator& right){
         mIt = right;
      }
      iterator& operator++() {	// preincrement
         ++mIt;
         return (*this);
      }
      iterator operator++(int) {	// postincrement
         iterator _Tmp = *this;
         ++*this;
         return (_Tmp);
      }
      iterator& operator--() {	// predecrement
         --mIt;
         return (*this);
      }
      iterator operator--(int) {	// postdecrement
         iterator _Tmp = *this;
         --*this;
         return (_Tmp);
      }
      bool operator==(const iterator& right) const {	// test for iterator equality
         return (mIt == right.mIt);
      }
      bool operator!=(const iterator& right) const {	// test for iterator unequality
         return (mIt != right.mIt);
      }
      tclPattern& refPattern() {
         return const_cast<tclPattern&>(mIt->second);
      }
   }; // class iterator

protected:
   /** @link aggregation 
    * @supplierCardinality 0..*
    */
   tlmPatternList mlmPattern;
   
   /** stores all ids already been used */
   tlsPatId mlsPatIds;

   static tclPattern mDefault;

};
#endif //TCLPATTERNLIST_H
