/**
 * @file tokenize.hpp
 * @author K.Fukunaga
 * @brief 文字列をトークンに変換するトークナイザの定義
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include "common.hpp"
#include "input.hpp"

/* 前方宣言 */
class Type;

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

	unique_ptr<Token> _next;			 /*!< 次のトークン */
	TokenKind _kind = TokenKind::TK_EOF; /*!< トークンの型 */
	int64_t _val = 0;					 /*!< kindがTK_NUMの場合、その数値(整数) */
	double _fval = 0.0;					 /*!< kindがTK_NUMの場合、その数値(浮動小数点) */
	shared_ptr<Type> _ty;				 /*!< kindがTK_NUMの場合、数値の型 */
	int _location = 0;					 /*!< トークン文字列の開始位置 */
	string _str = "";					 /*!< トークンが表す文字列 */
	int _line_no = 0;					 /*!< トークン文字列が含まれる行数  */

	/* コンストラクタ */
	Token();
	Token(const TokenKind &kind, const int &location);
	Token(const int64_t &value, const int &location);
	Token(const TokenKind &kind, const int &location, string &&str);

	/* メンバ関数 */

	bool is_equal(string &&op) const;
	bool is_equal(const string &op) const;
	bool is_end() const;
	bool is_typename() const;
	int64_t get_number() const;

	/* 静的メンバ関数 (public) */

	static unique_ptr<Token> tokenize_file(const string &input_path);

private:
	/* 静的メンバ関数 (private) */

	static string read_inputfile(const string &path);
	static unique_ptr<Token> tokenize(const string &filename, string &&input);
	static unique_ptr<Token> read_number(const string::const_iterator &start);
	static unique_ptr<Token> read_int_literal(const string::const_iterator &start);
	static unique_ptr<Token> read_char_literal(string::const_iterator &start);
	static size_t read_punct(string &&str);
	static char read_escaped_char(string::const_iterator &new_pos, string::const_iterator &&pos);
	static unique_ptr<Token> read_string_literal(string::const_iterator &itr);
	static string::const_iterator string_literal_end(string::const_iterator itr);
	static bool is_first_char_of_ident(const char &c);
	static bool is_char_of_ident(const char &c);
	static int from_hex(const char &c);
	static void add_line_number(Token *token);

	/** 型名 */
	static constexpr string_view type_names[] = {"void", "_Bool", "char", "short", "int", "long", "float", "double", "struct", "union",
												 "typedef", "enum", "static", "extern", "_Alignas", "signed", "unsigned",
												 "const", "volatile", "auto", "register", "restrict", "__restrict", "__restrict__", "_Noreturn"};

	/** 区切り文字一覧 */
	static constexpr string_view punctuators[] = {"<<=", ">>=", "...", "==", "!=", "<=", ">=", "->", "+=", "-=", "*=", "/=",
												  "++", "--", "%=", "&=", "|=", "^=", "&&", "||", "<<", ">>"};
};

/* 汎用関数 */

void error(string &&msg);
void error_at(string &&msg, const int &location);
void verror_at(string &&msg, const int &location, const int &line_no);
void error_token(string &&msg, Token *token);
