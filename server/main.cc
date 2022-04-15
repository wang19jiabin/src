#include "poller.h"
#include "queue.h"
#include "socket.h"
#include "worker.h"
#include <cassert>
#include <csignal>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

auto queue = Queue::create();

void init() {
  struct sigaction sa = {SIG_IGN};
  auto rv = sigemptyset(&sa.sa_mask);
  assert(rv == 0);
  rv = sigaction(SIGPIPE, &sa, nullptr);
  assert(rv == 0);
}

int main() {
  init();

  auto fd = socket("1116");
  auto rv = listen(fd, 128);
  assert(rv == 0);
  nonblock(fd);

  for (int i = 0; i < 4; ++i) {
    new std::thread([] {
      auto worker = Worker::create();
      worker->work();
    });
  }

  for (int i = 0; i < 4; ++i) {
    new std::thread([fd] {
      auto poller = Poller::create(fd);
      poller->poll();
    });
  }

  return pause();
}
