// $Id: cxid.cpp,v 1.10 2021-11-16 16:11:40-08 - - $

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <sstream>
#include <stdio.h>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "logstream.h"
#include "protocol.h"
#include "socket.h"

logstream outlog (cout);
struct cxi_exit: public exception {};

void reply_ls (accepted_socket& client_sock, cxi_header& header) {
   static const char ls_cmd[] = "ls -l 2>&1";
   FILE* ls_pipe = popen (ls_cmd, "r");
   if (ls_pipe == nullptr) { 
      outlog << ls_cmd << ": " << strerror (errno) << endl;
      header.command = cxi_command::NAK;
      header.nbytes = htonl (errno);
      send_packet (client_sock, &header, sizeof header);
      return;
   }
   string ls_output;
   char buffer[0x1000];
   for (;;) {
      char* rc = fgets (buffer, sizeof buffer, ls_pipe);
      if (rc == nullptr) break;
      ls_output.append (buffer);
   }
   pclose (ls_pipe);
   header.command = cxi_command::LSOUT;
   header.nbytes = htonl (ls_output.size());
   memset (header.filename, 0, FILENAME_SIZE);
   DEBUGF ('h', "sending header " << header);
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, ls_output.c_str(), ls_output.size());
   DEBUGF ('h', "sent " << ls_output.size() << " bytes");
}

void reply_get(accepted_socket& client_sock,cxi_header& header) {
//create ifstream to read file
   ifstream is(header.filename,ifstream::binary);
   //read ifstream into buffer to send 
   if(is) {
      //change command in header
      header.command = cxi_command::ACK;

      is.seekg(0,is.end);
      int len = is.tellg();
      is.seekg(0,is.beg);

      //read file into buffer
      char payload[len];
      is.read(payload,len);

      //add file length to nbytes field
      header.nbytes = len;

     //send payload to client
     send_packet(client_sock,&header,sizeof header);
     send_packet(client_sock,payload,len);
   } else {
      //transfer failed, change command to nak
      header.command = cxi_command::NAK;
      send_packet(client_sock,&header,sizeof header);
   }
   //close filestream, send header to client
   is.close();
}

void reply_put(accepted_socket& client_sock, cxi_header& header) {
   
   // get file contents
   char payload[header.nbytes + 1];
   recv_packet(client_sock, payload, header.nbytes);
   payload[header.nbytes] = '\0';

   // write file to local machine
   ofstream os (header.filename, std::ofstream::binary);
   if (os) {
      os.write(payload, header.nbytes);
      header.command = cxi_command::ACK;
   } else {
      header.command = cxi_command::NAK;
   }

   // send headers back to client
   send_packet(client_sock, &header, sizeof header);
   //delete[] payload;
}

void reply_rm(accepted_socket& client_sock, cxi_header& header) {
   //header with filename has been recieved, delete it
   int success = remove(header.filename);
   if(success != 0 ) {
   //error removing file(NAK)
   outlog<<"Error removing file "<<endl;
   header.command = cxi_command::NAK;

   } else {
   //remove(header.filename);
   outlog<<"File deleted successfully "<<endl;
   header.command = cxi_command::ACK;   
   }
   //send header back to client
   outlog<<"Sending header "<<header<<endl;
   send_packet(client_sock,&header,sizeof header);
}

void run_server (accepted_socket& client_sock) {
   outlog.execname (outlog.execname() + "*");
   outlog << "connected to " << to_string (client_sock) << endl;
   try {   
      for (;;) {
         cxi_header header; 
         recv_packet (client_sock, &header, sizeof header);
         DEBUGF ('h', "received header " << header);
         switch (header.command) {
            case cxi_command::LS: 
               reply_ls (client_sock, header);
               break;
            case cxi_command::GET:
               reply_get(client_sock,header);
               break;
            case cxi_command::PUT:
               reply_put(client_sock,header);
               break;
            case cxi_command::RM:
               reply_rm(client_sock,header);
               break;
            default:
               outlog << "invalid client header:" << header << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cxi_exit& error) {
      DEBUGF ('x', "caught cxi_exit");
   }
   outlog<<"finishing"<<endl;
   throw cxi_exit();
}

void fork_cxiserver (server_socket& server, accepted_socket& accept) {
   pid_t pid = fork();
   if (pid == 0) { // child
      server.close();
      run_server (accept);
      throw cxi_exit();
   }else {
      accept.close();
      if (pid < 0) {
         outlog << "fork failed: " << strerror (errno) << endl;
      }else {
         outlog << "forked cxiserver pid " << pid << endl;
      }
   }
}


void reap_zombies() {
   for (;;) {
      int status;
      pid_t child = waitpid (-1, &status, WNOHANG);
      if (child <= 0) break;
      if (status != 0) {
         outlog << "child " << child
                << " exit " << (status >> 8 & 0xFF)
                << " signal " << (status & 0x7F)
                << " core " << (status >> 7 & 1) << endl;
      }
   }
}

void signal_handler (int signal) {
   DEBUGF ('s', "signal_handler: caught " << strsignal (signal));
   reap_zombies();
}

void signal_action (int signal, void (*handler) (int)) {
   struct sigaction action;
   action.sa_handler = handler;
   sigfillset (&action.sa_mask);
   action.sa_flags = 0;
   int rc = sigaction (signal, &action, nullptr);
   if (rc < 0) outlog << "sigaction " << strsignal (signal)
                      << " failed: " << strerror (errno) << endl;
}



void usage() {
   cerr << "Usage: " << outlog.execname() << " port" << endl;
   throw cxi_exit();
}

in_port_t scan_options (int argc, char** argv) {
   for (;;) {
      int opt = getopt (argc, argv, "@:");
      if (opt == EOF) break;
      switch (opt) {
         case '@': debugflags::setflags (optarg);
                   break;
      }
   }
   if (argc - optind != 1) usage();
   return get_cxi_server_port (argv[optind]);
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   signal_action (SIGCHLD, signal_handler);
   try {
      in_port_t port = scan_options (argc, argv);
      server_socket listener (port);
      for (;;) {
         outlog << to_string (hostinfo()) << " accepting port "
             << to_string (port) << endl;
         accepted_socket client_sock;
         for (;;) {
            try {
               listener.accept (client_sock);
               break;
            }catch (socket_sys_error& error) {
               switch (error.sys_errno) {
                  case EINTR:
                     outlog << "listener.accept caught "
                         << strerror (EINTR) << endl;
                     break;
                  default:
                     throw;
               }
            }
         }
         outlog << "accepted " << to_string (client_sock) << endl;
         try {
            fork_cxiserver (listener, client_sock);
            reap_zombies();
         }catch (socket_error& error) {
            outlog << error.what() << endl;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cxi_exit& error) {
      DEBUGF ('x', "caught cxi_exit");
   }
   return 0;
}

