#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

void nonblock(int fd) {
  auto fl = fcntl(fd, F_GETFL);
  assert(fl != -1);
  auto rv = fcntl(fd, F_SETFL, fl | O_NONBLOCK);
  assert(rv == 0);
}

int socket(const char *port) {
  addrinfo *ais, hints = {AI_PASSIVE, AF_UNSPEC, SOCK_STREAM};
  auto rv = getaddrinfo(nullptr, port, &hints, &ais);
  assert(rv == 0);

  for (addrinfo *ai = ais; ai; ai = ai->ai_next) {
    auto fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
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
