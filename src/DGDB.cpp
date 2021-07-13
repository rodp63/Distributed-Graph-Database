#include <string>
#include "DGDB.h"
#include "tools.h"
#include "thirdparty/sqlite_orm/sqlite_orm.h"

namespace sq = sqlite_orm;

void DGDB::WaitResponse() {
  char buffer[1024];
  auto [n, from_addr] = udp_socket.RecvFrom(buffer, 1000);
  if (buffer[0] == 'M') {
    std::string message(buffer);
    std::cout << message.substr(4, message.size() - 7) << std::endl;
  }
  else {
    // Query response
  }
}

void DGDB::runConnection() {
  int s, ss, request_id, repo_count;
  char buffer[1024];
  std::string s_buffer;
  std::string bufferAux;
  std::string node_name;

  while (server || repository) {
    auto [n, from_addr] = udp_socket.RecvFrom(buffer, 1000);
    s_buffer = buffer;

    if (s_buffer[0] == 'M') {
      std::cout << "------------------\nAction: M" << std::endl;
      s_buffer.erase(0, 1);

      s = stoi(s_buffer.substr(0, 3));
      s_buffer.erase(0, 3);

      bufferAux = s_buffer.substr(0, s);
      s_buffer.erase(0, s);
      std::cout << "[INFO] Message: " << bufferAux << std::endl;

      request_id = stoi(s_buffer.substr(0, 3));

      Host client = sockaddrToHost(pending[request_id].client_sock);
      parseNewResponse(client, bufferAux, request_id);
      pending.erase(request_id);
    }

    if (s_buffer[0] == 'C') {
      s_buffer.erase(0, 1);
      std::vector<Attribute> attributes;
      std::vector<std::string> relations;
      std::cout << "------------------\nAction: C" << std::endl;

      s = stoi(s_buffer.substr(0, 3)) + 2;
      s_buffer.erase(0, 3);

      if (s_buffer[s - 2] == '0') {
        std::cout << "[INFO] No relations were sent" << std::endl;
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
        std::cout << "[INFO] No attributes were sent" << std::endl;
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

      request_id = stoi(s_buffer.substr(s, 3));
      s_buffer.erase(s, 3);

      node_name = s_buffer.substr(0, s - 2);

      if (server) {
        int r;

        if (repositories.size() < 2) {
          std::string err = "[ERROR] Repository has not been attached";
          std::cout << err << std::endl;
          parseNewResponse(sockaddrToHost(from_addr), err, request_id);
          continue;
        }
        else {
          r = s_buffer[0] % (repositories.size() - 1);
        }

        r++;

        std::cout << "[INFO] Sending: (" << node_name << ")" << std::endl;

        Pending c_pending;
        c_pending.client_sock = from_addr;
        pending[current_request] = c_pending;
        
        parseNewNode(node_name, repositories[r], attributes, relations,
                     current_request, repositories);
        ++current_request;
      }
      else if (repository) {
        repo_count = stoi(s_buffer.substr(s, 1));
        s_buffer.erase(s, 1);

        std::vector<Host> master_repos;
        while (repo_count--) {
          std::string repo_info;
          ss = stoi(s_buffer.substr(s, 2));
          s_buffer.erase(s, 2);

          repo_info = s_buffer.substr(s, ss);
          s_buffer.erase(s, ss);

          master_repos.push_back(stringToHost(repo_info));
        }

        repo_count = master_repos.size();

        Node new_node{node_name};
        std::string message;

        try {
          auto node = storage.get_all<Node>(sq::where(sq::c(&Node::name) =
                                                      node_name));
          if (node.empty()) {
            storage.insert(new_node);
          }
          else {
            message = "[WARNING] Node already exists. Skipping\n";
            std::cout << message;
          }
        }
        catch (std::system_error e) {
          message = std::string("[ERROR] ") + e.what();
          std::cout << message << std::endl;
          parseNewResponse(sockaddrToHost(from_addr), message, request_id);
          continue;
        }

        std::cout << "Node Struct:\n";
        std::cout << "  Name: " << node_name << std::endl;

        if (attributes.size())
          std::cout << "  Attributes:\n";

        for (auto& attr : attributes) {
          attr.node_name = node_name;
          std::cout << "  - " << attr.key << ": " << attr.value << std::endl;
          storage.insert(attr);
        }

        if (relations.size())
          std::cout << "  Relations:\n";

        for (const auto& rel : relations) {
          Node related_node{rel};
          std::cout << "  - " << rel << std::endl;

          try {
            auto node = storage.get_all<Node>(sq::where(sq::c(&Node::name) =
                                                related_node.name));
            if (node.empty()) {
              related_node.id = storage.insert(related_node);
            }
          }
          catch (std::system_error e) {
            message = std::string("[ERROR] ") + e.what();
            std::cout << message << std::endl;
            parseNewResponse(sockaddrToHost(from_addr), message, request_id);
            continue;
          }

          Relation new_relation{new_node.name, related_node.name};
          storage.insert(new_relation);

          if (repo_count) {
            int new_r = (rel[0] % (repo_count - 1)) + 1;
            parseNewNode(rel, master_repos[new_r], {}, {node_name});
          }
        }

        message += "OK! Node stored successfully";
        if (request_id)
          parseNewResponse(repositories[0], message, request_id);

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

      bufferAux = s_buffer[s - 5];
      depth = stoi(bufferAux);

      bufferAux = s_buffer[s - 4];
      leaf = stoi(bufferAux);

      bufferAux = s_buffer[s - 3];
      attr = stoi(bufferAux);

      bufferAux = s_buffer[s - 2];
      bufferAux += s_buffer[s - 1];
      int cnt = stoi(bufferAux);

      if (cnt == 0) {
        std::cout << "[INFO] No conditions were sent" << std::endl;
      }
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

      request_id = stoi(s_buffer.substr(s, 3));
      s_buffer.erase(s, 3);

      node_name = s_buffer.substr(0, s - 5);

      if (server) {
        std::cout << "Query Struct:\n";
        std::cout << "  Node: " << node_name << "\n";
        std::cout << "  Depth: " << depth << "\n";
        std::cout << "  Leaf?: " << leaf << "\n";
        std::cout << "  Attributes?: " << attr << "\n";

        if (conditions.size())
          std::cout << "  Conditions: " << conditions.size() << "\n";

        for (auto& cond : conditions) {
          std::cout << "  - " << cond.key << " "
                    << cond.op_to_string() << " "
                    << cond.value << " "
                    << cond.is_or_to_string() << std::endl;
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

      request_id = stoi(s_buffer.substr(s, 3));
      s_buffer.erase(s, 3);

      node_name = s_buffer.substr(0, s);

      if (server) {
        int r;

        if (repositories.size() < 2) {
          std::string err = "[ERROR] Repository has not been attached";
          std::cout << err << std::endl;
          parseNewResponse(sockaddrToHost(from_addr), err, request_id);
          continue;
        }
        else
          r = s_buffer[0] % (repositories.size() - 1);

        r++;

        std::cout << "[INFO] Sending: (" << node_name << ")" << std::endl;

        Pending c_pending;
        c_pending.client_sock = from_addr;
        pending[current_request] = c_pending;
        
        parseNewUpdate(node_name, is_node, set_value, repositories[r], attr, current_request);
        ++current_request;
      }
      else if (repository) {
        std::string message;
        std::cout << "Update Struct:\n";
        std::cout << "  Node: " << node_name << "\n";

        if (is_node) {
          // TODO: storage.update(new_node); -> try catch
          std::cout << "  New name: " << set_value << "\n";
        }
        else {
          // TODO: storage.update(new_attribute); -> try catch
          std::cout << "  Attribute: " << attr << "\n";
          std::cout << "  New value: " << set_value << "\n";
        }

        message = "OK! Node updated successfully";
        parseNewResponse(repositories[0], message, request_id);
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

      request_id = stoi(s_buffer.substr(s, 3));
      s_buffer.erase(s, 3);

      node_name = s_buffer.substr(0, s);

      if (server) {
        int r;

        if (repositories.size() < 2) {
          std::string err = "[ERROR] Repository has not been attached";
          std::cout << err << std::endl;
          parseNewResponse(sockaddrToHost(from_addr), err, request_id);
          continue;
        }
        else
          r = s_buffer[0] % (repositories.size() - 1);

        r++;

        Pending c_pending;
        c_pending.client_sock = from_addr;
        pending[current_request] = c_pending;

        std::cout << "[INFO] Sending: (" << node_name << ")" << std::endl;
        parseNewDelete(node_name, object, repositories[r], attr_or_rel,
                       current_request, repositories);
        ++current_request;
      }
      else if (repository) {
        repo_count = stoi(s_buffer.substr(s, 1));
        s_buffer.erase(s, 1);

        std::vector<Host> master_repos;
        while (repo_count--) {
          std::string repo_info;
          ss = stoi(s_buffer.substr(s, 2));
          s_buffer.erase(s, 2);

          repo_info = s_buffer.substr(s, ss);
          s_buffer.erase(s, ss);

          master_repos.push_back(stringToHost(repo_info));
        }

        repo_count = master_repos.size();

        std::string message;

        std::cout << "Delete Struct:\n";
        std::cout << "  Node: " << node_name << "\n";
        // TODO: Delete Node and all attributes from DB
        // TODO: Get relations and call delete recursively with 'parseNewDelete':
        // for (std::string &rel_node : relations) {
        //   storage.relations.delete({node_name, rel_node})
        //   int new_r = (rel_node[0] % (repo_count - 1)) + 1;
        //   parseNewDelete(rel_node, 2, repositories[new_r], node_name);
        // }
        message = "OK! Node deleted successfully";

        if (object == 2) {
          std::cout << "  Relation: " << attr_or_rel << "\n";

          if (repo_count) {
            int new_r = (attr_or_rel[0] % (repo_count - 1)) + 1;
            parseNewDelete(attr_or_rel, object, master_repos[new_r], node_name);
          }

          message = "OK! Relation deleted successfully";
        }
        else if (object == 1) {
          // TODO: Delete attribute from DB
          std::cout << "  Attribute: " << attr_or_rel << "\n";
          message = "OK! Attribute deleted successfully";
        }

        if (request_id)
          parseNewResponse(repositories[0], message, request_id);
      }
    }

    else if (s_buffer[0] == 'E') {
      std::cout << "------------------\nAction: E" << std::endl;
      s_buffer.erase(0, 1);
      std::string vIp;
      int vPort;

      vPort = stoi(s_buffer.substr(0, 5));
      s_buffer.erase(0, 5);

      vIp = s_buffer.substr(0, 16);
      s_buffer.erase(0, 16);

      trim(vIp);
      repository = 1;

      if (repository) {
        repositories.push_back(Host{vIp, vPort});
        std::cout << "[INFO] Repository registed." << std::endl;
      }
      else
        perror("[ERROR] no se pudo registrar Repositorio\n");
    }
  }
}

void DGDB::runMainServer() {
  runConnection();
}

void DGDB::runServer() {
  if (mode == 'S') {
    std::thread runThread(&DGDB::runMainServer, this);
    runThread.join();
  }
  else if (mode == 'E') {
    std::thread runThread(&DGDB::runRepository, this);
    runThread.join();
  }
}

void DGDB::setClient() {
  connection = 1;
}

void DGDB::closeClient() {
  connection = 0;
  udp_socket.Shutdown(SHUT_RDWR);
  udp_socket.Close();
}

void DGDB::setServer() {
  udp_socket.Bind(port);
  server = 1;
}

void DGDB::closeServer() {
  server = 0;
  udp_socket.Close();
}

void DGDB::setRepository() {
  udp_socket.Bind(port);

  registerRepository();
  repository = 1;
}

bool DGDB::setNode(std::vector<std::string> args) {
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
    return false;
  }

  parseNewNode(nameA, repositories[0], attributes, relations);
  return true;
}

void DGDB::parseNewNode(std::string nameA, Host host,
                        std::vector<Attribute> attributes,
                        std::vector<std::string> relations,
                        size_t request_id,
                        std::vector<Host> master_repos) {
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

  sprintf(tamano, "%03lu", request_id);
  buffer += std::string(tamano);

  sprintf(tamano, "%01lu", master_repos.size());
  buffer += std::string(tamano);

  for (auto& repo : master_repos) {
    std::string repo_str = repo.ip + ":" + std::to_string(repo.port);
    sprintf(tamano, "%02lu", repo_str.length());
    buffer += std::string(tamano) + repo_str;
  }

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror("error listen failed\n");
  }
}

bool DGDB::setQuery(std::vector<std::string> args) {
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
    return false;
  }

  parseNewQuery(nameA, depth, leaf, attr, repositories[0], conditions);
  return true;
}

void DGDB::parseNewQuery(std::string nameA, int depth, bool leaf, bool attr,
                         Host host, std::vector<Condition> conditions,
                         size_t request_id) {
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

  for (Condition& condition : conditions) {
    sprintf(tamano, "%03lu", condition.key.length());
    buffer += std::string(tamano) + condition.key;

    buffer += std::to_string(int(condition.op));

    sprintf(tamano, "%03lu", condition.value.length());
    buffer += std::string(tamano) + condition.value;

    buffer += std::to_string(int(condition.is_or));
  }

  sprintf(tamano, "%03lu", request_id);
  buffer += std::string(tamano);

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror("error listen failed\n");
  }
}

bool DGDB::setUpdate(std::vector<std::string> args) {
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
    std::cout << "[ERROR] Invalid input!" << std::endl;
    return false;
  }

  parseNewUpdate(nameA, is_node, set_value, repositories[0], attr);
  return true;
}

