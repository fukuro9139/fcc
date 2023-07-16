CC = g++

#For Linux
# CFLAGS = -std=c++20 -g -static

#For Windows
CFLAGS = -std=c++20 -g -static -fexec-charset=cp932

TARGET = fcc

SRCS = token.cpp fcc.cpp

OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)

all: clean $(OBJS) $(TARGET)

test: fcc
		./test.sh

clean:
#For Linux
#		rm -f fcc *.o *~ tmp*

#For Windows
		del fcc.exe *.o *~ tmp*
.PHONY: test clean
