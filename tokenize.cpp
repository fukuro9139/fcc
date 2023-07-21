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

using std::cerr;
using std::endl;
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
	cerr << msg << endl;
	exit(1);
}

/**
 * @brief エラー箇所を報告して終了する
 * @param msg エラーメッセージ
 * @param location エラー箇所
 */
void error_at(string &&msg, string::const_iterator &&location)
{
	size_t pos = location - current_input.begin();
	cerr << current_input << "\n";
	cerr << string(pos, ' ') << "^ ";
	cerr << msg << endl;
	exit(1);
}

/***************/
/* Token Class */
/***************/

/** @brief デフォルトコンストラクタ */
Token::Token() = default;

/**
 * @brief 種類を指定してオブジェクトを生成。
 *
 * @param kind トークンの種類
 * @param first トークン文字列の開始位置のイテレーター
 * @param last トークン文字列の末端位置のイテレーター
 */
Token::Token(TokenKind &&kind, const string::const_iterator &first, const string::const_iterator &last)
	: _kind(std::move(kind)), _location(first), _length(last - first)
{
}

/**
 * @brief 数値を表すトークンオブジェクトを生成。
 *
 * @param location トークン文字列の開始位置のイテレーター
 * @param val トークンが表す数値
 */
Token::Token(const string::const_iterator &location, int &&value)
	: _kind(TokenKind::TK_NUM), _location(location), _value(std::move(value))
{
}

/**
 * @brief 入力文字列をトークナイズする
 *
 * @param input 入力文字列
 * @return トークナイズ結果のトークン・リスト
 */
unique_ptr<Token> Token::tokenize(string &&input)
{
	/* 入力文字列の保存 */
	current_input = std::move(input);

	/* スタート地点としてダミーのトークンを作る */
	unique_ptr<Token> head = std::make_unique_for_overwrite<Token>();
	Token *current_token = head.get();
	auto itr = current_input.cbegin();
	auto last = current_input.cend();

	for (; itr != last;)
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
			current_token->_next = std::make_unique<Token>(itr, std::stoi(string(itr, last), &idx));
			current_token = current_token->_next.get();
			itr += idx;
			continue;
		}

		/* 変数 */
		if (is_first_char_of_ident(*itr))
		{
			/* 先頭は現在のイテレーター */
			auto start = itr;
			/* 1文字ずつ識別子となりうる文字かみていく */
			/* 識別子となりえない文字が出てくるまで1つの識別子として認識する */
			do
			{
				++itr;
			} while (is_char_of_ident(*itr));

			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			current_token->_next = std::make_unique<Token>(TokenKind::TK_IDENT, start, itr);
			current_token = current_token->_next.get();
			continue;
		}

		/* パンクチュエータ:構文的に意味を持つ記号またはキーワードこの段階では区別しない */
		size_t punct_len = read_punct(itr, last);
		if (punct_len)
		{
			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			current_token->_next = std::make_unique<Token>(TokenKind::TK_PUNCT, itr, itr + punct_len);
			current_token = current_token->_next.get();
			itr += punct_len;
			continue;
		}

		error_at("不正なトークンです", std::move(itr));
	}

	/* 最後に終端トークンを作成して繋ぐ */
	current_token->_next = std::make_unique<Token>(TokenKind::TK_EOF, itr, itr);
	/* キーワードトークンを識別子トークンから分離する */
	Token::convert_keywords(head->_next);
	/* ダミーの次のトークン以降を切り離して返す */
	return std::move(head->_next);
}

/**
 * @brief トークンが期待している演算子または識別子と一致するかどうか
 *
 * @param token 対象のトークン
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
bool Token::is_equal(const unique_ptr<Token> &token, string &&op)
{
	return op.length() == token->_length && std::equal(op.begin(), op.end(), token->_location);
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
 *
 * @param token 対象のトークン
 * @param op 比較する文字列
 * @return 次のトークン
 */
unique_ptr<Token> Token::skip(unique_ptr<Token> &&token, string &&op)
{
	if (!is_equal(token, std::move(op)))
	{
		error_at("不正な構文です", std::move(token->_location));
	}
	return std::move(token->_next);
}

/**
 * @brief 文字列の先頭がopと一致するか
 *
 * @param first 文字列の開始位置のイテレーター
 * @param last 文字列の末端位置のイテレーター
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
bool Token::start_with(const string::const_iterator &first, const string::const_iterator &last, string &&op)
{
	return last - first >= op.length() && std::equal(op.begin(), op.end(), first);
}

/**
 * @brief 文字列の先頭がパンクチュエーターかどうか判定しその長さを返す
 *
 * @details パンクチュエーターでなければ0を返す。判定は長さの長いパンクチュエーターから行われる。
 * @param first 文字列の開始位置のイテレーター
 * @param last 文字列の末端位置のイテレーター
 * @return パンクチュエーターの長さ
 */
size_t Token::read_punct(const string::const_iterator &first, const string::const_iterator &last)
{
	if (start_with(first, last, "==") || start_with(first, last, "!=") ||
		start_with(first, last, "<=") || start_with(first, last, ">="))
	{
		return 2;
	}
	return std::ispunct(*first) ? 1 : 0;
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
	static const std::vector<string> keywords = {"return", "if", "else", "for", "while"};

	for (size_t i = 0, sz = keywords.size(); i < sz; ++i)
	{
		if (keywords[i].length() == token->_length && std::equal(keywords[i].begin(), keywords[i].end(), token->_location))
		{
			return true;
		}
	}
	return false;
}
