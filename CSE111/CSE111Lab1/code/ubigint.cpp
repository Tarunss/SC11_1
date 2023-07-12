// $Id: ubigint.cpp,v 1.12 2020-10-19 13:14:59-07 - - $
using namespace std;
#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
#include "debug.h"
#include "relops.h"
#include "ubigint.h"
ubigint::ubigint (unsigned long that): ubig_value (that) {
   while(that > 0){
      int lastdig = that%10;
      ubig_value.push_back(lastdig);
      that = that/10;
   }
  
   //DEBUGF ('~', this << " -> " << uvalue)
}

ubigint::ubigint (const string& that): ubig_value(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (int i = that.length() - 1; i >= 0; i--) {
      char digit = that[i];
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      //uvalue = uvalue * 10 + digit - '0';
      ubig_value.push_back(static_cast<uint8_t>(digit));
      //cout<<"pushing back " <<static_cast<uint8_t>(digit) <<endl;
   }
    DEBUGF ('~', "that = \"" << that << "\"");
 
}

ubigint ubigint::operator+ (const ubigint& that) const {
   // DEBUGF ('u', *this << "+" << that);
   // ubigint result (uvalue + that.uvalue);
   // DEBUGF ('u', result);
   // return result;
   ubigint result;//putting results of addition into here
   //need to see which one is larger
   int shortSize;
   int longSize;
   int carry = 0;
   if(ubig_value.size() > that.ubig_value.size())
   {
      shortSize = that.ubig_value.size();
      longSize = ubig_value.size();
   }
   //else sizes are switched
   else
   {
      shortSize = ubig_value.size();
      longSize = that.ubig_value.size();
   }
   //now we can iterate through shorter vector
    for(int i =0; i<shortSize; i++)
    {
       int temp = 0;
       //pairing each element together in prep for addition
       //need to cast to int for operations 
       //(-0, +0 is using ASCII to convert)
       int dig1 = ubig_value[i] - '0';
       int dig2 = that.ubig_value[i] - '0';
       //if there is a carry, we check here and add it onto a digit
       temp = temp + dig1 + dig2 + carry;
       int remain = (temp%10);//stores remainder
       result.ubig_value.push_back(remain + '0');
       //+'0' is to convert to unsigned char again
       carry = temp/10;
    }
    //Now that we have added all digits from short vec, 
    //we add long vec digits at end of result
    for(int j =shortSize; j <longSize; j++)
    {
       int digRes;
       int rem;
       //in the case this is larger
       if(ubig_value.size() > that.ubig_value.size())
       {
          digRes = carry+ubig_value[j]- '0';
          carry = digRes/10;//store carry
          rem = digRes%10;//store remainder
          result.ubig_value.push_back(rem + '0');
       }
       //in the case that is larger
       else
       {
          digRes = carry+that.ubig_value[j] - '0';
          carry = digRes/10;//store carry
          rem = digRes%10;//store remainder
          result.ubig_value.push_back(rem + '0');
       }    
    }
    //need to add carry if there is still one left
    if(carry != 0)
    {
       result.ubig_value.push_back(carry + '0');
    }
    //trim result vector
    while(result.ubig_value.size() > 0 && result.ubig_value.back() == 0)
    {
       //cout<<"trimmed" <<endl;
       result.ubig_value.pop_back();
    }
   //  for(int i = result.ubig_value.size(); i >= 0; i--)
   //  {
   //     cout<<result.ubig_value[i];
   //  }
   return result;
   
}

