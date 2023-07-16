CC = g++

CFLAGS = -std=c++20 -g -static

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
		rm -f fcc *.o *~ tmp*

.PHONY: test clean
