// $Id: commands.cpp,v 1.21 2021-09-26 12:41:17-07 - - $

#include "commands.h"
#include "debug.h"
#include <sstream>
#include <locale>
#include <iomanip>

command_hash cmd_hash{
    {"cat", fn_cat},
    {"cd", fn_cd},
    {"echo", fn_echo},
    {"exit", fn_exit},
    {"ls", fn_ls},
    {"lsr", fn_lsr},
    {"make", fn_make},
    {"mkdir", fn_mkdir},
    {"prompt", fn_prompt},
    {"pwd", fn_pwd},
    {"rm", fn_rm},
    {"rmr", fn_rmr},
    {"#", fn_hash},
    
};

command_fn find_command_fn(const string &cmd)
{
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF('c', "[" << cmd << "]");
   const auto result{cmd_hash.find(cmd)};
   if (result == cmd_hash.end())
   {
      throw command_error(cmd + ": no such command");
   }
   return result->second;
}

command_error::command_error(const string &what) : runtime_error(what)
{
}

int exit_status_message()
{
   int status{exec::status()};
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   if (words.size() == 1)
   {
      cout << "cat: requires a filename" << endl;
      return;
   }

   map<string, inode_ptr> entries =
       state.getCwd()->getContents()->getDirents();
   //cout<<"past entries" <<endl;
   //need to copy contents of file to stdout
   //be careful if a directory is referenced
   for (unsigned int i = 1; i < words.size(); i++)
   {
      //if can't find elements in dirents
      if (entries.find(words[i]) == entries.end())
      {
         cout << "cat : " << words[i];
         cout << " : file or directory not found" << endl;
         return;
      }
      //case for where a directory is referenced
      if (entries.find(words[i])->
      second->getContents()->is_file() == false)
      {
         cout << "cat : " << words[i];
         cout << ": reference is a directory" << endl;
      }
   }
   for (unsigned int j = 1; j < words.size(); j++)
   {
      auto entry = entries.find(words[j]);
      //print out contents of file to cout using readfile
      cout << entry->second->getContents()->readfile() << endl;
   }
}
//CD sets the current directory to pathname given
//needs to use root if blank
void fn_cd(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   //first account for no args
   if(words.size() == 1 || words[1] == "/")
   {
      state.setThisDir("/");
      state.setCwd(state.getRoot());
      return;
   }
   //need to see what is referenced (dir, file)?
   map<string,inode_ptr> entries 
      = state.getCwd()->getContents()->getDirents();
   for(unsigned int i = 1; i < words.size(); i++)
   {
      //if directory doesn't exist
      if(entries.find(words[i]) == entries.end())
      {
         cout<<"cd: " <<words[i] << ":No such directory"
         <<endl; 
         return;
      }
      //if file is referenced
      if(entries.find(words[i])->second->getContents()->is_file())
      {
         cout<<"cd: " <<words[i] << ":Is a file"
         <<endl;
         return;
      }
   }
   //change state name
   state.setThisDir(state.getThisDir() + words[1] + "/");
   //change state
   state.setCwd(entries.find(words[1])->second);
}

void fn_echo(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   cout << word_range(words.cbegin() + 1, words.cend()) << endl;
}

void fn_exit(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   int x;
   if(words.size() == 1)
   {
      setStatusAndExit(0);
   }
   
   istringstream convert(words[1]);
   //exit status has no digits
   if(!(convert >> x))
   {
      setStatusAndExit(0);
   }
   //exit status has digits
   setStatusAndExit(x);
}
//needs to print out directories and files to 
//STDout. throws errors if not possible
void fn_ls(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   //we will use entries to simulate all files
   //in the current dir
   map<string,inode_ptr> entries;
   //first check for no args
   if(words.size() == 1 || words[1] == "/")
   {
      //we print out current directory
      //cout<<"printing it out over here"<<endl;
      cout<<state.getThisDir() << ":" <<endl;
      //entries = state.getCwd()->getContents()->printDirents();
      state.getCwd()->getContents()->printDirents();
   }
   else
   {
      //.. means root inode, we go from there
      if(words[1] == "..")
      {
         
         entries = state.getCwd()->getContents()->getDirents();
         //need directory that we will print
         inode_ptr retDir = state.getCwd();
         string retName = state.getThisDir();
         //need to check and see if the root node exists
         state.setCwd(entries.find("..")->second);
         wordvec newName = split(state.getThisDir(), "/");
         //use new name
         string newCDName = "/";
         //create a new wordvec w slashes in between old dirname
         for(unsigned int i = 0; i < newName.size()-1; i++)
         {
            newCDName = newCDName + newName[i] + "/";
            cout<<newCDName<<endl;
         }
         state.setThisDir(newCDName);
         //now we print out new directory
         cout<<state.getThisDir()<<":"<<endl; 
         //set entries to directory to be printed
         state.getCwd()->getContents()->printDirents();

         state.setCwd(retDir);
         state.setThisDir(retName);
      }
   }
}

