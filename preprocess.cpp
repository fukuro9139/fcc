/**
 * @file preprocess.cpp
 * @author K.Fukunaga
 * @brief プリプロセスを行う
 * @version 0.1
 * @date 2023-08-15
 *
 * プリプロセッサーは入力としてトークンのリストを受け取り、出力として新しいトークンのリストを返す。
 * プリプロセスは再帰的なマクロがあったとしてもそれが停止することが保証されるように設計されている。
 * マクロは各トークンに対して一度だけ適用される。
 * つまり、T の直接または間接マクロ展開の結果にマクロ トークン T が現れると、T はそれ以上展開されない。
 * 例えば、TがUとして定義され、UがTとして定義された場合、トークンTはUに展開され、
 * 次にTに展開され、マクロ展開はその時点で停止する。
 * 上記の動作を実現するために各トークンはそれまでに展開されたマクロ名の集合をもつ。
 * この集合を "hideset "と呼び、hidesetは最初は空でマクロを展開するたびに、
 * そのマクロ名が結果のトークンのhidesetに追加される。
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "preprocess.hpp"
#include "tokenize.hpp"
#include "parse.hpp"

CondIncl::CondIncl() = default;
CondIncl::CondIncl(unique_ptr<Token> &&token, const BlockKind &ctx, bool included)
	: _token(move(token)), _ctx(ctx), _included(included) {}

/** #if関連の条件リスト */
vector<unique_ptr<CondIncl>> PreProcess::cond_incl;

/* マクロの一覧 */
std::unordered_map<string, unique_ptr<Token>> PreProcess::macros;

/**
 * @brief プリプロセスを行う
 *
 * @param token トークンリストの先頭
 * @return unique_ptr<Token>
 */
unique_ptr<Token> PreProcess::preprocess(unique_ptr<Token> &&token)
{
	/* プリプロセスマクロとディレクティブを処理 */
	token = preprocess2(move(token));

	/* #ifと#endifの対応を確認 */
	if (!cond_incl.empty())
	{
		error_token("対応する#endifが存在しません", cond_incl.back()->_token.get());
	}

	/* 識別子を認識 */
	convert_keywords(token);
	return token;
}

/**
 * @brief トークンリストを先頭から巡回してプリプロセスマクロとディレクティブを処理する
 *
 * @param トークンリスト
 * @return 処理後のトークンリスト
 */
