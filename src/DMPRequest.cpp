// Copyright 2021 <The Three Musketeers UCSP>
#include "DMPRequest.hpp"
#include <string>

namespace DMP {

//====================== CREATE REQUEST ====================//

void CreateRequest::PrintStructure() const {
}

char* CreateRequest::ParseToCharBuffer() const {
  return nullptr;
}

//====================== READ REQUEST ====================//

void ReadRequest::PrintStructure() const {
}

char* ReadRequest::ParseToCharBuffer() const {
  return nullptr;
}

//====================== UPDATE REQUEST ====================//

void UpdateRequest::PrintStructure() const {
  std::cout << "kUpdateRequest" << "\n";
  std::cout << "\tNode or attribute: ";
  if (this->node_or_attribute == kAttribute) {
    std::cout << "Attribute\n";
  } else if (this->node_or_attribute == kNode) {
    std::cout << "Node\n";
  }
  std::cout << "\tQuery node size: " << this->query_node_size << "\n";
  std::cout << "\tQuery node value: " << this->query_node_value << "\n";

  if (this->node_or_attribute == kAttribute) {
    std::cout << "\tQuery attribute size: "
              << this->attribute_info->query_attribute_size << "\n";
    std::cout << "\tQuery attribute value: "
              << this->attribute_info->query_attribute_value << "\n";
    std::cout << "\tSet attribute size: "
              << this->attribute_info->set_attribute_size << "\n";
    std::cout << "\tSet attribute value: "
              << this->attribute_info->set_attribute_value << std::endl;
  } else if (this->node_or_attribute == kNode) {
    std::cout << "\tSet node size: "
              << this->node_info->set_node_size << "\n";
    std::cout << "\tSet node value: "
              << this->node_info->set_node_value << std::endl;
  }
}

char* UpdateRequest::ParseToCharBuffer() const {
  std::string parsed_structure("U");
  parsed_structure += std::to_string(node_or_attribute);
  parsed_structure += std::to_string(query_node_size);
  parsed_structure += query_node_value;

  if (this->node_or_attribute == kAttribute) {
    parsed_structure += std::to_string(query_attribute_size);
    parsed_structure += query_attribute_value;
    parsed_structure += std::to_string(set_attribute_size);
    parsed_structure += set_attribute_value;
  } else if (this->node_or_attribute == kNode) {
    parsed_structure += std::to_string(set_node_size);
    parsed_structure += set_node_value;
  }

  char* buffer = new char[parsed_structure.length() + 1];
  std::size_t length = parsed_structure.copy(buffer,
                                             parsed_structure.length(),
                                             0);
  buffer[length]='\0';
  return buffer;
}

//====================== DELETE REQUEST ====================//

void DeleteRequest::PrintStructure() const {
  std::cout << "kDeleteRequest" << "\n";
  std::cout << "\tNode or attribute or relation: ";
  if (this->node_or_attribute_or_relation == kAttribute) {
    std::cout << "Attribute\n";
  } else if (this->node_or_attribute_or_relation == kNode) {
    std::cout << "Node\n";
  } else if (this->node_or_attribute_or_relation == kRelation) {
    std::cout << "Relation\n";
  }
  std::cout << "\tQuery node size: " << this->query_node_size << "\n";
  std::cout << "\tQuery node value: " << this->query_node_value << "\n";

  if (this->node_or_attribute != kNode) {
    std::cout << "\tQuery attribute or relation size: "
              << this->attribute_or_relation_info->query_attribute_or_relation_size
              << "\n";
    std::cout << "\tQuery attribute or relation value: "
              << this->attribute_or_relation_info->query_attribute_or_relation_value
              << std::endl;
  }
}

char* DeleteRequest::ParseToCharBuffer() const {
  std::string parsed_structure("D");
  parsed_structure += std::to_string(node_or_attribute_or_relation);
  parsed_structure += std::to_string(query_node_size);
  parsed_structure += query_node_value;

  if (this->node_or_attribute != kNode) {
    parsed_structure += std::to_string(query_attribute_or_relation_size);
    parsed_structure += query_attribute_or_relation_value;
  }

  char* buffer = new char[parsed_structure.length() + 1];
  std::size_t length = parsed_structure.copy(buffer,
                                             parsed_structure.length(),
                                             0);
  buffer[length]='\0';
  return buffer;
}

//====================== PROCESS REQUEST ====================//

std::shared_ptr<ClientRequest> ProcessRequest(int connectionFD) {
  return nullptr;
}

}  // namespace DMP
