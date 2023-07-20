#include "tokenize.hpp"

using token_ptr = std::unique_ptr<Token>;
using std::cerr;
using std::endl;
using std::string;

/** @brief 入力文字列 */
static string current_input = "";

/**
 * @brief エラーを報告して終了する
 * @param msg エラーメッセージ
 */
void error(std::string &&msg)
{
	cerr << msg << endl;
	exit(1);
}

/**
 * @brief エラー箇所を報告して終了する
 * @param msg エラーメッセージ
 * @param location エラー箇所
 */
void error_at(std::string &&msg, std::string::const_iterator &&location)
{
	size_t pos = location - current_input.begin();
	cerr << current_input << "\n";
	cerr << string(pos, ' ') << "^ ";
	cerr << msg << endl;
	exit(1);
}

/** @brief コンストラクタ */
Token::Token() = default;

Token::Token(TokenKind &&kind, std::string::const_iterator &&first, const std::string::const_iterator &last)
	: _length(last - first), _kind(std::move(kind)), _location(std::move(first))
{
}

Token::Token(TokenKind &&kind, const std::string::const_iterator &first, std::string::const_iterator &&last)
	: _kind(std::move(kind)), _location(first), _length(last - first)
{
}

Token::Token(TokenKind &&kind, const std::string::const_iterator &first, const std::string::const_iterator &last)
	: _kind(std::move(kind)), _location(first), _length(last - first)
{
}

Token::Token(const std::string::const_iterator &location, int &&value)
	: _kind(TokenKind::TK_NUM), _location(location), _value(std::move(value))
{
}

/**
 * @brief 文字列inputをトークナイズして新しいトークン列を返す
 * @param input トークナイズする対象文字列
 */
token_ptr Token::tokenize(std::string &&input)
{
	/* 入力文字列の保存 */
	current_input = std::move(input);

	/* スタート地点としてダミーのトークンを作る */
	token_ptr head = std::make_unique_for_overwrite<Token>();
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
			current_token->_next = std::make_unique<Token>(TokenKind::TK_IDENT, std::move(start), itr);
			current_token = current_token->_next.get();
			continue;
		}

		/* パンクチュエータ:構文的に意味を持つ記号 */
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
	current_token->_next = std::make_unique<Token>(TokenKind::TK_EOF, itr, std::move(itr));
	/* ダミーの次のトークン以降を切り離して返す */
	return std::move(head->_next);
}

/**
 * @brief
 * トークンが期待している演算子と一致するかどうか。
 * @param op 期待している演算子
 * @return true 一致
 * @return false 不一致
 */
bool Token::is_equal(const token_ptr &token, std::string &&op)
{
	return op.length() == token->_length && std::equal(op.begin(), op.end(), token->_location);
}

/**
 * @brief
 * トークンが期待している演算子と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
 * @param op 期待している演算子
 * @return 次のトークン
 */
token_ptr Token::skip(token_ptr &&token, std::string &&op)
{
	if (!is_equal(token, std::move(op)))
	{
		error_at("不正な構文です", std::move(token->_location));
	}
	return std::move(token->_next);
}

/**
 * @brief
 * 文字列の先頭がopと一致するか
 * @return true 一致
 * @return false 不一致
 */
bool Token::start_with(const std::string::const_iterator &first, const std::string::const_iterator &last, std::string &&op)
{
	return last - first >= op.length() && std::equal(op.begin(), op.end(), first);
}

/**
 * @brief
 * itrから始まるパンクチュエーターを読み、その長さを返す
 */
size_t Token::read_punct(const std::string::const_iterator &first, const std::string::const_iterator &last)
{
	if (start_with(first, last, "==") || start_with(first, last, "!=") ||
		start_with(first, last, "<=") || start_with(first, last, ">="))
	{
		return 2;
	}
	return std::ispunct(*first) ? 1 : 0;
}

/**
 * @brief
 * cが識別子の先頭の文字となりうるか判定。
 * アルファベットの小文字 or 大文字 or アンダースコア'_'
 */
bool Token::is_first_char_of_ident(const char &c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

/**
 * @brief
 * cが識別子の先頭以外の文字となりうるか判定。
 * アルファベットの小文字 or 大文字 or アンダースコア'_' or 数字
 * */
bool Token::is_char_of_ident(const char &c)
{
	return is_first_char_of_ident(c) || ('0' <= c && c <= '9');
}
