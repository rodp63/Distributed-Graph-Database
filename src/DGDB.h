#ifndef HEADER_DGDB
#define HEADER_DGDB

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

#include "thirdparty/sqlite_orm/sqlite_orm.h"
#include "network/Network.h"

struct Node {
  int id;
  std::string name;

  Node() {}
  explicit Node(const std::string& name) : name(name) {}
  explicit Node(int id, const std::string& name) : id(id), name(name) {}
};

struct Attribute {
  std::string node_name;
  std::string key;
  std::string value;

  Attribute() {}
  Attribute(const std::string& node_name, const std::string& key,
            const std::string& value) :
    node_name(node_name), key(key), value(value) {}
};

struct Relation {
  std::string node_name1;
  std::string node_name2;

  Relation() {}
  Relation(const std::string& node1, const std::string& node2)
    : node_name1(node1), node_name2(node2) {}
};

inline auto InitStorage(const std::string& path) {
  using namespace sqlite_orm;
  auto storage = make_storage(path,
                              make_table("Nodes",
                                         make_column("id",
                                             &Node::id,
                                             primary_key()),
                                         make_column("name",
                                             &Node::name,
                                             unique())),
                              make_table("Attributes",
                                         make_column("node_name",
                                             &Attribute::node_name),
                                         make_column("key",
                                             &Attribute::key),
                                         make_column("value",
                                             &Attribute::value)),
                              make_table("Relations",
                                         make_column("node_name1",
                                             &Relation::node_name1),
                                         make_column("node_name2",
                                             &Relation::node_name2)));

  storage.sync_schema();

  return storage;
}

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
  int numberRepositories;
  int repository;

  int mainPort;
  std::string mainIp;

  std::map<int, std::string> connections;
  std::vector<int> socketRepositories; // se usa en el maestro
  //Repository_port  Repository_ip

  void runMainServer();

  void runConnection(int Pconnection);
  void connMasterRepository(int pPort, std::string pIp);

 public:
  explicit DGDB(char Pmode='S') : storage(InitStorage("./dgdb_data.sqlite3")) {
    mode = Pmode;
    ip="127.0.0.1";
    port=50000;
    connection=0;
    server=0;
    numberRepositories = 0;
    repository=0;
    socketRepositories.push_back(0);
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

  // CRUD DGDB
  void setNode(std::vector<std::string> args);
  void parseNewNode(std::string nameA, int conn=0,
                    std::vector<Attribute> attributes = {},
                    std::vector<std::string> relations = {});
};

#endif
