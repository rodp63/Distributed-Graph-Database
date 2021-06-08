// Copyright 2021 <The Three Musketeers UCSP>
#include "DMPRequest.hpp"

namespace DMP {

//====================== CREATE REQUEST ====================//

void CreateRequest::PrintStructure() const {
}

char* CreateRequest::ParseToCharBuffer() const {
  return nullptr;
}

//====================== READ REQUEST ====================//

void ReadRequest::PrintStructure() const {
  std::cout << "kCreateRequest:\n"
    "\tquery_node_size: " << this->query_node_size << "\n"
    "\tquery_node: " << this->query_node << "\n"
    "\tdeep: " << this->deep << "\n"
    "\tleaf: " << this->leaf << "\n"
    "\tattributes: " << this->attributes << "\n"
    "\tnumber_of_conditions: " << this->number_of_conditions << "\n";

    std::cout << "Conditionals:\n";
    if (this->number_of_conditions == 0) {std::cout << "[NO CONDITIONS]";}
    for (auto &condition: conditionals) {
      std::cout <<
        "\tquery_name_attribute_size: " << condition.query_name_attribute_size << "\n"
        "\tquery_name_attribute: " << condition.query_name_attribute << "\n"
        "\toperador: " << condition.operador << "\n"
        "\tquery_value_attribute_size: " << condition.query_value_attribute_size << "\n"
        "\tquery_value_attribute: " << condition.query_value_attribute << "\n"
        "\tis_and: " << condition.is_and << "\n\n";
    }
    std::cout << std::endl;
}

char* ReadRequest::ParseToCharBuffer() const {
  return nullptr;
}

//====================== UPDATE REQUEST ====================//

void UpdateRequest::PrintStructure() const {
}

char* UpdateRequest::ParseToCharBuffer() const {
  return nullptr;
}

//====================== DELETE REQUEST ====================//

void DeleteRequest::PrintStructure() const {
}

char* DeleteRequest::ParseToCharBuffer() const {
  return nullptr;
}

//====================== PROCESS REQUEST ====================//

std::shared_ptr<ClientRequest> ProcessRequest(int connectionFD) {
  char buffer[1000] = {0};
  recv(connectionFD, buffer, 1000, 0);

  if (buffer[0] == '\0') {
    return nullptr;
  }
  char action = buffer[0];
  std::string s = buffer;
  switch (action) {
    case 'R': {
      auto read_request = std::make_shared<ReadRequest>();

      read_request->query_node_size = stoi(s.substr(1, 2));
      read_request->query_node = s.substr(3, read_request->query_node_size);

      read_request->deep = stoi(s.substr(3 + read_request->query_node_size, 1));
      read_request->leaf = stoi(s.substr(4 + read_request->query_node_size, 1));
      read_request->attributes = stoi(s.substr(5 + read_request->query_node_size, 1));
      read_request->number_of_conditions = stoi(s.substr(6 + read_request->query_node_size, 2));

      DMP::ReadRequest::Conditional tmp_conditional;
      int index = 1 + 2 + read_request->query_node_size + 1 + 1 + 1 + 2;

      for (int i = 0; i < read_request->number_of_conditions; i++) {

        tmp_conditional.query_name_attribute_size = stoi(s.substr(index, 3));
        index += 3;
        tmp_conditional.query_name_attribute = s.substr(index, tmp_conditional.query_name_attribute_size);
        index += tmp_conditional.query_name_attribute_size;

        tmp_conditional.operador = stoi(s.substr(index,1));
        index++;

        tmp_conditional.query_value_attribute_size = stoi(s.substr(index, 3));
        index += 3;
        tmp_conditional.query_value_attribute = s.substr(index, tmp_conditional.query_value_attribute_size);
        index += tmp_conditional.query_value_attribute_size;

        tmp_conditional.is_and = stoi(s.substr(index,1));
        index++;

        read_request->conditionals.push_back(tmp_conditional);
      }

      return read_request;
    }
  return nullptr;
  }
}

}  // namespace DMP
