#include "test.h"
#include "include1.h"

#

/* */ #

int ret3(void)
{
	return 3;
}

int dbl(int x) { return x * x; }

int main()
{
	ASSERT(5, include1);
	ASSERT(7, include2);

#if 0

#include "/no/such/file"

  ASSERT(0, 1);

#if nested

#endif
#endif
	int m = 0;
#if 1
	m = 5;
#endif
	ASSERT(5, m);

#if 1
#if 0
#if 1
    foo bar
#endif
#endif
	m = 3;
#endif
	ASSERT(3, m);

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
	ASSERT(3, m);

#if 1
	m = 2;
#else
	m = 3;
#endif
	ASSERT(2, m);

#if 1
	m = 2;
#else
	m = 3;
#endif
	ASSERT(2, m);

#if 0
  m = 1;
#elif 0
	m = 2;
#elif 3 + 5
	m = 3;
#elif 1 * 5
	m = 4;
#endif
	ASSERT(3, m);

#if 1 + 5
	m = 1;
#elif 1
	m = 2;
#elif 3
	m = 2;
#endif
	ASSERT(1, m);

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
	ASSERT(2, m);

	int M1 = 5;

#define M1 3
	ASSERT(3, M1);
#define M2 4
	ASSERT(4, M2);

#define M3 3 + 4 +
	ASSERT(12, M3 5);

#define M4 3 + 4
	ASSERT(23, M4 * 5);

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
	ASSERT(5, m);
#undef M

#define M 5
#if M - 5
	m = 6;
#elif M
	m = 5;
#endif
	ASSERT(5, m);

	int M5 = 6;
#define M5 M5 + 3
	ASSERT(9, M5);

#define M6 M5 + 3
	ASSERT(12, M6);

	int M7 = 3;
#define M7 M8 * 5
#define M8 M7 + 2
	ASSERT(13, M7);

#ifdef M9
	m = 5;
#else
	m = 3;
#endif
	ASSERT(3, m);

#define M10
#ifdef M10
	m = 5;
#else
	m = 3;
#endif
	ASSERT(5, m);

#ifndef M11
	m = 3;
#else
	m = 5;
#endif
	ASSERT(3, m);

#define M12
#ifndef M12
	m = 3;
#else
	m = 5;
#endif
	ASSERT(5, m);

#if 0
#ifdef NO_SUCH_MACRO
#endif
#ifndef NO_SUCH_MACRO
#endif
#else
#endif

#define M13() 1
	int M13 = 5;
	ASSERT(1, M13());
	ASSERT(5, M13);

#define M14 ()
	ASSERT(3, ret3 M14);

#define M15(x, y) x + y
	ASSERT(7, M15(3, 4));

#define M16(x, y) x *y
	ASSERT(24, M16(3 + 4, 4 + 5));

#define M17(x, y) (x) * (y)
	ASSERT(63, M17(3 + 4, 4 + 5));

#define M18(x, y) x y
	ASSERT(9, M18(, 4 + 5));

#define M19(x, y) x *y
	ASSERT(20, M19((2 + 3), 4));
	ASSERT(12, M19((2, 3), 4));

#define dbl(x) M20(x) * x
#define M20(x) dbl(x) + 3
	ASSERT(10, dbl(2));

#define M21(x) #x
	ASSERT('a', M21(a!b  `""c)[0]);
	ASSERT('!', M21(a!b  `""c)[1]);
	ASSERT('b', M21(a!b  `""c)[2]);
	ASSERT(' ', M21(a!b  `""c)[3]);
	ASSERT('`', M21(a!b  `""c)[4]);
	ASSERT('"', M21(a!b  `""c)[5]);
	ASSERT('"', M21(a!b  `""c)[6]);
	ASSERT('c', M21(a!b  `""c)[7]);
	ASSERT(0, M21(a!b  `""c)[8]);

#define paste(x, y) x##y
	ASSERT(15, paste(1, 5));
	ASSERT(255, paste(0, xff));
	ASSERT(3, ({ int foobar=3; paste(foo,bar); }));
	ASSERT(5, paste(5, ));
	ASSERT(5, paste(, 5));

#define i 5
	ASSERT(101, ({ int i3=100; paste(1+i,3); }));
#undef i

#define paste2(x) x##5
	ASSERT(26, paste2(1 + 2));

#define paste3(x) 2##x
	ASSERT(23, paste3(1 + 2));

#define paste4(x, y, z) x##y##z
	ASSERT(123, paste4(1, 2, 3));

#define M22
#if defined(M22)
	m = 3;
#else
	m = 4;
#endif
	ASSERT(3, m);

#define M23
#if defined M23
	m = 3;
#else
	m = 4;
#endif
	ASSERT(3, m);

#if defined(M23) - 1
	m = 3;
#else
	m = 4;
#endif
	ASSERT(4, m);

#if defined(NO_SUCH_MACRO)
	m = 3;
#else
	m = 4;
#endif
	ASSERT(4, m);

#if no_such_symbol == 0
	m = 5;
#else
	m = 6;
#endif
	ASSERT(5, m);

#define STR(x) #x
#define M24(x) STR(x)
#define M25(x) M24(foo.x)
	ASSERT(0, strcmp(M25(bar), "foo.bar"));

#define M26(x) M24(foo. x)
	ASSERT(0, strcmp(M26(bar), "foo. bar"));

#define M27 foo
#define M28(x) STR(x)
#define M29(x) M28(x.M27)
	ASSERT(0, strcmp(M29(bar), "bar.foo"));

#define M30(x) M28(x. M27)
	ASSERT(0, strcmp(M30(bar), "bar. foo"));

	assert(1, size\
of(char), \
"sizeof(char)");

#include "include3.h"
	ASSERT(3, foo3);
#undef foo3

#include "include4.h"
	ASSERT(4, foo4);
#undef foo4

#define M31 "include3.h"
#include M31
	ASSERT(3, foo3);
#undef foo3

#define M32 < include4.h
#include M32 >
	ASSERT(4, foo4);
#undef foo4

	ASSERT(1, __STDC__);

	printf("OK\n");
	return 0;
}