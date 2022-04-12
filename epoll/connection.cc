#include "connection.h"
#include "queue.h"
#include <algorithm>
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>

extern std::unique_ptr<Queue> queue;

class ConnectionImpl : public Connection,
                       public std::enable_shared_from_this<ConnectionImpl> {
public:
  ConnectionImpl(int, int);
  ~ConnectionImpl();
  bool read() override;
  bool write() override;
  void reply(std::shared_ptr<Task>) override;

private:
  int _cfd;
  int _efd;
  std::string _reading;
  std::unique_ptr<Queue> _queue;
  std::shared_ptr<Task> _sending;
};

std::shared_ptr<Connection> Connection::create(int cfd, int efd) {
  return std::make_shared<ConnectionImpl>(cfd, efd);
}

ConnectionImpl::ConnectionImpl(int cfd, int efd) : _cfd(cfd), _efd(efd) {
  _queue = Queue::create();
}

ConnectionImpl::~ConnectionImpl() { close(_cfd); }

bool ConnectionImpl::read() {
  char buf[7];
  ssize_t n;

  while ((n = ::read(_cfd, buf, sizeof(buf))) > 0) {
    char *b = buf, *e;
    while ((e = std::find(b, buf + n, 0)) != buf + n) {
      _reading.append(b, e - b);
      auto task = std::make_shared<Task>();
      task->string = std::move(_reading);
      task->connection = shared_from_this();
      queue->push(task);
      _reading.clear();
      b = e + 1;
    }

    _reading.append(b, e - b);
  }

  if (n == -1 && errno == EAGAIN)
    return true;

  return false;
}

bool ConnectionImpl::write() {
  while (_sending || (_sending = _queue->pop())) {
    size_t len = _sending->string.size() - _sending->sent;
    ssize_t n = ::write(_cfd, _sending->string.c_str() + _sending->sent, len);
    if (n < 0) {
      return errno == EAGAIN;
    }

    if (n == len) {
      _sending.reset();
    } else {
      _sending->sent += n;
    }
  }

  return true;
}

void ConnectionImpl::reply(std::shared_ptr<Task> t) {
  _queue->push(t);
  epoll_event ev = {};
  ev.data.fd = _cfd;
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
  int rv = epoll_ctl(_efd, EPOLL_CTL_MOD, _cfd, &ev);
  assert(!rv);
}
