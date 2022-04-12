#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct s {
	int i;
	struct s *next;
};

void print(const struct s *s)
{
	while (s) {
		printf("%d%c", s->i, s->next ? ' ' : '\n');
		s = s->next;
	}
}

void insert(struct s **ss, int i)
{
	struct s *s = malloc(sizeof(struct s));
	s->i = i;
	s->next = *ss;
	*ss = s;
}

void erase(struct s **ss, int i)
{
	for (struct s * s; (s = *ss);) {
		if (s->i == i) {
			*ss = s->next;
			free(s);
		} else {
			ss = &s->next;
		}
	}
}

int main(void)
{
	int n = 3;
	struct s *head = NULL;
	for (int i = 0; i < n; ++i)
		insert(&head, i);

	srand(time(NULL));
	while (head) {
		print(head);
		erase(&head, rand() % n);
	}
}
