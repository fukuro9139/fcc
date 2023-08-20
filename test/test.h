#define ASSERT(x, y) assert(x, y, #y)

int printf(char *fmt, ...);
int sprintf(char *buf, char *fmt, ...);
int vsprintf(char *buf, char *fmt, void *ap);
int strcmp(char *p, char *q);
int memcmp(char *p, char *q, long n);
void exit(int n);

void assert(int expected, int actual, char *code)
{
	if (expected == actual)
	{
		printf("%s => %d\n", code, actual);
	}
	else
	{
		printf("%s => %d expected but got %d\n", code, expected, actual);
		exit(1);
	}
}