void DGDB::parseNewUpdate(std::string nameA, bool is_node,
                          std::string set_value,
                          Host host, std::string attr, size_t request_id) {
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

  sprintf(tamano, "%03lu", request_id);
  buffer += std::string(tamano);

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror("error listen failed\n");
  }
}

bool DGDB::setDelete(std::vector<std::string> args) {
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
    std::cout << "[ERROR] Invalid input!" << std::endl;
    return false;
  }

  parseNewDelete(nameA, object, repositories[0], attr_or_rel);
  return true;
}

void DGDB::parseNewDelete(std::string nameA, int object, Host host,
                          std::string attr_or_rel, size_t request_id,
                          std::vector<Host> master_repos) {
  char tamano[4];
  sprintf(tamano, "%03lu", nameA.length());
  std::string buffer;

  buffer = "D" + std::to_string(object);
  buffer += std::string(tamano) + nameA;

  if (object > 0) {
    sprintf(tamano, "%03lu", attr_or_rel.size());
    buffer += std::string(tamano) + attr_or_rel;
  }

  sprintf(tamano, "%03lu", request_id);
  buffer += std::string(tamano);

  sprintf(tamano, "%01lu", master_repos.size());
  buffer += std::string(tamano);

  for (auto& repo : master_repos) {
    std::string repo_str = repo.ip + ":" + std::to_string(repo.port);
    sprintf(tamano, "%02lu", repo_str.length());
    buffer += std::string(tamano) + repo_str;
  }

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror("error listen failed\n");
  }
}

