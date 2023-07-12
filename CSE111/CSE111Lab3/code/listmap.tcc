// $Id: listmap.tcc,v 1.15 2019-10-30 12:44:53-07 - - $

#include "listmap.h"
#include "debug.h"
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t,mapped_t,less_t>::~listmap() {
   while (not empty())
   {
      erase(begin());
   } 
   DEBUGF ('l', reinterpret_cast<const void*> (this));
}

//
// iterator listmap::insert (const value_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::insert (const value_type& pair) {
   less_t lesser;
   listmap<key_t,mapped_t,less_t>::iterator itor = begin();
   //while itorator !=end and is less than pair
   while(itor!=end() && lesser(pair.first,itor->first))
   {
       ++itor;
   }
   if(itor!=end() && not lesser(itor->first,pair.first)){
      itor->second = pair.second;
      return itor;
   }
   node* tmp = new node(itor.where, itor.where->prev, pair);
   tmp->next->prev = tmp;
   tmp->prev->next = tmp;
   DEBUGF ('l', &pair << "->" << pair);
   return iterator(tmp);
}

//
// listmap::find(const key_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::find (const key_type& that) {
   listmap<key_t,mapped_t,less_t>::iterator itor = begin();
   while(itor!=end() && itor->first != that)
   {
      ++itor;
   } 
   DEBUGF ('l', that);
   return itor;
}

//
// iterator listmap::erase (iterator position)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::erase (iterator position) {
   listmap<key_t,mapped_t,less_t>::iterator itor = position;
   ++itor;
  //erases position from circular linked list
   if (position.where != nullptr){
      position.where->prev->next = position.where->next;
      position.where->next->prev = position.where->prev;
      delete position.where;
  }   
   DEBUGF ('l', &*position);
   return itor;
}

