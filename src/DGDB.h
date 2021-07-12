#ifndef DGDB_H_
#define DGDB_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdint>

#include "network/Network.h"
#include "DBSchema.h"

static const char kDataSentSuccess[]= "Data sent successfully";
static const char kErrorInvalidInput[]= "[ERROR] Invalid input!";
static const char kErrorPErrorLenghtSend[]= "[ERROR] listen failed";

class DGDB {
 private:
  using Storage = decltype(InitStorage("dgdb_data.sqlite3"));

  struct Host {
    std::string ip;
    uint16_t port;

    Host(std::string ip, int port) : ip(ip), port(port) {}
  };

  Network::UDPSocket udp_socket;

  Storage storage;

  int port;
  std::string ip;
  int connection;
  int server;
  char mode;
  int number_repositories;
  int repository;

  int main_port;
  std::string main_ip;

  std::vector<Host> repositories;

  void RunMainServer();

  void RunConnection();
  void ConnMasterRepository(int, std::string);

 public:
  DGDB();
  
  void SetMainIp(std::string);
  void SetMainPort(int);
  void SetServer();
  void SetClient(std::string, int);
  void SetRepository();
  void CloseServer();
  void CloseClient();
  void CloseRepository();
  void RunServer();
  void RunRepository();
  void SetMode(char);
  void SetPort(int);
  void SetIp(std::string);
  void SetNumberRepositories(int);
  void RegisterRepository();

  // CRUD DGDB
  void SetNode(std::vector<std::string>);
  void ParseNewNode(std::string, Host, std::vector<Attribute>, std::vector<std::string>);

  void SetQuery(std::vector<std::string>);
  void ParseNewQuery(std::string, int, bool, bool, Host, std::vector<Condition>);

  void SetUpdate(std::vector<std::string>);
  void ParseNewUpdate(std::string, bool, std::string, Host, std::string);

  void SetDelete(std::vector<std::string>);
  void ParseNewDelete(std::string, int, Host host, std::string);
};

#endif // DGDB_H
