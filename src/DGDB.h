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
#include "rdt/RDT_UDP.h"
#include "DBSchema.h"

class DGDB {
 private:
  using Storage = decltype(InitStorage("dgdb_data.sqlite3"));

  struct Host {
    std::string ip;
    uint16_t port;

    Host(std::string ip, int port) : ip(ip), port(port) {}
  };

  RDT_UDP rdt_udp_socket;

  Storage storage;

  int port;
  std::string ip;
  int connection;
  int server;
  char mode;
  int numberRepositories;
  int repository;

  int mainPort;
  std::string mainIp;

  std::vector<Host> repositories;

  void runMainServer();

  void runConnection();
  void connMasterRepository(int pIp, std::string pPort);

 public:
  explicit DGDB(char Pmode='S')
    : rdt_udp_socket(10), storage(InitStorage("./dgdb_data.sqlite3")) {
    mode = Pmode;
    ip="127.0.0.1";
    port=50000;
    connection=0;
    server=0;
    numberRepositories = 0;
    repository=0;
    repositories.push_back(Host{ip, 50000});
  }
  void setMainIp(std::string ip) {
    mainIp = ip;
    repositories[0].ip = mainIp;
  };
  void setMainPort(int pport=50000) {
    mainPort = pport;
    repositories[0].port = mainPort;
  };

  void setServer();
  void setClient();
  void setRepository();
  void closeServer();
  void closeClient();
  void closeRepository();
  void runServer();
  void runRepository();
  void setMode(char Pmode='S') {
    mode = Pmode;
  }
  void setPort(int Pport=50000) {
    port = Pport;
  }
  void setIp(std::string Pip="127.0.0.1") {
    ip = Pip;
  }
  void setNumberRepositories(int r) {
    numberRepositories = r;
  }
  void registerRepository();

  // CRUD DGDB
  void setNode(std::vector<std::string> args);
  void parseNewNode(std::string nameA, Host host,
                    std::vector<Attribute> attributes = {},
                    std::vector<std::string> relations = {});

  void setQuery(std::vector<std::string> args);
  void parseNewQuery(std::string nameA, int depth, bool leaf, bool attr,
                     Host host, std::vector<Condition> conditions = {});

  void setUpdate(std::vector<std::string> args);
  void parseNewUpdate(std::string nameA, bool is_node, std::string set_value,
                      Host host, std::string attr = "");

  void setDelete(std::vector<std::string> args);
  void parseNewDelete(std::string nameA, int object, Host host,
                      std::string attr_or_rel = "");
};

#endif  // DGDB_H_
