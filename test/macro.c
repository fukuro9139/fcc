#include "include1.h"

int assert(int expected, int actual, char *code);
int printf(char *fmt, ...);
int sprintf(char *buf, char *fmt, ...);
int strcmp(char *p, char *q);
int memcmp(char *p, char *q, long n);

#

/* */ #

int ret3(void)
{
	return 3;
}

int main()
{
	assert(5, include1, "include1");
	assert(7, include2, "include2");

#if 0

#include "/no/such/file"

  assert(0, 1, "1");

#if nested

#endif
#endif
	int m = 0;
#if 1
	m = 5;
#endif
	assert(5, m, "m");

#if 1
#if 0
#if 1
    foo bar
#endif
#endif
	m = 3;
#endif
	assert(3, m, "m");

#if 1 - 1
#if 1
#endif
#if 1
#else
#endif
#if 0
#else
#endif
	m = 2;
#else
#if 1
	m = 3;
#endif
#endif
	assert(3, m, "m");

#if 1
	m = 2;
#else
	m = 3;
#endif
	assert(2, m, "m");

#if 1
	m = 2;
#else
	m = 3;
#endif
	assert(2, m, "m");

#if 0
  m = 1;
#elif 0
	m = 2;
#elif 3 + 5
	m = 3;
#elif 1 * 5
	m = 4;
#endif
	assert(3, m, "m");

#if 1 + 5
	m = 1;
#elif 1
	m = 2;
#elif 3
	m = 2;
#endif
	assert(1, m, "m");

#if 0
  m = 1;
#elif 1
#if 1
	m = 2;
#else
	m = 3;
#endif
#else
	m = 5;
#endif
	assert(2, m, "m");

	int M1 = 5;

#define M1 3
	assert(3, M1, "M1");
#define M2 4
	assert(4, M2, "M2");

#define M3 3 + 4 +
	assert(12, M3 5, "3");

#define M4 3 + 4
	assert(23, M4 * 5, "5");

#define ASSERT_ assert(
#define if 5
#define five "5"
#define END )
	ASSERT_ 5, if, five END;

#undef ASSERT_
#undef if
#undef five
#undef END

	if (0)
	{
	}

#define M 5
#if M
	m = 5;
#else
	m = 6;
#endif
	assert(5, m, "m");
#undef M

#define M 5
#if M - 5
	m = 6;
#elif M
	m = 5;
#endif
	assert(5, m, "m");

	int M5 = 6;
#define M5 M5 + 3
	assert(9, M5, "M5");

#define M6 M5 + 3
	assert(12, M6, "M6");

	int M7 = 3;
#define M7 M8 * 5
#define M8 M7 + 2
	assert(13, M7, "M7");

#ifdef M9
	m = 5;
#else
	m = 3;
#endif
	assert(3, m, "m");

#define M10
#ifdef M10
	m = 5;
#else
	m = 3;
#endif
	assert(5, m, "m");

#ifndef M11
	m = 3;
#else
	m = 5;
#endif
	assert(3, m, "m");

#define M12
#ifndef M12
	m = 3;
#else
	m = 5;
#endif
	assert(5, m, "m");

#if 0
#ifdef NO_SUCH_MACRO
#endif
#ifndef NO_SUCH_MACRO
#endif
#else
#endif

#define M8() 1
	int M8 = 5;
	assert(1, M8(), "M8()");
	assert(5, M8, "M8");

#define M9 ()
	assert(3, ret3 M9, "ret3 M9");
	
	printf("OK\n");
	return 0;
}