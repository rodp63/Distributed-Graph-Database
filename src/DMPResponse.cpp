// Copyright 2021 <The Three Musketeers UCSP>
#include "DMPResponse.hpp"

namespace DMP {

//====================== CREATE RESPONSE ====================//

void CreateResponse::PrintStructure() const {
  std::cout << "kCreateResponse" << std::endl;
}

char* CreateResponse::ParseToCharBuffer() const {
  std::string parsed_structure("c");

  char* buffer = new char[parsed_structure.length() + 1];
  std::size_t length = parsed_structure.copy(buffer,
                                             parsed_structure.length(),
                                             0);
  buffer[length]='\0';

  return buffer;
}

//====================== READ RESPONSE ====================//

void ReadResponse::PrintStructure() const {
}

char* ReadResponse::ParseToCharBuffer() const {
  return nullptr;
}

//====================== UPDATE RESPONSE ====================//

void UpdateResponse::PrintStructure() const {
}

char* UpdateResponse::ParseToCharBuffer() const {
  return nullptr;
}

//====================== DELETE RESPONSE ====================//

void DeleteResponse::PrintStructure() const {
}

char* DeleteResponse::ParseToCharBuffer() const {
  return nullptr;
}

//====================== PROCESS RESPONSE ====================//

std::shared_ptr<ServerResponse> ProcessResponse(int connectionFD) {
  return nullptr;
}

}  // namespace DMP