unique_ptr<Token> PreProcess::preprocess2(unique_ptr<Token> &&token)
{
	auto head = make_unique_for_overwrite<Token>();
	auto cur = head.get();

	while (TokenKind::TK_EOF != token->_kind)
	{
		/* マクロであれば展開する */
		if (expand_macro(token, move(token)))
		{
			continue;
		}

		/* 行頭'#'でなければそのまま */
		if (!is_hash(token))
		{
			cur->_next = move(token);
			cur = cur->_next.get();
			token = move(cur->_next);
			continue;
		}

		auto start = move(token);
		token = move(start->_next);

		/* '#'のみの行は無視する */
		if (token->_at_begining)
		{
			continue;
		}

		if (token->is_equal("include"))
		{
			token = move(token->_next);

			/* '#include'の次はファイル名 */
			if (TokenKind::TK_STR != token->_kind)
			{
				error_token("ファイル名ではありません", token.get());
			}

			string inc_path;
			if (token->_str[0] == '/')
			{
				inc_path = token->_str;
			}
			else
			{
				/* 現在のファイル */
				fs::path src_path = token->_file->_name;
				/* 文字列リテラルなので前後の'"'を取り除く */
				const auto name = token->_str.substr(1, token->_str.size() - 2);
				/* includeするファイルのパスを生成、現在のファイルからの相対パス */
				inc_path = src_path.replace_filename(name).string();
			}
			auto token2 = Token::tokenize_file(inc_path);

			/* 次の行頭までスキップする */
			token = skip_line(move(token->_next));
			/* includeしたトークンを繋ぐ */
			token = append(move(token2), move(token));
			continue;
		}

		if (token->is_equal("define"))
		{
			token = move(token->_next);
			if (TokenKind::TK_IDENT != token->_kind)
			{
				error_token("マクロ名は識別子である必要があります", token.get());
			}
			/* マクロを登録する */
			auto name = move(token);
			add_macro(name, copy_line(token, move(name->_next)));
			continue;
		}

		if (token->is_equal("undef"))
		{
			token = move(token->_next);
			if (TokenKind::TK_IDENT != token->_kind)
			{
				error_token("マクロ名は識別子である必要があります", token.get());
			}
			/* マクロを削除する */
			delete_macro(token->_str);
			token = skip_line(move(token->_next));
			continue;
		}

		if (token->is_equal("if"))
		{
			auto val = evaluate_const_expr(token, move(token));
			push_cond_incl(move(start), val);
			/* #if の後の条件式が0（偽）であった場合は#endifまでスキップ */
			if (val == 0)
			{
				token = skip_cond_incl(move(token));
			}
			continue;
		}

		if (token->is_equal("elif"))
		{
			/* 対になる#ifが存在しないまたは直前が#elseのときエラー */
			if (cond_incl.empty() || BlockKind::IN_ELSE == cond_incl.back()->_ctx)
			{
				error_token("対応する#ifが存在しません", start.get());
			}
			/* #elif節に入ったので種類を変える */
			cond_incl.back()->_ctx = BlockKind::IN_ELIF;

			/* 直前の節の条件式が偽である、かつこの節の条件式が真であるとき */
			if (!cond_incl.back()->_included && evaluate_const_expr(token, move(token)) != 0)
			{
				cond_incl.back()->_included = true;
			}
			/* それ以外はスキップ */
			else
			{
				token = skip_cond_incl(move(token));
			}
			continue;
		}

		if (token->is_equal("else"))
		{
			/* 対になる#ifが存在しないまたは直前が#elseのときエラー */
			if (cond_incl.empty() || BlockKind::IN_ELSE == cond_incl.back()->_ctx)
			{
				error_token("対応する#ifが存在しません", start.get());
			}
			/* else節に入ったので種類を変える */
			cond_incl.back()->_ctx = BlockKind::IN_ELSE;
			/* #elseと同じ行のトークンを無視 */
			token = skip_line(move(token->_next));

			/* #if節の方が有効な場合,else節はスキップ */
			if (cond_incl.back()->_included)
			{
				token = skip_cond_incl(move(token));
			}
			continue;
		}

		if (token->is_equal("endif"))
		{
			/* 対になる#ifが存在しないとき */
			if (cond_incl.empty())
			{
				error_token("対応する#ifが存在しません", start.get());
			}
			cond_incl.pop_back();
			token = skip_line(move(token->_next));
			continue;
		}

		error_token("無効なプリプロセッサディレクティブです", token.get());
	}

	cur->_next = move(token);
	return move(head->_next);
}

/**
 * @brief トークン1の末尾にトークン2を付け加える
 *
 * @param token1
 * @param token2
 * @return トークン1の末尾にトークン2を付け加えたトークン
 */
unique_ptr<Token> PreProcess::append(unique_ptr<Token> &&token1, unique_ptr<Token> &&token2)
{
	if (TokenKind::TK_EOF == token1->_kind)
	{
		return token2;
	}

	auto tok = token1.get();
	/* トークン1を末尾まで辿る */
	while (TokenKind::TK_EOF != tok->_next->_kind)
	{
		tok = tok->_next.get();
	}
	tok->_next = move(token2);
	return token1;
}

/**
 * @brief トークンを順番にみていってキーワードと一致していれば種類をキーワードに帰る
 *
 * @param token トークン列
 */
void PreProcess::convert_keywords(unique_ptr<Token> &token)
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
bool PreProcess::is_keyword(const Token *token)
{
	for (auto &kw : keywords)
	{
		if (kw == token->_str)
		{
			return true;
		}
	}
	return false;
}

