#include "DMPResponse.hpp"

namespace DMP {

//====================== CREATE RESPONSE ====================//

void CreateResponse::PrintStructure() const {
}

char *CreateResponse::ParseToCharBuffer() const {
  return nullptr;
}

//====================== READ RESPONSE ====================//

void ReadResponse::PrintStructure() const {
}

char *ReadResponse::ParseToCharBuffer() const {
  return nullptr;
}

//====================== UPDATE RESPONSE ====================//

void UpdateResponse::PrintStructure() const {
}

char *UpdateResponse::ParseToCharBuffer() const {
  return nullptr;
}

//====================== DELETE RESPONSE ====================//

void DeleteResponse::PrintStructure() const {
}

char *DeleteResponse::ParseToCharBuffer() const {
  return nullptr;
}

//====================== PROCESS RESPONSE ====================//

std::shared_ptr<ServerResponse> ProcessResponse(int connectionFD) {
  return nullptr;
}
