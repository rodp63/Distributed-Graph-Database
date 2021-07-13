#include <string>
#include "DGDB.h"
#include "tools.h"
#include "thirdparty/sqlite_orm/sqlite_orm.h"
#include "rdt/RDT_UDP.h"

namespace sq = sqlite_orm;

void DGDB::WaitResponse() {
  std::string message;

  while (true) {
    rdt_udp_socket.RecvFrom(&message);

    size_t pos;

    if (message[0] == 'M') {
      while ((pos = message.find("\\n")) != std::string::npos) {
        message.replace(pos, 2, "\n");
      }

      std::cout << message.substr(4, message.size() - 7) << std::endl;
      break;
    }
  }
}

void DGDB::runConnection() {
  int s, ss, request_id, repo_count;
  std::string s_buffer;
  std::string bufferAux;
  std::string node_name;

  while (server || repository) {
    s_buffer.clear();
    sockaddr_in from_addr = rdt_udp_socket.RecvFrom(&s_buffer);
    //std::cout << "From application: " << s_buffer << std::endl;

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
      parseNewMessageResponse(client, bufferAux, request_id);
      pending.erase(request_id);
    }

    else if (s_buffer[0] == 'I') {
      s_buffer.erase(0, 1);
      std::vector<Attribute> attributes;
      std::vector<std::string> relations;
      int cantidad, valid_node;
      std::cout << "------------------\nAction: I" << std::endl;

      s = stoi(s_buffer.substr(0, 3)) + 2;
      s_buffer.erase(0, 3);

      bufferAux = s_buffer[s - 2];
      cantidad = stoi(bufferAux);

      while (cantidad--) {
        ss = stoi(s_buffer.substr(s, 3));
        s_buffer.erase(s, 3);

        relations.push_back(s_buffer.substr(s, ss));
        s_buffer.erase(s, ss);
      }

      bufferAux = s_buffer[s - 1];
      cantidad = stoi(bufferAux);

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

      request_id = stoi(s_buffer.substr(s, 3));
      s_buffer.erase(s, 3);

      valid_node = stoi(s_buffer.substr(s, 1));
      s_buffer.erase(s, 1);

      node_name = s_buffer.substr(0, s - 2);

      auto top_node = pending[request_id].state->node_queue.front();

      if (top_node.first == node_name) {
        pending[request_id].state->node_queue.pop();

        if (valid_node) {
          std::string& response = pending[request_id].state->response;
          response += node_name;
          response += " [Depth = ";
          response += std::to_string(pending[request_id].state->depth - top_node.second);
          response += +"]\\n";

          if (pending[request_id].state->attr) {
            for (auto& attr : attributes) {
              response += "  " + attr.key;
              response += ": " + attr.value + "\\n";
            }
          }
        }

        for (std::string& rel_node : relations) {
          std::set<std::string>& visited = pending[request_id].state->visited;

          if (visited.find(rel_node) == visited.end()) {
            if (top_node.second > 0) {
              pending[request_id].state->node_queue.emplace(rel_node,
                  top_node.second - 1);
              visited.insert(rel_node);
            }
          }
        }

        if (pending[request_id].state->node_queue.empty()) {
          Host client = sockaddrToHost(pending[request_id].client_sock);
          bufferAux = "OK! Query complete\\n" + pending[request_id].state->response;
          parseNewMessageResponse(client, bufferAux, request_id);
          cud_blocking = false;
          pending.erase(request_id);
        }
        else {
          std::string next_node = pending[request_id].state->node_queue.front().first;
          int r = (next_node[0] % (repositories.size() - 1)) + 1;
          parseNewQuery(next_node,
                        pending[request_id].state->node_queue.front().second,
                        pending[request_id].state->leaf,
                        pending[request_id].state->attr,
                        repositories[r],
                        pending[request_id].state->conditions,
                        request_id);
        }
      }
      else {
        Host client = sockaddrToHost(pending[request_id].client_sock);
        bufferAux = "[ERROR] Internal Error";
        parseNewMessageResponse(client, bufferAux, request_id);
        pending.erase(request_id);
        cud_blocking = false;
      }
    }

    else if (s_buffer[0] == 'C') {
      if (cud_blocking) {
        std::string err = "[ERROR] Bussy Server. Try again later";
        std::cout << err << std::endl;
        parseNewMessageResponse(sockaddrToHost(from_addr), err);
        continue;
      }

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
          parseNewMessageResponse(sockaddrToHost(from_addr), err, request_id);
          continue;
        }
        else {
          r = (s_buffer[0] % (repositories.size() - 1)) + 1;
        }

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
            message = "[WARNING] Node already exists. Skipping\\n";
            std::cout << message;
          }
        }
        catch (std::system_error e) {
          message = std::string("[ERROR] ") + e.what();
          std::cout << message << std::endl;
          parseNewMessageResponse(sockaddrToHost(from_addr), message, request_id);
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
            parseNewMessageResponse(sockaddrToHost(from_addr), message, request_id);
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
          parseNewMessageResponse(repositories[0], message, request_id);

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
        int r;

        if (repositories.size() < 2) {
          std::string err = "[ERROR] Repository has not been attached";
          std::cout << err << std::endl;
          cud_blocking = false;
          parseNewMessageResponse(sockaddrToHost(from_addr), err, request_id);
          continue;
        }
        else {
          r = (s_buffer[0] % (repositories.size() - 1)) + 1;
        }

        std::cout << "[INFO] Sending: (" << node_name << ")" << std::endl;

        Pending c_pending;
        c_pending.client_sock = from_addr;
        c_pending.state = new QueryState(leaf, attr, depth);
        c_pending.state->visited.insert(node_name);
        c_pending.state->node_queue.emplace(node_name, depth);
        c_pending.state->conditions = conditions;

        pending[current_request] = c_pending;

        cud_blocking = true;
        parseNewQuery(node_name, depth, leaf, attr, repositories[r],
                      conditions, current_request);
        ++current_request;
      }
      else if (repository) {
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

        auto attributes = storage.get_all<Attribute>(sq::where(
                            sq::c(&Attribute::node_name) = node_name));
        auto db_rels = storage.get_all<Relation>(sq::where(
                         sq::c(&Relation::node_name1) = node_name));

        std::vector<std::string> relations;

        for (auto& rel : db_rels)
          relations.push_back(rel.node_name2);

        bool valid_node = (depth == 0 || !leaf);
        std::string next_lop = "&";

        for (auto& condition : conditions) {
          std::string& key = condition.key;
          std::string& value = condition.value;
          bool current = true;

          auto it = attributes.begin();

          for (; it != attributes.end(); ++it)
            if (it->key == key) break;

          if (it == attributes.end())
            current = false;
          else {
            if (condition.op == Condition::Operator::EQUAL)
              current = (it->value == value);
            else if (condition.op == Condition::Operator::LESS)
              current = (it->value < value);
            else if (condition.op == Condition::Operator::GREATER)
              current = (it->value > value);
            else if (condition.op == Condition::Operator::LIKE)
              current = (value.find(it->value) != std::string::npos);
          }

          if (next_lop == "&")
            valid_node = valid_node && current;
          else
            valid_node = valid_node || current;

          next_lop = condition.is_or_to_string();;
        }

        parseNewInfoResponse(node_name, repositories[0], attributes, relations,
                             request_id, valid_node);
      }
    }

    else if (s_buffer[0] == 'U') {
      if (cud_blocking) {
        std::string err = "[ERROR] Bussy Server. Try again later";
        std::cout << err << std::endl;
        parseNewMessageResponse(sockaddrToHost(from_addr), err);
        continue;
      }

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
          parseNewMessageResponse(sockaddrToHost(from_addr), err, request_id);
          continue;
        }
        else
          r = s_buffer[0] % (repositories.size() - 1);

        r++;

        std::cout << "[INFO] Sending: (" << node_name << ")" << std::endl;

        Pending c_pending;
        c_pending.client_sock = from_addr;
        pending[current_request] = c_pending;

        parseNewUpdate(node_name, is_node, set_value, repositories[r], attr,
                       current_request);
        ++current_request;
      }
      else if (repository) {
        std::string message;
        std::cout << "Update Struct:\n";
        std::cout << "  Node: " << node_name << "\n";

        if (is_node) {
          // TODO: storage.update(new_node); -> try catch
          // TODO: update all repos with related relations
          std::cout << "  New name: " << set_value << "\n";
        }
        else {
          // TODO: storage.update(new_attribute); -> try catch
          std::cout << "  Attribute: " << attr << "\n";
          std::cout << "  New value: " << set_value << "\n";
        }

        message = "OK! Node updated successfully";
        parseNewMessageResponse(repositories[0], message, request_id);
      }
    }

    else if (s_buffer[0] == 'D') {
      if (cud_blocking) {
        std::string err = "[ERROR] Bussy Server. Try again later";
        std::cout << err << std::endl;
        parseNewMessageResponse(sockaddrToHost(from_addr), err);
        continue;
      }

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
          parseNewMessageResponse(sockaddrToHost(from_addr), err, request_id);
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
          parseNewMessageResponse(repositories[0], message, request_id);
      }
    }

    else if (s_buffer[0] == 'E') {
      std::cout << "------------------\nAction: E" << std::endl;
      s_buffer.erase(0, 1);

      uint16_t vPort = ntohs(from_addr.sin_port);
      char sIp[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &from_addr.sin_addr, sIp, INET_ADDRSTRLEN);
      std::string vIp(sIp);

      connMasterRepository(vIp, vPort);

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
  rdt_udp_socket.Shutdown(SHUT_RDWR);
  rdt_udp_socket.Close();
}

