#Windowsでビルドするときは1に設定する
WINDOWS = 0
#デバッグ用
DEBUG = 0

ifeq ($(DEBUG), 0)
OPT = -O2 -w
else
OPT = -g
endif

CXX = g++
CC = gcc

ifeq ($(WINDOWS), 0)
CFLAGS = -std=c++20 -MMD -MP $(OPT)
else
CFLAGS = -std=c++20 -MMD -MP -fexec-charset=cp932 $(OPT)
endif

#プログラム名とオブジェクトファイル名
TARGET = fcc
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

#サフィックスルール適用対象
.SUFFIXES: .cpp .o

#テスト用ファイル
TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)

#プライマリターゲット
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

#サフィックスルール
.cpp.o:
	$(CXX) $(CFLAGS) -c $<

#makeとcleanをまとめて行う
all: clean $(TARGET)

#ダミー
.PHONY: test clean


#テスト
ifeq ($(WINDOWS), 0)
test/macro.exe: $(TARGET) test/macro.c
	./fcc -c -o test/macro.o test/macro.c
	$(CC) -o $@ test/macro.o -xc test/common

test/%.exe: $(TARGET) test/%.c
	$(CC) -o test/tmp_$*.c -E -P -C test/$*.c
	./fcc -c -o test/$*.o test/tmp_$*.c
	$(CC) -o $@ test/$*.o -xc test/common

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh
else
test/macro.exe: $(TARGET) test/macro.c
	./fcc -S -o test/macro.s test/macro.c
	
test/%.exe: $(TARGET) test/%.c
	$(CC) -o test/tmp_$*.c -E -P -C test/$*.c
	./fcc -S -o test/$*.s test/tmp_$*.c

test: $(TESTS)
endif

#不要ファイル削除
clean:
ifeq ($(WINDOWS), 0)
	$(RM) $(TARGET) $(OBJS) $(TESTS) *.d test/*.o test/tmp*
else
	del fcc.exe $(OBJS) *.d test\*.s test\tmp*
endif

#ヘッダフィルの依存関係
-include *.d