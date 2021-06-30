#include "DGDB.h"
#include "tools.h"
#include "thirdparty/sqlite_orm/sqlite_orm.h"

namespace sq = sqlite_orm;

void DGDB::runConnection(int Pconnection) {
  int n;
  int s, ss;
  char buffer[1024];
  char bufferAux[1024];
  std::string node_name;
  connections[Pconnection] = "";
  Network::TCPSocket conn_socket(Pconnection);

  while (server || repository) {
    n = conn_socket.Recv(buffer, 1);

    if (buffer[0] == 'C') {
      std::vector<Attribute> attributes;
      std::vector<std::string> relations;
      std::cout << "------------------\nAction: C" << std::endl;

      n = conn_socket.Recv(buffer, 3);

      if (n < 3) {
        perror("ERROR reading first size\n");
      }
      else {
        buffer[n + 1] = '\0';
        s = atoi(buffer) + 2;
      }

      n = conn_socket.Recv(buffer, s);

      if (n < s) {
        perror("ERROR reading second size\n");
      }
      else {
        int r;

        if (buffer[s - 2] == '0') {
          std::cout << "No se envio relaciones" << std::endl;
        }
        else {
          bufferAux[0] = buffer[s - 2];
          bufferAux[1] = '\0';
          int cantidad = atoi(bufferAux);

          while (cantidad--) {
            n = conn_socket.Recv(bufferAux, 3);
            ss = atoi(bufferAux);
            n = conn_socket.Recv(bufferAux, ss);
            relations.push_back(std::string(bufferAux));
          }
        }

        if (buffer[s - 1] == '0') {
          std::cout << "No se envio atributos" << std::endl;
        }
        else {
          bufferAux[0] = buffer[s - 1];
          bufferAux[1] = '\0';
          int cantidad = atoi(bufferAux);

          while (cantidad--) {
            std::pair<std::string, std::string> current;
            n = conn_socket.Recv(bufferAux, 3);
            ss = atoi(bufferAux);
            n = conn_socket.Recv(bufferAux, ss);
            current.first = bufferAux;

            n = conn_socket.Recv(bufferAux, 3);
            ss = atoi(bufferAux);
            n = conn_socket.Recv(bufferAux, ss);
            current.second = bufferAux;

            attributes.push_back({"", current.first, current.second});
          }
        }

        buffer[s - 2] = '\0';
        node_name = buffer;

        if (socketRepositories.size() > 1) {
          // enviar al  Repository
          // determinar que repository
          if (socketRepositories.size() < 2) {
            std::cout << "Repository has not been attached." << std::endl;
            return;
          }
          else
            r = buffer[0] % (socketRepositories.size() - 1);

          r++;
          std::cout << "xxxx--:" << socketRepositories.size() << std::endl;
          std::cout << "xxxx-r:" << r << std::endl;
          std::cout << "xxxx-s:" << socketRepositories[r] << std::endl;

          std::cout << "Sending: [" << node_name << "]" << std::endl;
          parseNewNode(node_name, socketRepositories[r], attributes, relations);
        }
        else if (repository) {
          Node new_node{node_name};
          std::cout << new_node.name << std::endl;

          try {
            storage.insert(new_node);
          }
          catch (std::system_error e) {
            std::cout << e.what() << std::endl;
            continue;
          }

          std::cout << "Store: " << node_name << std::endl;

          if (attributes.size())
            std::cout << "Attributes:\n";

          for (auto& attr : attributes) {
            attr.node_name = node_name;
            std::cout << "-> " << attr.key << " : " << attr.value << std::endl;
            storage.insert(attr);
          }

          if (relations.size())
            std::cout << "Relations:\n";

          for (const auto& rel : relations) {
            Node related_node{rel};

            try {
              auto node = storage.get_all<Node>(sq::where(sq::c(&Node::name) =
                                                          related_node.name));

              if (node.empty()) {
                related_node.id = storage.insert(related_node);
              }
            }
            catch (std::system_error e) {
              std::cout << e.what() << std::endl;
            }

            Relation new_relation{new_node.name, related_node.name};
            storage.insert(new_relation);
            std::cout << "> " << rel << std::endl;
          }

          attributes.clear();
          relations.clear();
        }
      }
    }

    else if (buffer[0] == 'R') {
      std::vector<Condition> conditions;
      int depth;
      bool leaf, attr;
      std::cout << "------------------\nAction: R" << std::endl;
      n = conn_socket.Recv(buffer, 3);

      if (n < 3) {
        perror("ERROR reading first size\n");
      }
      else {
        buffer[n + 1] = '\0';
        s = atoi(buffer) + 5;
      }

      n = conn_socket.Recv(buffer, s);

      if (n < s) {
        perror("ERROR reading second size\n");
      }
      else {
        bufferAux[0] = buffer[s - 5];
        bufferAux[1] = '\0';
        depth = atoi(bufferAux);

        bufferAux[0] = buffer[s - 4];
        bufferAux[1] = '\0';
        leaf = atoi(bufferAux);

        bufferAux[0] = buffer[s - 3];
        bufferAux[1] = '\0';
        attr = atoi(bufferAux);

        bufferAux[0] = buffer[s - 2];
        bufferAux[1] = buffer[s - 1];
        bufferAux[2] = '\0';
        int cnt = atoi(bufferAux);
        
        if (cnt == 0)
          std::cout << "No se envio condiciones" << std::endl;
        else {
          while (cnt--) {
            Condition current;
            n = conn_socket.Recv(bufferAux, 3);
            ss = atoi(bufferAux);
            n = conn_socket.Recv(bufferAux, ss);
            current.key = bufferAux;

            n = conn_socket.Recv(bufferAux, 1);
            current.op = Condition::Operator(atoi(bufferAux));

            n = conn_socket.Recv(bufferAux, 3);
            ss = atoi(bufferAux);
            n = conn_socket.Recv(bufferAux, ss);
            current.value = bufferAux;

            n = conn_socket.Recv(bufferAux, 1);
            current.is_or = atoi(bufferAux);

            conditions.push_back(current);
          }
        }

        buffer[s - 5] = '\0';
        node_name = buffer;

        if (server) {
          std::cout << "Query:\n";
          std::cout << "> Node: " << node_name << "\n";
          std::cout << "> Depth: " << depth << "\n";
          std::cout << "> Leaf?: " << leaf << "\n";
          std::cout << "> Attributes?: " << attr << "\n";
          if (conditions.size())
            std::cout << "> Conditions: " << conditions.size() << "\n";
          for (auto& cond : conditions) {
            std::cout << "  -> " << cond.key << " "
                      << cond.op_to_string() << " "
                      << cond.value << " "
                      << cond.is_or_to_string() << std::endl;
          }
        }
      }
    }

    else if (buffer[0] == 'U') {
      
    }

    else if (buffer[0] == 'D') {
    }

    else if (buffer[0] == 'E') {
      n = conn_socket.Recv(buffer, 21);

      if (n < 21) {
        perror("ERROR reading size in Repository command\n");
        printf("n = %d\n%s\n", n, buffer);
      }
      else {
        char vIp[17];
        char vPort[6];
        int vPort_int;
        char* pbuffer;

        strncpy(vPort, buffer, 5);
        vPort_int = atoi(vPort);

        pbuffer = &buffer[5];
        strncpy(vIp, pbuffer, 16);
        buffer[16] = '\0';

        std::string vIp_string = vIp;
        trim(vIp_string);

        connMasterRepository(vPort_int, vIp_string);

        if (repository) {
          socketRepositories.push_back(repository_socket.GetSocketId());
          std::cout << "Repository registed." << std::endl;
        }
        else
          perror("ERROR no se pudo registrar Repositorio\n");
      }
    }

    //n = write(ConnectFD,"I got your message",18);
    //if (n < 0) perror("ERROR writing to socket");
    /* perform read write operations ... */
  }

  connections.erase(Pconnection);
  shutdown(Pconnection, SHUT_RDWR);
  close(Pconnection);
}

