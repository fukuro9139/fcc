/**
 * @file tokenize.cpp
 * @author K.Fukunaga
 * @brief 文字列をトークンに変換するトークナイザの定義
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "tokenize.hpp"
#include "object.hpp"
#include "type.hpp"
#include <sstream>
#include <iterator>

/** 入力ファイルのリスト */
vector<unique_ptr<File>> Token::input_files;

/** 入力ファイル */
const File *Token::current_file = nullptr;

/** 行頭であるか */
bool Token::at_begining = false;

/** スペースであるか */
bool Token::has_space = false;

/***************/
/* Token Class */
/***************/

/* コンストラクタ */

Token::Token() = default;
Token::Token(const TokenKind &kind, const int &location)
	: _kind(kind), _location(location), _at_begining(at_begining), _file(current_file), _has_space(has_space)
{
	at_begining = false;
	has_space = false;
}

Token::Token(const int64_t &value, const int &location)
	: _kind(TokenKind::TK_NUM), _location(location), _val(move(value)), _at_begining(at_begining), _file(current_file),
	  _has_space(has_space)
{
	at_begining = false;
	has_space = false;
}

Token::Token(const TokenKind &kind, const int &location, string &&str)
	: _kind(kind), _location(location), _str(move(str)), _at_begining(at_begining), _file(current_file), _has_space(has_space)
{
	at_begining = false;
	has_space = false;
}

Token::Token(const Token &src)
	: _kind(src._kind), _val(src._val), _fval(src._fval), _ty(src._ty), _location(src._location),
	  _str(src._str), _file(src._file), _line_no(src._line_no), _at_begining(src._at_begining), _has_space(src._has_space)
{
	if (src._hideset)
	{
		_hideset = make_unique<std::unordered_set<string>>(*src._hideset);
	}
}

Token::Token(Token && src) = default;
Token & Token::operator=(Token && rhs) = default;

/**
 * @brief 入力されたパスのファイルを開いて中身を文字列として返す
 *
 * @param path ファイルパス
 * @return 読み取ったファイルの中身
 */
string Token::read_inputfile(const string &path)
{
	string input_data;

	if (path == "-")
	{
		string buf;
		/* 標準入力から読み取れなくなるまで読み込みを続ける */
		while (std::getline(std::cin, buf))
		{
			if (buf.empty())
			{
				break;
			}
			else
			{
				input_data += buf;
			}
		}
	}
	else
	{
		std::ifstream ifs(path);
		if (!ifs)
		{
			std::cerr << "ファイルが開けませんでした： " << path << std::endl;
			exit(1);
		}

		/* ファイルから読み込む */
		input_data = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	}

	/* ファイルが空または改行で終わっていない場合、'\n'を付け加える */
	if (input_data.empty() || input_data.back() != '\n')
	{
		input_data.push_back('\n');
	}

	return input_data;
}

/**
 * @brief 入力されたパスにあるファイルを開いてトークナイズする
 *
 * @param path ファイルパス
 * @return トークナイズした結果のトークンリスト
 */
unique_ptr<Token> Token::tokenize_file(const string &input_path)
{
	static int file_no = 0;
	/* ファイルを開いて中身を読み込む */
	auto content = read_inputfile(input_path);
	/* File構造体を生成 */
	auto file = make_unique<File>(input_path, ++file_no, content);
	/* リストに追加 */
	input_files.emplace_back(move(file));
	/* トークナイズ */
	return tokenize(input_files.back().get());
}

/**
 * @brief 入力文字列をトークナイズする
 *
 * @param filename 入力ファイル
 * @param input 入力文字列
 * @return トークナイズした結果のトークン・リスト
 */
