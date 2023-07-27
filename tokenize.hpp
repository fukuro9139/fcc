/**
 * @file tokenize.hpp
 * @author K.Fukunaga
 * @brief 文字列をトークンに変換するトークナイザの定義
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <locale>
#include <vector>

/* 汎用関数 */

void error(std::string &&msg);
void error_at(std::string &&msg, const int &location);

/** @brief トークンの種類 */
enum class TokenKind
{
	TK_PUNCT,	/*!< パンクチュエータ:構文的に意味を持つ記号 */
	TK_IDENT,	/*!< 識別子 */
	TK_KEYWORD, /*!< キーワード */
	TK_NUM,		/*!< 整数 */
	TK_EOF,		/*!< 入力の終わりを表すトークン */
};

/**
 * @brief トークンを表すクラス
 *
 */
class Token
{
public:
	/* メンバ変数 (public) */

	std::unique_ptr<Token> _next;		 /*!< 次のトークン */
	TokenKind _kind = TokenKind::TK_EOF; /*!< トークンの型 */
	int _value = 0;						 /*!< kindがTK_NUMの場合、その数値 */
	int _location = 0;					 /*!< トークン文字列の開始位置 */
	std::string _str;					 /*!< トークンが表す文字列 */

	/* コンストラクタ */
	Token();
	Token(const TokenKind &kind);
	Token(const int &location, const int &value);
	Token(const TokenKind &kind, const int &location, std::string &&str);

	/* メンバ関数 */

	bool is_equal(std::string &&op) const;
	int get_number() const;

	/* 静的メンバ関数 (public) */

	static std::unique_ptr<Token> tokenize(const std::string &input);
	static std::unique_ptr<Token> skip(std::unique_ptr<Token> &&token, std::string &&op);
	static bool consume(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token, std::string &&str);

private:
	/* 静的メンバ関数 (private) */

	static bool start_with(const std::string &str, std::string &&op);
	static size_t read_punct(std::string &&str);
	static bool is_first_char_of_ident(const char &c);
	static bool is_char_of_ident(const char &c);
	static void convert_keywords(std::unique_ptr<Token> &token);
	static bool is_keyword(Token *&token);
};
