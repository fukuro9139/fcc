/**
 * @file tokenize.cpp
 * @author K.Fukunaga
 * @brief 文字列をトークンに変換するトークナイザの定義
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "tokenize.hpp"

using std::string;
using std::unique_ptr;

/** 入力文字列 */
static string current_input = "";

/***********/
/* 汎用関数 */
/***********/

/**
 * @brief エラーを報告して終了する
 *
 * @param msg エラーメッセージ
 */
void error(string &&msg)
{
	std::cerr << msg << std::endl;
	exit(1);
}

/**
 * @brief エラー箇所を報告して終了する
 * @param msg エラーメッセージ
 * @param location エラー箇所
 */
void error_at(string &&msg, const int &location)
{
	std::cerr << current_input << "\n";
	std::cerr << string(location, ' ') << "^ ";
	std::cerr << msg << std::endl;
	exit(1);
}

/***************/
/* Token Class */
/***************/

/* コンストラクタ */

Token::Token() = default;
Token::Token(const TokenKind &kind) : _kind(kind) {}
Token::Token(const int &location, const int &value) : _kind(TokenKind::TK_NUM), _location(location), _value(std::move(value)) {}
Token::Token(const TokenKind &kind, const int &location, std::string &&str) : _kind(kind), _location(location), _str(std::move(str)) {}

/**
 * @brief 入力文字列をトークナイズする
 *
 * @param input 入力文字列
 * @return トークナイズ結果のトークン・リスト
 */
unique_ptr<Token> Token::tokenize(const string &input)
{
	/* 入力文字列の保存 */
	current_input = input;

	/* スタート地点としてダミーのトークンを作る */
	unique_ptr<Token> head = std::make_unique_for_overwrite<Token>();
	auto current_token = head.get();
	auto itr = current_input.cbegin();
	const auto first = current_input.cbegin();
	const auto last = current_input.cend();

	while (itr != last)
	{
		/* 空白文字をスキップ */
		if (std::isspace(*itr))
		{
			++itr;
			continue;
		}

		/* 数値 */
		if (std::isdigit(*itr))
		{
			/* 数値変換する。変換にした数値を持つ数値トークンを生成し */
			/* current_tokenに繋ぎcurrent_tokenを一つ進める */
			size_t idx;
			current_token->_next = std::make_unique<Token>(itr - first, std::stoi(string(itr, last), &idx));
			current_token = current_token->_next.get();
			itr += idx;
			continue;
		}

		/* 文字列リテラル */
		if ('"' == *itr)
		{
			/* 文字列リテラルを読み込む。読み取った文字列をもつトークンを生成し */
			/* current_tokenに繋ぎcurrent_tokenを一つ進める */
			current_token->_next = read_string_literal(itr);
			current_token = current_token->_next.get();
			continue;
		}

		/* 変数 */
		if (is_first_char_of_ident(*itr))
		{
			/* 先頭は現在のイテレーター */
			const auto start = itr;
			/* 1文字ずつ識別子となりうる文字かみていく */
			/* 識別子となりえない文字が出てくるまで1つの識別子として認識する */
			do
			{
				++itr;
			} while (is_char_of_ident(*itr));

			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			current_token->_next = std::make_unique<Token>(TokenKind::TK_IDENT, start - first, std::string(start, itr));
			current_token = current_token->_next.get();
			continue;
		}

		/* パンクチュエータ:構文的に意味を持つ記号またはキーワードこの段階では区別しない */
		size_t punct_len = read_punct(std::string(itr, last));
		if (punct_len)
		{
			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			current_token->_next = std::make_unique<Token>(TokenKind::TK_PUNCT, itr - first, std::string(itr, itr + punct_len));
			current_token = current_token->_next.get();
			itr += punct_len;
			continue;
		}

		error_at("不正なトークンです", itr - first);
	}

	/* 最後に終端トークンを作成して繋ぐ */
	current_token->_next = std::make_unique<Token>(TokenKind::TK_EOF);
	/* キーワードトークンを識別子トークンから分離する */
	Token::convert_keywords(head->_next);
	/* ダミーの次のトークン以降を切り離して返す */
	return std::move(head->_next);
}

/**
 * @brief トークンを順番にみていってキーワードと一致していれば種類をキーワードに帰る
 *
 * @param token トークン列
 */
void Token::convert_keywords(unique_ptr<Token> &token)
{
	for (Token *t = token.get(); TokenKind::TK_EOF != t->_kind; t = t->_next.get())
	{
		if (TokenKind::TK_IDENT == t->_kind && is_keyword(t))
		{
			t->_kind = TokenKind::TK_KEYWORD;
		}
	}
}

/**
 * @brief トークンの識別子がキーワードかどうか判定する
 *
 * @param token 対象のトークン
 * @return true キーワードである
 * @return false キーワードではない
 */
