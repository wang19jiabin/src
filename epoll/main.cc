#include "connection.h"
#include "poller.h"
#include "queue.h"
#include <cassert>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <sys/epoll.h>
#include <thread>
#include <unistd.h>

auto queue = Queue::create();

namespace {

void nonblock(int fd) {
  int fl = fcntl(fd, F_GETFL);
  assert(fl != -1);
  int rv = fcntl(fd, F_SETFL, fl | O_NONBLOCK);
  assert(rv == 0);
}

void ignore(int sig) {
  struct sigaction sa = {};
  sa.sa_handler = SIG_IGN;
  int rv = sigemptyset(&sa.sa_mask);
  assert(rv == 0);
  rv = sigaction(sig, &sa, nullptr);
  assert(rv == 0);
}

int bind(const char *port) {
  addrinfo *ais, hints = {};
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int rv = getaddrinfo(nullptr, port, &hints, &ais);
  assert(rv == 0);

  for (addrinfo *ai = ais; ai; ai = ai->ai_next) {
    int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (fd < 0) {
      perror("socket");
      continue;
    }

    int val = 1;
    rv = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    assert(rv == 0);

    if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
      freeaddrinfo(ais);
      return fd;
    }

    perror("bind");
    close(fd);
  }

  assert(0);
}
} // namespace

int main() {
  ignore(SIGPIPE);

  int fd = bind("1116");
  int rv = listen(fd, 128);
  assert(rv == 0);
  nonblock(fd);

  auto work = [] {
    while (true) {
      auto task = queue->wait();
      task->string += "\n";
      auto conn = task->connection.lock();
      if (conn)
        conn->reply(task);
    }
  };

  for (int i = 0; i < 4; ++i)
    new std::thread(work);

  auto poll = [](int fd) {
    auto poller = Poller::create(fd);
    poller->poll();
  };

  for (int i = 0; i < 4; ++i)
    new std::thread(poll, fd);

  return pause();
}
