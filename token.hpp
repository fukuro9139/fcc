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
    using _shared_ptr_token = std::shared_ptr<Token>;
    /* 次の入力トークン */
    std::shared_ptr<Token> next = nullptr;

    Token();
    Token(const TokenKind &kind, const std::string &str);
    ~Token();

    static void error(const std::string &err);
    static bool consume(const char &op);
    static void expect(const char &op);
    static int expect_number();
    static _shared_ptr_token new_token( const TokenKind &kind, _shared_ptr_token token, const std::string &str);
    static void tokenize(const std::string &str);
    static bool at_eof();
    

private:
    /** トークンの型 */
    TokenKind _kind;
    /** kindがTK_NUMの場合、その数値 */
	int _val;
    /** トークン文字列 */
	std::string _str;
    /** 現在着目しているトークン */
    static _shared_ptr_token _token_cur;
};
