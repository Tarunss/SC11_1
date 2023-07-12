// $Id: cxi.cpp,v 1.6 2021-11-08 00:01:44-08 - - $

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "logstream.h"
#include "protocol.h"
#include "socket.h"

logstream outlog(cout);
struct cxi_exit : public exception
{
};

unordered_map<string, cxi_command> command_map{
    {"exit", cxi_command::EXIT},
    {"help", cxi_command::HELP},
    {"ls", cxi_command::LS},
    {"get", cxi_command::GET},
    {"put", cxi_command::PUT},
    {"rm", cxi_command::RM},
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cxi_help()
{
   cout << help;
}

void cxi_ls(client_socket &server)
{
   cxi_header header;
   header.command = cxi_command::LS;
   DEBUGF('h', "sending header " << header << endl);
   send_packet(server, &header, sizeof header);
   recv_packet(server, &header, sizeof header);
   DEBUGF('h', "received header " << header << endl);
   if (header.command != cxi_command::LSOUT)
   {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }
   else
   {
      size_t host_nbytes = ntohl(header.nbytes);
      auto buffer = make_unique<char[]>(host_nbytes + 1);
      recv_packet(server, buffer.get(), host_nbytes);
      DEBUGF('h', "received " << host_nbytes << " bytes");
      buffer[host_nbytes] = '\0';
      cout << buffer.get();
   }
}
//get function
void cxi_get(client_socket &server, string filename)
{
   //create header
   cxi_header header;
   header.command = cxi_command::GET;

   //set filename for header
   strcpy(header.filename,filename.c_str());

   //send the filename to the server to be read
   outlog<< "Sending header " <<header << endl;
   send_packet(server,&header, sizeof header);

   //recieve header back (with new command)
   recv_packet(server,&header, sizeof header);
   outlog << "Recieving header " <<header << endl;

   //if process was successfu;
   if(header.command == cxi_command::ACK) {
      char payload[header.nbytes+1];

      //recieve the buffer for file
      recv_packet(server, payload, header.nbytes);
      payload[header.nbytes] = '\0';

      //write the payload to local file system
      ofstream os(header.filename,std::ofstream::binary);
      os.write(payload, header.nbytes);
   } else {
      //copy unsuccessful
      outlog<<"GET: File transfer unsuccessful"<<endl;
   }
}

void cxi_put(client_socket &server, string filename)
{
   //create header
   cxi_header header;
   header.command = cxi_command::PUT;

   //copy filename into header
   strcpy(header.filename,filename.c_str());

   // read file & send to server
   ifstream is (header.filename,ifstream::binary);
   if (is) {
      // get file length
      is.seekg(0, is.end);
      int len = is.tellg();
      is.seekg(0, is.beg);

      // read file into buffer
      char payload[len];
      is.read(payload, len);

      // add file length to nbytes filed
      header.nbytes = len;

      // send file to server
      outlog<< "Sending header " << header <<endl;
      send_packet(server, &header, sizeof header);
      send_packet(server, payload, len);

      //get response from server
      recv_packet(server, &header, sizeof header);
      outlog<< "Recieving header " <<header <<endl;

   } else {
      outlog << "error: could not find file"<< endl;
   }
   
   // close file
   is.close();

   if (header.command == cxi_command::ACK) {
      outlog << "PUT: server put file successfully" << endl;
   } else if (header.command == cxi_command::NAK) {
      outlog << "PUT: failed to put file in server" << endl;
   }
}
  

void cxi_rm(client_socket &server, string filename)
{
   //create header
   cxi_header header;
   header.command = cxi_command::RM;

   //set filename for header
   strcpy(header.filename,filename.c_str());

   //send the filename to the server to be read
   outlog <<"Sending header "<< header<< endl;
   send_packet(server,&header, sizeof header);

   //recieve header with new command
   recv_packet(server, &header, sizeof header);
   outlog <<"Recieving header "<< header<< endl;

   //if successful
   if(header.command == cxi_command::ACK) {
      outlog<<"RM: File deleted successfully "<<endl;
   } else {
      outlog<<"RM: Failed to delete file "<<endl;
   }
}
void usage()
{
   cerr << "Usage: " << outlog.execname() << " host port" << endl;
   throw cxi_exit();
}

pair<string, in_port_t> scan_options(int argc, char **argv)
{
   for (;;)
   {
      int opt = getopt(argc, argv, "@:");
      if (opt == EOF)
         break;
      switch (opt)
      {
      case '@':
         debugflags::setflags(optarg);
         break;
      }
   }
   if (argc - optind != 2)
      usage();
   string host = argv[optind];
   in_port_t port = get_cxi_server_port(argv[optind + 1]);
   return {host, port};
}

int main(int argc, char **argv)
{
   outlog.execname(basename(argv[0]));
   outlog << to_string(hostinfo()) << endl;
   try
   {
      auto host_port = scan_options(argc, argv);
      string host = host_port.first;
      in_port_t port = host_port.second;
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server(host, port);
      outlog << "connected to " << to_string(server) << endl;
      for (;;)
      {
         string line;
         getline(cin, line);
         if (cin.eof())
            throw cxi_exit();
         outlog << "command " << line << endl;
         //get filename set up
         vector<string> contents;
         std::istringstream ss(line);
         std::string token;
         while (getline(ss, token, ' '))
         {
            contents.push_back(token);
         }
         const auto &itor = command_map.find(contents[0]);
         cxi_command cmd = itor == command_map.end()
                               ? cxi_command::ERROR
                               : itor->second;
         switch (cmd)
         {
         case cxi_command::EXIT:
            throw cxi_exit();
            break;
         case cxi_command::HELP:
            cxi_help();
            break;
         case cxi_command::LS:
            cxi_ls(server);
            break;
         case cxi_command::GET:
            cxi_get(server, contents[1]);
            break;
         case cxi_command::PUT:
            cxi_put(server, contents[1]);
            break;
         case cxi_command::RM:
            cxi_rm(server, contents[1]);
            break;
         default:
            outlog << line << ": invalid command" << endl;
            break;
         }
      }
   }
   catch (socket_error &error)
   {
      outlog << error.what() << endl;
   }
   catch (cxi_exit &error)
   {
      DEBUGF('x', "caught cxi_exit");
   }
   outlog << "finishing up" << endl;
   return 0;
}
