#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>

struct msg {
	int seq, ack;
};

struct arg {
	const char *from, *to;
};

pthread_barrier_t barr;

socklen_t make_addr(struct sockaddr_un *addr, const char *path)
{
	addr->sun_family = AF_LOCAL;
	sprintf(addr->sun_path, "@%s", path);
	socklen_t len = SUN_LEN(addr);
	addr->sun_path[0] = 0;
	return len;
}

void start(const char *from, const char *to)
{
	int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	struct sockaddr_un addr;
	socklen_t len = make_addr(&addr, from);
	if (bind(fd, (struct sockaddr *)&addr, len) < 0)
		assert(!"bind");

	pthread_barrier_wait(&barr);
	for (int seq = 0, ack = 0; seq < 3;) {
		struct msg msg = {.seq = seq };
		len = make_addr(&addr, to);
		sendto(fd, &msg, sizeof(msg), 0, (struct sockaddr *)&addr, len);

		while (1) {
			len = sizeof(addr);
			recvfrom(fd, &msg, sizeof(msg), 0, &addr, &len);
			if (msg.ack) {
				printf("%s <- %s: ack %d\n", from, to, msg.ack);
				assert(msg.ack == ++seq);
				break;
			}

			printf("%s <- %s: seq %d\n", from, to, msg.seq);
			assert(msg.seq == ack++);
			msg.ack = ack;
			sendto(fd, &msg, sizeof(msg), 0, &addr, len);
		}
	}
}

void *thread(void *p)
{
	struct arg *arg = p;
	start(arg->from, arg->to);
	return NULL;
}

int main(void)
{
	struct arg arg = { "thread", "main" };
	pthread_t t;
	pthread_barrier_init(&barr, NULL, 2);
	pthread_create(&t, NULL, thread, &arg);
	start(arg.to, arg.from);
	pthread_join(t, NULL);
}
