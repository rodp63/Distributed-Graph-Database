#include "RDT.h"

RDT_UDP_Server::RDT_UDP_Server(int port) : num_clients(0) {}

void RDT_UDP_Server::AcceptClient() {
  sequence.push_back(0);
  ++num_clients;
}

void RDT_UDP_Server::Send(int client_id, std::string msg) {
}

void RDT_UDP_Server::Read(int client_id) {
}

RDT_UDP_Client::RDT_UDP_Client(std::string addr, int port) {
} 

void RDT_UDP_Client::Send(std::string msg) {
}

void RDT_UDP_Client::Read() {
}