void DGDB::runMainServer() {
  for (;;) {
    int new_connection = server_socket.AcceptConnection();

    std::thread(&DGDB::runConnection, this, new_connection).detach();
  }
}

void DGDB::runServer() {
  if (mode == 'S') {
    std::thread runThread(&DGDB::runMainServer, this);
    runThread.join();
  }
  else if (mode == 'E') {
    std::cout << "RRRRR" << std::endl;
    std::thread runThread(&DGDB::runRepository, this);
    runThread.join();
  }
}

void DGDB::setClient() {
  client_socket.Init();
  client_socket.Connect(ip, port);
  connection = 1;
}

void DGDB::closeClient() {
  connection = 0;
  client_socket.Shutdown(SHUT_RDWR);
  client_socket.Close();
}

void DGDB::setServer() {
  server_socket.Init();
  server_socket.Bind(port);
  server_socket.SetListenerSocket();
  server = 1;
}

void DGDB::closeServer() {
  server = 0;
  server_socket.Close();
}

void DGDB::setRepository() {
  std::cout << "setRepository" << std::endl;
  repository_socket.RenewSocket();
  repository_socket.Bind(port);
  repository_socket.SetListenerSocket();

  registerRepository();
  repository = 1;
}

void DGDB::setNode(std::vector<std::string> args) {
  std::string nameA = args[0];
  std::vector<Attribute> attributes;
  std::vector<std::string> relations;
  bool error = false;

  for (size_t i = 1; i < args.size(); i += 2) {
    if (args[i] == "-a") {
      if (i + 1 < args.size()) {
        int equal_pos = args[i+1].find('=');
        std::string key = args[i+1].substr(0, equal_pos);
        std::string value = args[i+1].substr(equal_pos + 1);
        attributes.emplace_back(Attribute{"", key, value});
      }
      else
        error = true;
    }
    else if (args[i] == "-r") {
      if (i + 1 < args.size()) {
        relations.push_back(args[i+1]);
      }
      else
        error = true;
    }
    else
      error = true;
  }

  if (error) {
    std::cout << "[ERROR] Invalid input!" << std::endl;
    return;
  }

  std::cout << "Data sent successfully" << std::endl;
  parseNewNode(nameA, client_socket.GetSocketId(), attributes, relations);
}

