// $Id: scanner.cpp,v 1.2 2021-09-26 10:35:44-07 - - $

#include <cassert>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
using namespace std;

#include "scanner.h"
#include "debug.h"

char scanner::get() {
   if (not good()) throw runtime_error ("scanner::get() past EOF"); 
   int currchar {nextchar};
   nextchar = instream.get();
   return char (currchar);
}

token scanner::scan() {
   while (good() and isspace (nextchar)) get();
   if (not good()) return {tsymbol::SCANEOF};
   if (nextchar == '_' or isdigit (nextchar)) {
      token result {tsymbol::NUMBER, {get()}};
      while (good() and isdigit (nextchar)) result.lexinfo += get();
      return result;
   }
   return {tsymbol::OPERATOR, {get()}};
}

ostream& operator<< (ostream& out, tsymbol symbol) {
   const char* sym_name {""};
   switch (symbol) {
      case tsymbol::NUMBER  : sym_name = "NUMBER"  ; break;
      case tsymbol::OPERATOR: sym_name = "OPERATOR"; break;
      case tsymbol::SCANEOF : sym_name = "SCANEOF" ; break;
      default               : assert (false)       ; break;
   }
   return out << sym_name;
}

ostream& operator<< (ostream& out, const token& token) {
   out << "{" << token.symbol << ", \"" << token.lexinfo << "\"}";
   return out;
}

