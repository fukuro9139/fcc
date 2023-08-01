CXX = g++

#For Linux
CFLAGS = -std=c++20 -g -MMD -MP -static
#For Windows
#CFLAGS = -std=c++20 -g -MMD -MP -static -fexec-charset=cp932

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
all: clean $(OBJS) $(TARGET)

#ダミー
.PHONY: test clean

#テスト
test/%.exe: fcc test/%.c
	$(CC) -o test/tmp_$*.c -E -P -C test/$*.c
	./fcc -o test/$*.s test/tmp_$*.c
	$(CC) -o $@ test/$*.s -xc test/common

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh

#不要ファイル削除
clean:
#For Linux
	$(RM) $(TARGET) $(OBJS) $(TESTS) *.d test/*.s test/tmp*

#For Windows
#	del fcc.exe $(OBJS)

#ヘッダフィルの依存関係
-include *.d