/**
 * @file tokenize.cpp
 * @author K.Fukunaga
 * @brief 文字列をトークンに変換するトークナイザの定義
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "tokenize.hpp"
#include "object.hpp"
#include "input.hpp"

using std::string;
using std::unique_ptr;

/** 入力文字列 */
static string current_input = "";

/* 入力ファイル */
static string current_filename = "";

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
 * @brief エラー箇所の位置を受け取ってエラー出力
 *
 * @param msg エラーメッセージ
 * @param location エラー箇所の位置
 */
void error_at(string &&msg, const int &location)
{

	/* エラー箇所が含まれる行の行数を取得 */
	int line_no = 1;
	for (int l = 0; l < location; ++l)
	{
		if (current_input[l] == '\n')
		{
			++line_no;
		}
	}

	verror_at(std::move(msg), location, line_no);
}

/**
 * @brief 下記のフォーマットでエラー箇所を報告して終了する
 * foo.c:10: x = y + 1;
 *               ^ <error message here>
 * @param msg エラーメッセージ
 * @param location エラー箇所
 * @param line_no エラー箇所の行数
 */
void verror_at(std::string &&msg, const int &location, const int &line_no)
{
	int line_start = location;
	/* エラー箇所が含まれる行の先頭位置を探す */
	while (line_start > 0 && current_input[line_start - 1] != '\n')
	{
		--line_start;
	}

	int line_end = location;
	int max = current_input.end() - current_input.begin();
	/* エラー箇所が含まれる行の末尾の位置を探す */
	while (line_end < max - 1 && current_input[line_end + 1] != '\n')
	{
		++line_end;
	}

	/* 行頭の空白をスキップ */
	while (std::isspace(current_input[line_start]) && line_start < line_end)
	{
		++line_start;
	}

	/* ファイル名 */
	std::string filename = current_filename + ":" + std::to_string(line_no) + ": ";
	int indent = filename.size();
	std::cerr << filename;

	/* エラー箇所が含まれる行を出力 */
	std::cerr << current_input.substr(line_start, line_end - line_start + 1) << "\n";

	/* エラーメッセージを出力 */
	std::cerr << string(indent + location - line_start, ' ') << "^ ";
	std::cerr << msg << std::endl;
	exit(1);
}

/**
 * @brief トークンを受け取ってエラーメッセージを出力
 *
 * @param msg エラーメッセージ
 * @param token エラー箇所を含むトークン
 */
void error_token(std::string &&msg, Token *token)
{
	verror_at(std::move(msg), token->_location, token->_line_no);
}

/***************/
/* Token Class */
/***************/

/* コンストラクタ */

Token::Token() = default;
Token::Token(const TokenKind &kind, const int &location) : _kind(kind), _location(location) {}
Token::Token(const int64_t &value, const int &location) : _kind(TokenKind::TK_NUM), _location(location), _value(std::move(value)) {}
Token::Token(const TokenKind &kind, const int &location, std::string &&str) : _kind(kind), _location(location), _str(std::move(str)) {}

/**
 * @brief 入力されたパスにあるファイルを開いてトークナイズする
 *
 * @param path ファイルパス
 * @return トークナイズした結果のトークンリスト
 */
unique_ptr<Token> Token::tokenize_file(const string &path)
{
	return tokenize(path, Input::read_file(path));
}

/**
 * @brief 入力文字列をトークナイズする
 *
 * @param filename 入力ファイル
 * @param input 入力文字列
 * @return トークナイズした結果のトークン・リスト
 */
