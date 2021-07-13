#ifndef DGDB_RDT_RDT_H_
#define DGDB_RDT_RDT_H_

#include <vector>
#include <string>
#include <cstdint>
#include "../network/Network.h"
#include "RDT_UDP_Helper.h"

class RDT_UDP {
 public:
  explicit RDT_UDP(size_t max_packet_length=1000)
    : rdt_udp_helper(max_packet_length) {}
  void SendTo(const std::string& ip_addr, uint16_t port, const std::string& data,
             int flags=0);
  void RecvFrom(std::string* data);
  void Bind(uint16_t port);
  void Shutdown(int flags=0);
  void Close();

 private:
  Network::UDPSocket udp_socket;
  RDT_UDP_Helper rdt_udp_helper;
};

#endif  // DGDB_RDT_RDT_H_