ubigint ubigint::operator- (const ubigint& that) const {
   ubigint result;
   int shortSize;
   int longSize;
   int borrow = 0;
   if(ubig_value.size() > that.ubig_value.size())
   {
      shortSize = that.ubig_value.size();
      longSize = ubig_value.size();
   }
   //else sizes are switched
   else
   {
      shortSize = ubig_value.size();
      longSize = that.ubig_value.size();
   }
   for(int i = 0; i < shortSize; i++)
   {
      int temp = 0;
      //here we check to see if left < right
      int left = ubig_value[i] - '0';
      int right = that.ubig_value[i] - '0';
      if (left < right)
      {
       //add ten from borrowed digit
       temp = 10 + left - right - borrow;
       borrow = 1;
       result.ubig_value.push_back(temp +'0');
      }      
      //else we subtract as usual, setting borrow to 0
      else
      {
         temp = left-right-borrow;
         borrow = 0;
         result.ubig_value.push_back(temp + '0');
      }
   
   }
   //now that we iterated through the shortest vector, we simply
   //iterate through longer vector and subtract 0
    for(int j = shortSize; j < longSize; j++)
      {
         int temp = 0;
         int left = ubig_value[j] - '0';
         //in the case borrow is greater than left
         if(left < borrow)
         {
            temp = 10+left-borrow;
            result.ubig_value.push_back(temp + '0');
            borrow = 1;
         }
         else
         {
            temp = left - borrow;
            result.ubig_value.push_back(temp + '0');
            borrow = 0;
         }
      }
      //trimming vector
      while(result.ubig_value.size() > 0 && 
      result.ubig_value.back() == '0')
      {
       //cout<<"trimming"<<endl;
       result.ubig_value.pop_back();
      }
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result;
   //need to create vector with size of this+that
   int size1 = that.ubig_value.size();
   int size2 = ubig_value.size();
   int carry = 0;
   int digit = 0;
   //pushing back enough zeros to create said vector
   for(int i =0; i < size1+size2; i++)
   {
      result.ubig_value.push_back('0');
   }
   //use algorithm given
   for(int j = 0; j < size1; j++)
   {
      int u = that.ubig_value[j] - '0'; //setting variable u
      carry = 0;
      for(int k = 0; k < size2; k++)
      {
         int v = ubig_value[k] - '0'; //setting variable v
         digit = (result.ubig_value[j+k] - '0') + (u * v) + carry;
         result.ubig_value[j+k] = (digit %10) + '0';
         carry = digit/10;
      }
      result.ubig_value[j + size2] = carry + '0';
   }
//remove leading zeros
while (result.ubig_value.size() > 0 and 
      result.ubig_value.back() == '0') 
      result.ubig_value.pop_back();

return result;
}

void ubigint::multiply_by_2() {
   //to multiply, we iterate through and double each dig
   int carry = 0;
   int digit = 0;
   int length = ubig_value.size();
   for(int i = 0; i < length; i++)
   {
      int temp = ubig_value[i] - '0'; 
      digit = (temp*2) + carry;
      //set carry to 0 after adding
      carry = 0;
      //put back doubled digit
      int  newdig = (digit%10);
      ubig_value[i] = newdig + '0'; 
      carry = digit/10;
   }
   //need to add back carry after loop
   ubig_value.push_back(carry + '0');
   //remove leading zeros
   while (ubig_value.size() > 0 && ubig_value.back() == '0') 
   {
      ubig_value.pop_back();
   }
}
void ubigint::divide_by_2() {
   //uvalue /= 2;
   int length = ubig_value.size();
   for(int i = 0; i < length; i++)
   {
      //divide by 2, if next higher digit is odd, add 5 to current
      //digit
      int num1 = (ubig_value[i] - '0')/2;
      //need to check and see if vector ends at i+1
      if(i+1 < length)
      {
         int next = ubig_value[i+1] - '0';
         //if next is odd, we add 5 to current digit.
         if(next%2 != 0)
         {
            num1 = num1+5;
         }
      } 
      ubig_value[i] = num1 + '0';
   }
   //remove leading zeros
   while (ubig_value.size() > 0 && ubig_value.back() == '0') 
   {
      ubig_value.pop_back();
   }
}

struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   DEBUGF ('/', "quotient = " << quotient);
   DEBUGF ('/', "remainder = " << remainder);
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   int size1 = ubig_value.size();
   int size2 = that.ubig_value.size();
   //need to check if lengths are the same
   if(size1 != size2)
      return false;
   else//need to check if elements are the same
   {
      for(int i =0; i < size1; i++)
      {
         if(ubig_value[i] != that.ubig_value[i])
         {
            return false;
         }
      }
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   int size1 = ubig_value.size();
   int size2 = that.ubig_value.size();
   //same logic with ==,instead we compare using <
   if(size1 < size2)
      return true;
   else if(size1 > size2)
      return false;
   //if none of these are true, then lengths are equal
   else
   {
      //iterating through and comparing elements
      //since they are stored backwards, we go from end of vector
      for(int i = 0; i < size1; i++)
      {
         if(ubig_value[i] < that.ubig_value[i])
            return true;
         else if (ubig_value[i] > that.ubig_value[i])
            return false;          
      }
   }
   return false;
}


ostream& operator<< (ostream& out, const ubigint& that) { 
   //iterate through and print out the vector as a number 
   //using reverse constant iterator
   int i = 0;
   auto beg = that.ubig_value.crbegin();
   auto end = that.ubig_value.crend();
   while(beg != end)
   {
      //checks to see max character limit
      if(i == 69)
      {
         out <<"\\"<<endl;
         i = 0;
      }
      out <<(*beg);
      ++beg;
      ++i;
   }
   return out;
}

