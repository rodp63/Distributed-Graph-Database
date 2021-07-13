#include "RDT_UDP_Helper.h"

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

#include <iostream>

uint16_t RDT_UDP_Helper::CalculateChecksum(const std::string& data) const {
  uint16_t sum = 0;

  for (const auto& c : data) {
    // std::cout << "c - '0' = " << (c - '0') << std::endl;
    sum += (c - '0');
  }

  // std::cout << "calculated checksum = " << sum << std::endl;
  return sum;
}

std::vector<std::string> RDT_UDP_Helper::SplitData(const std::string& data,
    size_t split_size) const {
  std::vector<std::string> strings;

  for (size_t i = 0; i < data.size(); i += split_size) {
    strings.push_back(data.substr(i, split_size));
  }

  return strings;
}

std::vector<RDT_UDP_Helper::Packet> RDT_UDP_Helper::MakePackets(
  const std::string& data) {
  auto split_data = SplitData(data, kMaxPacketLength);
  std::vector<RDT_UDP_Helper::Packet> packets;
  uint32_t alternate_bit = 0;

  for (const auto& s : split_data) {
    auto checksum = CalculateChecksum(s);
    packets.push_back(Packet(checksum, s.size(), alternate_bit, s));
    alternate_bit = !alternate_bit;
    // std::cout << "Packet: " << packets.back().ToString() << std::endl;
  }

  return packets;
}

bool RDT_UDP_Helper::IsCorrupt(const Packet& packet) const {
  // std::cout << "packet.checksum = " << packet.checksum << "\n";
  // std::cout << CalculateChecksum(packet.data);
  return packet.checksum != CalculateChecksum(packet.data);
}

RDT_UDP_Helper::Packet::Packet(const std::string& packet_str) {
  std::istringstream is(packet_str);
  std::vector<std::string> packet_data;
  std::string data;

  for (int i = 0; i < 3; ++i) {
    std::getline(is, data, '\t');
    packet_data.push_back(data);
  }

  std::getline(is, data);
  packet_data.push_back(data);

  this->checksum        = stoi(packet_data[0]);
  this->len             = stoi(packet_data[1]);
  this->sequence_number = stoi(packet_data[2]);
  this->data            = packet_data[3];
}

std::string RDT_UDP_Helper::Packet::ToString() const {
  return std::to_string(checksum) + "\t" +
         std::to_string(len) + "\t" +
         std::to_string(sequence_number) + "\t" +
         data;
}

RDT_UDP_Helper::AckPacket::AckPacket(const std::string& packet_str) {
  std::istringstream is(packet_str);
  std::vector<std::string> packet_data;
  std::string data;

  for (int i = 0; i < 2; ++i) {
    std::getline(is, data, '\t');
    packet_data.push_back(data);
  }

  std::getline(is, data);
  packet_data.push_back(data);

  this->checksum   = stoi(packet_data[0]);
  this->len        = stoi(packet_data[1]);
  this->ack_number = stoi(packet_data[2]);
}

std::string RDT_UDP_Helper::AckPacket::ToString() const {
  return std::to_string(checksum) + "\t" +
         std::to_string(len) + "\t" +
         std::to_string(ack_number);
}
