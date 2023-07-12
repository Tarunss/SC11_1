// $Id: main.cpp,v 1.13 2021-02-01 18:58:18-08 - - $
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <regex>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string, string>;
using str_str_pair = str_str_map::value_type;
//declare space of operations
str_str_map space;

void runProgram(const string &infilename, istream &infile)
{
   regex comment_regex{R"(^\s*(#.*)?$)"};
   regex key_value_regex{R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex{R"(^\s*([^=]+?)\s*$)"};
   //regex whitespace {R"\s*"};
   for (int tracker = 1; ; ++tracker)
   {
      string line;
      smatch result;
      getline(infile, line);

      //if at end of file
      if (infile.eof())
         break;
      cout << infilename <<": " << tracker << ": " << line << endl;
      //if a comment
      if (regex_search(line, result, comment_regex))
      {
         continue;
      }
      //checking options
      //if line input is in format: key = value
      else if (regex_search(line, result, key_value_regex))
      {
         //check to see if group 1 is empty
         if (result.str(1).empty() || isWhitespace(result.str(1)))
         {
            //check to see if group 2 is empty
            if (result.str(2).empty() || isWhitespace(result.str(2)))
            {
               //code to implement "=" operation
               //print out all elements in space
               if (space.begin() == space.end())
               {
                  cout << endl;
               }
               else
               {
                  str_str_map::iterator rbeg = space.end();
                  --rbeg; //due to anchor
                  //realized I needed to iterate backwards due to sort
                  while (rbeg != space.begin())
                  {
                     cout << (*rbeg).first << " = " << (*rbeg).second << endl;
                     --rbeg;
                  }
                  cout << (*rbeg).first << " = " << (*rbeg).second << endl;
               }
            }
            else
            {
               //code to implement "=value" operation
               //print out all elements in space that have same value
               str_str_map::iterator rbeg = space.end();
               --rbeg; //due to anchor
               //iterate backwards for lexographic printing
               while (rbeg != space.begin())
               {
                  if ((*rbeg).second == result.str(2))
                  {
                     cout << (*rbeg).first << " = " << (*rbeg).second << endl;
                  }
                  --rbeg;
               }
               if ((*rbeg).second == result.str(2))
               {
                  cout << (*rbeg).first << " = " << (*rbeg).second << endl;
               }
            }
         }
         else
         {
            if (result.str(2).empty() || isWhitespace(result.str(2)))
            {
               //code to implement "key =" operation
               str_str_map::iterator element = space.find(result.str(1));
               if (element == space.end())
                  cout << result.str(1) << ": key not found" << endl;
               else
                  space.erase(element);
            }
            else
            {
               //code to implement "key = value" operation
               str_str_pair value(result.str(1), result.str(2));
               space.insert(value);
               cout << result.str(1) << " = " << result.str(2) << endl;
            }
         }
      }
      //if line format is in format: key
      else if (regex_search(line, result, trimmed_regex))
      {
         str_str_map::iterator element = space.find(result.str(1));
         if (element == space.end())
            cout << result.str(1) << " : key not found" << endl;
         else
            cout << (*element).first << " = " << (*element).second << endl;
      }
   }
}
void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

int main(int argc, char **argv)
{   
   sys_info::execname(argv[0]);
   //scan_options(argc, argv);

   bool runProg = false;
   //reads in string (word by word) that is ran with program
   for(char** argp = &argv[optind]; argp != &argv[argc]; ++argp)
   {
      runProg = true;
      string filename = *argp;
      //case where "-" is entered as a filename
      if(filename.compare("-") == 0)
      {
         runProgram("-",cin);
         continue;
      }
      //implementing filereading into program
      ifstream fileIn(filename);
      if(fileIn.fail())
      {
         syscall_error(filename);
         continue;
      }
      runProgram(filename, fileIn);
      fileIn.close();
   }
   //for inf loop
   if(!runProg) runProgram("-",cin);
}