unique_ptr<Token> Token::tokenize(const string &filename, string &&input)
{
	/* 入力文字列の保存 */
	current_input = input;

	/* 入力ファイル名を保存 */
	current_filename = filename;

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

		/* 行コメント */
		/* 入力文字列の末尾は'\n'であることが保証されているため */
		/* itr+1 != lastのチェックは不要 */
		if ('/' == *itr && '/' == *(itr + 1))
		{
			itr += 2;
			while ('\n' != *itr)
			{
				++itr;
			}
			continue;
		}

		/* ブロックコメント */
		if ('/' == *itr && '*' == *(itr + 1))
		{
			itr += 2;
			while (itr != last && !('*' == *itr && '/' == *(itr + 1)))
			{
				++itr;
			}
			if (itr == last)
			{
				error_at("ブロックコメントが閉じられていません", itr - first);
			}
			itr += 2;
			continue;
		}

		/* 数値 */
		if (std::isdigit(*itr))
		{
			/* 数値変換する。変換にした数値を持つ数値トークンを生成し */
			/* current_tokenに繋ぎcurrent_tokenを一つ進める */
			size_t idx;
			current_token->_next = std::make_unique<Token>(std::stoll(string(itr, last), &idx), itr - first);
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

		/* 文字リテラル */
		if ('\'' == *itr)
		{
			current_token->_next = read_char_literal(itr);
			current_token = current_token->_next.get();
			itr += current_token->_str.size();
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

		error_token("不正なトークンです", current_token);
	}

	/* 最後に終端トークンを作成して繋ぐ */
	current_token->_next = std::make_unique<Token>(TokenKind::TK_EOF, last - first);
	/* 行数をセットする */
	add_line_number(head->_next.get());
	/* キーワードトークンを識別子トークンから分離する */
	Token::convert_keywords(head->_next.get());
	/* ダミーの次のトークン以降を切り離して返す */
	return std::move(head->_next);
}

/**
 * @brief トークンを順番にみていってキーワードと一致していれば種類をキーワードに帰る
 *
 * @param token トークン列
 */
void Token::convert_keywords(Token *token)
{
	for (Token *t = token; TokenKind::TK_EOF != t->_kind; t = t->_next.get())
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
	/* 識別子一覧 */
	static const std::vector<string> kws = {
		"return", "if", "else", "for", "while", "int", "sizeof", "char",
		"struct", "union", "short", "long", "void", "typedef", "_Bool",
		"enum", "static"};

	for (auto &kw : kws)
	{
		if (token->is_equal(kw))
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
			buf += read_escaped_char(p, p + 1);
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
 * @param pos '\'の次の文字
 * @param new_pos エスケープシーケンスの次の文字を返す
 * @return 対応する文字
 * @details \a, \b, \t, \n \v, \f, \r, \e
 */
char Token::read_escaped_char(string::const_iterator &new_pos, string::const_iterator &&pos)
{
	/* 16進数エスケープ */
	if (*pos == 'x')
	{
		++pos;
		if (!std::isxdigit(*pos))
		{
			error_at("無効な16進数エスケープシーケンスです", pos - current_input.begin());
		}

		int c = 0;
		/* 16進数と解釈できる文字が続く限り読み続ける */
		for (; std::isxdigit(*pos); ++pos)
		{
			c = (c << 4) + from_hex(*pos);
		}
		new_pos = pos;
		return c;
	}

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
 * @brief 文字リテラルを読み取る
 *
 * @param start 開始位置("'"の位置)
 * @return 文字リテラルのトークン
 */
unique_ptr<Token> Token::read_char_literal(string::const_iterator &start)
{
	auto pos = start + 1;
	if (current_input.end() == pos)
	{
		error_at("文字リテラルが閉じられていません", start - current_input.begin());
	}
	char c;
	/* エスケープされている場合 */
	if (*pos == '\\')
	{
		c = read_escaped_char(pos, pos + 1);
	}
	else
	{
		c = *pos++;
	}

	/* ２個めの"'"を探す */
	while (pos != current_input.end() && *pos != '\'')
	{
		++pos;
	}

	/* 見つからなければ閉じられていない */
	if (pos == current_input.end())
	{
		error_at("文字リテラルが閉じられていません", start - current_input.begin());
	}

	auto token = std::make_unique<Token>(c, start - current_input.begin());
	token->_str = string(start, pos + 1);
	return token;
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
 * @brief トークンが期待している演算子または識別子と一致するかどうか
 *
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
bool Token::is_equal(const std::string &op) const
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
	static const std::vector<string> kws = {"void", "_Bool", "char", "short", "int", "long", "struct", "union",
											"typedef", "enum", "static"};

	for (auto &kw : kws)
	{
		if (is_equal(kw))
		{
			return true;
		}
	}
	/* typedefされた定義を検索する */
	if (Object::find_typedef(this))
	{
		return true;
	}
	return false;
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
 *
 * @param token 対象のトークン
 * @param op 比較する文字列
 * @return 次のトークン
 */
Token *Token::skip(Token *token, std::string &&op)
{
	if (!token->is_equal(std::move(op)))
	{
		error_token(op + "が必要です", token);
	}
	return token->_next.get();
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタをnext_tokenにセットしtrueを返す。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param op 比較する文字列
 * @return 一致：true, 不一致：false
 */
bool Token::consume(Token **next_token, Token *current_token, std::string &&str)
{
	if (current_token->is_equal(std::move(str)))
	{
		*next_token = current_token->_next.get();
		return true;
	}
	*next_token = current_token;
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
bool Token::start_with(const std::string &str, const std::string &op)
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
	static const std::vector<string> kws = {"==", "!=", "<=", ">=", "->"};

	for (auto &kw : kws)
	{
		if (start_with(str, kw))
		{
			return kw.size();
		}
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
int64_t Token::get_number() const
{
	if (TokenKind::TK_NUM != this->_kind)
	{
		error_at("数値ではありません", this->_location);
	}
	return this->_value;
}

/**
 * @brief 16進数のchar型文字を対応する10進数に変換する
 *
 * @param c 16進数を表す文字
 * @return 変換した文字
 */
int Token::from_hex(const char &c)
{
	if ('0' <= c && c <= '9')
	{
		return c - '0';
	}
	if ('a' <= c && c <= 'f')
	{
		return c - 'a' + 10;
	}
	return c - 'A' + 10;
}

/**
 * @brief トークンリストを辿って行数をセットする
 *
 * @param token トークンリストの先頭
 */
void Token::add_line_number(Token *token)
{
	int pos = 0;
	const int total = current_input.end() - current_input.begin();
	int n = 1;

	while (token->_kind != TokenKind::TK_EOF && pos != total)
	{
		if (pos == token->_location)
		{
			token->_line_no = n;
			token = token->_next.get();
		}
		if (current_input[pos] == '\n')
		{
			++n;
		}
		++pos;
	}
}
