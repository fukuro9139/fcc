#デバッグ用
DEBUG = 0

ifeq ($(DEBUG), 0)
OPT = -O2 -w
else
OPT = -g
endif

CFLAGS = -std=c++20 -MMD -MP $(OPT)

#プログラム名とオブジェクトファイル名
FCC = bin/fcc
SRCS = $(wildcard src/*.cpp)
OBJS = $(addprefix obj/, $(notdir $(SRCS:.cpp=.o)))

#テスト用ファイル
TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)

#プライマリターゲット
$(FCC): $(OBJS)
	@mkdir -p bin/
	$(CXX) -o $@ $^

#オブジェクトファイル
obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CXX) $(CFLAGS) -c $< -o $@

#makeとcleanをまとめて行う
all: clean $(FCC)

#テスト
test/%.exe: $(FCC) test/%.c
	$(FCC) -I test -o $@ test/$*.c -xc test/common

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	test/driver.sh

#不要ファイル削除
clean:
	$(RM) $(FCC) $(OBJS) $(TESTS) $(SAMPLE_CALC) $(SAMPLE_QUEEN) obj/*.d test/*.o

#ヘッダフィルの依存関係
-include *.d

#ダミー
.PHONY: test clean