#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

struct {
	int cnt;
	const char *log;
	struct termios old;
} g = {.log = "log" };

void save(void)
{
	if (tcsetattr(STDIN_FILENO, TCSANOW, &g.old) < 0)
		perror(__func__);

	int fd = open(g.log, O_WRONLY | O_CREAT, 0600);
	if (fd < 0) {
		perror(__func__);
		return;
	}

	if (write(fd, &g.cnt, sizeof(g.cnt)) < 0)
		perror(__func__);
}

int init(void)
{
	if (tcgetattr(STDIN_FILENO, &g.old) < 0) {
		perror(__func__);
		return -1;
	}

	struct termios new = g.old;
	new.c_lflag &= ~(ECHO | ICANON);
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new);

	setbuf(stdout, NULL);
	signal(SIGINT, exit);
	return atexit(save);
}

void load(void)
{
	int fd = open(g.log, O_RDONLY);
	if (fd < 0)
		return;

	if (read(fd, &g.cnt, sizeof(g.cnt)) < 0) {
		perror(__func__);
		g.cnt = 0;
	}

	close(fd);
}

void count(void)
{
	for (int i = 0; i < 10; ++i) {
		printf("\r%d", i);
		sleep(1);
	}
}

void start(void)
{
	char c;
	while (read(STDIN_FILENO, &c, 1) > 0) {
		count();
		printf("\r%d\n\a", ++g.cnt);
		tcflush(STDIN_FILENO, TCIFLUSH);
	}
}

int main(int c, char **v)
{
	if (init())
		return 1;

	if (c > 1)
		g.cnt = atoi(v[1]);
	else
		load();

	printf("%d\n", g.cnt);
	start();
}
