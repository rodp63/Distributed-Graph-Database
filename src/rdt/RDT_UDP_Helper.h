#ifndef RDT_RDTUDPHELPER_H_
#define RDT_RDTUDPHELPER_H_

#include <string>
#include <vector>

class RDT_UDP_Helper {
  struct Packet {
    uint16_t checksum;
    uint16_t len;
    uint32_t sequence_number;
    std::string data;

    Packet(uint16_t checksum, uint16_t len, uint32_t seq_no,
           const std::string& data) : checksum(checksum), len(len),
      sequence_number(seq_no), data(data) {}
    explicit Packet(const std::string& packet_str);
    bool IsCorrupt();
    std::string ToString();
  };

  struct AckPacket {
    uint16_t checksum;
    uint16_t len;
    uint32_t sequence_number;

    AckPacket(uint16_t checksum, uint16_t len, uint32_t seq_no)
      : checksum(checksum), len(len), sequence_number(seq_no) {}
    explicit AckPacket(const std::string& packet_str);
    std::string ToString();
  };

 public:
  std::vector<Packet> MakePackets(const std::string& data);

 private:
  uint16_t CalculateChecksum(const std::string& data);
  std::vector<std::string> SplitData(const std::string& data);
};

#endif  // RDT_RDTUDPHELPER_H
