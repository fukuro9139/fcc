#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
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