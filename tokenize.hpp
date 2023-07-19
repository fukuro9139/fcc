#pragma once
#include <memory>
#include <string>
#include <iostream>
#include <locale>

void error(std::string &&msg);
void error_at(std::string &&msg, std::string::const_iterator &&location);

/** @brief トークンの種類 */
enum class TokenKind
{
	TK_PUNCT, /* パンクチュエータ:構文的に意味を持つ記号 */
	TK_IDENT, /* 識別子 */
	TK_NUM,	  /* 整数 */
	TK_EOF,	  /* 入力の終わりを表すトークン */
};

/**
 * @brief
 * トークン：プログラムコードを意味ごとに分解したもの
*/
class Token
{
public:
	std::unique_ptr<Token> _next = nullptr; /** 次のトークン */
	TokenKind _kind;						/** トークンの型 */
	int _value = 0;							/** kindがTK_NUMの場合、その数値 */
	std::string::const_iterator _location;		/** トークン文字列の開始位置 */
	size_t _length = 0;						/** トークンの長さ */

	Token();
	Token(TokenKind &&kind, const std::string::const_iterator &first, std::string::const_iterator &&last);
	Token(const std::string::const_iterator &location, int &&val);

	static std::unique_ptr<Token> tokenize(std::string &&input);
	static bool is_equal(const std::unique_ptr<Token> &tok, std::string &&op);
	static std::unique_ptr<Token> skip(std::unique_ptr<Token> &&tok, std::string &&op);

private:
	static bool start_with(const std::string::const_iterator &first, const std::string::const_iterator &last, std::string &&op);
	static size_t read_punct(const std::string::const_iterator &first, const std::string::const_iterator &last);
};