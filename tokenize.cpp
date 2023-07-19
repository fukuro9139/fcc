#include "tokenize.hpp"

using token_ptr = std::unique_ptr<Token>;
using token_const_ptr = std::unique_ptr<const Token>;
using std::cerr;
using std::endl;
using std::string;

/** @brief 入力文字列 */
static string current_input = "";

/**
 * @brief エラーを報告して終了する
 * @param msg エラーメッセージ
 */
void error(const std::string &msg)
{
	cerr << msg << endl;
	exit(1);
}

/**
 * @brief エラー箇所を報告して終了する
 * @param msg エラーメッセージ
 * @param loc エラー箇所
 */
void error_at(const std::string &msg, const std::string::const_iterator &loc)
{
	size_t pos = loc - current_input.begin();
	cerr << current_input << "\n";
	cerr << string(pos, ' ') << "^ ";
	cerr << msg << endl;
	exit(1);
}

/** @brief コンストラクタ */
Token::Token() = default;

Token::Token(const TokenKind &kind, const std::string::const_iterator &start, const std::string::const_iterator &last)
	: _kind(kind), _loc(start), _len(last - start)
{
}

Token::Token(const std::string::const_iterator &loc, const int &val)
	: _kind(TokenKind::TK_NUM), _loc(loc) , _val(val)
{
}

/** @brief デストラクタ */
Token::~Token() = default;

/**
 * @brief 文字列inputをトークナイズして新しいトークン列を返す
 * @param input トークナイズする対象文字列
 */
token_const_ptr Token::tokenize(const std::string &input)
{
	current_input = input;
	token_ptr head = std::make_unique_for_overwrite<Token>();
	Token *cur = head.get();
	auto itr = current_input.cbegin();
	auto last = current_input.cend();

	for ( ; itr != last ; )
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
			/* 数値変換 */
			size_t idx;
			const string sub_str(itr, last);
			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			cur->_next = std::make_unique<Token>(itr, std::stoi(sub_str, &idx));
			cur = cur->_next.get();
			itr += idx;
			continue;
		}

		/* 変数 */
		if ('a' <= *itr && *itr <= 'z')
		{
			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			cur->_next = std::make_unique<Token>(TokenKind::TK_IDENT, itr, itr + 1);
			cur = cur->_next.get();
			++itr;
			continue;
		}

		/* 区切り文字 */
		size_t punct_len = read_punct(itr, last);
		if (punct_len)
		{
			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			cur->_next = std::make_unique<Token>(TokenKind::TK_PUNCT, itr, itr + punct_len);
			cur = cur->_next.get();
			itr += punct_len;
			continue;
		}

		error_at("不正なトークンです", itr);
	}
	cur->_next = std::make_unique<Token>(TokenKind::TK_EOF, itr, itr);
	return std::move(head->_next);
}

/**
 * @brief
 * トークンが期待している演算子と一致するかどうか。
 * @param op 期待している演算子
 * @return true 一致
 * @return false 不一致
 */
bool Token::is_equal(const Token* &tok, const std::string &op)
{
	return op.length() == tok->_len && std::equal(op.begin(), op.end(), tok->_loc);
}

/**
 * @brief
 * トークンが期待している演算子と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
 * @param op 期待している演算子
 * @return 次のトークン
 */
const Token* Token::skip(const Token* &tok, const std::string &op)
{
	if (!is_equal(tok, op))
	{
		error_at(op + "ではありません", tok->_loc);
	}
	return tok->_next.get();
}

/**
 * @brief
 * 文字列の先頭がopと一致するか
 * @return true 一致
 * @return false 不一致
 */
bool Token::start_with(const std::string::const_iterator &start, const std::string::const_iterator &last, const std::string &op)
{
	return last - start >= op.length() && std::equal(op.begin(), op.end(), start);
}

/**
 * @brief
 * itrから始まる記号トークンを読み、その長さを返す
 */
size_t Token::read_punct(const std::string::const_iterator &start, const std::string::const_iterator &last)
{
	if (start_with(start, last, "==") || start_with(start, last, "!=") ||
		start_with(start, last, "<=") || start_with(start, last, ">="))
	{
		return 2;
	}
	return std::ispunct(*start) ? 1 : 0;
}