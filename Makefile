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
test: fcc
	./test.sh
	./test-driver.sh

#不要ファイル削除
clean:
#For Linux
	$(RM) $(TARGET) $(OBJS) *~ tmp*

#For Windows
#	del fcc.exe $(OBJS)

#ヘッダフィルの依存関係
-include *.d