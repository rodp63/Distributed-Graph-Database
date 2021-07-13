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

class DGDB {
 private:
  using Storage = decltype(InitStorage("dgdb_data.sqlite3"));

  struct Host {
    std::string ip;
    uint16_t port;

    Host(std::string ip, int port) : ip(ip), port(port) {}
  };

  struct Pending {
    struct sockaddr_in client_sock;
    int desired_responses = 1;
  };

  Network::UDPSocket udp_socket;

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

  size_t current_request = 1;
  std::map<int, Pending> pending;

  void runMainServer();

  void runConnection();

 public:
  explicit DGDB(char Pmode='S') : storage(InitStorage("./dgdb_data.sqlite3")) {
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
  };
  void setMainPort(int pport=50000) {
    mainPort = pport;
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

  Host sockaddrToHost(struct sockaddr_in sock);
  Host stringToHost(std::string host);

  void WaitResponse();
  void parseNewResponse(Host host, std::string message, size_t request_id = 0);

  // CRUD DGDB
  bool setNode(std::vector<std::string> args);
  void parseNewNode(std::string nameA, Host host,
                    std::vector<Attribute> attributes = {},
                    std::vector<std::string> relations = {},
                    size_t request_id = 0,
                    std::vector<Host> master_repos = {});

  bool setQuery(std::vector<std::string> args);
  void parseNewQuery(std::string nameA, int depth, bool leaf, bool attr,
                     Host host, std::vector<Condition> conditions = {},
                     size_t request_id = 0);

  bool setUpdate(std::vector<std::string> args);
  void parseNewUpdate(std::string nameA, bool is_node, std::string set_value,
                      Host host, std::string attr = "",
                      size_t request_id = 0);

  bool setDelete(std::vector<std::string> args);
  void parseNewDelete(std::string nameA, int object, Host host,
                      std::string attr_or_rel = "",
                      size_t request_id = 0,
                      std::vector<Host> master_repos = {});
};

#endif  // DGDB_H_
