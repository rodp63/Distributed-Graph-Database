#ifndef RDT_RDT_H_
#define RDT_RDT_H_

#include <vector>
#include <string>
#include <cstdint>
#include "../network/Network.h"
#include "RDT_UDP_Helper.h"

class RDT_UDP {
 public:
  int SendTo(const std::string& ip_addr, uint16_t port, const char* buffer,
             int len, int flags=0);
  int RecvFrom(char* buffer, int len, int flags=0);

 private:
  Network::UDPSocket server;
  RDT_UDP_Helper rdt_udp_helper;
};

#endif  // RDT_RDT_H_
