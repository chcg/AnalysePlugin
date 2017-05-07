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
/**
tclResultList contains a list of results but also 
the corresponding patterns used.
the syncrosiastion between both lists is the index in the vectors
*/

#ifndef TCLRESULTLIST_H
#define TCLRESULTLIST_H
#include "tclPatternList.h"
#include "tclResult.h"
#include "tclPattern.h"

/**
 * Resultlist enhances the patternlist by the results for all different patterns
 * It also provides an iterator to move over all results in order of the original text. 
 */
class tclResultList : public tclPatternList {
public:
   class const_iterator;
   class iterator;

   typedef std::map < tPatId, tclResult > tlmResult;

   /**
   * copy old in new and clear old 
   */
   void moveResult(tPatId oldPattId, tPatId newPattId);

   /**
    * removes all entries from the list. 
    */
   virtual void clear();

   /**
   * set the values in the pattern with index i
   * overloaded to invalidate the results if required
   */
   virtual bool setPattern(tPatId i, const tclPattern& pattern);

   /**
   * result list is dirty when at least one result is dirty
   */
   bool getIsDirty() const ;

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
    * remove the entry with given index 
    */
   virtual void remove(tPatId i);
   
   tclResult& refResult(tPatId i){
      return mlmResult[i];
   }

   const_iterator begin() const {
      return mlmResult.begin();
   }
   const_iterator end() const {
      return mlmResult.end();
   }
   iterator begin() {
      return mlmResult.begin();
   }
   iterator end() {
      return mlmResult.end();
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
      const_iterator(const tlmResult::const_iterator& right){
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
      const tclResult& getResult() const {
         return mIt->second;
      }
      tPatId getPatId() const {
         return mIt->first;
      }
   protected:
      tlmResult::const_iterator mIt;
   };  // class const_iterator

   class iterator : public const_iterator {
   public:
      iterator(){}
      iterator(const iterator& right){
         mIt = right.mIt;
      }
      iterator(const tlmResult::iterator& right){
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
      tclResult& refResult() {
         return const_cast<tclResult&>(mIt->second);
      }
   }; // class iterator


protected:
   /**
   * @link aggregation
   * @supplierCardinality 0..*
   */
   /*# tclResult lnktclResult; */
   tlmResult mlmResult;
};
#endif //TCLRESULTLIST_H
