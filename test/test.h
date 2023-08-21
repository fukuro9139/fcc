#if __linux__

#define _POSIX_C_SOURCE 200809L
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT(x, y) assert(x, y, #y)

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
#else

#include <stdarg.h>

#define ASSERT(x, y) assert(x, y, #y)

void assert(int expected, int actual, char *code);
int printf(char *fmt, ...);
int sprintf(char *buf, char *fmt, ...);
int vsprintf(char *buf, char *fmt, void *ap);
int strcmp(char *p, char *q);
int strncmp(char *p, char *q, long n);
int memcmp(char *p, char *q, long n);
void exit(int n);
int vsprintf();
long strlen(char *s);
void *memcpy(void *dest, void *src, long n);
void *memset(void *s, int c, long n);

#endif /* __linux__ */