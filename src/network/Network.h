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

const int kMaxSize = 2048;

class UDPSocket {
 private:
  int sock;

 public:
  UDPSocket() {
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock == -1) {
      perror("Failed initializing socket");
      exit(1);
    }
  }

  ~UDPSocket() {
    close(sock);
  }

  void SendTo(const std::string& ip_addr, int port, const char* buffer, int len,
            int flags = 0) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
    addr.sin_port = htons(port);

    int ret = send(sock, buffer, len, flags);

    if (ret < 0) {
      perror("Failed sending via UDPSocket.");
      close(sock);
      exit(1);
    }
  }

  int RecvFrom(char* buffer, int len, int flags = 0) {
    sockaddr_in from;
    int size = sizeof(from);
    int ret = recvfrom(sock, buffer, len, flags, (sockaddr*)&from,
                       (socklen_t*)&size);

    if (ret < 0) {
      perror("Failed recieving via UDPSocket.");
      close(sock);
      exit(1);
    }

    buffer[ret] = '\0';
    return ret;
  }

  void Bind(int port) {
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
};

class TCPSocket {
 private:
  int sock;

 public:
  TCPSocket() {
  }

  TCPSocket(int sock) : sock(sock) {}

  void Init() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == -1) {
      perror("Failed initializing socket");
      exit(1);
    }
  }

  int GetSocketId() {
    return sock;
  }

  void RenewSocket() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == -1) {
      perror("Failed initializing socket");
      exit(1);
    }
  }

  int Send(const char* buffer, int len, int flags = 0) {
    int ret = send(sock, buffer, len, flags);

    if (ret < 0) {
      perror("Failed sending via TCPSocket.");
      close(sock);
      exit(1);
    }

    return ret;
  }

  int Recv(char* buffer, int len, int flags = 0) {
    sockaddr_in from;
    int size = sizeof(from);
    int ret = recvfrom(sock, buffer, len, flags, (sockaddr*)&from,
                       (socklen_t*)&size);

    if (ret < 0) {
      perror("Failed recieving via TCPSocket.");
      close(sock);
      exit(1);
    }

    buffer[ret] = '\0';
    return ret;
  }

  void Bind(int port) {
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

  void SetListenerSocket() {
    int ret = listen(sock, 10);

    if (ret < 0) {
      perror("Listen failed on TCPSocket.");
      close(sock);
      exit(1);
    }
  }

  int AcceptConnection() {
    int new_connection = accept(sock, NULL, NULL);

    if (new_connection < 0) {
      perror("Accepting connection failed on TCPSocket.");
      close(new_connection);
    }

    return new_connection;
  }

  void Connect(std::string ip, int port) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int ret = inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

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
};

#endif  // NETWORK_NETWORK_H_
