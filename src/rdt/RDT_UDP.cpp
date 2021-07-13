#include "RDT_UDP.h"
#include <iostream>
#include <arpa/inet.h>

const size_t kMaxAckSize = 50;
const size_t kMaxPacketHeaderSize = 14;
const size_t kMaxPacketSize = kMaxPacketHeaderSize + 10;
const size_t kTimeoutSeconds = 5;

void RDT_UDP::SendTo(const std::string& ip_addr, uint16_t port,
                     const std::string& data, int flags) {
  auto packets = rdt_udp_helper.MakePackets(data);
  int n = 0;
  sockaddr_in from;
  std::string ack_str;
  bool alternate_bit = 0;

  for (const auto& p : packets) {
    // std::cout << "Sending packet..." << std::endl;
    // std::cout << "Packet: " << p.ToString() << std::endl;
    udp_socket.SendTo(ip_addr, port, p.ToString().c_str(), p.ToString().size(),
                      flags);

    while (true) {
      // std::cout << "Waiting for AckPacket..." << std::endl;
      auto [n_, from_] = udp_socket.RecvFromTillTimeout(&ack_str, kMaxAckSize,
                         kTimeoutSeconds);
      n = std::move(n_);
      from = std::move(from_);

      if (n == 0) {
        // std::cout << "Ack Timeout. Resending packet...\n";
        // std::cout << "Packet: " << p.ToString() << std::endl;
        udp_socket.SendTo(ip_addr, port, p.ToString().c_str(), p.ToString().size(),
                          flags);
      }
      else {
        // std::cout << "Received AckPacket: " << ack_str << std::endl;
        RDT_UDP_Helper::AckPacket ack_packet(ack_str);

        if (ack_packet.ack_number == alternate_bit)
          break;
      }
    }

    alternate_bit = !alternate_bit;
  }
}

sockaddr_in RDT_UDP::RecvFrom(std::string* data) {
  RDT_UDP_Helper::AckPacket ack_packet(0, 0, 0);
  bool alternate_bit = 0;
  std::string ack_packet_str;
  std::string s_buffer;
  sockaddr_in from;
  int packet_count = 0;

  while (true) {
    // std::cout << "Waiting for packet with alternate_bit = " << alternate_bit <<
    // std::endl;
    auto [n, from_] = udp_socket.RecvFromTillTimeout(&s_buffer, kMaxPacketSize,
                      kTimeoutSeconds);
    std::string from_addr(INET_ADDRSTRLEN, 0);
    inet_ntop(AF_INET, &from_.sin_addr, &from_addr[0], INET_ADDRSTRLEN);
    uint16_t from_port = ntohs(from.sin_port);

    if (n == 0) {
      // std::cout << "Nothing was received" << std::endl;
      break;
    }

    if (packet_count == 0) {
      from.sin_addr = from_.sin_addr;
      from.sin_port = from_.sin_port;
    }

    // std::cout << "from_addr: " << from_addr << "\n"
    // "from_port: " << from_port << std::endl;

    // std::cout << "Received packet:\n";
    // std::cout << s_buffer << std::endl;

    RDT_UDP_Helper::Packet packet{s_buffer};
    ++packet_count;

    // std::cout << "packet.sequence_number = " <<  packet.sequence_number << "\n"
    // << "IsCorrupt: " << rdt_udp_helper.IsCorrupt(packet) << std::endl;
    if (packet.sequence_number == alternate_bit
        && !rdt_udp_helper.IsCorrupt(packet)) {
      (*data).append(packet.data);
      ack_packet = RDT_UDP_Helper::AckPacket{0, 0, alternate_bit};
      alternate_bit = !alternate_bit;
    }
    else {
      ack_packet = RDT_UDP_Helper::AckPacket{0, 0, !alternate_bit};
    }

    ack_packet_str = ack_packet.ToString();
    // std::cout << "Sending ACK to " << from_addr << " " << from_port << "\n";
    // std::cout << "AckPacket: " << ack_packet_str << std::endl;
    udp_socket.SendTo(from_addr, from_port,
                      ack_packet_str.c_str(), ack_packet_str.size());

    if (packet.len < 10) {
      // std::cout << "broke" << std::endl;
      break;
    }
  }

  return from;
}

void RDT_UDP::Bind(uint16_t port) {
  udp_socket.Bind(port);
}

void RDT_UDP::Shutdown(int flags) {
  udp_socket.Shutdown(flags);
}

void RDT_UDP::Close() {
  udp_socket.Close();
}
