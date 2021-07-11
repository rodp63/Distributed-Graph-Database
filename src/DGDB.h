#ifndef DGDB_H
#define DGDB_H

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

#include "network/Network.h"
#include "DBSchema.h"

static const char kDataSentSuccess[]= "Data sent successfully";
static const char kErrorInvalidInput[]= "[ERROR] Invalid input!";
static const char kErrorPErrorLenghtSend[]= "[ERROR] listen failed";

class DGDB {
 private:
  using Storage = decltype(InitStorage("dgdb_data.sqlite3"));

  Network::TCPSocket server_socket;
  Network::TCPSocket client_socket;
  Network::TCPSocket repository_socket;

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

  std::map<int, std::string> connections;
  std::vector<int> socket_repositories; // se usa en el maestro
  //Repository_port  Repository_ip

  void RunMainServer();

  void RunConnection(int);
  void ConnMasterRepository(int, std::string);

 public:
  explicit DGDB(char);
  
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
  void ParseNewNode(std::string, int, std::vector<Attribute>, std::vector<std::string>);

  void SetQuery(std::vector<std::string>);
  void ParseNewQuery(std::string, int, bool, bool,int, std::vector<Condition>);

  void SetUpdate(std::vector<std::string>);
  void ParseNewUpdate(std::string, bool, std::string,int, std::string);

  void SetDelete(std::vector<std::string>);
  void ParseNewDelete(std::string, int, int, std::string);
};

#endif // DGDB_H