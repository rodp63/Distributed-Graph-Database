#ifndef DGDB_RDT_RDTUDPHELPER_H_
#define DGDB_RDT_RDTUDPHELPER_H_

#include <string>
#include <vector>

class RDT_UDP_Helper {
  struct Packet {
    uint16_t checksum;
    uint16_t len;
    uint32_t sequence_number;
    std::string data;

    Packet(uint16_t checksum, uint16_t len, uint32_t seq_no,
           const std::string& data)
      : checksum(checksum), len(len), sequence_number(seq_no), data(data) {}
    explicit Packet(const std::string& packet_str);
    std::string ToString() const;
  };

  struct AckPacket {
    uint16_t checksum;
    uint16_t len;
    uint32_t ack_number;

    AckPacket(uint16_t checksum, uint16_t len, uint32_t ack_no)
      : checksum(checksum), len(len), ack_number(ack_no) {}
    explicit AckPacket(const std::string& packet_str);
    std::string ToString() const;
  };

 public:
  const size_t kMaxPacketLength;

  explicit RDT_UDP_Helper(size_t max_packet_length)
    : kMaxPacketLength(max_packet_length) {}
  std::vector<Packet> MakePackets(const std::string& data);

 private:
  uint16_t CalculateChecksum(const std::string& data) const;
  std::vector<std::string> SplitData(const std::string& data,
                                     size_t split_size) const;
  bool IsCorrupt(const Packet& packet) const;
};

#endif  // DGDB_RDT_RDTUDPHELPER_H
