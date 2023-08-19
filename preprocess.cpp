/**
 * @file preprocess.cpp
 * @author K.Fukunaga
 * @brief プリプロセスを行う
 * @version 0.1
 * @date 2023-08-15
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "preprocess.hpp"
#include "tokenize.hpp"
#include "parse.hpp"
#include "input.hpp"

CondIncl::CondIncl() = default;
CondIncl::CondIncl(unique_ptr<Token> &&token, const BlockKind &ctx, bool included)
	: _token(move(token)), _ctx(ctx), _included(included) {}

Macro::Macro(unique_ptr<Token> &&body, const bool &objlike)
	: _body(move(body)), _is_objlike(objlike)
{
}

/** #if関連の条件リスト */
vector<unique_ptr<CondIncl>> PreProcess::cond_incl;

/** マクロの一覧 */
std::unordered_map<string, unique_ptr<Macro>> PreProcess::macros;

/** 入力オプション */
const Input *PreProcess::input_options = nullptr;

/**
 * @brief プリプロセスを行う
 *
 * @param token トークンリストの先頭
 * @return unique_ptr<Token>
 */
unique_ptr<Token> PreProcess::preprocess(unique_ptr<Token> &&token, const unique_ptr<Input> &in)
{
	/* 入力オプション */
	input_options = in.get();

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
			bool dquote;
			string filename = read_include_filename(token, move(token->_next), dquote);

			/* includeするパスを検索する */
			auto inc_path = search_include_path(start->_file->_name, filename, dquote);
			/* 見つからなければエラー */
			if (inc_path.empty())
			{
				error_token("ファイルが見つかりません", start.get());
			}

			/* includeしたトークンを繋ぐ */
			token = include_file(move(token), inc_path);
			continue;
		}

		if (token->is_equal("define"))
		{
			/* マクロの定義を読み取る */
			read_macro_definition(token, move(token->_next));
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

		if (token->is_equal("ifdef"))
		{
			bool defined = find_macro(token->_next);
			/* #ifdef節に入る */
			push_cond_incl(move(start), defined);
			/* 余分なトークンを飛ばす */
			token = skip_line(move(token->_next->_next));
			/* 定義されていなければ節を飛ばす */
			if (!defined)
			{
				token = skip_cond_incl(move(token));
			}
			continue;
		}

		if (token->is_equal("ifndef"))
		{
			bool defined = find_macro(token->_next);
			/* #ifndef節に入る */
			push_cond_incl(move(start), !defined);
			/* 余分なトークンを飛ばす */
			token = skip_line(move(token->_next->_next));
			/* 定義されていれば節を飛ばす */
			if (defined)
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
		/* #if 0にネストされた#if, #ifdef, #ifndefはスキップする */
		if (is_hash(token) && (token->_next->is_equal("if") ||
							   token->_next->is_equal("ifdef") ||
							   token->_next->is_equal("ifndef")))
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
		/* #if 0にネストされた#if, #ifdef, #ifndefはスキップする */
		if (is_hash(token) && (token->_next->is_equal("if") ||
							   token->_next->is_equal("ifdef") ||
							   token->_next->is_equal("ifndef")))
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
	/* 文末までのトークンを読み取る */
	auto expr = read_const_expr(next_token, move(current_token->_next));

	/* 定数式の中のマクロを展開 */
	expr = preprocess2(move(expr));

	/* #ifの後に条件式がなければエラー */
	if (TokenKind::TK_EOF == expr->_kind)
	{
		error_token("条件式が存在しません", start);
	}

	/* 未定義のマクロは0として扱う */
	for (auto t = expr.get(); TokenKind::TK_EOF != t->_kind; t = t->_next.get())
	{
		if (TokenKind::TK_IDENT == t->_kind)
		{
			auto next = move(t->_next);
			*t = move(*new_num_token(0, t));
			t->_next = move(next);
		}
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
 * @brief 定数式を読み取り途中にdefinedマクロがあれば展開する
 *
 * @param next_token 次の文頭のトークンを返すための参照
 * @param current_token 開始位置のトークン
 * @return 読み取り結果
 */
unique_ptr<Token> PreProcess::read_const_expr(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	auto tok = copy_line(next_token, move(current_token));
	auto head = make_unique_for_overwrite<Token>();
	auto cur = head.get();

	while (TokenKind::TK_EOF != tok->_kind)
	{
		/* 'defined(M)'または'define M'はMが定義されていれば1されていなければ0 */
		if (tok->is_equal("defined"))
		{
			auto start = move(tok);
			tok = move(start->_next);
			bool has_paren = tok->is_equal("(");
			if (has_paren)
			{
				tok = move(tok->_next);
			}

			if (TokenKind::TK_IDENT != tok->_kind)
			{
				error_token("definedの引数はマクロ名である必要があります", start.get());
			}

			cur->_next = new_num_token(macros.contains(tok->_str) ? 1 : 0, start.get());
			cur = cur->_next.get();

			tok = move(tok->_next);
			if (has_paren)
			{
				tok = skip(move(tok), ")");
			}
			continue;
		}
		cur->_next = move(tok);
		cur = cur->_next.get();
		tok = move(cur->_next);
	}
	cur->_next = move(tok);
	return move(head->_next);
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
 * @brief マクロの定義を読み込む
 *
 * @param next_token 次の文頭のトークンを返すための参照
 * @param current_token 開始位置のトークン
 */
void PreProcess::read_macro_definition(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	if (TokenKind::TK_IDENT != current_token->_kind)
	{
		error_token("マクロ名は識別子である必要があります", current_token.get());
	}
	auto name = move(current_token);
	current_token = move(name->_next);

	/* 関数マクロ */
	if (!current_token->_has_space && current_token->is_equal("("))
	{
		auto params = read_macro_params(current_token, move(current_token->_next));
		auto m = add_macro(name, false, copy_line(next_token, move(current_token)));
		m->_params = move(params);
	}
	/* オブジェクトマクロ */
	else
	{
		add_macro(name, true, copy_line(next_token, move(current_token)));
	}
}

/**
 * @brief 定義済みのマクロを検索して定義されていれば展開先のトークンリストを返す
 *
 * @param token マクロに対応するトークン
 * @return マクロの展開先のトークンリスト
 */
Macro *PreProcess::find_macro(const unique_ptr<Token> &token)
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
 * @param token マクロを名前を表しているトークン
 * @param is_objlike マクロがオブジェクトマクロかどうか
 * @param body マクロの展開先
 * @return Token*
 */
Macro *PreProcess::add_macro(const unique_ptr<Token> &token, const bool &is_objlike, unique_ptr<Token> &&body)
{
	if (macros.contains(token->_str))
	{
		warn_token("マクロが再定義されています", token.get());
	}
	macros[token->_str] = make_unique<Macro>(move(body), is_objlike);
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
	auto name = current_token->_str;
	auto macro_token = move(current_token);

	if (macro_token->_hideset && macro_token->_hideset->contains(name))
	{
		next_token = move(macro_token);
		return false;
	}

	auto m = find_macro(macro_token);
	if (!m)
	{
		next_token = move(macro_token);
		return false;
	}

	/* オブジェクトマクロ */
	if (m->_is_objlike)
	{

		auto body = substitute_obj_macro(macro_token, m->_body);
		/* 展開したマクロのトークンリストの末尾に現在のトークンシルトを接続する */
		next_token = append(move(body), move(macro_token->_next));
		next_token->_at_begining = macro_token->_at_begining;
		next_token->_has_space = macro_token->_has_space;
		return true;
	}

	/* 関数マクロ */
	/* 引数を取らない関数マクロはただの変数として扱う */
	if (!macro_token->_next->is_equal("("))
	{
		next_token = move(macro_token);
		return false;
	}

	/* 引数を読み取る */
	auto args = read_macro_args(current_token, move(macro_token->_next->_next), *m->_params);
	/* 引数を代入してマクロを展開する */
	auto body = substitute_func_macro(macro_token, m->_body, *args);
	next_token = append(move(body), move(current_token));
	next_token->_has_space = macro_token->_has_space;
	next_token->_at_begining = macro_token->_at_begining;
	return true;
}

/**
 * @brief オブジェクトマクロを展開する
 *
 * @param dst 展開元のトークン
 * @param macro 登録された展開先のトークンリスト
 * @return コピーされた展開先のトークンリスト
 */
unique_ptr<Token> PreProcess::substitute_obj_macro(const unique_ptr<Token> &dst, const unique_ptr<Token> &macro)
{
	/* マクロの展開先のトークンリストをコピーする */
	auto head = make_unique_for_overwrite<Token>();
	copy_macro_token(head.get(), macro.get(), dst->_str, dst->_hideset);
	return move(head->_next);
}

/**
 * @brief 関数マクロを展開する
 *
 * @param dst 展開元のトークン
 * @param macro 展開するマクロ
 * @param args 関数マクロの引数
 * @return マクロ展開後のトークンリスト
 */
unique_ptr<Token> PreProcess::substitute_func_macro(const unique_ptr<Token> &dst, const unique_ptr<Token> &macro, const MacroArgs &args)
{
	auto name = dst->_str;

	/* マクロの展開先のトークンリストをコピーする */
	auto head = make_unique_for_overwrite<Token>();
	auto cur = head.get();
	auto tok = macro.get();

	while (TokenKind::TK_EOF != tok->_kind)
	{
		/* #引数は引数をそのまま文字列リテラルとして置き換える */
		if (tok->is_equal("#"))
		{
			if (!args.contains(tok->_next->_str))
			{
				error_token("'#'の後にはマクロ引数が必要です", tok->_next.get());
			}
			cur->_next = stringize(dst.get(), args.at(tok->_next->_str).get());
			cur->_next->_at_begining = tok->_at_begining;
			cur->_next->_has_space = tok->_has_space;
			cur = cur->_next.get();
			tok = tok->_next->_next.get();
			continue;
		}

		/* ##演算子 */
		if (tok->is_equal("##"))
		{
			if (tok == head.get())
			{
				error_token("##演算子はマクロ定義の冒頭では使えません", tok);
			}
			if (TokenKind::TK_EOF == tok->_next->_kind)
			{
				error_token("##演算子はマクロ定義の末尾では使えません", tok);
			}

			/* ##演算子の二番目のオペランドが引数の場合 */
			if (args.contains(tok->_next->_str))
			{
				auto arg_token = args.at(tok->_next->_str).get();
				/* 引数が空でないとき引数の先頭トークンと連結する */
				if (TokenKind::TK_EOF != arg_token->_kind)
				{
					*cur = move(*paste(cur, arg_token));
					for (auto t = arg_token->_next.get(); TokenKind::TK_EOF != t->_kind; t = t->_next.get())
					{
						cur->_next = Token::copy_token(t);
						cur = cur->_next.get();
					}
				}
				tok = tok->_next->_next.get();
				continue;
			}

			/* ##演算子の二番目のオペランドが引数以外の場合 */
			*cur = move(*paste(cur, tok->_next.get()));
			tok = tok->_next->_next.get();
			continue;
		}

		/* (引数トークン)##(トークン) */
		if (args.contains(tok->_str) && tok->_next->is_equal("##"))
		{
			auto rhs = tok->_next->_next.get();
			auto arg_token = args.at(tok->_str).get();

			/* １番目のオペランドの引数トークンが空の場合 */
			if (TokenKind::TK_EOF == arg_token->_kind)
			{
				/* 2番目のオペランドのトークンが引数の場合 */
				if (args.contains(rhs->_str))
				{
					for (auto t = args.at(rhs->_str).get(); TokenKind::TK_EOF != t->_kind; t = t->_next.get())
					{
						cur->_next = Token::copy_token(t);
						cur = cur->_next.get();
					}
				}
				/* 2番目のオペランドのトークンが引数でないトークンの場合 */
				else
				{
					cur->_next = Token::copy_token(rhs);
					cur = cur->_next.get();
				}
				tok = rhs->_next.get();
				continue;
			}
			/* １番目のオペランドの引数トークンがではない場合、引数の展開だけして連結は次のループに任せる */
			for (auto t = arg_token; TokenKind::TK_EOF != t->_kind; t = t->_next.get())
			{
				cur->_next = Token::copy_token(t);
				cur = cur->_next.get();
			}
			tok = tok->_next.get();
			continue;
		}

		/* マクロの引数トークンの場合。マクロの引数にマクロが含まれる場合はマクロは完全に展開する */
		if (args.contains(tok->_str))
		{
			copy_macro_token(cur, args.at(tok->_str).get(), name, dst->_hideset);
			cur->_next = preprocess2(move(cur->_next));
			cur->_next->_has_space = tok->_has_space;
			cur->_next->_at_begining = tok->_at_begining;
			while (TokenKind::TK_EOF != cur->_next->_kind)
			{
				cur = cur->_next.get();
			}
			tok = tok->_next.get();
			continue;
		}

		/* マクロの引数トークンではない場合 */
		auto t = Token::copy_token(tok);
		add_hideset(t->_hideset, name);
		if (dst->_hideset)
		{
			t->_hideset->merge(*dst->_hideset);
		}
		cur->_next = move(t);
		cur = cur->_next.get();
		tok = tok->_next.get();
	}
	cur->_next = Token::copy_token(tok);
	return move(head->_next);
}

/**
 * @brief 関数マクロの引数の定義を読み取る
 *
 * @param next__token 次ののトークンリストを返すための参照
 * @param current_token 引数の開始位置のトークン
 * @return 読み取った引数リスト
 */
unique_ptr<vector<string>> PreProcess::read_macro_params(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	auto params = make_unique<vector<string>>();
	bool first = true;

	/* ')'が出てくるまで読み込み続ける */
	while (!current_token->is_equal(")"))
	{
		/* ２個目以降は','区切りが必要 */
		if (!first)
		{
			current_token = skip(move(current_token), ",");
		}
		first = false;

		/* 引数の名前は識別子である必要がある */
		if (TokenKind::TK_IDENT != current_token->_kind)
		{
			error_token("識別子ではありません", current_token.get());
		}

		params->emplace_back(current_token->_str);
		current_token = move(current_token->_next);
	}
	next_token = move(current_token->_next);
	return params;
}

/**
 * @brief 関数マクロの引数となる定数式を1個分読み込む
 *
 * @param next__token 次ののトークンリストを返すための参照
 * @param current_token 引数の開始位置のトークン
 * @return 読み取った引数に対応するトークンリスト
 */
unique_ptr<Token> PreProcess::resd_macro_arg_one(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	auto head = make_unique_for_overwrite<Token>();
	auto cur = head.get();
	/* '()'の深さ */
	int level = 0;

	/* 深さが0 かつ ','または')'が出てくるまで読み込み続ける */
	while (level > 0 || (!current_token->is_equal(",") && !current_token->is_equal(")")))
	{
		if (TokenKind::TK_EOF == current_token->_kind)
		{
			error_token("引数が足りません", current_token.get());
		}
		/* '()'が出てきたら深さを変化させる */
		if (current_token->is_equal("("))
		{
			++level;
		}
		else if (current_token->is_equal(")"))
		{
			--level;
		}

		cur->_next = Token::copy_token(current_token.get());
		cur = cur->_next.get();
		current_token = move(current_token->_next);
	}
	cur->_next = new_eof_token(current_token);
	next_token = move(current_token);
	return move(head->_next);
}

/**
 * @brief 関数マクロの引数となる定数式を読み込み定義された名前と対応付ける
 *
 * @param next__token 次ののトークンリストを返すための参照
 * @param current_token 引数の開始位置のトークン
 * @return 定義された引数名と読み取った定数式を対応させたリスト
 */
unique_ptr<MacroArgs> PreProcess::read_macro_args(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token, const vector<string> &params)
{
	auto args = make_unique<MacroArgs>();
	bool first = true;

	for (const auto &pp : params)
	{
		if (!first)
		{
			/* 2個目以降では','区切りが必要 */
			current_token = skip(move(current_token), ",");
		}
		first = false;
		(*args)[pp] = resd_macro_arg_one(current_token, move(current_token));
	}
	if (!current_token->is_equal(")"))
	{
		error_token("引数が多すぎます", current_token.get());
	}
	next_token = move(current_token->_next);
	return args;
}

/**
 * @brief hidesetにマクロ名を追加する
 *
 * @param hs 対象となるhideset
 * @param name 追加するマクロの名前
 */
void PreProcess::add_hideset(unique_ptr<Hideset> &hs, const string &name)
{
	/* hsが空なら新しく作成する */
	if (!hs)
	{
		hs = make_unique<Hideset>();
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

/**
 * @brief トークンが期待している文字列であれば次のトークンを返す。それ以外であればエラー
 *
 * @param token 対象のトークン
 * @param op 期待している文字列
 */
unique_ptr<Token> PreProcess::skip(unique_ptr<Token> &&token, const string &op)
{
	if (!token->is_equal(op))
	{
		error_token("\'" + op + "\'が必要です", token.get());
	}
	return move(token->_next);
}

/**
 * @brief マクロの展開先のトークンリストをdstにコピーして繋ぐ
 *
 * @param dst トークンリストを繋ぐ先
 * @param macro マクロの展開先
 * @param name マクロの名前
 * @param hs マクロの展開元のhideset
 */
void PreProcess::copy_macro_token(Token *dst, const Token *macro, const string &name, const unique_ptr<Hideset> &hs)
{
	auto tok = macro;
	auto cur = dst;

	/* 末尾まで順番に一つずつコピー */
	while (TokenKind::TK_EOF != tok->_kind)
	{
		cur->_next = Token::copy_token(tok);
		tok = tok->_next.get();
		cur = cur->_next.get();
		/* hidesetに展開するマクロ名を追加 */
		add_hideset(cur->_hideset, name);
		/* 展開元のトークンのhidesetとマージする */
		if (hs)
		{
			cur->_hideset->merge(*hs);
		}
	}
	/* EOFトークンをコピー */
	cur->_next = Token::copy_token(tok);
}

/**
 * @brief 文字列に含まれる特殊文字をエスケープする
 *
 * @param str 変換対象の文字列
 * @return エスケープした文字列
 */
string PreProcess::quate_string(const string &str)
{
	string buf;
	/* 文字列リテラルは'"'で始まる */
	buf.push_back('"');
	/* 特殊文字はエスケープしてコピー */
	for (const auto &c : str)
	{
		if (c == '\\' || c == '"')
		{
			buf.push_back('\\');
		}
		buf.push_back(c);
	}
	/* 文字列リテラルは'"'で終わる */
	buf.push_back('"');
	return buf;
}

/**
 * @brief 文字列を文字列リテラルトークンに変換する
 *
 * @param str 対象文字列
 * @param ref エラー報告用情報のテンプレートにするトークン
 * @return 文字列リテラルトークン
 */
unique_ptr<Token> PreProcess::new_str_token(const string &str, const Token *ref)
{
	/* 特殊文字をエスケープ */
	string s = quate_string(str);
	s.push_back('\n');
	/* 文字列をもった仮想的なファイルをトークナイズする */
	return vir_file_tokenize(s, ref);
}

/**
 * @brief 数値を数値トークンに変換する
 *
 * @param val 数値
 * @param ref エラー報告用情報のテンプレートにするトークン
 * @return 数値トークン
 */
unique_ptr<Token> PreProcess::new_num_token(const int &val, const Token *ref)
{
	string s = std::to_string(val);
	s.push_back('\n');
	return vir_file_tokenize(s, ref);
}

/**
 * @brief トークンが持つ文字列を結合して１つの文字列にする
 *
 * @param token 文字列化するトークンリスト
 * @return トークンを統合した文字列
 */
string PreProcess::join_tokens(const Token *start, const Token *end)
{
	string buf;
	for (auto t = start; t != end && TokenKind::TK_EOF != t->_kind; t = t->_next.get())
	{
		if (t != start && t->_has_space)
		{
			buf.push_back(' ');
		}
		buf += Token::reverse_str_literal(t);
	}
	buf.shrink_to_fit();
	return buf;
}

/**
 * @brief 文字列化演算子'#'に対応するための関数。トークンリストargのトークンを文字列リテラル
 * として統合し、トークナイズしたトークンを返す。
 *
 * @param ref マクロの展開元のトークン
 * @param arg 文字列化するトークン列
 * @return unique_ptr<Token>
 */
unique_ptr<Token> PreProcess::stringize(const Token *ref, const Token *arg)
{
	auto s = join_tokens(arg, nullptr);
	return new_str_token(s, ref);
}

/**
 * @brief 2つのトークンを連結する
 *
 * @param lhs 左側のトークン
 * @param rhs 右側のトークン
 * @return 連結したトークン
 */
unique_ptr<Token> PreProcess::paste(const Token *lhs, const Token *rhs)
{
	string buf = Token::reverse_str_literal(lhs);
	buf += Token::reverse_str_literal(rhs);
	buf.push_back('\n');
	/* ファイルをトークナイズする */
	auto tok = vir_file_tokenize(buf, lhs);
	if (TokenKind::TK_EOF != tok->_next->_kind)
	{
		error_token("連結した文字列\'" + buf + "\'は無効なトークンです", lhs);
	}
	return tok;
}

/**
 * @brief 文字列strだけを持つ仮想的なファイルをトークナイズする。
 *
 * @param str トークナイズする文字列(末尾が'\n'で終わっていること)
 * @param ref 新たに作成するトークンのテンプレート
 * @return トークナイズした結果
 */
unique_ptr<Token> PreProcess::vir_file_tokenize(const string &str, const Token *ref)
{
	/* ファイル構造体の実体を管理するための配列 */
	static vector<unique_ptr<File>> files;

	files.push_back(make_unique<File>(ref->_file->_name, ref->_file->_file_no, str));
	/* ファイルをトークナイズする */
	auto tok = Token::tokenize(files.back().get());
	return tok;
}

/**
 * @brief #includeするファイルのファイル名を取得する
 *
 * @param next_token 次のトークンを返すための参照
 * @param current_token インクルードの開始位置
 * @param is_dquote #include "..."であるか
 * @return string インクルードするファイル名
 */
string PreProcess::read_include_filename(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token, bool &is_dquote)
{
	/* #include "..." */
	if (TokenKind::TK_STR == current_token->_kind)
	{
		is_dquote = true;
		/* エスケープを無効にして元々の文字列を戻す。
		 * 例："C:\note"に含まれる"\n"は改行文字ではない */
		string name = Token::reverse_str_literal(current_token.get());
		/* インクルードの後のトークンは無視 */
		next_token = skip_line(move(current_token->_next));
		/* 文字列リテラルの前後の'"'を消す */
		return name.substr(1, name.size() - 2);
	}

	/* #include <...> */
	if (current_token->is_equal("<"))
	{
		is_dquote = false;
		auto start = current_token.get();
		auto end = current_token->_next.get();

		while (!end->is_equal(">"))
		{
			if (TokenKind::TK_EOF == end->_kind || end->_at_begining)
			{
				error_token("\'>\'がありません", end);
			}
			end = end->_next.get();
		}
		auto name = join_tokens(start->_next.get(), end);
		/* インクルードの後のトークンは無視 */
		next_token = skip_line(move(end->_next));
		return name;
	}

	/* #include FOO
	 * FOOはマクロであり"..."または<...>に展開される
	 */
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		/* マクロを展開する */
		auto tok = preprocess2(copy_line(next_token, move(current_token)));
		return read_include_filename(tok, move(tok), is_dquote);
	}

	/* どれにも当てはまらなければエラー */
	error_token("ファイル名ではありません", current_token.get());
	return "";
}

/**
 * @brief ファイルをインクルードする。
 *
 * @param follow_token インクルードの後に続くトークン
 * @param path インクルードするファイルのパス
 * @return インクルードしたファイルをトークナイズし、follow_tokenを後ろに接続したトークンリスト
 */
unique_ptr<Token> PreProcess::include_file(unique_ptr<Token> &&follow_token, const string &path)
{
	auto include_token = Token::tokenize_file(path);
	return append(move(include_token), move(follow_token));
}

/**
 * @brief インクルードファイルのパスを検索する。見つからなければ空文字列を返す。
 *
 * @param current_path 現在処理しているファイルのパス
 * @param filename インクルードファイルの名前
 * @param dquote #include "..."形式であるか
 * @return インクルードファイルのパス
 */
string PreProcess::search_include_path(const string &current_path, const string &filename, const bool &dquote)
{

	constexpr string_view std_inc_path[] = {"/usr/local/include", "/usr/include/x86_64-linux-gnu", "/usr/include"};

	fs::path pfilename = filename;
	/* 絶対パス */
	if (pfilename.is_absolute())
	{
		return filename;
	}

	/* 現在のファイル */
	fs::path pcurrent_path = current_path;
	/* インクルードするファイルのパス */
	fs::path inc_path;

	/* #include "..."形式では現在のパスからの相対パスから探す */
	if (dquote)
	{
		/* 現在のファイルからの相対パス */
		inc_path = pcurrent_path.replace_filename(filename);
		/* ファイルが存在するとき読み込む */
		if (fs::is_regular_file(inc_path))
		{
			return inc_path.string();
		}
	}

	/* -Iオプションで追加されたパスを検索する */
	for (const auto &base_path : input_options->_include)
	{
		/* includeするファイルのパスを生成、base_pathからの相対パス */
		inc_path = fs::path(base_path) / pfilename;
		/* ファイルが存在するときパスを返す */
		if (fs::is_regular_file(inc_path))
		{
			return inc_path.string();
		}
	}

	/* 標準インクルードパスを検索する */
	for (const auto &base_path : std_inc_path)
	{
		/* includeするファイルのパスを生成、base_pathからの相対パス */
		inc_path = fs::path(base_path) / pfilename;
		/* ファイルが存在するときパスを返す */
		if (fs::is_regular_file(inc_path))
		{
			return inc_path.string();
		}
	}

	/* 見つからなければ空文字列を返す */
	return "";
}