bool Token::is_keyword(Token *&token)
{
	static const std::vector<string> keywords = {
		"return",
		"if",
		"else",
		"for",
		"while",
		"int",
		"sizeof",
		"char",
	};

	for (size_t i = 0, sz = keywords.size(); i < sz; ++i)
	{
		if (keywords[i].size() == token->_str.size() && std::equal(keywords[i].begin(), keywords[i].end(), token->_str.begin()))
		{
			return true;
		}
	}
	return false;
}

/**
 * @brief '"'で閉じられた文字列リテラルの終わりを探す
 *
 * @param itr 文字列リテラルの始まりの'"'の次の位置
 * @return 文字列リテラルの終わりの'"'の位置
 */
string::const_iterator Token::string_literal_end(string::const_iterator itr)
{
	auto start = itr;

	/* '"'が出てくるか末尾まで到達するまで読み込み続ける */
	for (; itr != current_input.end() && *itr != '"'; ++itr)
	{
		/* 途中で改行や'\0'が出てきたらエラーとする */
		if (*itr == '\n' || *itr == '\0')
		{
			error_at("文字列が閉じられていません", start - current_input.begin());
		}
		/* エスケープシーケンスは無視する */
		if (*itr == '\\')
		{
			++itr;
		}
	}
	/* 末尾まで'"'が見つからなければエラーとする */
	if (itr == current_input.end())
	{
		error_at("文字列が閉じられていません", start - current_input.begin());
	}
	return itr;
}

/**
 * @brief 文字列リテラルを読み込む
 *
 * @param start 文字列リテラルの開始位置。(1個目の'"'の位置)
 * @return 文字列リテラルを表すトークン
 */
unique_ptr<Token> Token::read_string_literal(string::const_iterator &itr)
{
	auto start = itr + 1;
	auto end = string_literal_end(itr + 1);
	string buf = "";

	for (auto p = start; p != end;)
	{
		/* エスケープシーケンス */
		if (*p == '\\')
		{
			/* エスケープされた部分を読む */
			buf += read_escaped_char(itr, std::move(++itr));
			p += 2;
		}
		else
		{
			buf += *p;
			++p;
		}
	}
	itr = end + 1;

	return std::make_unique<Token>(TokenKind::TK_STR, start - current_input.begin(), std::move(buf));
}

/**
 * @brief エスケープシーケンスに対応する文字を返す
 *
 * @param c '\'の次の文字
 * @return 対応する文字
 * @details \a, \b, \t, \n \v, \f, \r, \e
 */
char Token::read_escaped_char(string::const_iterator &new_pos, string::const_iterator &&pos)
{
	/* 8進数エスケープ */
	/* 最大で3桁まで読む */
	if ('0' <= *pos && *pos <= '7')
	{
		int c = *pos++ - '0';
		if ('0' <= *pos && *pos <= '7')
		{
			c = (c << 3) + (*pos++ - '0');
			if ('0' <= *pos && *pos <= '7')
			{
				c = (c << 3) + (*pos++ - '0');
			}
		}
		new_pos = pos;
		return c;
	}
	new_pos = pos + 1;

	switch (*pos)
	{
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 't':
		return '\t';
	case 'n':
		return '\n';
	case 'v':
		return '\v';
	case 'f':
		return '\f';
	case 'r':
		return '\r';
	case 'e':
		return 27;
	default:
		return *pos;
	}
}

/**
 * @brief トークンが期待している演算子または識別子と一致するかどうか
 *
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
bool Token::is_equal(std::string &&op) const
{
	return this->_str.size() == op.size() && std::equal(op.begin(), op.end(), this->_str.begin());
}

/**
 * @brief トークンが型を表す識別子であるか
 *
 * @return true 型を表す識別子である
 * @return false 型を表す識別子ではない
 */
bool Token::is_typename() const
{
	return is_equal("char") || is_equal("int");
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
 *
 * @param token 対象のトークン
 * @param op 比較する文字列
 * @return 次のトークン
 */
std::unique_ptr<Token> Token::skip(std::unique_ptr<Token> &&token, std::string &&op)
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
bool Token::consume(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token, std::string &&str)
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
bool Token::start_with(const std::string &str, std::string &&op)
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
size_t Token::read_punct(std::string &&str)
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
bool Token::is_first_char_of_ident(const char &c)
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
bool Token::is_char_of_ident(const char &c)
{
	return is_first_char_of_ident(c) || ('0' <= c && c <= '9');
}

/**
 * @brief トークンが表す値を返す。トークンが数値型でなければエラーとする。
 *
 * @return int トークンが表す値
 */
int Token::get_number() const
{
	if (TokenKind::TK_NUM != this->_kind)
	{
		error_at("数値ではありません", this->_location);
	}
	return this->_value;
}