#include "poller.h"
#include "connection.h"
#include <cassert>
#include <map>
#include <sys/epoll.h>
#include <sys/socket.h>

class PollerImpl : public Poller {
public:
  PollerImpl(int);
  void poll() override;

private:
  void accept();
  void io(int, uint32_t);

private:
  int _sfd;
  int _efd;
  std::map<int, std::shared_ptr<Connection>> _connections;
};

std::unique_ptr<Poller> Poller::create(int sfd) {
  return std::make_unique<PollerImpl>(sfd);
}

PollerImpl::PollerImpl(int sfd) : _sfd(sfd) {
  _efd = epoll_create1(0);
  assert(_efd != -1);
  epoll_event ev = {EPOLLIN};
  ev.data.fd = _sfd;
  int rv = epoll_ctl(_efd, EPOLL_CTL_ADD, _sfd, &ev);
  assert(rv == 0);
}

void PollerImpl::poll() {
  epoll_event evs[128];
  while (true) {
    int n = epoll_wait(_efd, evs, sizeof(evs) / sizeof(evs[0]), -1);
    if (n < 0) {
      perror("epoll_wait");
      continue;
    }

    for (int i = 0; i < n; ++i) {
      const auto &ev = evs[i];
      if (ev.data.fd == _sfd)
        accept();
      else
        io(ev.data.fd, ev.events);
    }
  }
}

void PollerImpl::accept() {
  int fd;
  epoll_event ev = {EPOLLIN | EPOLLOUT | EPOLLET};

  while ((fd = accept4(_sfd, nullptr, nullptr, SOCK_NONBLOCK)) != -1) {
    _connections[fd] = Connection::create(fd, _efd);
    ev.data.fd = fd;
    int rv = epoll_ctl(_efd, EPOLL_CTL_ADD, fd, &ev);
    assert(rv == 0);
  }

  assert(errno == EAGAIN);
}

void PollerImpl::io(int fd, uint32_t evs) {
  auto conn = _connections.at(fd);

  if (evs & EPOLLIN && !conn->read()) {
    _connections.erase(fd);
    return;
  }

  if (evs & EPOLLOUT && !conn->write())
    _connections.erase(fd);
}