void fn_lsr(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   //first we check words
   if(words.size() == 1 || words[1] == "/" || words[1] == ".")
   {
      fn_ls(state,words);
   }
   else
   {
      //check the subdir
      //get directory first
      map<string,inode_ptr> entries = 
      state.getCwd()->getContents()->getDirents();
      //iterate through, check for subdirectories
      auto beg1 = entries.begin();
      auto end1 = entries.end();
      while(beg1 != end1)
      {
         if(beg1->first == "." || beg1->first == "..")
            continue;
         if(!(beg1->second->getContents()->is_file()))
         {
            fn_ls(state,words);
         }
      }
   }
}

void fn_make(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   if (words.size() == 1)
   {
      cout << "Please add name for new file" << endl;
      return;
   }

   map<string, inode_ptr> entries =
       state.getCwd()->getContents()->getDirents();
   cout << "past entries" << endl;
   //need to check whether or not entry exists already
   if (entries.find(words[1]) != entries.end())
   {
      cout << "mkdir: " << words[1];
      cout << ": entry already exists" << endl;
      return;
   }
   string entryName = words[1];
   //need to make string to hold file contents
   string fileWords;
   for (unsigned int i = 2; i < words.size(); i++)
   {
      fileWords += words[i] + " ";
   }
   //we split string into vector of words
   wordvec fileContents = split(fileWords, " ");
   //call file_sys func
   inode_ptr newFile = state.getCwd()->getContents()->mkfile(entryName);
   //add word vector to contents of file
   newFile->getContents()->writefile(fileContents);
}

void fn_mkdir(inode_state &state, const wordvec &words)
{
   //need to check for arguments
   if (words.size() == 1)
   {
      cout << "Please add name for new directory" << endl;
   }
   cout << "making entries" << endl;
   //since we defined root, we can getCwd right away
   map<string, inode_ptr> entries =
       state.getCwd()->getContents()->getDirents();
   //need to check whether or not entry exists already
   if (entries.find(words[1]) != entries.end())
   {
      cout << "mkdir: " << words[1];
      cout << ": entry already exists" << endl;
      return;
   }
   //set new directory
   string entryName = words[1];
   //call file_sys func
   inode_ptr newDir = state.getCwd()->getContents()->mkdir(entryName);
   //adding directory and root to new directory
   newDir->getContents()->addEntry(".",newDir);
   newDir->getContents()->addEntry("..",state.getCwd());
   cout << "mkdir done" << endl;
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_prompt(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   if(words.size() == 1)
   {
      cout<<"prompt: needs new prompt";
      return;
   }
   string prompt;
   for(unsigned int i= 0; i < words.size()-1; i++)
   {
      prompt = prompt + words[i] + " ";
   }
   state.setPrompt(prompt);
}

void fn_pwd(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   cout<<state.getThisDir() <<endl;
}

void fn_rm(inode_state &state, const wordvec &words)
{
   //search through and find entry to delete
   
   if(words.size() == 1)
   {
      cout<<"rm: need directory or file";
      return;
   }
   //make a directory to search through
   map<string,inode_ptr> entries;
   // if(!(entries.find(words[1])->second->getContents()->is_file()))
   // {

   // }
   entries = state.getCwd()->getContents()->getDirents();
   if(entries.find(words[1]) != entries.end())
   {
      cout<<"rm: File or directory does not exist " <<endl;
      return;
   }
   //additional check to see whether or not it is a directory

   //else, erase the entry
   state.getCwd()->getContents()->getDirents().erase(words[1]);
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_rmr(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   if(words.size() == 1)
   {
      cout << "rmr: need a file/directory name " << endl;
      return;
   }
}
void fn_hash(inode_state &state, const wordvec &words)
{
   //function does nothing
}