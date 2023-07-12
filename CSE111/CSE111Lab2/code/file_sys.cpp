// $Id: file_sys.cpp,v 1.10 2021-04-10 14:23:40-07 - - $

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <map>
#include <iomanip>
using namespace std;

#include "debug.h"
#include "file_sys.h"

size_t inode::next_inode_nr{1};

ostream &operator<<(ostream &out, file_type type)
{
   switch (type)
   {
   case file_type::PLAIN_TYPE:
      out << "PLAIN_TYPE";
      break;
   case file_type::DIRECTORY_TYPE:
      out << "DIRECTORY_TYPE";
      break;
   default:
      assert(false);
   };
   return out;
}

inode_state::inode_state()
{
   DEBUGF('i', "root = " << root << ", cwd = " << cwd
                         << ", prompt = \"" << prompt() << "\"");
   //need to make sure the root node is correctly made here
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   cwd = root;
   //have to set both .. and . to itself
   root->contents->getDirents().insert(pair<string, inode_ptr>(".", root));
   thisDir = "/";
   root->contents->getDirents().insert(pair<string, inode_ptr>("..", root));
}

const string &inode_state::prompt() const { return prompt_; }

ostream &operator<<(ostream &out, const inode_state &state)
{
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type) : inode_nr(next_inode_nr++)
{
   switch (type)
   {
   case file_type::PLAIN_TYPE:
      contents = make_shared<plain_file>();
      contents->changeFile(true);
      break;
   case file_type::DIRECTORY_TYPE:
      contents = make_shared<directory>();
      contents->changeFile(false);
      break;
   default:
      assert(false);
   }
   DEBUGF('i', "inode " << inode_nr << ", type = " << type);
}

size_t inode::get_inode_nr() const
{
   DEBUGF('i', "inode = " << inode_nr);
   return inode_nr;
}

map<string, inode_ptr> base_file::getDirents()
{
   throw file_error("is a " + error_file_type());
}

file_error::file_error(const string &what) : runtime_error(what)
{
}

const wordvec &base_file::readfile() const
{
   throw file_error("is a " + error_file_type());
}

void base_file::writefile(const wordvec &)
{
   throw file_error("is a " + error_file_type());
}

void base_file::remove(const string &)
{
   throw file_error("is a " + error_file_type());
}

inode_ptr base_file::mkdir(const string &)
{
   throw file_error("is a " + error_file_type());
}

inode_ptr base_file::mkfile(const string &)
{
   throw file_error("is a " + error_file_type());
}
void base_file::addEntry(const string &filename, inode_ptr pointer)
{
   DEBUGF('i', filename);
   DEBUGF('i', pointer);
   throw file_error("is a " + error_file_type());
}

size_t plain_file::size() const
{
   size_t size{0};
   DEBUGF('i', "size = " << size);
   for (unsigned int i = 0; i < data.size(); i++)
   {
      string temp = data[i];
      size += temp.length();
   }
   return size;
}

const wordvec &plain_file::readfile() const
{
   DEBUGF('i', data);
   return data;
}

void plain_file::writefile(const wordvec &words)
{
   data.clear();
   data = words;
   DEBUGF('i', words);
}
//for compiler
void plain_file::printDirents() {}

size_t directory::size() const
{
   size_t size{0};
   DEBUGF('i', "size = " << size);
   return size;
}

void directory::remove(const string &filename)
{
   DEBUGF('i', filename);
   //helper func for removing from directory easily
   dirents.erase(filename);
}
void directory::addEntry(const string &filename, inode_ptr pointer)
{
   dirents.insert(std::pair<string, inode_ptr>(filename, pointer));
}
void directory::printDirents()
{
   auto beg1 = dirents.begin();
   auto end1 = dirents.end();
   while (beg1 != end1)
   {
      cout << std::setw(6);
      cout << beg1->second->get_inode_nr() << " ";
      cout << std::setw(6);
      cout << beg1->second->getContents()->size();
      cout << " ";
      cout << beg1->first;
      //if entry is a directory we need a /
      if (!(beg1->second->getContents()->is_file()))
      {
         cout << "/";
      }
      cout << endl;
      beg1++;
   }
}

inode_ptr directory::mkdir(const string &dirname)
{
   DEBUGF('i', dirname);
   //takes in directory name, creates a new inode and directory, returns pointer to new inode
   inode_ptr newNode = make_shared<inode>(file_type::DIRECTORY_TYPE);
   //sets directory name
   dirents.insert(pair<string, inode_ptr>(dirname, newNode));
   return newNode;
}
inode_ptr directory::mkfile(const string &filename)
{
   //takes in file name, creates a new inode and file, returns pointer to new inode
   inode_ptr newNode = make_shared<inode>(file_type::PLAIN_TYPE);
   //set file name
   dirents.insert(pair<string, inode_ptr>(filename, newNode));
   DEBUGF('i', filename);
   return newNode;
}