/**
 * @brief トークンが行頭で'#'であるか
 *
 * @param token 対象トークン
 * @return true 行頭'#'である
 * @return false 行頭'#'ではない
 */
bool PreProcess::is_hash(const unique_ptr<Token> &token)
{
	return token->_at_begining && token->is_equal("#");
}

/**
 * @brief 一部のプリプロセッサディレクティブは次の行の前に余分なトークンを許容する。
 * この関数では次の行頭までトークンをスキップする
 *
 * @param token 確認するトークン
 * @return 直後の行頭のトークン
 */
unique_ptr<Token> PreProcess::skip_line(unique_ptr<Token> &&token)
{
	if (token->_at_begining)
	{
		return token;
	}

	warn_token("このトークンは無視されます", token.get());
	while (!token->_at_begining)
	{
		token = move(token->_next);
	}
	return token;
}

/**
 * @brief トークンの内容をコピーしてkindをTK_EOF, strを""に変更する
 *
 * @param src コピー元
 * @return 生成したトークン
 */
unique_ptr<Token> PreProcess::new_eof_token(const unique_ptr<Token> &src)
{
	auto t = Token::copy_token(src.get());
	t->_kind = TokenKind::TK_EOF;
	t->_str = "";
	return t;
}

/**
 * @brief #elseまたは#endifが出てくるまでトークンをスキップする
 *
 * @param token スキップを開始するトークン
 * @return #endifまたは末尾にあたるトークン
 */
unique_ptr<Token> PreProcess::skip_cond_incl(unique_ptr<Token> &&token)
{
	while (TokenKind::TK_EOF != token->_kind)
	{
		/* #if 0にネストされた#if~#endifはスキップする */
		if (is_hash(token) && token->_next->is_equal("if"))
		{
			token = skip_cond_incl2(move(token->_next->_next));
			continue;
		}

		if (
			is_hash(token) &&
			(token->_next->is_equal("else") || token->_next->is_equal("endif") || token->_next->is_equal("elif")))
		{
			break;
		}
		token = move(token->_next);
	}
	return token;
}

/**
 * @brief #endifが出てくるまでトークンをスキップする
 *
 * @param token スキップを開始するトークン
 * @return #endifまたは末尾にあたるトークン
 */
unique_ptr<Token> PreProcess::skip_cond_incl2(unique_ptr<Token> &&token)
{
	while (TokenKind::TK_EOF != token->_kind)
	{
		/* #if 0にネストされた#if~#endifはスキップする */
		if (is_hash(token) && token->_next->is_equal("if"))
		{
			token = skip_cond_incl2(move(token->_next->_next));
			continue;
		}

		if (is_hash(token) && token->_next->is_equal("endif"))
		{
			return move(token->_next->_next);
		}
		token = move(token->_next);
	}
	return token;
}

/**
 * @brief 次の行頭または末尾までのトークンを全てコピーする。
 * この関数は#if文を評価するためのトークンリストを作成するために用いる。
 *
 * @param next_token 次の文頭のトークンを返すための参照
 * @param current_token 開始位置のトークン
 * @return 生成したトークンリスト
 */
unique_ptr<Token> PreProcess::copy_line(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	auto head = make_unique_for_overwrite<Token>();
	auto cur = head.get();

	for (; !current_token->_at_begining; current_token = move(current_token->_next))
	{
		cur->_next = Token::copy_token(current_token.get());
		cur = cur->_next.get();
	}
	cur->_next = new_eof_token(current_token);
	next_token = move(current_token);
	return move(head->_next);
}

/**
 * @brief 次の文末までを定数式として評価する
 *
 * @param next_token 次の文頭のトークンを返すための参照
 * @param current_token 開始位置のトークン
 * @return 評価結果
 */
