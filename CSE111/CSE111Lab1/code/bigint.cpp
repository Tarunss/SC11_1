// $Id: bigint.cpp,v 1.4 2021-09-26 10:35:44-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

 bigint bigint::operator- () const {
    return {uvalue, not is_negative};
 }

 bigint bigint::operator+ (const bigint& that) const {
    //add if same sign
    bigint result;
    if(is_negative == that.is_negative)
    {
       result.uvalue = uvalue + that.uvalue;
       result.is_negative = is_negative;
       return result;
    }
    //subtract if not 
    //3 cases: either this<that, this > that, or this == that
    //case 1
    if(uvalue < that.uvalue)
    {
       //cout<<"subtracting " <<endl;
       result.uvalue = that.uvalue - uvalue;
       result.is_negative = that.is_negative;
    }
    //case 2
    else if(uvalue > that.uvalue)
    {
       //cout<<"subtracting 2" <<endl;
       result.uvalue = uvalue - that.uvalue;
       result.is_negative = is_negative;
    }
    //case 3
    else
    {
       //cout << "subtracting 3" <<endl;
       result.uvalue = uvalue-that.uvalue;
       result.is_negative = false;
    }
    return result;
 }

bigint bigint::operator- (const bigint& that) const {
   //case with same sign
   bigint result;
   if(is_negative == that.is_negative)
   {
      //case if left is bigger than right
      if(uvalue > that.uvalue)
      {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = is_negative;
      }//case if left is smaller than right
      else if(uvalue < that.uvalue)
      {
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = !(that.is_negative);
      }
      else//if not these 2 cases, they are equal
      {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = false;
      }
   }
   else // different signs means that you use operator+
   {
      result.uvalue = uvalue + that.uvalue;
      result.is_negative = is_negative;
   }

   return result;
}


bigint bigint::operator* (const bigint& that) const {
   bigint result;
   //in case we are multiplying by zero
   static const ubigint ZERO (0);
   result.uvalue = uvalue * that.uvalue;
   //same sign multiplied is positive
   if(is_negative == that.is_negative)
      result.is_negative = false;
   //different signs mean negative
   else
      result.is_negative = true;
   
   //multiplied by zero is also positive
   if(result.uvalue == ZERO)
      result.is_negative = false;
   
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result;
   //in case we get zero as an answer
   static const ubigint ZERO (0);
   result.uvalue = uvalue / that.uvalue;
   //same sign divided is positive
   if(is_negative == that.is_negative)
      result.is_negative = false;
   //different signs mean negative
   else
      result.is_negative = true;
   
   //answer = 0 is also positive
   if(result.uvalue == ZERO)
      result.is_negative = false;
   
   return result;  
}

bigint bigint::operator% (const bigint& that) const {
   bigint result;
   result.uvalue = uvalue%that.uvalue;
   result.is_negative = is_negative;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << (that.is_negative ? "-" : "") << that.uvalue;
}

