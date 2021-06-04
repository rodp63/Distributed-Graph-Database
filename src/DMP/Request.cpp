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