long PreProcess::evaluate_const_expr(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	auto start = current_token.get();
	/* 文末までのトークンをコピー */
	auto expr = copy_line(next_token, move(current_token->_next));

	/* 定数式の中のマクロを展開 */
	expr = preprocess2(move(expr));

	/* #ifの後に条件式がなければエラー */
	if (TokenKind::TK_EOF == expr->_kind)
	{
		error_token("条件式が存在しません", start);
	}

	Token *rest;
	/* 定数式を評価 */
	auto val = Node::const_expr(&rest, expr.get());

	/* 定数式の評価後に余ったトークンがあればエラー */
	if (TokenKind::TK_EOF != rest->_kind)
	{
		error_token("余分なトークンが存在します", rest);
	}

	return val;
}

/**
 * @brief #ifをリストに追加する
 *
 * @param token #ifに対応するトークン
 * @return 追加したCondIncl構造体のポインタ
 */
CondIncl *PreProcess::push_cond_incl(unique_ptr<Token> &&token, bool included)
{
	auto ci = make_unique<CondIncl>(move(token), BlockKind::IN_THEN, included);
	cond_incl.emplace_back(move(ci));
	return cond_incl.back().get();
}

/**
 * @brief 定義済みのマクロを検索して定義されていれば展開先のトークンリストを返す
 *
 * @param token マクロに対応するトークン
 * @return マクロの展開先のトークンリスト
 */
Token *PreProcess::find_macro(const unique_ptr<Token> &token)
{
	if (TokenKind::TK_IDENT != token->_kind)
	{
		return nullptr;
	}

	if (macros.contains(token->_str))
	{
		return macros[token->_str].get();
	}
	else
	{
		return nullptr;
	}
}

/**
 * @brief マクロを登録する。マクロが定義済みの場合は上書きする。
 *
 * @param macro
 * @return Token*
 */
Token *PreProcess::add_macro(const unique_ptr<Token> &token, unique_ptr<Token> &&body)
{
	if (macros.contains(token->_str))
	{
		warn_token("マクロが再定義されています", token.get());
	}
	macros[token->_str] = move(body);
	return macros[token->_str].get();
}

/**
 * @brief トークンがマクロで登録済みならマクロを展開しtrueを返す。そうではないならfalseを返す
 *
 * @param next__token マクロを展開した後のトークンリストを返すための参照
 * @param current_token 展開するマクロに対応するトークン
 * @return true トークンがマクロである
 * @return false マクロを展開できない
 */
bool PreProcess::expand_macro(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	if (current_token->_hideset && current_token->_hideset->contains(current_token->_str))
	{
		next_token = move(current_token);
		return false;
	}

	auto m = find_macro(current_token);
	if (!m)
	{
		next_token = move(current_token);
		return false;
	}

	auto name = current_token->_str;

	/* マクロの展開先のトークンリストをコピーする */
	auto head = make_unique_for_overwrite<Token>();
	auto cur = head.get();

	/* 末尾まで順番に一つずつコピー */
	while (TokenKind::TK_EOF != m->_kind)
	{
		cur->_next = Token::copy_token(m);
		m = m->_next.get();
		cur = cur->_next.get();
		/* hidesetに展開するマクロ名を追加 */
		add_hideset(cur->_hideset, name);
	}
	/* EOFトークンをコピー */
	cur->_next = Token::copy_token(m);

	/* 展開したマクロのトークンリストの末尾に現在のトークンシルトを接続する */
	next_token = append(move(head->_next), move(current_token->_next));
	return true;
}

/**
 * @brief hidesetにマクロ名を追加する
 *
 * @param hs 対象となるhideset
 * @param name 追加するマクロの名前
 */
void PreProcess::add_hideset(Hideset &hs, const string &name)
{
	/* hsが空なら新しく作成する */
	if (!hs)
	{
		hs = make_unique<std::unordered_set<string>>();
	}
	hs->insert(name);
}

/**
 * @brief マクロを削除する
 *
 * @param name 削除するマクロの名前
 */
void PreProcess::delete_macro(const string &name)
{
	if (macros.contains(name))
	{
		macros.erase(name);
	}
}