void DGDB::setServer() {
  rdt_udp_socket.Bind(port);
  server = 1;
}

void DGDB::closeServer() {
  server = 0;
  rdt_udp_socket.Close();
}

void DGDB::setRepository() {
  rdt_udp_socket.Bind(port);

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

  rdt_udp_socket.SendTo(host.ip, host.port, buffer);
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

  rdt_udp_socket.SendTo(host.ip, host.port, buffer);
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

  rdt_udp_socket.SendTo(host.ip, host.port, buffer);
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

  rdt_udp_socket.SendTo(host.ip, host.port, buffer);
}

void DGDB::parseNewMessageResponse(Host host, std::string message,
                                   size_t request_id) {
  char tamano[4];
  sprintf(tamano, "%03lu", message.length());
  std::string buffer;

  buffer = "M" + std::string(tamano) + message;

  sprintf(tamano, "%03lu", request_id);
  buffer += std::string(tamano);

  rdt_udp_socket.SendTo(host.ip, host.port, buffer);
}

void DGDB::parseNewInfoResponse(std::string nameA, Host host,
                                std::vector<Attribute> attributes,
                                std::vector<std::string> relations,
                                size_t request_id, bool valid_node) {
  char tamano[4];
  sprintf(tamano, "%03lu", nameA.length());
  std::string buffer;

  buffer = "I" + std::string(tamano) + nameA;
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

  sprintf(tamano, "%01lu", (size_t)valid_node);
  buffer += std::string(tamano);

  rdt_udp_socket.SendTo(host.ip, host.port, buffer);
}

void DGDB::registerRepository() {
  std::cout << "[INFO] Attached to " << ip << std::endl;
  std::string buffer;
  buffer = "E";
  std::cout << "*" << buffer.c_str() << "*" << std::endl;
  rdt_udp_socket.SendTo(mainIp, mainPort, buffer);
}

void DGDB::runRepository() {
  runConnection();
}

void DGDB::connMasterRepository(std::string pIp, uint16_t pPort) {
  std::cout << pIp << "-" << pPort << std::endl;
  repository = 1;
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
