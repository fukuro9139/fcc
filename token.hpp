#pragma once

#include<string>
#include<memory>
#include <cstdlib>
#include <iostream>

namespace Parser{

    /** @brief トークンの種類 */
    enum class TokenKind {
        TK_RESERVED,	/* 記号 */
        TK_NUM,			/* 整数トークン */
        TK_EOF,			/* 入力の終わりを表すトークン */
    };

    /**
     * @brief 
     * 入力文字列から整数トークンと記号トークンからなるトークン列を生成する。
     */
    struct Token
    {
        using _unique_ptr_token = std::unique_ptr<Token>;
        using _itr_str = std::string::const_iterator;

    public:
       
        /** @brief 次のトークン */
        _unique_ptr_token next = nullptr;
		/** @brief 入力文字列*/
		static std::string user_input;

        constexpr Token();
        Token(const TokenKind &kind, const _itr_str &itr);
        ~Token();

        static void error(const std::string &msg);
        static void error_at(const std::string &msg, const _itr_str &loc);
        static bool consume(const std::string &op);
        static void expect(const std::string &op);
        static int expect_number();
        static Token* new_token( const TokenKind &kind, Token *cur, const _itr_str &itr);
        static void tokenize(const std::string &str);
        static bool at_eof();
        

    private:
        /** @brief トークンの型 */
        TokenKind _kind;
        /** @brief kindがTK_NUMの場合、その数値 */
        int _val = 0;
        /** @brief トークン文字列 */
        _itr_str _str;
		/** @brief トークンの長さ */
		size_t _len;

        /** @brief 現在着目しているトークン */
        static _unique_ptr_token _token_cur;
		/** @brief パース対象とする記号*/
		static const std::string _ops;

		static bool _comp_op(const std::string &op);
		
    };

}
