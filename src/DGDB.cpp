#include <string>
#include "DGDB.h"

#include "thirdparty/sqlite_orm/sqlite_orm.h"
#include "tools.h"

namespace sq = sqlite_orm;

// TODO: default contructor shouldn't define some variables
DGDB::DGDB() : storage(InitStorage("./dgdb_data.sqlite3")),
  ip("127.0.0.1"),
  port(50000),
  connection(0),
  server(0),
  number_repositories(0),
  repository(0) {
  repositories.push_back(Host{ip, 50000});
}

void DGDB::SetMainIp(std::string ip) {
  main_ip = ip;
};

void DGDB::SetMainPort(int p_port) {
  main_port = p_port;
};

void DGDB::SetMode(char p_mode) {
  mode = p_mode;
}

void DGDB::SetPort(int p_port) {
  port = p_port;
}

void DGDB::SetIp(std::string p_ip) {
  ip = p_ip;
}

void DGDB::SetNumberRepositories(int r) {
  number_repositories = r;
}

void DGDB::RunConnection() {
  int s, ss;
  char buffer[1024];
  std::string s_buffer;
  std::string bufferAux;
  std::string node_name;

  while (server || repository) {
    auto [n, from_addr] = udp_socket.RecvFrom(buffer, 1000);
    s_buffer = buffer;

    if (s_buffer[0] == 'C') {
      s_buffer.erase(0, 1);
      std::vector<Attribute> attributes;
      std::vector<std::string> relations;
      std::cout << "------------------\nAction: C" << std::endl;

      s = stoi(s_buffer.substr(0, 3)) + 2;
      s_buffer.erase(0, 3);

      if (s_buffer[s - 2] == '0') {
        std::cout << "No se envio relaciones" << std::endl;
      }
      else {
        bufferAux = s_buffer[s - 2];
        int cantidad = stoi(bufferAux);

        while (cantidad--) {
          ss = stoi(s_buffer.substr(s, 3));
          s_buffer.erase(s, 3);

          relations.push_back(s_buffer.substr(s, ss));
          s_buffer.erase(s, ss);
        }
      }

      if (s_buffer[s - 1] == '0') {
        std::cout << "No se envio atributos" << std::endl;
      }
      else {
        bufferAux = s_buffer[s - 1];
        int cantidad = stoi(bufferAux);

        while (cantidad--) {
          std::pair<std::string, std::string> current;
          ss = stoi(s_buffer.substr(s, 3));
          s_buffer.erase(s, 3);

          current.first = s_buffer.substr(s, ss);
          s_buffer.erase(s, ss);

          ss = stoi(s_buffer.substr(s, 3));
          s_buffer.erase(s, 3);

          current.second = s_buffer.substr(s, ss);
          s_buffer.erase(s, ss);

          attributes.push_back({"", current.first, current.second});
        }
      }

      node_name = s_buffer.substr(0, s - 2);

      if (server) {
        // enviar al  Repository
        // determinar que repository
        int r;

        if (repositories.size() < 2) {
          std::cout << "Repository has not been attached." << std::endl;
          return;
        }
        else {
          r = s_buffer[0] % (repositories.size() - 1);
        }

        r++;

        std::cout << "[DGDB] Node Sending: [" << node_name << "]" << std::endl;
        ParseNewNode(node_name, repositories[r], attributes, relations);
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

    else if (s_buffer[0] == 'R') {
      s_buffer.erase(0, 1);
      std::vector<Condition> conditions;
      int depth;
      bool leaf, attr;
      std::cout << "------------------\nAction: R" << std::endl;

      s = stoi(s_buffer.substr(0, 3)) + 5;
      s_buffer.erase(0, 3);

      if (n < s) {
        perror("ERROR reading second size\n");
      }
      else {
        bufferAux = s_buffer[s - 5];
        depth = stoi(bufferAux);

        bufferAux = s_buffer[s - 4];
        leaf = stoi(bufferAux);

        bufferAux = s_buffer[s - 3];
        attr = stoi(bufferAux);

        bufferAux = s_buffer[s - 2];
        bufferAux += s_buffer[s - 1];
        int cnt = stoi(bufferAux);

        if (cnt == 0)
          std::cout << "No se envio condiciones" << std::endl;
        else {
          while (cnt--) {
            Condition current;

            ss = stoi(s_buffer.substr(s, 3));
            s_buffer.erase(s, 3);

            current.key = s_buffer.substr(s, ss);
            s_buffer.erase(s, ss);

            current.op = Condition::Operator(s_buffer[s] - '0');
            s_buffer.erase(s, 1);

            ss = stoi(s_buffer.substr(s, 3));
            s_buffer.erase(s, 3);

            current.value = s_buffer.substr(s, ss);
            s_buffer.erase(s, ss);

            current.is_or = s_buffer[s] - '0';
            s_buffer.erase(s, 1);

            conditions.push_back(current);
          }
        }

        node_name = s_buffer.substr(0, s - 5);

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

    else if (s_buffer[0] == 'U') {
      s_buffer.erase(0, 1);
      std::string set_value, attr;
      bool is_node;
      std::cout << "------------------\nAction: U" << std::endl;

      is_node = s_buffer[0] - '0';
      s_buffer.erase(0, 1);

      s = stoi(s_buffer.substr(0, 3));
      s_buffer.erase(0, 3);

      if (is_node) {
        ss = stoi(s_buffer.substr(s, 3));
        s_buffer.erase(s, 3);

        set_value = s_buffer.substr(s, ss);
        s_buffer.erase(s, ss);
      }
      else {
        ss = stoi(s_buffer.substr(s, 3));
        s_buffer.erase(s, 3);
        attr = s_buffer.substr(s, ss);
        s_buffer.erase(s, ss);

        ss = stoi(s_buffer.substr(s, 3));
        s_buffer.erase(s, 3);
        set_value = s_buffer.substr(s, ss);
        s_buffer.erase(s, ss);
      }

      node_name = s_buffer.substr(0, s);

      if (server) {
        int r;

        if (repositories.size() < 2) {
          std::cout << "Repository has not been attached." << std::endl;
          return;
        }
        else
          r = s_buffer[0] % (repositories.size() - 1);

        r++;

        std::cout << "Sending: [" << node_name << "]" << std::endl;
        ParseNewUpdate(node_name, is_node, set_value, repositories[r], attr);
      }
      else if (repository) {
        std::cout << "Update:\n";
        std::cout << "> Node: " << node_name << "\n";

        if (is_node) {
          std::cout << "  > New name: " << set_value << "\n";
        }
        else {
          std::cout << "  > Attribute: " << attr << "\n";
          std::cout << "  > New value: " << set_value << "\n";
        }
      }
    }

    else if (s_buffer[0] == 'D') {
      s_buffer.erase(0, 1);
      std::string attr_or_rel;
      int object;
      std::cout << "------------------\nAction: D" << std::endl;

      object = s_buffer[0] - '0';
      s_buffer.erase(0, 1);

      s = stoi(s_buffer.substr(0, 3));
      s_buffer.erase(0, 3);

      if (object > 0) {
        ss = stoi(s_buffer.substr(s, 3));
        s_buffer.erase(s, 3);

        attr_or_rel = s_buffer.substr(s, ss);
        s_buffer.erase(s, ss);
      }

      node_name = s_buffer.substr(0, s);

      if (server) {
        int r;

        if (repositories.size() < 2) {
          std::cout << "Repository has not been attached." << std::endl;
          return;
        }
        else
          r = s_buffer[0] % (repositories.size() - 1);

        r++;

        std::cout << "Sending: [" << node_name << "]" << std::endl;
        ParseNewDelete(node_name, object, repositories[r], attr_or_rel);
      }
      else if (repository) {
        std::cout << "Delete:\n";
        std::cout << "> Node: " << node_name << "\n";

        if (object == 2) {
          std::cout << "  > Relation: " << attr_or_rel << "\n";
        }
        else if (object == 1) {
          std::cout << "  > Attribute: " << attr_or_rel << "\n";
        }
      }
    }

    else if (s_buffer[0] == 'E') {
      s_buffer.erase(0, 1);
      std::string vIp;
      int vPort;

      vPort = stoi(s_buffer.substr(0, 5));
      s_buffer.erase(0, 5);

      vIp = s_buffer.substr(0, 16);
      s_buffer.erase(0, 16);

      trim(vIp);
      ConnMasterRepository(vPort, vIp);

      if (repository) {
        repositories.push_back(Host{vIp, vPort});
        std::cout << "Repository registed." << std::endl;
      }
      else
        perror("ERROR no se pudo registrar Repositorio\n");
    }
    else std::cout << "[BUG DETECTED] RunConnection" << std::endl;
  }
}

void DGDB::RunMainServer() {
  RunConnection();
}

void DGDB::RunServer() {
  if (mode == 'S') {
    std::thread runThread(&DGDB::RunMainServer, this);
    runThread.join();
  }
  else if (mode == 'E') {
    std::thread runThread(&DGDB::RunRepository, this);
    runThread.join();
  }
}

void DGDB::SetClient(std::string client_ip, int client_port) {
  port = client_port;
  ip = client_ip;
  connection = 1;
}

void DGDB::CloseClient() {
  connection = 0;
  udp_socket.Shutdown(SHUT_RDWR);
  udp_socket.Close();
}

void DGDB::SetServer() {
  udp_socket.Bind(port);
  server = 1;
}

void DGDB::CloseServer() {
  server = 0;
  udp_socket.Close();
}

void DGDB::SetRepository() {
  std::cout << "[DGDB] setting Repository" << std::endl;
  udp_socket.Bind(port);

  RegisterRepository();
  repository = 1;
}

// CRUD

void DGDB::SetNode(std::vector<std::string> args) {
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
    std::cout << kErrorInvalidInput << std::endl;
    return;
  }

  std::cout << "Data sent successfully" << std::endl;
  ParseNewNode(nameA, repositories[0], attributes, relations);
}

void DGDB::ParseNewNode(std::string nameA, Host host,
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

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror(kErrorPErrorLenghtSend);
  }
}

void DGDB::SetQuery(std::vector<std::string> args) {
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
    std::cout << kErrorInvalidInput << std::endl;
    return;
  }

  std::cout << "Data sent successfully" << std::endl;
  ParseNewQuery(nameA, depth, leaf, attr, repositories[0],
                conditions);
}

void DGDB::ParseNewQuery(std::string nameA, int depth, bool leaf, bool attr,
                         Host host, std::vector<Condition> conditions) {
  char tamano[4];
  sprintf(tamano, "%03lu", nameA.length());
  std::string buffer;

  buffer = "R" + std::string(tamano) + nameA;

  sprintf(tamano, "%01lu", static_cast<long unsigned>(depth));
  buffer += std::string(tamano);

  buffer += leaf ? '1' : '0';
  buffer += attr ? '1' : '0';

  sprintf(tamano, "%02lu", static_cast<long unsigned>(conditions.size()));
  buffer += std::string(tamano);

  for (Condition& condition : conditions) {
    sprintf(tamano, "%03lu", condition.key.length());
    buffer += std::string(tamano) + condition.key;

    buffer += std::to_string(static_cast<int>(condition.op));

    sprintf(tamano, "%03lu", static_cast<long unsigned>(condition.value.length()));
    buffer += std::string(tamano) + condition.value;

    buffer += std::to_string(static_cast<int>(condition.is_or));
  }

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror(kErrorPErrorLenghtSend);
  }
}

