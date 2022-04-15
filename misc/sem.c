#include <semaphore.h>
#include <pthread.h>
#include <poll.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

struct {
	sem_t sem;
	pthread_barrier_t barr;
} g;

enum { THREADS = 3 };

enum msg { REQ, RSP, BUSY, IDLE };

const char *msg2str(enum msg msg)
{
	switch (msg) {
	case REQ:
		return "req";
	case RSP:
		return "rsp";
	case BUSY:
		return "BUSY";
	case IDLE:
		return "IDLE";
	default:
		assert(0);
	}
}

socklen_t id2sun(struct sockaddr_un *sun, int id)
{
	sun->sun_family = AF_LOCAL;
	snprintf(sun->sun_path, sizeof(sun->sun_path), "@%d", id);
	socklen_t len = SUN_LEN(sun);
	sun->sun_path[0] = 0;
	return len;
}

int sun2id(struct sockaddr_un *sun, socklen_t len)
{
	len -= offsetof(struct sockaddr_un, sun_path);
	sun->sun_path[len] = 0;
	return atoi(sun->sun_path + 1);
}

int Bind(int id)
{
	int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	struct sockaddr_un sun;
	socklen_t len = id2sun(&sun, id);
	int ret = bind(fd, (struct sockaddr *)&sun, len);
	assert(!ret);
	return fd;
}

void Send(int id, int fd, int msg, int to)
{
	struct sockaddr_un sun;
	socklen_t len = id2sun(&sun, to);
	int ret = sem_post(&g.sem);
	assert(!ret);
	ssize_t n = sendto(fd, &msg, sizeof(int), 0, (struct sockaddr *)&sun, len);
	assert(n == sizeof(int));
	printf("%d --> %d: send %s\n", id, to, msg2str(msg));
}

void Recv(int id, int fd, int *msg, int *from)
{
	struct sockaddr_un sun;
	socklen_t len = sizeof(sun);
	ssize_t n = recvfrom(fd, msg, sizeof(int), 0, (struct sockaddr *)&sun, &len);
	assert(n == sizeof(int));
	int ret = sem_trywait(&g.sem);
	assert(!ret);
	*from = sun2id(&sun, len);
	printf("%d <- %d: recv %s\n", id, *from, msg2str(*msg));
}

int Poll(int fd, int t)
{
	struct pollfd fds = {
		.fd = fd,
		.events = POLLIN
	};

	int n = poll(&fds, 1, t);
	assert(n != -1);
	return n;
}

void start(int id, int fd)
{
	for (int to = 0; to < THREADS; ++to) {
		if (to != id)
			Send(id, fd, REQ, to);
	}
}

void *thread(void *p)
{
	int id = (int)p;
	int fd = Bind(id);
	pthread_barrier_wait(&g.barr);
	start(id, fd);

	while (1) {
		int n = Poll(fd, 0);
		if (n == 0) {
			printf("note over %d: IDLE\n", id);
			Send(id, fd, IDLE, 0);
			Poll(fd, -1);
			printf("note over %d: BUSY\n", id);
			Send(id, fd, BUSY, 0);
		}

		int msg, from;
		Recv(id, fd, &msg, &from);
		assert(msg == REQ || msg == RSP);
		if (msg == REQ)
			Send(id, fd, RSP, from);
	}
}

void check(int msg, int from)
{
	static bool idle[THREADS];
	assert(msg == BUSY || msg == IDLE);
	idle[from] = (msg == IDLE);

	bool busy = false;
	char s[256];
	int n = snprintf(s, sizeof(s), "note over 0: ");
	for (int id = 1; id < THREADS; ++id) {
		if (!idle[id])
			busy = true;

		n += snprintf(s + n, sizeof(s) - n, "%d is %s\\n", id, idle[id] ? "IDLE" : "BUSY");
	}

	if (busy) {
		s[n - 2] = 0;
		printf("%s\n", s);
		return;
	}

	printf("note over 0: ALL IDLE\n");
	return;

	int sem;
	int ret = sem_getvalue(&g.sem, &sem);
	assert(!ret);
	if (sem)
		printf("note over 0: STILL BUSY\\nsem=%d\n", sem);
	else
		printf("note over 0: ALL IDLE\\nsem=%d\n", sem);
}

void Main(void)
{
	int fd = Bind(0);
	pthread_barrier_wait(&g.barr);
	start(0, fd);

	while (1) {
		int msg, from;
		Recv(0, fd, &msg, &from);
		switch (msg) {
		case REQ:
			Send(0, fd, RSP, from);
			break;
		case BUSY:
		case IDLE:
			check(msg, from);
			break;
		}
	}
}

int main(void)
{
	setbuf(stdout, NULL);
	sem_init(&g.sem, 0, 0);
	pthread_barrier_init(&g.barr, NULL, THREADS);

	for (int id = 1; id < THREADS; ++id) {
		pthread_t t;
		pthread_create(&t, NULL, thread, (void *)id);
	}

	Main();
}
