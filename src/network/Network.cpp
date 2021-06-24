#include "Network.h"

UDPSocket::UDPSocket() {
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock == -1) {
    perror("Failed initializing socket");
    exit(1);
  }
}

void UDPSocket::SendTo(const std::string& ip_addr, unsigned short int port,
                       const char* buffer, int len, int flags) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
  addr.sin_port = htons(port);

  int ret = sendto(sock, buffer, len, flags, (sockaddr*)&addr, sizeof(addr));

  if (ret < 0) {
    perror("Failed sending via UDPSocket.");
    close(sock);
    exit(1);
  }
}

int UDPSocket::RecvFrom(char* buffer, int len, int flags) {
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

void UDPSocket::Bind(unsigned short int port) {
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

void TCPSocket::Init() {
  if (sock != -2) {
    perror("TCPSocket already initialized with Init(). Use RenewSocket() instead.");
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

int TCPSocket::GetSocketId() {
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

void TCPSocket::Bind(unsigned short int port) {
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

void TCPSocket::Connect(std::string ip, unsigned short int port) {
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
