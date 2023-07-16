#pragma once

#include<string>
#include<memory>
#include <cstdlib>
#include <iostream>

extern std::string user_input;

namespace Token{

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
        using _str_itr = std::string::const_iterator;
        /* 次の入力トークン */
        std::shared_ptr<Token> next = nullptr;

        Token();
        Token(const TokenKind &kind, const _str_itr &itr);
        ~Token();

        static void error(const std::string &msg);
        static void error_at(const std::string &msg, const _str_itr &loc);
        static bool consume(const char &op);
        static void expect(const char &op);
        static int expect_number();
        static _shared_ptr_token new_token( const TokenKind &kind, _shared_ptr_token token, const _str_itr &itr);
        static void tokenize(const std::string &str);
        static bool at_eof();
        

    private:
        /** トークンの型 */
        TokenKind _kind;
        /** kindがTK_NUMの場合、その数値 */
        int _val;
        /** トークン文字列 */
        _str_itr _str;
        /** 現在着目しているトークン */
        static _shared_ptr_token _token_cur;
    };

}