void DGDB::SetUpdate(std::vector<std::string> args) {
  std::string nameA = args[0];
  std::string set_value, attr;
  bool is_node = true, error = false;

  size_t i = 1;

  if (i < args.size()) {
    if (args[i] == "--attr") {
      if (i + 1 < args.size()) {
        int equal_pos = args[i+1].find('=');
        std::string key = args[i+1].substr(0, equal_pos);
        std::string value = args[i+1].substr(equal_pos + 1);
        attr = key;
        set_value = value;
        is_node = false;
      }
      else
        error = true;
    }
    else if (args[i] == "--name") {
      if (i + 1 < args.size())
        set_value = args[i+1];
      else
        error = true;
    }
    else
      error = true;
  }
  else
    error = true;

  if (error) {
    std::cout << kErrorInvalidInput << std::endl;
    return;
  }

  std::cout << "Data sent successfully" << std::endl;
  ParseNewUpdate(nameA, is_node, set_value, repositories[0], attr);
}

void DGDB::ParseNewUpdate(std::string nameA, bool is_node,
                          std::string set_value,
                          Host host, std::string attr) {
  char tamano[4];
  sprintf(tamano, "%03lu", nameA.length());
  std::string buffer;

  buffer = "U" + (is_node ? std::string("1") : std::string("0"));
  buffer += std::string(tamano) + nameA;

  if (is_node) {
    sprintf(tamano, "%03lu", set_value.size());
    buffer += std::string(tamano) + set_value;
  }
  else {
    sprintf(tamano, "%03lu", attr.size());
    buffer += std::string(tamano) + attr;

    sprintf(tamano, "%03lu", set_value.size());
    buffer += std::string(tamano) + set_value;
  }

  std::cout << buffer << std::endl;

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror(kErrorPErrorLenghtSend);
  }
}

