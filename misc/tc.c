#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct termios t;

void reset(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int main(void)
{
	if (!tcgetattr(STDIN_FILENO, &t))
		atexit(reset);

	pid_t pid = fork();
	if (!pid)
		execlp("gdb", "gdb", NULL);

	sleep(1);
}
