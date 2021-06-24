#ifndef RDT_RDT_H_
#define RDT_RDT_H_

#include <vector>
#include "../udp/UDP.h"

class RDT_UDP_Server {
  UDP_Server server;
  std::vector<int> sequence;
  int num_clients;

  RDT_UDP_Server(int port) : num_clients(0) {}

  void AcceptClient() {
    sequence.push_back(0);
    ++num_clients;
  }

  void Send(int client_id, std::string msg) {
  }

  void Read(int client_id) {
  }
};

class RDT_UDP_Client {
  RDT_UDP_Client(std::string addr, int port) {
  } 

  void Send(std::string msg) {
  }

  void Read() {
  }
};

#endif  // RDT_RDT_H_