void DGDB::SetDelete(std::vector<std::string> args) {
  std::string nameA = args[0];
  std::string attr_or_rel;
  int object = 0;
  bool error = false;

  size_t i = 1;

  if (i < args.size()) {
    if (args[i] == "--relation") {
      if (i + 1 < args.size()) {
        attr_or_rel = args[i+1];
        object = 2;
      }
      else
        error = true;
    }
    else if (args[i] == "--attr") {
      if (i + 1 < args.size()) {
        attr_or_rel = args[i+1];
        object = 1;
      }
      else
        error = true;
    }
    else
      error = true;
  }
  else
    object = 0;

  if (error) {
    std::cout << kErrorInvalidInput << std::endl;
    return;
  }

  std::cout << "Data sent successfully" << std::endl;
  ParseNewDelete(nameA, object, repositories[0], attr_or_rel);
}

void DGDB::ParseNewDelete(std::string nameA, int object, Host host,
                          std::string attr_or_rel) {
  char tamano[4];
  sprintf(tamano, "%03lu", nameA.length());
  std::string buffer;

  buffer = "D" + std::to_string(object);
  buffer += std::string(tamano) + nameA;

  if (object > 0) {
    sprintf(tamano, "%03lu", attr_or_rel.size());
    buffer += std::string(tamano) + attr_or_rel;
  }

  std::cout << buffer << std::endl;

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror(kErrorPErrorLenghtSend);
  }
}

/// Protocolo

void DGDB::RegisterRepository() {
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

  int n = udp_socket.SendTo(main_ip, main_port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    std::cout << "Registering Repository:[" << buffer << "] failed" << std::endl;
  }

  //std::cout << "Bytes sent: " << n << std::endl;
}

void DGDB::RunRepository() {
  RunConnection();
}

void DGDB::ConnMasterRepository(int pIp, std::string pPort) {
  std::cout << pIp << "-" << pPort << std::endl;
  repository = 1;
}