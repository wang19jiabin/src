#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <netdb.h>
#include <assert.h>

int Connect(const char *host, const char *port)
{
	struct addrinfo *ais, hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM
	};

	int e = getaddrinfo(host, port, &hints, &ais);
	assert(!e);

	for (struct addrinfo * ai = ais; ai; ai = ai->ai_next) {
		int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd < 0) {
			perror("socket");
			continue;
		}

		if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
			freeaddrinfo(ais);
			return fd;
		}

		perror("connect");
		close(fd);
	}

	assert(0);
}

void *thread(void *p)
{
	int fd = Connect("127.0.0.1", "1116");
	char buf[4096];
	size_t len = 0;

	for (size_t i = 0; i < 11; ++i) {
		for (size_t j = 0; j < i; ++j)
			buf[len++] = '0' + j;

		buf[len++] = 0;
	}

	ssize_t n = write(fd, buf, len);
	assert(n == len);

	while (len > 0) {
		n = read(fd, buf, sizeof(buf));
		assert(n > 0);
		len -= n;
		write(STDOUT_FILENO, buf, n);
	}

	return NULL;
}

int main(void)
{
	pthread_t t[1000];
	size_t n = sizeof(t) / sizeof(t[0]);

	for (size_t i = 0; i < n; ++i) {
		int e = pthread_create(t + i, NULL, thread, NULL);
		assert(!e);
	}

	for (size_t i = 0; i < n; ++i) {
		int e = pthread_join(t[i], NULL);
		assert(!e);
	}
}
