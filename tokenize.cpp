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
	static const std::vector<string> keywords = {"return", "if", "else", "for", "while", "int"};

	for (size_t i = 0, sz = keywords.size(); i < sz; ++i)
	{
		if (keywords[i].size() == token->_str.size() && std::equal(keywords[i].begin(), keywords[i].end(), token->_str.begin()))
		{
			return true;
		}
	}
	return false;
}
