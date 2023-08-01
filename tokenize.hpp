/**
 * @file tokenize.hpp
 * @author K.Fukunaga
 * @brief 文字列をトークンに変換するトークナイザの定義
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <locale>
#include <vector>

/** @brief トークンの種類 */
enum class TokenKind
{
	TK_PUNCT,	/*!< パンクチュエータ:構文的に意味を持つ記号 */
	TK_IDENT,	/*!< 識別子 */
	TK_KEYWORD, /*!< キーワード */
	TK_STR,		/*!< 文字列リテラル */
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
	int64_t _value = 0;					 /*!< kindがTK_NUMの場合、その数値 */
	int _location = 0;					 /*!< トークン文字列の開始位置 */
	std::string _str = "";				 /*!< トークンが表す文字列 */
	int _line_no = 0;					 /*!< トークン文字列が含まれる行数  */

	/* コンストラクタ */
	Token();
	Token(const TokenKind &kind, const int &location);
	Token(const int &location, const int64_t &value);
	Token(const TokenKind &kind, const int &location, std::string &&str);

	/* メンバ関数 */

	bool is_equal(std::string &&op) const;
	bool is_equal(const std::string &op) const;
	int get_number() const;
	bool is_typename() const;

	/* 静的メンバ関数 (public) */

	static std::unique_ptr<Token> tokenize(const std::string &filename, std::string &&input);
	static std::unique_ptr<Token> tokenize_file(const std::string &filepath);
	static Token *skip(Token *token, std::string &&op);
	static bool consume(Token **next_token, Token *current_token, std::string &&str);

private:
	/* 静的メンバ関数 (private) */

	static bool start_with(const std::string &str, const std::string &op);
	static size_t read_punct(std::string &&str);
	static bool is_first_char_of_ident(const char &c);
	static bool is_char_of_ident(const char &c);
	static void convert_keywords(Token *token);
	static bool is_keyword(Token *&token);
	static std::unique_ptr<Token> read_string_literal(std::string::const_iterator &itr);
	static char read_escaped_char(std::string::const_iterator &new_pos, std::string::const_iterator &&pos);
	static std::string::const_iterator string_literal_end(std::string::const_iterator itr);
	static int from_hex(const char &c);
	static void add_line_number(Token *token);
};

/* 汎用関数 */

void error(std::string &&msg);
void error_at(std::string &&msg, const int &location);
void verror_at(std::string &&msg, const int &location, const int &line_no);
void error_token(std::string &&msg, Token *token);