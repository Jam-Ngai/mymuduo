#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <thread>

class Client {
 public:
  Client();
  ~Client() { ::close(clientfd_); }

  bool Connet(const std::string &peerip, const uint16_t &peerport);
  bool Send(const std::string &buffer);
  bool Recv(std::string &buffer, const size_t &maxlen);
  const int &clientfd() const { return clientfd_; }

 private:
  int clientfd_;
};

Client::Client() {
  if ((clientfd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                            0)) < 0) {
    perror("Crearte client socket error!");
    exit(-1);
  }
}

bool Client::Connet(const std::string &peerip, const uint16_t &peerport) {
  sockaddr_in peeraddr;
  std::memset(&peeraddr, 0, sizeof peeraddr);
  peeraddr.sin_family = AF_INET;
  peeraddr.sin_port = htons(peerport);
  inet_aton(peerip.c_str(), &peeraddr.sin_addr);
  if (::connect(clientfd_, (sockaddr *)&peeraddr, sizeof peeraddr) < 0) {
    if (errno != EINPROGRESS) {
      perror("Connect failed!");
      return false;
    }
  }
  int epollfd = epoll_create1(EPOLL_CLOEXEC);
  epoll_event event;
  event.events = EPOLLOUT;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd_, &event);
  epoll_event revents;
  std::memset(&revents, 0, sizeof revents);
  epoll_wait(epollfd, &revents, 16, 10000);
  if (revents.events == EPOLLOUT) {
    std::cout << "Connected!" << std::endl;
    return true;
  }
  std::cout << "Connected failed!" << std::endl;
  return false;
}

bool Client::Send(const std::string &buffer) {
  if (::send(clientfd_, buffer.data(), buffer.size(), 0) < 0) return false;
  return true;
}

bool Client::Recv(std::string &buffer, const size_t &maxlen) {
  buffer.clear();
  buffer.resize(maxlen);
  pollfd fds;
  fds.fd = clientfd_;
  fds.events = POLLIN;
  if (poll(&fds, 1, -1) < 0) {
    return false;
  } else {
    if (fds.revents == POLLIN) {
      int datasize = ::recv(clientfd_, buffer.data(), maxlen, 0);
      if (datasize <= 0) {
        buffer.clear();
        return false;
      }
      buffer.resize(datasize);
      return true;
    }
    return false;
  }
}

int main(int argc, char *argv[]) {
  Client client;
  if (argc != 3) {
    std::cout << "Usage: prog ip port" << std::endl;
    exit(-1);
  }
  if (!client.Connet(argv[1], atoi(argv[2]))) exit(-1);
  if (!client.Send(std::to_string(client.clientfd()) + ": hello world.")) {
    perror("Send failed!");
    exit(-1);
  }
  std::string buf;
  if (!client.Recv(buf, 1024)) {
    perror("Receive failed!");
    exit(-1);
  } else {
    std::cout << "Receive: " << buf << std::endl;
  }
  return 0;
}