void DGDB::parseNewResponse(Host host, std::string message, size_t request_id) {
  char tamano[4];
  sprintf(tamano, "%03lu", message.length());
  std::string buffer;

  buffer = "M" + std::string(tamano) + message;

  sprintf(tamano, "%03lu", request_id);
  buffer += std::string(tamano);

  int n = udp_socket.SendTo(host.ip, host.port, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    perror("error listen failed\n");
  }
}

void DGDB::registerRepository() {
  std::cout << "[INFO] Attached to " << ip << std::endl;
  std::string buffer;
  char vip[17];
  char vport[6];
  sprintf(vport, "%05d", port);
  vport[5] = '\0';
  sprintf(vip, "%16s", ip.c_str());
  std::string sport = vport;
  std::string sip = vip;
  buffer = "E" + sport + sip;

  int n = udp_socket.SendTo(mainIp, mainPort, buffer.c_str(), buffer.length());

  if (n > 0 && (size_t)n != buffer.length()) {
    std::cout << "Registering Repository: [" << buffer << "] failed" << std::endl;
  }
}

void DGDB::runRepository() {
  runConnection();
}

DGDB::Host DGDB::sockaddrToHost(struct sockaddr_in sock) {
  char buffer[69];
  inet_ntop(AF_INET, &(sock.sin_addr), buffer, INET_ADDRSTRLEN);
  std::string from_ip = buffer;
  int from_port = ntohs(sock.sin_port);
  return Host(from_ip, from_port);
}

DGDB::Host DGDB::stringToHost(std::string host) {
  int separator = host.find(':');
  return Host(host.substr(0, separator), stoi(host.substr(separator + 1)));
}