unique_ptr<Token> Token::tokenize(const File *file)
{
	current_file = file;

	/* スタート地点としてダミーのトークンを作る */
	unique_ptr<Token> head = make_unique_for_overwrite<Token>();
	auto current_token = head.get();
	auto itr = current_file->_contents.cbegin();
	const auto first = current_file->_contents.cbegin();
	const auto last = current_file->_contents.cend();

	/* フラグをセット */
	at_begining = true;
	has_space = false;

	while (itr != last)
	{
		/* 改行 */
		if ('\n' == *itr)
		{
			at_begining = true;
			has_space = false;

			++itr;
			continue;
		}

		/* 空白文字をスキップ */
		if (std::isspace(*itr))
		{
			++itr;
			has_space = true;
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
			has_space = true;
			continue;
		}

		/* 数値 */
		if (std::isdigit(*itr) || ('.' == *itr && std::isdigit(*(itr + 1))))
		{
			current_token->_next = read_number(itr);
			current_token = current_token->_next.get();
			itr += current_token->_str.size();
			continue;
		}

		/* 文字列リテラル */
		if ('"' == *itr)
		{
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
			current_token->_next = make_unique<Token>(TokenKind::TK_IDENT, start - first, string(start, itr));
			current_token = current_token->_next.get();
			continue;
		}

		/* パンクチュエータ:構文的に意味を持つ記号またはキーワードこの段階では区別しない */
		size_t punct_len = read_punct(string(itr, last));
		if (punct_len)
		{
			/* 新しいトークンを生成してcurに繋ぎcurを一つ進める */
			current_token->_next = make_unique<Token>(TokenKind::TK_PUNCT, itr - first, string(itr, itr + punct_len));
			current_token = current_token->_next.get();
			itr += punct_len;
			continue;
		}

		error_token("不正なトークンです", current_token);
	}

	/* 最後に終端トークンを作成して繋ぐ */
	current_token->_next = make_unique<Token>(TokenKind::TK_EOF, last - first);
	/* 行数をセットする */
	add_line_number(head->_next.get());
	/* ダミーの次のトークン以降を切り離して返す */
	return move(head->_next);
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
	const auto first = current_file->_contents.cbegin();
	const auto last = current_file->_contents.cend();

	/* '"'が出てくるか末尾まで到達するまで読み込み続ける */
	for (; itr != last && *itr != '"'; ++itr)
	{
		/* 途中で改行や'\0'が出てきたらエラーとする */
		if (*itr == '\n' || *itr == '\0')
		{
			error_at("文字列が閉じられていません", start - first);
		}
		/* エスケープシーケンスは無視する */
		if (*itr == '\\')
		{
			++itr;
		}
	}
	/* 末尾まで'"'が見つからなければエラーとする */
	if (itr == last)
	{
		error_at("文字列が閉じられていません", start - first);
	}
	return itr;
}

/**
 * @brief 文字列リテラルを読み込む
 *
 * @param itr 文字列リテラルの開始位置。(1個目の'"'の位置)
 * @return 文字列リテラルを表すトークン
 */
