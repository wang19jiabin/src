#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <assert.h>

int main(int c, char **v)
{
	int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	struct sockaddr_un sun = {.sun_family = AF_LOCAL };
	strcpy(sun.sun_path, "@path");
	socklen_t len = SUN_LEN(&sun);
	sun.sun_path[0] = 0;

	for (int i = 1; i < c; ++i) {
		int msg = atoi(v[i]);
		ssize_t n = sendto(fd, &msg, sizeof(msg), 0, (struct sockaddr *)&sun, len);
		assert(n == sizeof(msg));
	}
}