void DGDB::parseNewNode(std::string nameA, int conn,
                        std::vector<Attribute> attributes,
                        std::vector<std::string> relations) {
  char tamano[4];
  sprintf(tamano, "%03lu", nameA.length());
  std::string buffer;

  buffer = "C" + std::string(tamano) + nameA;
  buffer += char('0' + relations.size());
  buffer += char('0' + attributes.size());

  for (std::string& node : relations) {
    sprintf(tamano, "%03lu", node.length());
    buffer += std::string(tamano) + node;
  }

  for (auto& attr : attributes) {
    sprintf(tamano, "%03lu", attr.key.length());
    buffer += std::string(tamano) + attr.key;
    sprintf(tamano, "%03lu", attr.value.length());
    buffer += std::string(tamano) + attr.value;
  }

  Network::TCPSocket conn_sock(conn);
  int n = conn_sock.Send(buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror("error listen failed\n");
  }
}

void DGDB::setQuery(std::vector<std::string> args) {
  std::string nameA = args[0];
  std::vector<Condition> conditions;
  int depth = 0;
  bool leaf = false, attr = false, error = false;

  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i] == "-d") {
      if (i + 1 < args.size())
        depth = atoi(args[++i].c_str());
      else
        error = true;
    }
    else if (args[i] == "-c") {
      if (i + 1 < args.size()) {
        ++i;
        int space_pos = args[i].find(' ');
        int space_pos_2 = args[i].find(' ', space_pos+3);
        if (space_pos_2 == std::string::npos)
          space_pos_2 = args[i].size();
        std::string key = args[i].substr(0, space_pos);
        std::string value = args[i].substr(space_pos+3, space_pos_2-space_pos-3);
        char op = args[i][space_pos+1];
        char lo = space_pos_2 + 1 < args[i].size() ? args[i][space_pos_2+1] : '&';
        conditions.emplace_back(key, value, op, lo);
      }
      else
        error = true;
    }
    else if (args[i] == "--leaf")
      leaf = true;
    else if (args[i] == "--attr")
      attr = true;
    else
      error = true;
  }

  if (error) {
    std::cout << "[ERROR] Invalid input!" << std::endl;
    return;
  }

  std::cout << "Data sent successfully" << std::endl;
  parseNewQuery(nameA, depth, leaf, attr, client_socket.GetSocketId(), conditions);
}

void DGDB::parseNewQuery(std::string nameA, int depth, bool leaf, bool attr,
                      int conn, std::vector<Condition> conditions) {
  char tamano[4];
  sprintf(tamano, "%03lu", nameA.length());
  std::string buffer;

  buffer = "R" + std::string(tamano) + nameA;

  sprintf(tamano, "%01lu", (long)depth);
  buffer += std::string(tamano);

  buffer += leaf ? '1' : '0';
  buffer += attr ? '1' : '0';

  sprintf(tamano, "%02lu", conditions.size());
  buffer += std::string(tamano);

  for (Condition &condition : conditions) {
    sprintf(tamano, "%03lu", condition.key.length());
    buffer += std::string(tamano) + condition.key;

    buffer += std::to_string(int(condition.op));

    sprintf(tamano, "%03lu", condition.value.length());
    buffer += std::string(tamano) + condition.value;

    buffer += std::to_string(int(condition.is_or));
  }

  Network::TCPSocket conn_sock(conn);
  int n = conn_sock.Send(buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror("error listen failed\n");
  }
}

/// Protocolo

void DGDB::registerRepository() {
  /*
    int action; // X
    int port; // 5B
    char ip[16] VB
  */
  std::cout << ip << std::endl;
  std::string buffer;
  char vip[17];
  char vport[6];
  sprintf(vport, "%05d", port);
  vport[5] = '\0';
  sprintf(vip, "%16s", ip.c_str());
  std::cout << vip << std::endl;
  std::string sport = vport;
  std::string sip = vip;
  buffer = "E" + sport + sip;
  std::cout << "*" << buffer.c_str() << "*" << std::endl;

  // Set conn to Main
  Network::TCPSocket main_socket;
  main_socket.Init();
  main_socket.Connect(mainIp, mainPort);

  std::cout << "*2*" << std::endl;

  std::cout << "*3*" << std::endl;
  int n = main_socket.Send(buffer.c_str(), buffer.length());
  std::cout << "*4*" << std::endl;

  if (n > 0 && (size_t)n != buffer.length()) {
    std::cout << "Registing Repository:[" << buffer << "] failed" << std::endl;
  }

  std::cout << n << std::endl;
}

void DGDB::runRepository() {
  for (;;) {
    int new_connection = repository_socket.AcceptConnection();

    std::thread(&DGDB::runConnection, this, new_connection).detach();
  }
}

void DGDB::connMasterRepository(int pPort, std::string pIp) {
  std::cout << pPort << "-" << pIp << std::endl;
  repository_socket.RenewSocket();
  repository_socket.Connect(pIp, pPort);
  repository = 1;
}
