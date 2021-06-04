// Copyright 2021 <The Three Musketeers UCSP>
#include "DMPRequest.hpp"
#include <string>

namespace DMP {

//====================== CREATE REQUEST ====================//

void CreateRequest::PrintStructure() const {
  std::cout << "kCreateRequest:\n"
    "\tname_node_size: " << this->name_node_size << "\n"
    "\tname_node: " << this->name_node << "\n"
    "\tnumber_of_atributes: " << this->number_of_attributes << "\n"
    "\tnumber_of_relations: " << this->number_of_relations << std::endl;
}

char* CreateRequest::ParseToCharBuffer() const {
  std::string parsed_structure("C");

  parsed_structure += std::to_string(name_node_size);
  parsed_structure += std::string(name_node);
  parsed_structure += std::to_string(number_of_attributes);
  parsed_structure += std::to_string(number_of_relations);

  char* buffer = new char[parsed_structure.length() + 1];
  std::size_t length = parsed_structure.copy(buffer,
                                             parsed_structure.length(),
                                             0);
  buffer[length]='\0';

  return buffer;
}

//====================== READ REQUEST ====================//

void ReadRequest::PrintStructure() const {
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
    return nullptr;
}

}  // namespace DMP
