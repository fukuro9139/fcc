#pragma once

#include<string>
#include<memory>
#include <cstdlib>
#include <iostream>

/* トークンの種類 */
enum class TokenKind {
	TK_RESERVED,	/* 記号 */
	TK_NUM,			/* 整数トークン */
	TK_EOF,			/* 入力の終わりを表すトークン */
};

struct Token
{
public:
    /* トークンの型 */
    TokenKind kind;
    /* 次の入力トークン */
    std::unique_ptr<Token> next = nullptr;
    /* kindがTK_NUMの場合、その数値 */
	int val;
    /* トークン文字列 */
	std::string str;

    /* 現在着目しているトークン */
    static std::unique_ptr<Token> token;

    
    void error(const std::string &err);
    bool consume(const char &op);
    void expect(const char &op);
    int expect_number();
    bool at_eof();

};
