#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <locale>

/***********/
/* 汎用関数 */
/***********/

/**
 * @brief エラーを報告して終了する
 *
 * @param msg エラーメッセージ
 */
void error(std::string &&msg);

/**
 * @brief エラー箇所を報告して終了する
 * @param msg エラーメッセージ
 * @param location エラー箇所
 */
void error_at(std::string &&msg, std::string::const_iterator &&location);

/** @brief トークンの種類
 *
 */
enum class TokenKind
{
	TK_PUNCT,	/*!< パンクチュエータ:構文的に意味を持つ記号 */
	TK_IDENT,	/*!< 識別子 */
	TK_KEYWORD, /*!< キーワード */
	TK_NUM,		/*!< 整数 */
	TK_EOF,		/*!< 入力の終わりを表すトークン */
};

/**
 * @brief トークン：プログラムコードを意味ごとに分解したもの
 */
class Token
{
public:
	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	std::unique_ptr<Token> _next = nullptr; /*!< 次のトークン */
	TokenKind _kind;						/*!< トークンの型 */
	int _value = 0;							/*!< kindがTK_NUMの場合、その数値 */
	std::string::const_iterator _location;	/*!< トークン文字列の開始位置 */
	size_t _length = 0;						/*!< トークンの長さ */

	/*****************/
	/* コンストラクタ */
	/*****************/

	/**
	 * @brief Construct a new Token object
	 *
	 */
	Token();

	/**
	 * @brief Construct a new Token object
	 *
	 * @param kind トークンの種類
	 * @param first トークン文字列の開始位置のイテレーター
	 * @param last トークン文字列の末端位置のイテレーター
	 */
	Token(TokenKind &&kind, std::string::const_iterator &&first, const std::string::const_iterator &last);

	/**
	 * @brief Construct a new Token object
	 *
	 * @param kind トークンの種類
	 * @param first トークン文字列の開始位置のイテレーター
	 * @param last トークン文字列の末端位置のイテレーター
	 */
	Token(TokenKind &&kind, const std::string::const_iterator &first, std::string::const_iterator &&last);

	/**
	 * @brief Construct a new Token object
	 *
	 * @param kind トークンの種類
	 * @param first トークン文字列の開始位置のイテレーター
	 * @param last トークン文字列の末端位置のイテレーター
	 */
	Token(TokenKind &&kind, const std::string::const_iterator &first, const std::string::const_iterator &last);

	/**
	 * @brief Construct a new Token object
	 *
	 * @param location トークン文字列の開始位置のイテレーター
	 * @param val トークンが表す数値
	 */
	Token(const std::string::const_iterator &location, int &&val);

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	/**
	 * @brief 文字列inputをトークナイズする
	 *
	 * @param input 入力文字列
	 * @return std::unique_ptr<Token> トークナイズの結果
	 */
	static std::unique_ptr<Token> tokenize(std::string &&input);

	/**
	 * @brief トークンが期待している演算子または識別子と一致するかどうか
	 *
	 * @param token 対象のトークン
	 * @param op 比較する文字列
	 * @return true 一致
	 * @return false 不一致
	 */
	static bool is_equal(const std::unique_ptr<Token> &token, std::string &&op);

	/**
	 * @brief トークンが期待している演算子または識別子と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
	 *
	 * @param token 対象のトークン
	 * @param op 比較する文字列
	 * @return std::unique_ptr<Token> 次のトークン
	 */
	static std::unique_ptr<Token> skip(std::unique_ptr<Token> &&token, std::string &&op);

private:
	/**************************/
	/* 静的メンバ関数 (private) */
	/**************************/

	/**
	 * @brief 文字列の先頭がopと一致するか
	 *
	 * @param first 文字列の開始位置のイテレーター
	 * @param last 文字列の末端位置のイテレーター
	 * @param op 比較する文字列
	 * @return true 一致
	 * @return false 不一致
	 */
	static bool start_with(const std::string::const_iterator &first, const std::string::const_iterator &last, std::string &&op);

	/**
	 * @brief 文字列の先頭がパンクチュエーターかどうか判定しその長さを返す
	 *
	 * @details パンクチュエーターでなければ0を返す。判定は長さの長いパンクチュエーターから行われる。
	 * @param first 文字列の開始位置のイテレーター
	 * @param last 文字列の末端位置のイテレーター
	 * @return size_t パンクチュエーターの長さ
	 */
	static size_t read_punct(const std::string::const_iterator &first, const std::string::const_iterator &last);

	/**
	 * @brief cが識別子の先頭の文字となりうるか判定。
	 *
	 * @details アルファベットの小文字 or 大文字 or アンダースコア'_'
	 *
	 * @param c 対象の文字
	 * @return true なりうる
	 * @return false ならない
	 */
	static bool is_first_char_of_ident(const char &c);

	/**
	 * @brief cが識別子の先頭以外の文字となりうるか判定。
	 *
	 * @details アルファベットの小文字 or 大文字 or アンダースコア'_' or 数字
	 * @param c cが識別子の先頭以外の文字となりうるか判定。
	 * @return true なりうる
	 * @return false ならない
	 */
	static bool is_char_of_ident(const char &c);

	/**
	 * @brief トークンを順番にみていってキーワードと一致していれば種類をキーワードに帰る
	 *
	 * @param token トークン列
	 */
	static void convert_keywords(std::unique_ptr<Token> &token);
};