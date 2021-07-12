#include "RDT_UDP_Helper.h"

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

uint16_t RDT_UDP_Helper::CalculateChecksum(const std::string& data) const {
  uint16_t sum = 0;

  for (const auto& c : data) {
    sum += (c - '0');
  }

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
  uint32_t alternate = 0;

  for (const auto& s : split_data) {
    auto checksum = CalculateChecksum(s);
    packets.push_back(Packet(checksum, s.size(), alternate, s));
    alternate = !alternate;
  }

  return packets;
}

bool RDT_UDP_Helper::IsCorrupt(const Packet& packet) const {
  return packet.checksum != CalculateChecksum(packet.data);
}

RDT_UDP_Helper::Packet::Packet(const std::string& packet_str) {
  std::istringstream is(packet_str);
  std::vector<std::string> packet_data;
  std::string data;

  for (int i = 0; i < 3; ++i) {
    std::getline(is, data, '\n');
    packet_data.push_back(data);
  }

  std::getline(is, data, '*');
  packet_data.push_back(data);

  this->checksum        = stoi(packet_data[0]);
  this->len             = stoi(packet_data[1]);
  this->sequence_number = stoi(packet_data[2]);
  this->data            = packet_data[3];
}

std::string RDT_UDP_Helper::Packet::ToString() const {
  return std::to_string(checksum) + "\n" +
         std::to_string(len) + "\n" +
         std::to_string(sequence_number) + "\n" +
         data;
}

RDT_UDP_Helper::AckPacket::AckPacket(const std::string& packet_str) {
  std::istringstream is(packet_str);
  std::vector<std::string> packet_data;
  std::string data;

  for (int i = 0; i < 2; ++i) {
    std::getline(is, data, '\n');
    packet_data.push_back(data);
  }

  std::getline(is, data, '*');
  packet_data.push_back(data);

  this->checksum        = stoi(packet_data[0]);
  this->len             = stoi(packet_data[1]);
  this->sequence_number = stoi(packet_data[2]);
}

std::string RDT_UDP_Helper::AckPacket::ToString() const {
  return std::to_string(checksum) + "\n" +
         std::to_string(len) + "\n" +
         std::to_string(sequence_number);
}
