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

	std::unique_ptr<Token> _next; /*!< 次のトークン */
	TokenKind _kind;			  /*!< トークンの型 */
	int _value;					  /*!< kindがTK_NUMの場合、その数値 */
	int _location;				  /*!< トークン文字列の開始位置 */
	std::string _str;			  /*!< トークンが表す文字列 */

	/* コンストラクタ */
	Token() = default;

	Token(const TokenKind &kind)
		: _kind(kind){};
	Token(const int &location, const int &value)
		: _kind(TokenKind::TK_NUM), _location(location), _value(std::move(value)){};
	Token(const TokenKind &kind, const int &location, std::string &&str)
		: _kind(kind), _location(location), _str(std::move(str)){};

	/* メンバ関数 */

	bool is_equal(std::string &&op);

	/* 静的メンバ関数 (public) */

	static std::unique_ptr<Token> tokenize(std::string &&input);
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

/* インライン関数 */

/**
 * @brief トークンが期待している演算子または識別子と一致するかどうか
 *
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
inline bool Token::is_equal(std::string &&op)
{
	return this->_str.size() == op.size() && std::equal(op.begin(), op.end(), this->_str.begin());
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
 *
 * @param token 対象のトークン
 * @param op 比較する文字列
 * @return 次のトークン
 */
inline std::unique_ptr<Token> Token::skip(std::unique_ptr<Token> &&token, std::string &&op)
{
	if (!token->is_equal(std::move(op)))
	{
		error_at("不正な構文です", token->_location);
	}
	return std::move(token->_next);
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタをnext_tokenにセットしtrueを返す。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param op 比較する文字列
 * @return 一致：true, 不一致：false
 */
inline bool Token::consume(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token, std::string &&str)
{
	if (current_token->is_equal(std::move(str)))
	{
		next_token = std::move(current_token->_next);
		return true;
	}
	next_token = std::move(current_token);
	return false;
}

/**
 * @brief 文字列の先頭がopと一致するか
 *
 * @param first 文字列の開始位置のイテレーター
 * @param last 文字列の末端位置のイテレーター
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
inline bool Token::start_with(const std::string &str, std::string &&op)
{
	return str.size() >= op.size() && std::equal(op.begin(), op.end(), str.begin());
}

/**
 * @brief 文字列の先頭がパンクチュエーターかどうか判定しその長さを返す
 *
 * @details パンクチュエーターでなければ0を返す。判定は長さの長いパンクチュエーターから行われる。
 * @param first 文字列の開始位置のイテレーター
 * @param last 文字列の末端位置のイテレーター
 * @return パンクチュエーターの長さ
 */
inline size_t Token::read_punct(std::string &&str)
{
	if (start_with(str, "==") || start_with(str, "!=") ||
		start_with(str, "<=") || start_with(str, ">="))
	{
		return 2;
	}
	return std::ispunct(str.front()) ? 1 : 0;
}

/**
 * @brief cが識別子の先頭の文字となりうるか判定。
 *
 * @details アルファベットの小文字 or 大文字 or アンダースコア'_'
 * @param c 対象の文字
 * @return なりうる:true, ならない:false
 */
inline bool Token::is_first_char_of_ident(const char &c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

/**
 * @brief cが識別子の先頭以外の文字となりうるか判定。
 *
 * @details アルファベットの小文字 or 大文字 or アンダースコア'_' or 数字
 * @param c cが識別子の先頭以外の文字となりうるか判定。
 * @return なりうる:true, ならない:false
 */
inline bool Token::is_char_of_ident(const char &c)
{
	return is_first_char_of_ident(c) || ('0' <= c && c <= '9');
}