#ifndef NETWORK_NETWORK_H_
#define NETWORK_NETWORK_H_

#include <cstdint>
#include <string>
#include <utility>
#include <netinet/in.h>

namespace Network {

class UDPSocket {
 private:
  int sock;

 public:
  UDPSocket();

  int GetSocketId() const;
  int SendTo(const std::string& ip_addr, uint16_t port,
              const char* buffer, int len, int flags=0);
  std::pair<int, sockaddr_in> RecvFrom(char* buffer, int len, int flags=0);
  void Bind(uint16_t port);
  void Shutdown(int flags=0);
  void Close();
};

class TCPSocket {
 private:
  int sock;

 public:
  TCPSocket() : sock(-2) {}
  explicit TCPSocket(int sock) : sock(sock) {}

  void Init();
  void RenewSocket();
  int GetSocketId() const;
  void Bind(uint16_t port);
  int Send(const char* buffer, int len, int flags=0);
  int Recv(char* buffer, int len, int flags=0);
  void SetListenerSocket();
  int AcceptConnection();
  void Connect(std::string ip_addr, uint16_t port);
  void Shutdown(int flags=0);
  void Close();
};

}  // namespace Network

#endif  // NETWORK_NETWORK_H_