unique_ptr<Token> Token::read_string_literal(string::const_iterator &itr)
{
	auto start = itr + 1;
	auto end = string_literal_end(start);
	string buf = "\"";

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

	/* 末尾に'"'を付け加える */
	buf.push_back('"');

	return make_unique<Token>(TokenKind::TK_STR, start - current_file->_contents.cbegin(), move(buf));
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
			error_at("無効な16進数エスケープシーケンスです", pos - current_file->_contents.cbegin());
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
 * @brief 数値を読み込む
 *
 * @param start 開始位置
 * @return 数値トークン
 */
unique_ptr<Token> Token::read_number(const string::const_iterator &start)
{
	unique_ptr<Token> token;

	/* '.'から始まる場合は必ず小数である */
	if ('.' != *start)
	{
		/* 整数値として読んでみる */
		token = read_int_literal(start);

		/* これらが整数値の後についていなければ整数 */
		constexpr string_view float_mark = ".eEfF";
		if (float_mark.find(*(start + token->_str.size())) == string::npos)
		{
			return token;
		}
	}

	/* そうでなければ小数である */
	size_t idx = 0;
	double val = 0;
	auto itr = start;

	try
	{
		val = std::stod(string(start, current_file->_contents.cend()), &idx);
	}
	catch (const std::invalid_argument &e)
	{
		error_at("無効な数値です", start - current_file->_contents.begin());
	}

	/* 変換した数値の桁数だけイテレーターを進める */
	itr += idx;

	shared_ptr<Type> ty;
	if ('f' == *itr || 'F' == *itr)
	{
		ty = Type::FLOAT_BASE;
		++itr;
	}
	else if ('l' == *itr || 'L' == *itr)
	{
		ty = Type::DOUBLE_BASE;
		++itr;
	}
	else
	{
		ty = Type::DOUBLE_BASE;
	}

	token = make_unique<Token>(TokenKind::TK_NUM, start - current_file->_contents.begin(), string(start, itr));
	token->_fval = val;
	token->_ty = ty;

	return token;
}

/**
 * @brief 整数値を読み込む。
 *
 * @param start 開始位置
 * @return 数値トークン
 */
unique_ptr<Token> Token::read_int_literal(const string::const_iterator &start)
{
	auto itr = start;
	int base = 0;

	/* 2進数以外はstollがプレフィックスで判断可能 */
	if (*itr == '0' && std::tolower(*(itr + 1)) == 'b' && std::isdigit(*(itr + 2)))
	{
		itr += 2;
		base = 2;
	}

	size_t idx = 0;
	int64_t val = 0;
	try
	{
		val = std::stoull(string(itr, current_file->_contents.cend()), &idx, base);
	}
	catch (const std::invalid_argument &e)
	{
		error_at("無効な数値です", itr - current_file->_contents.cbegin());
	}

	/* 変換した数値の桁数だけイテレーターを進める */
	itr += idx;

	/* 現在のイテレータ位置から末尾までの文字数 */
	int res = current_file->_contents.cend() - itr;

	/* 数値の次の3文字（サフィックスの可能性がある）を取り出す */
	string suffix = string(itr, itr + std::min(3, res));

	bool u = false, l = false;

	/* サフィックス(U, L, LL)を読み取る */
	if (suffix.starts_with("LLU") || suffix.starts_with("LLu") ||
		suffix.starts_with("llU") || suffix.starts_with("llu") ||
		suffix.starts_with("ULL") || suffix.starts_with("Ull") ||
		suffix.starts_with("uLL") || suffix.starts_with("ull"))
	{
		u = l = true;
		itr += 3;
	}
	else if (suffix.starts_with("LU") || suffix.starts_with("Lu") ||
			 suffix.starts_with("lU") || suffix.starts_with("lu") ||
			 suffix.starts_with("UL") || suffix.starts_with("uL") ||
			 suffix.starts_with("Ul") || suffix.starts_with("ul"))
	{
		u = l = true;
		itr += 2;
	}
	else if (suffix.starts_with("LL") || suffix.starts_with("ll"))
	{
		l = true;
		itr += 2;
	}
	else if (res != 0 && ('L' == *itr || 'l' == *itr))
	{
		l = true;
		itr += 1;
	}
	else if (res != 0 && ('U' == *itr || 'u' == *itr))
	{
		u = true;
		itr += 1;
	}

	shared_ptr<Type> ty;
	if ('0' != *start)
	{
		if (l && u)
		{
			ty = Type::ULONG_BASE;
		}
		else if (l)
		{
			ty = Type::LONG_BASE;
		}
		else if (u)
		{
			ty = (val >> 32) ? Type::ULONG_BASE : Type::UINT_BASE;
		}
		else
		{
			ty = (val >> 31) ? Type::LONG_BASE : Type::INT_BASE;
		}
	}
	else
	{
		if (l && u)
		{
			ty = Type::ULONG_BASE;
		}
		else if (l)
		{
			ty = (val >> 63) ? Type::ULONG_BASE : Type::LONG_BASE;
		}
		else if (u)
			ty = (val >> 32) ? Type::ULONG_BASE : Type::UINT_BASE;
		else if (val >> 63)
			ty = Type::ULONG_BASE;
		else if (val >> 32)
			ty = Type::LONG_BASE;
		else if (val >> 31)
			ty = Type::UINT_BASE;
		else
			ty = Type::INT_BASE;
	}

	auto token = make_unique<Token>(TokenKind::TK_NUM, start - current_file->_contents.cbegin(), string(start, itr));
	token->_val = val;
	token->_ty = ty;

	return token;
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
	if (current_file->_contents.cend() == pos)
	{
		error_at("文字リテラルが閉じられていません", start - current_file->_contents.cbegin());
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
	while (pos != current_file->_contents.cend() && *pos != '\'')
	{
		++pos;
	}

	/* 見つからなければ閉じられていない */
	if (pos == current_file->_contents.cend())
	{
		error_at("文字リテラルが閉じられていません", start - current_file->_contents.cbegin());
	}

	auto token = make_unique<Token>(c, start - current_file->_contents.cbegin());
	token->_str = string(start, pos + 1);
	token->_ty = Type::INT_BASE;
	return token;
}

/**
 * @brief トークンが期待している演算子または識別子と一致するかどうか
 *
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
bool Token::is_equal(string &&op) const
{
	return this->_str == op;
}

/**
 * @brief トークンが期待している演算子または識別子と一致するかどうか
 *
 * @param op 比較する文字列
 * @return 一致:true, 不一致:false
 */
bool Token::is_equal(const string &op) const
{
	return this->_str == op;
}

/**
 * @brief トークンが配列、配列、共用体の初期化式の末尾であるかを判定
 *
 * @return true 末尾である
 * @return false 末尾ではない
 */
bool Token::is_end() const
{
	return is_equal("}") || (is_equal(",") && _next->is_equal("}"));
}

/**
 * @brief トークンが型を表す識別子であるか
 *
 * @param 対象のトークン
 * @return true 型を表す識別子である
 * @return false 型を表す識別子ではない
 */
bool Token::is_typename() const
{
	/* 標準の型指定子 */
	for (auto &name : type_names)
	{
		if (name == _str)
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
 * @brief 文字列の先頭がパンクチュエーターかどうか判定しその長さを返す
 *
 * @details パンクチュエーターでなければ0を返す。判定は長さの長いパンクチュエーターから行われる。
 * @param first 文字列の開始位置のイテレーター
 * @param last 文字列の末端位置のイテレーター
 * @return パンクチュエーターの長さ
 */
size_t Token::read_punct(string &&str)
{
	for (const auto &kw : punctuators)
	{
		if (str.starts_with(kw))
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
	return this->_val;
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
	const int total = current_file->_contents.end() - current_file->_contents.begin();
	int n = 1;

	while (token->_kind != TokenKind::TK_EOF && pos != total)
	{
		if (pos == token->_location)
		{
			token->_line_no = n;
			token = token->_next.get();
			continue;
		}
		if (current_file->_contents[pos] == '\n')
		{
			++n;
		}
		++pos;
	}
}

/**
 * @brief 現在トークナイズしているファイルのポインタを返す
 *
 * @return 現在トークナイズしているファイルのポインタ
 */
const File *Token::get_current_file()
{
	return current_file;
}

/**
 * @brief インプットファイルのリストの参照を返す
 *
 * @return インプットファイルのリストの参照
 */
const vector<unique_ptr<File>> &Token::get_input_files()
{
	return input_files;
}

/**
 * @brief プリプロセスしたトークンを出力する
 *
 * @param token トークンリスト
 * @param output_path 出力先
 */
void Token::print_token(const unique_ptr<Token> &token, const string &output_path)
{
	auto os = open_file(output_path);

	int line = 1;
	for (auto tok = token.get(); TokenKind::TK_EOF != tok->_kind; tok = tok->_next.get())
	{
		if (line > 1 && tok->_at_begining)
		{
			*os << "\n";
		}
		if (tok->_has_space && !tok->_at_begining)
		{
			*os << " ";
		}
		*os << reverse_str_literal(tok);
		++line;
	}
	*os << endl;
}

/**
 * @brief トークンに対応する元々の入力文字列を返す。
 * 文字列リテラルは特殊文字がエスケープされているので元々の文字列から該当部分を出力する
 *
 * @param token 対象トークン
 * @return トークンに対応する元々の入力文字列
 */
string Token::reverse_str_literal(const Token *token)
{
	if (TokenKind::TK_STR != token->_kind)
	{
		return token->_str;
	}

	/* 文字列リテラル */
	const string &str = token->_file->_contents;
	string buf;
	buf.push_back('"');

	for (int i = token->_location; str[i] != '"'; ++i)
	{
		if (str[i] == '\\')
		{
			buf.push_back(str[i]);
			++i;
		}
		buf.push_back(str[i]);
	}
	buf.push_back('"');

	return buf;
}

/**
 * @brief _next以外の要素をコピーして新たなトークンを生成し、そのトークンへのポインタを返す。
 *
 * @param src コピー元
 * @return 生成したトークンへのポインタ
 */
unique_ptr<Token> Token::copy_token(const Token *src)
{
	return make_unique<Token>(*src);
}