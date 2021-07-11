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

  void runMainServer();

  void runConnection(int);
  void connMasterRepository(int, std::string);

 public:
  explicit DGDB(char);
  
  void setMainIp(std::string);
  void setMainPort(int);
  void setServer();
  void setClient();
  void setRepository();
  void closeServer();
  void closeClient();
  void closeRepository();
  void runServer();
  void runRepository();
  void setMode(char);
  void setPort(int);
  void setIp(std::string);
  void setNumberRepositories(int);
  void registerRepository();

  // CRUD DGDB
  void setNode(std::vector<std::string>);
  void parseNewNode(std::string, int, std::vector<Attribute>, std::vector<std::string>);

  void setQuery(std::vector<std::string>);
  void parseNewQuery(std::string, int, bool, bool,int, std::vector<Condition>);

  void setUpdate(std::vector<std::string>);
  void parseNewUpdate(std::string, bool, std::string,int, std::string);

  void setDelete(std::vector<std::string>);
  void parseNewDelete(std::string, int, int, std::string);
};

#endif // DGDB_H