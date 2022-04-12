#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <assert.h>

pthread_barrier_t barr;

void io(int from, int to)
{
	ssize_t n;
	char buf[4096];
	while ((n = read(from, buf, sizeof(buf))) > 0)
		write(to, buf, n);
}

void *server(void *port)
{
	int fd;
	struct addrinfo *ai, *ais, hints = {
		.ai_flags = AI_PASSIVE,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM
	};

	getaddrinfo(NULL, port, &hints, &ais);
	for (ai = ais; ai; ai = ai->ai_next) {
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd < 0)
			continue;

		if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
			listen(fd, 128);
			pthread_barrier_wait(&barr);
			fd = accept(fd, NULL, NULL);
			break;
		}

		close(fd);
	}

	assert(ai);
	freeaddrinfo(ais);
	io(fd, STDOUT_FILENO);
	return NULL;
}

void client(const char *host, const char *port)
{
	int fd;
	struct addrinfo *ai, *ais, hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM
	};

	getaddrinfo(host, port, &hints, &ais);
	for (ai = ais; ai; ai = ai->ai_next) {
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd < 0)
			continue;

		if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0)
			break;

		close(fd);
	}

	assert(ai);
	freeaddrinfo(ais);
	io(STDIN_FILENO, fd);
}

int main(void)
{
	pthread_barrier_init(&barr, NULL, 2);
	pthread_t t;
	pthread_create(&t, NULL, server, "1116");
	pthread_barrier_wait(&barr);
	client("127.0.0.1", "1116");
}
