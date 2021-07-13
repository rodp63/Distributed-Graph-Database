#include "Network.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <utility>
#include <iostream>

namespace Network {

UDPSocket::UDPSocket() {
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock == -1) {
    perror("Failed initializing socket");
    exit(1);
  }
}

int UDPSocket::GetSocketId() const {
  return sock;
}

int UDPSocket::SendTo(const std::string& ip_addr, uint16_t port,
                      const char* buffer, int len, int flags) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
  addr.sin_port = htons(port);

  int ret = sendto(sock, buffer, len, flags, reinterpret_cast<sockaddr*>(&addr),
                   sizeof(addr));

  if (ret < 0) {
    perror("Failed sending via UDPSocket.");
    close(sock);
    exit(1);
  }

  return ret;
}

std::pair<int, sockaddr_in> UDPSocket::RecvFrom(char* buffer, int len,
    int flags) {
  sockaddr_in from;
  int size = sizeof(from);
  int ret = recvfrom(sock, buffer, len, flags,
                     reinterpret_cast<sockaddr*>(&from),
                     reinterpret_cast<socklen_t*>(&size));

  if (ret < 0) {
    perror("Failed recieving via UDPSocket.");
    close(sock);
    exit(1);
  }

  buffer[ret] = '\0';
  return std::make_pair(ret, from);
}

std::pair<int, sockaddr_in> UDPSocket::RecvFromTillTimeout(std::string* s,
    size_t max_size,
    size_t timeout) {
  struct timespec ts;
  ts.tv_sec = timeout;
  ts.tv_nsec = 0;

  struct pollfd poll_fd;
  poll_fd.fd = GetSocketId();
  poll_fd.events = POLLIN;
  poll_fd.revents = 0;

  int ret = ppoll(&poll_fd, 1, &ts, NULL);

  int n;
  sockaddr_in from;
  char buffer[max_size];

  if (ret < 0) {
    perror("Error ppoll in UDPSocket::RecvFromTillTimeout.");
    n = 0;
  }
  else if (ret == 0) {
    n = 0;
  }
  else {
    if (poll_fd.revents == POLLIN) {
      auto [n_, from_] = RecvFrom(buffer, max_size);
      n = std::move(n_);
      from = std::move(from_);
      *s = std::string(buffer);
    }
    else {
      (*s).clear();
      perror("Error! revents is not POLLIN in UDPSocket::RecvFromTillTimeout.");
      n = 0;
    }
  }

  // std::cout << "Received buffer:\n" << buffer << "\nfrom RecvFromTillTimeout()" <<
            // std::endl;
  return std::make_pair(n, from);
}

void UDPSocket::Bind(uint16_t port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int ret = bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

  if (ret < 0) {
    perror("Socket binding error.");
    close(sock);
    exit(1);
  }
}

void UDPSocket::Shutdown(int flags) {
  shutdown(sock, flags);
}

void UDPSocket::Close() {
  close(sock);
}

void TCPSocket::Init() {
  if (sock != -2) {
    perror("TCPSocket already initialized with Init()."
           "Use RenewSocket() instead.");
    return;
  }

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (sock == -1) {
    perror("Failed initializing socket");
    exit(1);
  }
}

void TCPSocket::Shutdown(int flags) {
  shutdown(sock, flags);
}

void TCPSocket::Close() {
  close(sock);
}

int TCPSocket::GetSocketId() const {
  return sock;
}

void TCPSocket::RenewSocket() {
  sock = -2;
  Init();
}

int TCPSocket::Send(const char* buffer, int len, int flags) {
  int ret = send(sock, buffer, len, flags);

  if (ret < 0) {
    perror("Failed sending via TCPSocket.");
    close(sock);
    exit(1);
  }

  return ret;
}

int TCPSocket::Recv(char* buffer, int len, int flags) {
  int ret = recv(sock, buffer, len, flags);

  if (ret < 0) {
    perror("Failed recieving via TCPSocket.");
    close(sock);
    exit(1);
  }

  buffer[ret] = '\0';
  return ret;
}

void TCPSocket::Bind(uint16_t port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int ret = bind(sock, (sockaddr*)&addr, sizeof(addr));

  if (ret < 0) {
    perror("Socket binding error.");
    close(sock);
    exit(1);
  }
}

void TCPSocket::SetListenerSocket() {
  int ret = listen(sock, 10);

  if (ret < 0) {
    perror("Listen failed on TCPSocket.");
    close(sock);
    exit(1);
  }
}

int TCPSocket::AcceptConnection() {
  int new_connection = accept(sock, NULL, NULL);

  if (new_connection < 0) {
    perror("Accepting connection failed on TCPSocket.");
    close(new_connection);
  }

  return new_connection;
}

void TCPSocket::Connect(std::string ip_addr, uint16_t port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  int ret = inet_pton(AF_INET, ip_addr.c_str(), &addr.sin_addr);

  if (ret < 0) {
    perror("error: first parameter is not a valid address family");
    close(sock);
    exit(EXIT_FAILURE);
  }
  else if (ret == 0) {
    perror("Invalid IP address");
    close(sock);
    exit(EXIT_FAILURE);
  }

  ret = connect(sock, (const struct sockaddr*)&addr, sizeof(addr));

  if (ret < 0) {
    perror("connect failed");
    close(sock);
    exit(EXIT_FAILURE);
  }
}

}  // namespace Network
