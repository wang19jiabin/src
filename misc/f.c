#include <stdio.h>
#include <stdlib.h>

int f(int i)
{
	int x = 1, y = 1, z = 1;
	while (--i > 0) {
		z = x + y;
		x = y;
		y = z;
	}
	return z;
}

int F(int i)
{
	if (i < 2)
		return 1;

	return F(i - 1) + F(i - 2);

}

void p(int n, int (*f)(int))
{
	for (int i = 0; i < n; ++i) {
		printf("%d%c", f(i), i == n - 1 ? '\n' : ' ');
		fflush(stdout);
	}
}

int main(int c, char **v)
{
	if (c != 2)
		return 1;

	int n = atoi(v[1]);
	p(n, f);
	p(n, F);
}
