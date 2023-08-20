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
RM = rm -f
else
CFLAGS = -std=c++20 -MMD -MP -fexec-charset=cp932 $(OPT)
RM = del
endif

#プログラム名とオブジェクトファイル名
TARGET = fcc
SRCS = $(wildcard src/*.cpp)
OBJS = $(addprefix obj/, $(notdir $(SRCS:.cpp=.o)))

#テスト用ファイル
TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)

#プライマリターゲット
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^

#オブジェクトファイル
obj/%.o: src/%.cpp
ifeq ($(WINDOWS), 0)
	@mkdir -p obj
else
	@if not exist obj mkdir obj
endif
	$(CXX) $(CFLAGS) -c $< -o $@

#makeとcleanをまとめて行う
all: clean $(TARGET)

#テスト
ifeq ($(WINDOWS), 0)
test/%.exe: $(TARGET) test/%.c
	./fcc -I test -c -o test/$*.o test/$*.c
	$(CC) -o $@ test/$*.o -xc test/common

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh
else
test/%.exe: $(TARGET) test/%.c
	./fcc -I test -S -o test/$*.s test/$*.c

test: $(TESTS)
endif

#不要ファイル削除
clean:
ifeq ($(WINDOWS), 0)
	$(RM) $(TARGET) $(OBJS) $(TESTS) obj/*.d test/*.o
else
	$(RM) fcc.exe obj\* test\*.s /Q
endif

#ヘッダフィルの依存関係
-include *.d

#ダミー
.PHONY: test clean