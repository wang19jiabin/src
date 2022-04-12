#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <stdio.h>
#include <assert.h>

void Send(int fd)
{
	const char resp[] = "HTTP/1.0 200 OK\r\n\r\ntest";
	ssize_t n = write(fd, resp, sizeof(resp) - 1);
	printf("write %ld\n", n);
}

void Read(int fd)
{
	char buf[4096];
	ssize_t n = read(fd, buf, sizeof(buf));
	if (n < 0) {
		perror("read");
	} else if (n == 0) {
		printf("EOF\n");
	} else {
		write(STDOUT_FILENO, buf, n);
		Send(fd);
	}

	close(fd);
}

void Accept(int fd, int efd)
{
	struct epoll_event event = {.events = EPOLLIN };
	if ((event.data.fd = accept(fd, NULL, NULL)) == -1) {
		perror("accept");
		return;
	}

	int ret = epoll_ctl(efd, EPOLL_CTL_ADD, event.data.fd, &event);
	assert(ret == 0);
	printf("accept %d\n", event.data.fd);
}

int Listen(const char *port)
{
	struct addrinfo *ais, hints = {
		.ai_flags = AI_PASSIVE,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM
	};

	int err = getaddrinfo(NULL, port, &hints, &ais);
	assert(err == 0);

	for (struct addrinfo * ai = ais; ai; ai = ai->ai_next) {
		int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd < 0) {
			perror("socket");
			continue;
		}

		int val = 1;
		int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
		assert(ret == 0);

		if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
			ret = listen(fd, 128);
			assert(ret == 0);
			freeaddrinfo(ais);
			return fd;
		}

		perror("bind");
		close(fd);
	}

	assert(0);
}

int main(void)
{
	struct sigaction sa = {.sa_handler = SIG_IGN,.sa_flags = 0 };
	int ret = sigemptyset(&sa.sa_mask);
	assert(ret == 0);
	ret = sigaction(SIGPIPE, &sa, NULL);
	assert(ret == 0);

	int efd = epoll_create1(0);
	assert(efd != -1);
	struct epoll_event event = {.events = EPOLLIN };
	event.data.fd = Listen("1116");
	ret = epoll_ctl(efd, EPOLL_CTL_ADD, event.data.fd, &event);
	assert(ret == 0);

	while (1) {
		struct epoll_event events[128];
		int n = epoll_wait(efd, events, sizeof(events) / sizeof(events[0]), -1);
		if (n < 0) {
			perror("epoll_wait");
			continue;
		}

		for (int i = 0; i < n; ++i) {
			if (events[i].data.fd == event.data.fd)
				Accept(event.data.fd, efd);
			else
				Read(events[i].data.fd);
		}
	}
}
