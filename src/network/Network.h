#ifndef NETWORK_NETWORK_H_
#define NETWORK_NETWORK_H_

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <deque>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>

class UDPSocket {
 private:
  int sock;

 public:
  UDPSocket();

  void SendTo(const std::string& ip_addr, unsigned short int port,
              const char* buffer, int len, int flags = 0);
  int RecvFrom(char* buffer, int len, int flags = 0);
  void Bind(unsigned short int port);
};

class TCPSocket {
 private:
  int sock;

 public:
  TCPSocket() : sock(-2) {}
  TCPSocket(int sock) : sock(sock) {}

  void Init();
  void RenewSocket();
  int GetSocketId();
  void Bind(unsigned short int port);
  int Send(const char* buffer, int len, int flags = 0);
  int Recv(char* buffer, int len, int flags = 0);
  void SetListenerSocket();
  int AcceptConnection();
  void Connect(std::string ip, unsigned short int port);
  void Shutdown(int flags = 0);
  void Close();
};

#endif  // NETWORK_NETWORK_H_
