#ifndef DISTRIBUTED_GRAPH_DATABASE_DMP_RESPONSE_H_
#define DISTRIBUTED_GRAPH_DATABASE_DMP_RESPONSE_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace DMP {

enum ResponseType {
  kCreateResponse,
  kReadResponse,
  kUpdateResponse,
  kDeleteResponse,
};

struct ServerResponse {
  char accion;

  ServerResponse(char action, ResponseType type) : accion(action), _type(type) {}

  virtual ~ServerResponse() {};

  ResponseType type() const {
    return _type;
  };

  virtual void PrintStructure() const {};
  virtual char* ParseToCharBuffer() const {
    return nullptr;
  };

 private:
  ResponseType _type;
};

struct CreateResponse : ServerResponse {

  CreateResponse() : ServerResponse('c', kCreateResponse) {}
  ~CreateResponse() {}

  void PrintStructure() const override;
  char *ParseToCharBuffer() const override;

};

struct ReadResponse : ServerResponse {

  ReadResponse() : ServerResponse('r', kReadResponse) {}
  ~ReadResponse() {}

  void PrintStructure() const override;
  char *ParseToCharBuffer() const override;

};

struct UpdateResponse : ServerResponse {

  UpdateResponse() : ServerResponse('u', kUpdateResponse) {}
  ~UpdateResponse() {}

  void PrintStructure() const override;
  char *ParseToCharBuffer() const override;

};

struct DeleteResponse : ServerResponse {

  DeleteResponse(): ServerResponse('d', kDeleteResponse) {}
  ~DeleteResponse() {}

  void PrintStructure() const override;
  char *ParseToCharBuffer() const override;
};

std::shared_ptr<ServerResponse> ProcessResponse(int connectionFD);

}

#endif // DISTRIBUTED_GRAPH_DATABASE_DMP_RESPONSE_H_