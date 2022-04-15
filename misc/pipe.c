#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>

void Close(int fds[2])
{
	int ret = close(fds[0]);
	assert(ret == 0);
	ret = close(fds[1]);
	assert(ret == 0);
}

int cat(int fds[2])
{
	int fd = dup2(fds[1], STDOUT_FILENO);
	assert(fd != -1);
	Close(fds);
	return execlp("cat", "cat", "/etc/passwd", NULL);
}

int grep(int fds[2])
{
	int fd = dup2(fds[0], STDIN_FILENO);
	assert(fd != -1);
	Close(fds);
	return execlp("grep", "grep", "root", NULL);
}

int main(void)
{
	int fds[2];
	int ret = pipe(fds);
	assert(ret == 0);

	int (*cmds[])(int *) = { cat, grep };
	for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); ++i) {
		pid_t pid = fork();
		assert(pid != -1);
		if (pid == 0)
			return cmds[i] (fds);
	}

	Close(fds);
	while (wait(NULL) != -1) ;
	perror("");
}
