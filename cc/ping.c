#include <netinet/icmp6.h>
#include <sys/time.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>
#include <stdio.h>

uint16_t id, seq;

struct timeval elapse(const struct timeval *start, const struct timeval *end)
{
	struct timeval tv = {
		.tv_sec = end->tv_sec - start->tv_sec,
		.tv_usec = end->tv_usec - start->tv_usec
	};

	if (tv.tv_usec < 0) {
		tv.tv_sec--;
		tv.tv_usec += 1000000;
	}

	return tv;
}

void parse(const void *buf, size_t len, const struct timeval *end)
{
	const struct icmp6_hdr *icmp6 = buf;
	if (icmp6->icmp6_type != ICMP6_ECHO_REPLY || icmp6->icmp6_id != id)
		return;

	struct timeval tv, *start = (struct timeval *)(icmp6 + 1);
	tv = elapse(start, end);
	size_t rrt = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	printf("seq=%u, rtt=%zu\n", icmp6->icmp6_seq, rrt);
}

void Read(int fd, const struct timeval *end)
{
	struct sockaddr_storage ss;
	char buf[1024];

	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};

	struct msghdr msg = {
		.msg_name = &ss,
		.msg_namelen = sizeof(ss),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	ssize_t n = recvmsg(fd, &msg, 0);
	if (n < 0) {
		perror("recvmsg");
		return;
	}

	if (msg.msg_flags & MSG_TRUNC) {
		fprintf(stderr, "recvmsg: MSG_TRUNC\n");
		return;
	}

	parse(buf, n, end);
}

void Send(int fd, void *name, socklen_t namelen)
{
	struct icmp6_hdr icmp6;
	icmp6.icmp6_type = ICMP6_ECHO_REQUEST;
	icmp6.icmp6_code = 0;
	icmp6.icmp6_id = id;
	icmp6.icmp6_seq = seq++;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	struct iovec iov[2] = {
		[0].iov_base = &icmp6,
		[0].iov_len = sizeof(icmp6),
		[1].iov_base = &tv,
		[1].iov_len = sizeof(tv)
	};

	struct msghdr msg = {
		.msg_name = name,
		.msg_namelen = namelen,
		.msg_iov = iov,
		.msg_iovlen = 2
	};

	if (sendmsg(fd, &msg, 0) < 0)
		perror("sendmsg");
}

int main(int c, char **v)
{
	if (c != 2) {
		fprintf(stderr, "Usage: %s host\n", v[0]);
		return -1;
	}

	struct addrinfo *ai, hints = {
		.ai_flags = AI_CANONNAME,
		.ai_family = AF_INET6,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = IPPROTO_ICMPV6
	};

	int e = getaddrinfo(v[1], NULL, &hints, &ai);
	if (e != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
		return -1;
	}

	if (ai->ai_canonname)
		printf("%s\n", ai->ai_canonname);

	struct pollfd fd = {.events = POLLIN };
	fd.fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (fd.fd < 0) {
		perror("socket");
		return -1;
	}

	id = getpid() & 0xffff;
	setuid(getuid());

	for (int timeout = 0;;) {
		struct timeval tv, start, end;
		gettimeofday(&start, NULL);

		int n = poll(&fd, 1, timeout);
		if (n == 0) {
			timeout = 1000;
			Send(fd.fd, ai->ai_addr, ai->ai_addrlen);
			continue;
		}

		gettimeofday(&end, NULL);
		tv = elapse(&start, &end);
		if (tv.tv_sec > 0)
			timeout = 0;
		else
			timeout = (1000000 - tv.tv_usec) / 1000;

		if (n < 0) {
			perror("poll");
			continue;
		}

		if (fd.revents & POLLIN)
			Read(fd.fd, &end);
	}
}
