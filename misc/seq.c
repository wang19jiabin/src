#include <unistd.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <assert.h>
#include <stdio.h>

struct msg {
	int seq, ack;
};

_Thread_local int seq, ack;

pthread_barrier_t barr;

socklen_t make_addr(struct sockaddr_un *addr, const char *path)
{
	addr->sun_family = AF_LOCAL;
	sprintf(addr->sun_path, "@%s", path);
	socklen_t len = SUN_LEN(addr);
	addr->sun_path[0] = 0;
	return len;
}

int Bind(const char *path)
{
	int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	struct sockaddr_un addr;
	socklen_t len = make_addr(&addr, path);
	if (bind(fd, (struct sockaddr *)&addr, len) < 0)
		assert(!"bind");

	return fd;
}

int Read(int fd, const char *from, const char *to)
{
	struct msg msg;
	struct sockaddr_un addr;
	socklen_t len = sizeof(addr);
	recvfrom(fd, &msg, sizeof(msg), 0, (struct sockaddr *)&addr, &len);
	if (msg.ack) {
		printf("%s <- %s: ack %d\n", from, to, msg.ack);
		assert(msg.ack == seq);
		return 0;
	}

	printf("%s <- %s: seq %d\n", from, to, msg.seq);
	assert(msg.seq == ack);
	msg.ack = ++ack;
	sendto(fd, &msg, sizeof(msg), 0, (struct sockaddr *)&addr, len);
	return 1;
}

void Send(int fd, const char *from, const char *to)
{
	struct msg msg = {.seq = seq++ };
	struct sockaddr_un addr;
	socklen_t len = make_addr(&addr, to);
	if (sendto(fd, &msg, sizeof(msg), 0, (struct sockaddr *)&addr, len) < 0)
		assert(!"sendto");

	while (Read(fd, from, to)) ;
}

void client(const char *to)
{
	int fd = Bind(__func__);
	pthread_barrier_wait(&barr);

	for (int i = 0; i < 3; ++i)
		Send(fd, __func__, to);

	while (Read(fd, __func__, to)) ;
}

void *server(void *to)
{
	int fd = Bind(__func__);
	pthread_barrier_wait(&barr);

	Read(fd, __func__, to);
	for (int i = 0; i < 5; ++i)
		Send(fd, __func__, to);

	while (Read(fd, __func__, to)) ;
	return NULL;
}

int main(void)
{
	pthread_t t;
	pthread_barrier_init(&barr, NULL, 2);
	pthread_create(&t, NULL, server, "client");
	client("server");
}
