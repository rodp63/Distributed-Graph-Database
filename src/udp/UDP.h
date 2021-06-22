#ifndef UDP_UDP_H_
#define UDP_UDP_H_

#include <vector>
#include <deque>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>

const int MAX_SIZE = 2048;

class UDP_Server {
 public:
  std::vector<sockaddr> clients;
  std::vector<std::deque<std::string>> wait_messages;
  int socket_fd;

  void Listen() {
    std::thread(this->ActivateListener).detach();
  }

  void RegisterClient(sockaddr client_addr) {
  }

  static void ActivateListener() {
  }

  void Send(int client_id, std::string message) {
  }

  void Read() {
  }
};

class UDP_Client {
  struct sockaddr_in server_addr;
  int socket_fd;
  int client_id;

  void RegisterClient(sockaddr client_addr) {
  }

  void Send(int client_id, std::string message) {
  }

  void Read() {
  }
};

#endif  // UDP_UDP_H_
