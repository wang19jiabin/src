#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

const char *family[] = {
	[AF_INET] = "AF_INET",
	[AF_INET6] = "AF_INET6"
};

const char *type[] = {
	[SOCK_STREAM] = "SOCK_STREAM",
	[SOCK_DGRAM] = "SOCK_DGRAM",
	[SOCK_RAW] = "SOCK_RAW",
};

const char *protocol[] = {
	[IPPROTO_IP] = "IPPROTO_IP",
	[IPPROTO_TCP] = "IPPROTO_TCP",
	[IPPROTO_UDP] = "IPPROTO_UDP",
	[IPPROTO_ICMP] = "IPPROTO_ICMP",
	[IPPROTO_ICMPV6] = "IPPROTO_ICMPV6"
};

void ntop(const void *sa, char *buf, socklen_t len, in_port_t * port)
{
	const void *addr;
	int af = ((struct sockaddr *)sa)->sa_family;
	if (af == AF_INET) {
		const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
		addr = &sin->sin_addr;
		*port = sin->sin_port;
	} else if (af == AF_INET6) {
		const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
		addr = &sin6->sin6_addr;
		*port = sin6->sin6_port;
	} else {
		assert(0);
	}

	if (!inet_ntop(af, addr, buf, len)) {
		perror("inet_ntop");
		assert(0);
	}

	*port = ntohs(*port);
}

int main(int c, char **v)
{
	const char *host = NULL, *port = NULL;
	struct addrinfo hints = { };
	for (int i; (i = getopt(c, v, "h:p:P")) != -1;) {
		switch (i) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'P':
			hints.ai_flags = AI_PASSIVE;
			break;
		default:
			return 1;
		}

	}

	struct addrinfo *ais;
	int e = getaddrinfo(host, port, &hints, &ais);
	if (e != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
		return 1;
	}

	for (const struct addrinfo * ai = ais; ai; ai = ai->ai_next) {
		printf("%s\n", family[ai->ai_family]);
		printf("%s\n", type[ai->ai_socktype]);
		printf("%s\n", protocol[ai->ai_protocol]);
		char addr[INET6_ADDRSTRLEN];
		in_port_t port;
		ntop(ai->ai_addr, addr, sizeof(addr), &port);
		printf("%s:%d\n\n", addr, port);
	}
}
