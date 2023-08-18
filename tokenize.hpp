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
struct File;

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
	const File *_file = nullptr;				 /*!< トークンが含まれるファイル */
	int _line_no = 0;					 /*!< トークン文字列が含まれる行数  */
	bool _at_begining = false;			 /*!< トークンが行頭であるか  */
	bool _has_space = false;			 /*!< トークンの直前にスペースが存在するか */
	unique_ptr<Hideset> _hideset;		 /*!< マクロ展開に利用する、既に展開済みのマクロ */

	/* コンストラクタ */
	Token();
	Token(const TokenKind &kind, const int &location);
	Token(const int64_t &value, const int &location);
	Token(const TokenKind &kind, const int &location, string &&str);
	/* コピーコンストラクタ */
	Token(const Token &src);
	/* ムーブコンストラクタ */
	Token(Token &&src);
	Token &operator=(Token &&rhs);

	/* メンバ関数 */

	bool is_equal(string &&op) const;
	bool is_equal(const string &op) const;
	bool is_end() const;
	bool is_typename() const;
	int64_t get_number() const;

	/* 静的メンバ関数 (public) */

	static unique_ptr<Token> tokenize_file(const string &input_path);
	static unique_ptr<Token> tokenize(const File *file);
	static void print_token(const unique_ptr<Token> &token, const string &output_path);
	static string reverse_str_literal(const Token *token);
	static const vector<unique_ptr<File>> &get_input_files();
	static const File *get_current_file();
	static unique_ptr<Token> copy_token(const Token *src);

private:
	/* 静的メンバ関数 (private) */

	static string read_inputfile(const string &path);
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
	static string remove_backslash_newline(const string &str);

	/** 型名 */
	static constexpr string_view type_names[] = {"void", "_Bool", "char", "short", "int", "long", "float", "double", "struct", "union",
												 "typedef", "enum", "static", "extern", "_Alignas", "signed", "unsigned",
												 "const", "volatile", "auto", "register", "restrict", "__restrict", "__restrict__", "_Noreturn"};

	/** 区切り文字一覧 */
	static constexpr string_view punctuators[] = {"<<=", ">>=", "...", "==", "!=", "<=", ">=", "->", "+=", "-=", "*=", "/=",
												  "++", "--", "%=", "&=", "|=", "^=", "&&", "||", "<<", ">>", "##"};

	static vector<unique_ptr<File>> input_files;
	static const File *current_file;
	static bool at_begining;
	static bool has_space;
};

struct File
{
	string _name;	  /*!< ファイル名（フルパス） */
	int _file_no;	  /*!< ファイルの通し番号 */
	string _contents; /*!< ファイルの中身 */

	File(const string &name, const int &file_no, const string &content)
		: _name(name), _file_no(file_no), _contents(content)
	{
	}
};
