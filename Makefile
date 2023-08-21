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
FCC = bin/fcc
SRCS = $(wildcard src/*.cpp)
OBJS = $(addprefix obj/, $(notdir $(SRCS:.cpp=.o)))
SAMPLE_CALC = sample/bin/calc
SAMPLE_QUEEN = sample/bin/eightqueen

#テスト用ファイル
TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)

#プライマリターゲット
$(FCC): $(OBJS)
ifeq ($(WINDOWS), 0)
	@mkdir -p bin/
else
	@if not exist bin mkdir bin
endif
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
all: clean $(FCC)

#テスト
ifeq ($(WINDOWS), 0)
test/%.exe: $(FCC) test/%.c
	$(FCC) -I test -o $@ test/$*.c -xc test/common

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh
else
test/%.exe: $(FCC) test/%.c
	$(FCC) -I test -S -o test/$*.s test/$*.c

test: $(TESTS)
endif

#サンプル
ifeq ($(WINDOWS), 0)
sample: $(SAMPLE_CALC) $(SAMPLE_QUEEN)

$(SAMPLE_CALC): $(FCC)
	$(FCC) -o $@ sample/src/calculator.c

$(SAMPLE_QUEEN): $(FCC)
	$(FCC) -o $@ sample/src/eightqueen.c
endif

#不要ファイル削除
clean:
ifeq ($(WINDOWS), 0)
	$(RM) $(FCC) $(OBJS) $(TESTS) $(SAMPLE_CALC) $(SAMPLE_QUEEN) obj/*.d test/*.o
else
	$(RM) bin\fcc.exe obj\* test\*.s /Q
endif

#ヘッダフィルの依存関係
-include *.d

#ダミー
.PHONY: test clean