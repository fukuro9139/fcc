/**
 * @file parse.cpp
 * @author K.Fukunaga
 * @brief
 * C言語の再帰下降構文解析を行うパーサーの定義
 *
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "parse.hpp"

/** 現在パースしている関数 */
static Object *current_function = nullptr;

/** 現在の関数で出てくるgoto文とラベルのリスト */
static Node *gotos = nullptr;
static Node *labels = nullptr;

/** breakでjumpに利用するためのラベル名 */
static string brk_label = "";

/** continueでjumpに利用するためのラベル名 */
static string cont_label = "";

/** 現在しているswitch文のノードへのポインタ、それ以外の場合はnullptr */
static Node *current_switch = nullptr;

/**************/
/* Node Class */
/**************/

/* コンストラクタ */

Node::Node() = default;

Node::Node(const NodeKind &kind, Token *token) : _kind(kind), _token(token) {}

Node::Node(const NodeKind &kind, unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token)
	: _kind(kind), _lhs(move(lhs)), _rhs(move(rhs)), _token(token) {}

Node::Node(const NodeKind &kind, unique_ptr<Node> &&lhs, Token *token)
	: _kind(kind), _lhs(move(lhs)), _token(token) {}

Node::Node(const int64_t &val, Token *token)
	: _kind(NodeKind::ND_NUM), _val(val), _token(token) {}

Node::Node(const int64_t &val, const shared_ptr<Type> &ty, Token *token)
	: _kind(NodeKind::ND_NUM), _val(val), _token(token), _ty(ty) {}

Node::Node(const Object *var, Token *token) : _kind(NodeKind::ND_VAR), _var(var), _token(token) {}

/**
 * @brief 型キャストに対応するノードを作成する
 *
 * @param expr 型キャストの対象とする式
 * @param ty キャスト後の型
 * @return unique_ptr<Node> 型キャストに対応するノード
 */
unique_ptr<Node> Node::new_cast(unique_ptr<Node> &&expr, const shared_ptr<Type> &ty)
{
	/* 型を確定 */
	Type::add_type(expr.get());

	auto node = make_unique<Node>(NodeKind::ND_CAST, move(expr), expr->_token);
	node->_ty = make_shared<Type>(*ty);
	return node;
}

/**
 * @brief 左辺 + 右辺の計算結果を表すノードを生成する。
 *
 * @details
 * C言語では、+演算子はポインタ演算を行うためにオーバーロードされている。
 * もしpがポインタである場合、p+nはnを加えるのではなく、sizeof(*p)*nをpの値に加える。
 * そのため、p+nはpからn個先の要素（バイトではなく）を指すようになる。
 * 言い換えれば、ポインタ値に加える前に整数値をスケールする必要があり、この関数はそのスケーリングを処理する。
 * @param lhs 左辺
 * @param rhs 右辺
 * @param token ノードと対応するトークン
 * @return 対応するASTノード
 */
unique_ptr<Node> Node::new_add(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 + 数 */
	if (lhs->_ty->is_numeric() && rhs->_ty->is_numeric())
	{
		return make_unique<Node>(NodeKind::ND_ADD, move(lhs), move(rhs), token);
	}

	/* ptr + ptr は無効な演算 */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		error_token("無効な演算です", token);
	}

	/* "数 + ptr" を "ptr + 数" に変換する*/
	if (!lhs->_ty->_base && rhs->_ty->_base)
	{
		std::swap(lhs, rhs);
	}

	/* ptr + 数 */
	rhs = make_unique<Node>(NodeKind::ND_MUL, move(rhs), make_unique<Node>(lhs->_ty->_base->_size, Type::LONG_BASE, token), token);
	return make_unique<Node>(NodeKind::ND_ADD, move(lhs), move(rhs), token);
}

/**
 * @brief 左辺 - 右辺の計算結果を表すノードを生成する。
 *
 * @details
 * C言語では、-演算子も'+'演算子と同様にポインタ演算を行うためにオーバーロードされている。
 * ポインタ - ポインタは2つのポインタ間にある要素の数を返す。
 * @param lhs 左辺
 * @param rhs 右辺
 * @param token ノードと対応するトークン
 * @return 対応するASTノード
 */
unique_ptr<Node> Node::new_sub(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 - 数 */
	if (lhs->_ty->is_numeric() && rhs->_ty->is_numeric())
	{
		return make_unique<Node>(NodeKind::ND_SUB, move(lhs), move(rhs), token);
	}

	/* ptr - 数 */
	if (lhs->_ty->_base && !rhs->_ty->_base)
	{
		rhs = make_unique<Node>(NodeKind::ND_MUL, move(rhs), make_unique<Node>(lhs->_ty->_base->_size, Type::LONG_BASE, token), token);
		Type::add_type(rhs.get());
		auto node = make_unique<Node>(NodeKind::ND_SUB, move(lhs), move(rhs), token);
		return node;
	}

	/* ptr - ptr */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		int sz = lhs->_ty->_base->_size;
		auto node = make_unique<Node>(NodeKind::ND_SUB, move(lhs), move(rhs), token);
		node->_ty = Type::LONG_BASE;
		return make_unique<Node>(NodeKind::ND_DIV, move(node), make_unique<Node>(sz, token), token);
	}

	/* 数 - ptr はエラー */
	error_token("無効な演算です", token);

	/* コンパイルエラー対策。直前のerror_token()で終了されるのでnullptrが返ることはない */
	return nullptr;
}

/**
 * @brief グローバル変数の仮名としてユニークな名前を生成する
 *
 * @return 生成した名前。".L..id"となる。(idは生成順)
 */
string Node::new_unique_name()
{
	static int id = 0;
	return ".L.." + std::to_string(id++);
}

/**
 * @brief 仮名を付けてグローバル変数を生成する
 *
 * @param ty 生成するグローバル変数の型
 * @return unique_ptr<Object>
 */
Object *Node::new_anonymous_gvar(shared_ptr<Type> &ty)
{
	return Object::new_gvar(new_unique_name(), ty);
}

/**
 * @brief 文字列リテラルとしてstrをもつグローバル変数を生成する。
 *
 * @param str 文字列リテラル
 * @return 生成したグローバル変数オブジェクトへのポインタ
 */
Object *Node::new_string_literal(const string &str)
{
	/* 文字列リテラルの型はchar型配列で長さは文字数+'\0'終端 */
	auto ty = Type::array_of(Type::CHAR_BASE, str.size() + 1);

	/* 仮名を使ってオブジェクトを生成 */
	auto obj = new_anonymous_gvar(ty);

	/* init_dataに文字列を入れて'\0'終端を追加 */
	obj->_init_data = make_unique<unsigned char[]>(str.size() + 1);

	for (int i = 0; i < str.size(); i++)
	{
		obj->_init_data[i] = str[i];
	}

	obj->_init_data[str.size()] = '\0';

	return obj;
}

/**
 * @brief 後置++, --に対応するノードを生成する
 *
 * @param node インクリメント/デクリメントの対象ノード
 * @param token ノードに対応するトークン
 * @param addend インクリメントかデクリメントか
 * @return 後置++, --に対応するノード
 * @details 'A++'を'(typeof A)((A += 1) - 1)'と読み替える
 */
unique_ptr<Node> Node::new_inc_dec(unique_ptr<Node> &&node, Token *token, int addend)
{
	Type::add_type(node.get());
	auto ty = node->_ty;
	return new_cast(new_add(to_assign(new_add(move(node),
											  make_unique<Node>(addend, token),
											  token)),
							make_unique<Node>(-addend, token),
							token),
					ty);
}

/**
 * @brief トークン・リストを構文解析して関数ごとにASTを構築する
 *
 * @param list トークン・リスト
 * @return 構文解析結果
 * @details program = (typedef | function-definition | global-variable)*
 */
unique_ptr<Object> Node::parse(const unique_ptr<Token> &list)
{
	auto token = list.get();

	/* トークンリストを最後まで辿る*/
	while (TokenKind::TK_EOF != token->_kind)
	{
		VarAttr attr = {};
		auto base = declspec(&token, token, &attr);

		/* typedef */
		if (attr._is_typedef)
		{
			token = parse_typedef(token, base);
			continue;
		}

		/* 関数 */
		if (is_function(token))
		{
			token = function_definition(token, move(base), &attr);
			continue;
		}

		/* グローバル変数 */
		token = global_variable(token, move(base), &attr);
	}

	return move(Object::globals);
}

/**
 * @brief プログラム を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @see expression compound-statement expression-statement
 *
 * @details 下記のEBNF規則に従う。 @n
 * statement = "return" expression? ";" @n
 * 			 | "if" "(" expression ")" statement ("else" statement)? @n
 *      	 | "switch" "(" expression ")" statement @n
 *  	     | "case" const-expr ":" statement @n
 * 	     	 | "default" ":" statement @n
 * 			 | "for" "(" expression-statement expression? ";" expression? ")" statement @n
 * 			 | "while" "(" expression ")" statement @n
 * 			 | "do" statement "while" "(" expression ")" ";" @n
 * 			 | "goto" ident ";" @n
 * 			 | "continue" ";" @n
 * 			 |  ident ":" statement @n
 * 			 | "{" compound-statement @n
 * 			 | expression-statement
 */
unique_ptr<Node> Node::statement(Token **next_token, Token *current_token)
{
	/* return */
	if (current_token->is_equal("return"))
	{
		auto node = make_unique<Node>(NodeKind::ND_RETURN, current_token);

		/* 戻り値なし */
		if (consume(next_token, current_token->_next.get(), ";"))
		{
			return node;
		}

		/* 戻り値をもつ */
		auto expr = expression(&current_token, current_token->_next.get());

		/* 最後は';'で終わるはず */
		*next_token = skip(current_token, ";");

		/* 式の型を決定する */
		Type::add_type(expr.get());

		/* return先の型にキャストする */
		node->_lhs = new_cast(move(expr), current_function->_ty->_return_ty);
		return node;
	}

	/* if */
	if (current_token->is_equal("if"))
	{
		auto node = make_unique<Node>(NodeKind::ND_IF, current_token);

		/* ifの次は'('がくる */
		current_token = skip(current_token->_next.get(), "(");

		/* 条件文 */
		node->_condition = expression(&current_token, current_token);

		/* 条件文のは')'がくる */
		current_token = skip(current_token, ")");
		node->_then = statement(&current_token, current_token);

		/* else節が存在する */
		if (current_token->is_equal("else"))
		{
			node->_else = statement(&current_token, current_token->_next.get());
		}
		*next_token = current_token;
		return node;
	}

	/* switch */
	if (current_token->is_equal("switch"))
	{
		auto node = make_unique<Node>(NodeKind::ND_SWITCH, current_token);
		current_token = skip(current_token->_next.get(), "(");
		/* switchの条件式 */
		node->_condition = expression(&current_token, current_token);
		current_token = skip(current_token, ")");

		/* 現在のswを保存 */
		auto sw = current_switch;
		current_switch = node.get();

		/* breakラベルの設定 */
		auto brk = brk_label;
		brk_label = node->_brk_label = new_unique_name();

		/* 各ケース文 */
		node->_then = statement(next_token, current_token);

		current_switch = sw;
		brk_label = brk;
		return node;
	}

	/* case */
	if (current_token->is_equal("case"))
	{
		if (!current_switch)
		{
			error_token("case文はswitch文の中でしか使えません", current_token);
		}

		auto node = make_unique<Node>(NodeKind::ND_CASE, current_token);
		auto val = const_expr(&current_token, current_token->_next.get());
		current_token = skip(current_token, ":");

		/* ユニークなラベル名を設定 */
		node->_label = new_unique_name();

		node->_lhs = statement(next_token, current_token);
		node->_val = val;

		/* リストの先頭に追加 */
		node->case_next = current_switch->case_next;
		current_switch->case_next = node.get();
		return node;
	}

	/* default */
	if (current_token->is_equal("default"))
	{
		if (!current_switch)
		{
			error_token("default文はswitch文の中でしか使えません", current_token);
		}
		auto node = make_unique<Node>(NodeKind::ND_CASE, current_token);
		current_token = skip(current_token->_next.get(), ":");

		/* ユニークなラベル名を設定 */
		node->_label = new_unique_name();

		node->_lhs = statement(next_token, current_token);

		/* リストの先頭にデフォルトのノードへの参照を追加 */
		current_switch->default_case = node.get();
		return node;
	}

	/* for */
	if (current_token->is_equal("for"))
	{
		auto node = make_unique<Node>(NodeKind::ND_FOR, current_token);

		/* forの次は'('がくる */
		current_token = skip(current_token->_next.get(), "(");

		/* for文のブロックスコープに入る */
		Object::enter_scope();

		/* 現在のラベルを保存 */
		auto brk = brk_label;
		auto cont = cont_label;
		/* forを抜けるラベルを設定 */
		node->_brk_label = brk_label = new_unique_name();
		node->_cont_label = cont_label = new_unique_name();

		/* 型指定子がきたら変数が定義されている */
		if (current_token->is_typename())
		{
			auto base = declspec(&current_token, current_token, nullptr);
			node->_init = declaration(&current_token, current_token, base, nullptr);
		}
		else
		{
			node->_init = expression_statement(&current_token, current_token);
		}

		/* 次のトークンが';'でなければ条件が存在する */
		if (!current_token->is_equal(";"))
		{
			node->_condition = expression(&current_token, current_token);
		}
		current_token = skip(current_token, ";");

		/* 次のトークンが')'でなければ加算処理が存在する */
		if (!current_token->is_equal(")"))
		{
			node->_inc = expression(&current_token, current_token);
		}
		current_token = skip(current_token, ")");
		/* forの中の処理 */
		node->_then = statement(next_token, current_token);
		/* for文のブロックスコープを抜ける */
		Object::leave_scope();

		/* 保存していた値を代入してfor文に入る前のラベルに戻す */
		brk_label = brk;
		cont_label = cont;
		return node;
	}

	/* while */
	if (current_token->is_equal("while"))
	{
		auto node = make_unique<Node>(NodeKind::ND_FOR, current_token);

		/* whileの次は'('がくる */
		current_token = skip(current_token->_next.get(), "(");
		node->_condition = expression(&current_token, current_token);

		/* 条件文のは')'がくる */
		current_token = skip(current_token, ")");

		/* 現在のラベルを保存 */
		auto brk = brk_label;
		auto cont = cont_label;

		/* while文を抜けるラベルを設定 */
		node->_brk_label = brk_label = new_unique_name();
		node->_cont_label = cont_label = new_unique_name();

		/* while文の中身 */
		node->_then = statement(next_token, current_token);

		/* ラベルを設定しなおす */
		brk_label = brk;
		cont_label = cont;
		return node;
	}

	/* do while */
	if (current_token->is_equal("do"))
	{
		auto node = make_unique<Node>(NodeKind::ND_DO, current_token);

		/* 現在のラベルを一時保存 */
		auto brk = brk_label;
		auto cont = cont_label;

		/* 新しいラベルを生成 */
		brk_label = node->_brk_label = new_unique_name();
		cont_label = node->_cont_label = new_unique_name();

		/* doの中身の処理 */
		node->_then = statement(&current_token, current_token->_next.get());

		/* ラベルを復元 */
		brk_label = brk;
		cont_label = cont;

		current_token = skip(current_token, "while");
		current_token = skip(current_token, "(");

		/* 条件式 */
		node->_condition = expression(&current_token, current_token);

		current_token = skip(current_token, ")");
		*next_token = skip(current_token, ";");
		return node;
	}

	/* goto */
	if (current_token->is_equal("goto"))
	{
		auto node = make_unique<Node>(NodeKind::ND_GOTO, current_token);
		/* ソース内に書かれている名前 */
		node->_label = current_token->_next->_str;
		/* リストの先頭に追加 */
		node->_goto_next = gotos;
		gotos = node.get();

		*next_token = skip(current_token->_next->_next.get(), ";");
		return node;
	}

	/* break */
	if (current_token->is_equal("break"))
	{
		if (brk_label.empty())
		{
			error_token("break文はループの中でしか使えません", current_token);
		}
		auto node = make_unique<Node>(NodeKind::ND_GOTO, current_token);
		node->_unique_label = brk_label;
		*next_token = skip(current_token->_next.get(), ";");
		return node;
	}

	/* continue */
	if (current_token->is_equal("continue"))
	{
		if (cont_label.empty())
		{
			error_token("continue文はループの中でしか使えません", current_token);
		}
		auto node = make_unique<Node>(NodeKind::ND_GOTO, current_token);
		node->_unique_label = cont_label;
		*next_token = skip(current_token->_next.get(), ";");
		return node;
	}

	/* ラベル */
	if (TokenKind::TK_IDENT == current_token->_kind && current_token->_next->is_equal(":"))
	{
		auto node = make_unique<Node>(NodeKind::ND_LABEL, current_token);
		node->_label = current_token->_str;
		node->_unique_label = new_unique_name();
		node->_lhs = statement(next_token, current_token->_next->_next.get());
		/* リストの先頭に繋ぐ */
		node->_goto_next = labels;
		labels = node.get();
		return node;
	}

	/* ブロック */
	if (current_token->is_equal("{"))
	{
		return compound_statement(next_token, current_token->_next.get());
	}

	/* 式 */
	return expression_statement(next_token, current_token);
}

/**
 * @brief ブロックを読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n compound-statement = (typedef | declaration | statement)* "}"
 */
unique_ptr<Node> Node::compound_statement(Token **next_token, Token *current_token)
{

	auto node = make_unique<Node>(NodeKind::ND_BLOCK, current_token);

	auto head = make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* ブロックスコープに入る */
	Object::enter_scope();

	/* '}'が出てくるまでstatementをパースする */
	while (!current_token->is_equal("}"))
	{
		/* 変数宣言、定義 */
		if (current_token->is_typename() && !current_token->_next->is_equal(":"))
		{
			/* 型指定子を読み取る */
			VarAttr attr = {};
			auto base = declspec(&current_token, current_token, &attr);

			/* typedefの場合 */
			if (attr._is_typedef)
			{
				current_token = parse_typedef(current_token, base);
				continue;
			}

			/* 関数定義 */
			if (is_function(current_token))
			{
				current_token = function_definition(current_token, move(base), &attr);
				continue;
			}

			/* 外部宣言 */
			if (attr._is_extern)
			{
				current_token = global_variable(current_token, move(base), &attr);
				continue;
			}

			/* それ以外の場合は変数の定義または宣言 */
			cur->_next = declaration(&current_token, current_token, base, &attr);
		}
		/* 宣言、定義以外の文 */
		else
		{
			cur->_next = statement(&current_token, current_token);
		}

		cur = cur->_next.get();
		Type::add_type(cur);
	}

	/* ブロックスコープから抜け出す */
	Object::leave_scope();

	/* ダミーのheadからの次のトークン以降を切り離し、新しいノードのbodyに繋ぐ*/
	node->_body = move(head->_next);
	/* '}' の次のトークンからパースを続ける*/
	*next_token = current_token->_next.get();
	return node;
}

/**
 * @brief 関数宣言、定義を読み取る。
 *
 * @param token 現在処理しているトークン
 * @param base 関数の戻り値の型のベース
 * @return 次のトークン
 * @details 下記のEBNF規則に従う。 @n function-definition = declarator ( "{" compound-statement | ";" )
 */
Token *Node::function_definition(Token *token, shared_ptr<Type> &&base, VarAttr *attr)
{
	/* 型を判定 */
	auto ty = declarator(&token, token, base);

	/* 関数名がなければエラー */
	if (!ty->_name)
	{
		error_token("関数名がありません", ty->_name_pos);
	}

	/* 新しい関数を生成する。 */
	auto fn = Object::new_gvar(ty->_name->_str, ty);
	/* 関数であるフラグをセット */
	fn->_is_function = true;

	/* 定義か宣言か、後ろに";"がくるなら宣言 */
	fn->_is_definition = !consume(&token, token, ";");

	/* staticかどうか */
	fn->_is_static = attr->_is_static;

	/* 宣言であるなら現在のトークンを返して抜ける */
	if (!fn->_is_definition)
	{
		return token;
	}

	current_function = fn;

	/* 関数のブロックスコープに入る */
	Object::enter_scope();

	/* 引数をローカル変数として作成 */
	Object::create_params_lvars(ty->_params);
	fn->_params = move(Object::locals);

	/* 可変長引数を持つ場合 */
	if (ty->_is_variadic)
	{
		fn->_va_area = Object::new_lvar("__va_area__", Type::array_of(Type::CHAR_BASE, 136));
	}

	/* 引数の次は"{"がくる */
	token = skip(token, "{");

	Object::push_scope("__func__")->_var = new_string_literal(fn->_name);

	/* 関数の中身を読み取る */
	fn->_body = compound_statement(&token, token);

	/* ローカル変数をセット */
	fn->_locals = move(Object::locals);

	/* 関数のブロックスコープを抜ける */
	Object::leave_scope();

	/* gotoとラベルの紐づけ */
	resolve_goto_label();

	current_function = nullptr;

	return token;
}

/**
 * @brief 変数宣言を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
 */
unique_ptr<Node> Node::declaration(Token **next_token, Token *current_token, shared_ptr<Type> &base, const VarAttr *attr)
{
	/* ノードリストの先頭としてダミーのノードを作成 */
	auto head = make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* 初回かどうか */
	bool first = true;

	/* ";"が出てくるまで読み取りを続ける */
	while (!current_token->is_equal(";"))
	{
		/* 2個目以降の宣言には",""区切りが必要 */
		if (!first)
		{
			current_token = skip(current_token, ",");
		}
		/* 初回フラグを下げる */
		first = false;

		/* 変数の最終的な型を決定 */
		auto ty = declarator(&current_token, current_token, base);

		/* 変数がvoid型で宣言されていたらエラー */
		if (TypeKind::TY_VOID == ty->_kind)
		{
			error_token("変数がvoid型で宣言されています", current_token);
		}
		/* 変数名が省略されていたらエラー */
		if (!ty->_name)
		{
			error_token("変数名が宣言されていません", ty->_name_pos);
		}

		/* static指定されたローカル変数 */
		if (attr && attr->_is_static)
		{
			/* ブローバル変数として仮名をつけて登録 */
			auto var = new_anonymous_gvar(ty);
			/* ローカル変数に名前を登録してグローバル変数のオブジェクトを参照 */
			Object::push_scope(ty->_name->_str)->_var = var;

			/* 初期化式を持つ場合 */
			if (current_token->is_equal("="))
			{
				gvar_initializer(&current_token, current_token->_next.get(), var);
				continue;
			}
		}

		const auto var = Object::new_lvar(ty->_name->_str, ty);

		/* アライン指定がある場合 */
		if (attr && attr->_align)
		{
			var->_align = attr->_align;
		}

		/* 宣言の後に初期化式をもつ場合 */
		if (current_token->is_equal("="))
		{
			auto expr = lvar_initializer(&current_token, current_token->_next.get(), var);
			/* ノードリストの末尾に単文ノードとして追加 */
			cur->_next = make_unique<Node>(NodeKind::ND_EXPR_STMT, move(expr), current_token);
			/* ノードリストの末尾を更新 */
			cur = cur->_next.get();

			/* 変数の型が不完全 */
			if (var->_ty->_size < 0)
			{
				error_token("変数の型が不完全です", ty->_name);
			}

			if (TypeKind::TY_VOID == var->_ty->_kind)
			{
				error_token("変数がvoid型で宣言されています", ty->_name);
			}
		};
	}
	auto node = make_unique<Node>(NodeKind::ND_BLOCK, current_token);
	/* ヘッダの次のノード以降を切り離してnodeのbodyに繋ぐ */
	node->_body = move(head->_next);
	*next_token = current_token->_next.get();
	return node;
}

/**
 * @brief 変数の初期化式を読み取って生成する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 変数の型
 * @param new_ty 初期化式から補完した完全な型を返すための参照
 * @return 生成した初期化式
 */
unique_ptr<Initializer> Node::initializer(Token **next_token, Token *current_token, shared_ptr<Type> ty, shared_ptr<Type> &new_ty)
{
	auto init = Object::new_initializer(ty, true);
	initializer2(next_token, current_token, init.get());

	/* フレキシブル配列メンバをもち、初期化式で要素数が確定できる場合 */
	if ((TypeKind::TY_STRUCT == ty->_kind || TypeKind::TY_UNION == ty->_kind) && ty->_is_flexible)
	{
		/* 構造体の型の実体をコピー */
		ty = Type::copy_struct_type(ty);

		auto mem = ty->_members.get();
		/* 末尾まで辿る */
		while (mem->_next)
		{
			mem = mem->_next.get();
		}
		/* 要素数が定まっていなかったメンバーの型を要素数が確定しいている初期化式の中の型で上書きする */
		mem->_ty = init->_children[mem->_idx]->_ty;
		/* 構造体のサイズを更新する */
		ty->_size += mem->_ty->_size;

		new_ty = ty;
		return init;
	}

	new_ty = init->_ty;
	return init;
}

/**
 * @brief 最適的に初期化式の中身を読み取っていく
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param 現在、処理している初期化式
 * @details 以下のEBNF規則に従う。 @n
 * initializer = string-initialize | array-initializer | struct-initializer | union-initializer |assign
 */
void Node::initializer2(Token **next_token, Token *current_token, Initializer *init)
{
	/* 文字列 */
	if (TypeKind::TY_ARRAY == init->_ty->_kind && TokenKind::TK_STR == current_token->_kind)
	{
		string_initializer(next_token, current_token, init);
		return;
	}

	/* 配列 */
	if (TypeKind::TY_ARRAY == init->_ty->_kind)
	{
		if (current_token->is_equal("{"))
		{
			array_initializer1(next_token, current_token, init);
		}
		else
		{
			array_initializer2(next_token, current_token, init);
		}
		return;
	}

	/* 構造体 */
	if (TypeKind::TY_STRUCT == init->_ty->_kind)
	{
		if (current_token->is_equal("{"))
		{
			struct_initializer1(next_token, current_token, init);
			return;
		}
		/* 構造体は他の構造体で初期化できる */
		auto expr = assign(next_token, current_token);
		Type::add_type(expr.get());
		if (TypeKind::TY_STRUCT == expr->_ty->_kind)
		{
			init->_expr = move(expr);
			return;
		}
		struct_initializer2(next_token, current_token, init);
		return;
	}

	/* 共用体 */
	if (TypeKind::TY_UNION == init->_ty->_kind)
	{
		union_initializer(next_token, current_token, init);
		return;
	}

	/* スカラー値の初期化式は'{}'で囲むことができる。
	 * 例: int x = {1};
	 */
	if (current_token->is_equal("{"))
	{
		initializer2(&current_token, current_token->_next.get(), init);
		*next_token = skip(current_token, "}");
		return;
	}

	init->_expr = assign(next_token, current_token);
}

/**
 * @brief 文字列型の変数の初期化式を生成する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param init 初期化式
 * @details 以下のEBNF規則に従う。 @n
 * string_initializer = string-literal
 */
void Node::string_initializer(Token **next_token, Token *current_token, Initializer *init)
{

	/* 前後の'"'を削除 */
	auto str = current_token->_str.substr(1);
	str.pop_back();

	if (init->_is_flexible)
	{
		/* initの実体を初期化式の文字列の長さ+1を持つ配列として書き換える */
		*init = move(*Object::new_initializer(Type::array_of(init->_ty->_base, str.size() + 1), false));
	}

	int len = std::min(init->_ty->_array_length, static_cast<int>(str.size()));
	for (int i = 0; i < len; ++i)
	{
		init->_children[i]->_expr = make_unique<Node>(static_cast<int64_t>(str[i]), current_token);
	}
	*next_token = current_token->_next.get();
}

/**
 * @brief 初期化式で与えられた配列の要素数を数える
 *
 * @param token 現在のトークン
 * @param ty 配列の型
 * @return 配列の要素数
 */
int Node::count_array_init_element(Token *token, const Type *ty)
{
	auto dummy = Object::new_initializer(ty->_base, false);
	int cnt = 0;

	for (; !consume_end(&token, token); ++cnt)
	{
		/* 2個目以降では','区切りが必要 */
		if (cnt > 0)
		{
			token = skip(token, ",");
		}
		initializer2(&token, token, dummy.get());
	}
	return cnt;
}

/**
 * @brief 配列の変数の初期化式（前後に'{}'を持つ）を読み取りを生成する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param init 初期化式
 * @details 以下のEBNF規則に従う。 @n
 * array_initializer1 = "{" identifier ("," identifier)*  ","? "}"
 */
void Node::array_initializer1(Token **next_token, Token *current_token, Initializer *init)
{
	/* 配列の初期化式は"{"で始まる */
	current_token = skip(current_token, "{");

	if (init->_is_flexible)
	{
		auto len = count_array_init_element(current_token, init->_ty.get());
		*init = move(*Object::new_initializer(Type::array_of(init->_ty->_base, len), false));
	}

	/* 各要素について再帰的に初期化式を構成していく */
	for (int i = 0; !consume_end(next_token, current_token); ++i)
	{
		/* 2個目以降は","区切りに必要 */
		if (i > 0)
		{
			current_token = skip(current_token, ",");
		}
		if (i < init->_ty->_array_length)
		{
			initializer2(&current_token, current_token, init->_children[i].get());
		}
		else
		{
			current_token = skip_excess_element(current_token);
		}
	}
}

/**
 * @brief 配列の変数の初期化式（前後に'{}'を持たない）を読み取り生成する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param init 初期化式
 * @details 以下のEBNF規則に従う。 @n
 * array_initializer2 =  identifier ("," identifier)*
 */
void Node::array_initializer2(Token **next_token, Token *current_token, Initializer *init)
{
	if (init->_is_flexible)
	{
		auto len = count_array_init_element(current_token, init->_ty.get());
		*init = move(*Object::new_initializer(Type::array_of(init->_ty->_base, len), false));
	}

	/* 各要素について再帰的に初期化式を構成していく */
	for (int i = 0; i < init->_ty->_array_length && !current_token->is_end(); i++)
	{
		/* 2個目以降は","区切りに必要 */
		if (i > 0)
		{
			current_token = skip(current_token, ",");
		}
		initializer2(&current_token, current_token, init->_children[i].get());
	}
	*next_token = current_token;
}

/**
 * @brief 構造体の変数の初期化式を生成する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param init 初期化式
 * @details 以下のEBNF規則に従う。 @n
 * struct_initializer1 = "{" initializer ("," initializer)* ","? "}"
 */
void Node::struct_initializer1(Token **next_token, Token *current_token, Initializer *init)
{
	current_token = skip(current_token, "{");

	auto mem = init->_ty->_members.get();

	bool first = true;
	while (!consume_end(next_token, current_token))
	{
		/* 2個目以降では','区切りが必要 */
		if (!first)
		{
			current_token = skip(current_token, ",");
		}
		first = false;

		if (mem)
		{
			initializer2(&current_token, current_token, init->_children[mem->_idx].get());
			mem = mem->_next.get();
		}
		else
		{
			current_token = skip_excess_element(current_token);
		}
	}
}

/**
 * @brief 構造体の変数の初期化式(前後に'{}'を持たない)を読み取り生成する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param init 初期化式
 * @details 以下のEBNF規則に従う。 @n
 * struct_initializer2 = initializer ("," initializer)*
 */
void Node::struct_initializer2(Token **next_token, Token *current_token, Initializer *init)
{
	bool first = true;
	for (auto mem = init->_ty->_members.get(); mem && !current_token->is_end(); mem = mem->_next.get())
	{
		if (!first)
		{
			current_token = skip(current_token, ",");
		}
		first = false;
		initializer2(&current_token, current_token, init->_children[mem->_idx].get());
	}
	*next_token = current_token;
}

/**
 * @brief 共用体の変数の初期化式を生成する。共用体は1要素しか初期化式をもたない。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param init 初期化式
 */
void Node::union_initializer(Token **next_token, Token *current_token, Initializer *init)
{
	if (current_token->is_equal("{"))
	{
		initializer2(&current_token, current_token->_next.get(), init->_children[0].get());
		consume(&current_token, current_token, ",");
		*next_token = skip(current_token, "}");
	}
	else
	{
		initializer2(next_token, current_token, init->_children[0].get());
	}
}

/**
 * @brief 余分な初期値を無視する。
 *
 * @param token 現在のトークン
 * @return 次のトークン
 */
Token *Node::skip_excess_element(Token *token)
{
	if (token->is_equal("{"))
	{
		token = skip_excess_element(token->_next.get());
		return skip(token, "}");
	}
	assign(&token, token);
	return token;
}

/**
 * @brief 初期化式の代入先を再帰的に構成する
 *
 * @details 例 x[2][2] ={{..},{..}};という初期化式は
 * *(*(x+i)+j) = v;(i =0,1, j=0,1)という形に変換される。
 * このときの左辺の代入先を表すノードを生成して返す
 *
 * @param desg 変数の中で現在ノードを作成している位置
 * @param token ノードと対応するトークン
 * @return 生成したノード
 */
unique_ptr<Node> Node::init_desg_expr(InitDesg *desg, Token *token)
{
	/* 配列ではない変数なら変数を表すノードを生成して返す */
	if (desg->_var)
	{
		return make_unique<Node>(desg->_var, token);
	}

	if (desg->_member)
	{
		auto node = make_unique<Node>(NodeKind::ND_MEMBER, init_desg_expr(desg->_next, token), token);
		node->_member = desg->_member;
		return node;
	}

	auto lhs = init_desg_expr(desg->_next, token);
	auto rhs = make_unique<Node>(desg->_idx, token);
	return make_unique<Node>(NodeKind::ND_DEREF, new_add(move(lhs), move(rhs), token), token);
}

/**
 * @brief ローカル変数を初期化するノードを構成する。
 *
 * @param init 初期化式
 * @param ty 変数の型
 * @param desg  変数の中で現在ノードを作成している位置
 * @param token ノードと対応するトークン
 * @return 生成したノード
 */
unique_ptr<Node> Node::create_lvar_init(Initializer *init, Type *ty, InitDesg *desg, Token *token)
{
	if (TypeKind::TY_ARRAY == ty->_kind)
	{
		auto node = make_unique<Node>(NodeKind::ND_NULL_EXPR, token);
		auto len = ty->_array_length;
		for (int i = 0; i < len; ++i)
		{
			InitDesg desg2 = {desg, i};
			auto rhs = create_lvar_init(init->_children[i].get(), ty->_base.get(), &desg2, token);
			node = make_unique<Node>(NodeKind::ND_COMMA, move(node), move(rhs), token);
		}
		return node;
	}

	if (TypeKind::TY_STRUCT == ty->_kind && !init->_expr)
	{
		auto node = make_unique<Node>(NodeKind::ND_NULL_EXPR, token);

		for (auto mem = ty->_members; mem; mem = mem->_next)
		{
			InitDesg desg2 = {desg, 0, mem};
			auto rhs = create_lvar_init(init->_children[mem->_idx].get(), mem->_ty.get(), &desg2, token);
			node = make_unique<Node>(NodeKind::ND_COMMA, move(node), move(rhs), token);
		}
		return node;
	}

	if (TypeKind::TY_UNION == ty->_kind)
	{
		InitDesg desg2 = {desg, 0, ty->_members};
		return create_lvar_init(init->_children[0].get(), ty->_members->_ty.get(), &desg2, token);
	}

	/* 初期化式が与えられない場合はなにもしない（変数は最初に0クリアされる） */
	if (!init->_expr)
	{
		return make_unique<Node>(NodeKind::ND_NULL_EXPR, token);
	}
	auto lhs = init_desg_expr(desg, token);

	return make_unique<Node>(NodeKind::ND_ASSIGN, move(lhs), move(init->_expr), token);
}

/**
 * @brief ローカル変数の初期化式を読み取って生成する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param obj 変数を表すオブジェクト
 * @return 対応するASTノード
 * @details
 * 初期化式が与えられなかった場合、各要素は0にセットする。
 * 最初に変数に与えられたメモリ領域全体を0クリアしてその後に
 * ユーザーが指定した初期値があれば設定する。
 */
unique_ptr<Node> Node::lvar_initializer(Token **next_token, Token *current_token, Object *var)
{
	auto init = initializer(next_token, current_token, var->_ty, var->_ty);
	InitDesg desg = {nullptr, 0, nullptr, var};

	auto lhs = make_unique<Node>(NodeKind::ND_MEMZERO, current_token);
	lhs->_var = var;
	auto rhs = create_lvar_init(init.get(), var->_ty.get(), &desg, current_token);
	return make_unique<Node>(NodeKind::ND_COMMA, move(lhs), move(rhs), current_token);
}

/**
 * @brief 整数valの内部表現を1バイトずつbuf[offset]に書き込む
 *
 * @param buf データの書き込み先
 * @param val 書き込む数値
 * @param sz 書き込むサイズ（byte）
 * @param offset 書き込むスタート地点のオフセット
 */
void Node::write_buf(unsigned char buf[], int64_t val, int sz, int offset)
{
	/* 数値の内部表現を見るための共用体 */
	union
	{
		int64_t val;
		unsigned char byte[8];
	} v;

	if (sz > 8)
	{
		unreachable();
	}

	v.val = val;
	for (int i = 0; i < sz; ++i)
	{
		buf[offset + i] = v.byte[i];
	}
}

/**
 * @brief 浮動小数点数valの内部表現を1バイトずつbuf[offset]に書き込む
 *
 * @param buf データの書き込み先
 * @param val 書き込む数値
 * @param sz 書き込むサイズ（byte）
 * @param offset 書き込むスタート地点のオフセット
 */
void Node::write_fval_buf(unsigned char buf[], double val, int sz, int offset)
{
	/* float, double型の数値の内部表現を見るための共用体 */
	union
	{
		float fval;
		double dval;
		unsigned char byte[8];
	} v;

	if (4 == sz)
	{
		v.fval = val;
		for (int i = 0; i < 4; ++i)
		{
			buf[offset + i] = v.byte[i];
		}
	}
	else if (8 == sz)
	{
		v.dval = val;
		for (int i = 0; i < 8; ++i)
		{
			buf[offset + i] = v.byte[i];
		}
	}
	else
	{
		unreachable();
	}
}

/**
 * @brief グローバル変数の初期化データを生成する
 *
 * @param init 初期化式
 * @param ty 書き込む型
 * @param buf データの書き込み先
 * @param offset オフセット
 */
Relocation *Node::write_gvar_data(Relocation *cur, Initializer *init, Type *ty, unsigned char buf[], int offset)
{

	if (TypeKind::TY_ARRAY == ty->_kind)
	{
		int sz = ty->_base->_size;
		for (int i = 0; i < ty->_array_length; ++i)
		{
			cur = write_gvar_data(cur, init->_children[i].get(), ty->_base.get(), buf, offset + sz * i);
		}
		return cur;
	}

	if (TypeKind::TY_STRUCT == ty->_kind)
	{
		for (auto mem = ty->_members.get(); mem; mem = mem->_next.get())
		{
			cur = write_gvar_data(cur, init->_children[mem->_idx].get(), mem->_ty.get(), buf, offset + mem->_offset);
		}
		return cur;
	}

	if (TypeKind::TY_UNION == ty->_kind)
	{
		return write_gvar_data(cur, init->_children[0].get(), ty->_members->_ty.get(), buf, offset);
	}

	if (!init->_expr)
	{
		return cur;
	}

	if (ty->is_flonum())
	{
		auto val = evaluate_double(init->_expr.get());
		write_fval_buf(buf, val, ty->_size, offset);
		return cur;
	}

	string label = "";
	auto val = evaluate2(init->_expr.get(), &label);

	if (label.empty())
	{
		write_buf(buf, val, ty->_size, offset);
		return cur;
	}

	auto rel = make_unique<Relocation>();
	rel->_offset = offset;
	rel->_label = label;
	rel->_addend = val;
	cur->_next = move(rel);
	return cur->_next.get();
}

/**
 * @brief グローバル変数の初期化式を生成する
 *
 * @details
 * グローバル変数の初期化式はコンパイル時に評価され、.dataセクションに配置される。
 * この関数では初期化式オブジェクトをバイト単位の配列に変換する。コンパイル時に
 * 定数式ではない式を含む場合エラーとする。
 * @param next_token
 * @param current_token
 * @param var
 * @return unique_ptr<Node>
 */
void Node::gvar_initializer(Token **next_token, Token *current_token, Object *var)
{
	auto init = initializer(next_token, current_token, var->_ty, var->_ty);
	auto head = make_unique<Relocation>();

	auto buf = make_unique<unsigned char[]>(var->_ty->_size);
	write_gvar_data(head.get(), init.get(), var->_ty.get(), buf.get(), 0);
	var->_init_data = move(buf);
	var->_rel = move(head->_next);
}

/**
 * @brief 関数の引数を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 宣言されている型
 * @return 引数の情報を含む関数の型
 * @details 以下のEBNF規則に従う。 @n
 * function-parameters = ( "void" | parameters ("," parameters)* ("," "...")?)? ")" @n
 * parameters = declspec declarator
 */
shared_ptr<Type> Node::function_parameters(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	if (current_token->is_equal("void") && current_token->_next->is_equal(")"))
	{
		*next_token = current_token->_next->_next.get();
		return Type::func_type(ty);
	}

	auto head = make_unique_for_overwrite<Type>();
	auto cur = head.get();
	bool is_variadic = false;

	/* ")"が出てくるまで読み取りを続ける */
	while (!current_token->is_equal(")"))
	{
		if (head.get() != cur)
		{
			/* 2個目以降の引数では","区切りが必要 */
			current_token = skip(current_token, ",");
		}

		/* 可変長引数 */
		if (current_token->is_equal("..."))
		{
			is_variadic = true;
			current_token = current_token->_next.get();
			skip(current_token, ")");
			break;
		}

		/* 引数の型を決定 */
		auto ty2 = declspec(&current_token, current_token, nullptr);
		ty2 = declarator(&current_token, current_token, ty2);

		auto name = ty2->_name;

		/* 関数の引数では、T型の配列はT型へのポインタとして解釈する。例： *argv[] は **argv に変換される */
		if (TypeKind::TY_ARRAY == ty2->_kind)
		{
			ty2 = Type::pointer_to(ty2->_base);
			ty2->_name = name;
		}
		/* 同様に関数もポインタとして解釈する */
		else if(TypeKind::TY_FUNC == ty2->_kind){
			ty2 = Type::pointer_to(ty2);
			ty2->_name = name;
		}
		cur->_next = make_shared<Type>(*ty2);
		cur = cur->_next.get();
	}

	/* 引数が存在しない場合 */
	if (cur == head.get())
	{
		is_variadic = true;
	}

	ty = Type::func_type(ty);
	ty->_params = move(head->_next);
	ty->_is_variadic = is_variadic;
	*next_token = current_token->_next.get();
	return ty;
}

/**
 * @brief 配列の宣言の要素数部分を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 宣言されている型
 * @return 配列の型
 * @details
 * 次のEBNF規則に従う。 @n array-dimensions = ("static" | "restrict")* const-expr? "]" type-suffix
 */
shared_ptr<Type> Node::array_dimensions(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	/* static, restrictを無視する */
	while (current_token->is_equal("static") || current_token->is_equal("restrict"))
	{
		current_token = current_token->_next.get();
	}

	/* 要素数の指定がない場合は長さを-1にする */
	if (current_token->is_equal("]"))
	{
		ty = type_suffix(next_token, current_token->_next.get(), move(ty));
		return Type::array_of(ty, -1);
	}

	int sz = const_expr(&current_token, current_token);
	current_token = skip(current_token, "]");
	ty = type_suffix(next_token, current_token, move(ty));
	return Type::array_of(ty, sz);
}

/**
 * @brief 宣言を変数 or 関数か判断し結果の型を返す
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 宣言されている型
 * @return 変数 or 関数の型
 * @details
 * 次のEBNF規則に従う。 @n
 * type-suffix = "(" function-parameters | "[" array-dimension | ε @n
 * function-parameters = parameter ("," parameter)* @n
 * parameter = declspec declarator
 */
shared_ptr<Type> Node::type_suffix(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	/* 識別子名の後に"("があれば関数 */
	if (current_token->is_equal("("))
	{
		return function_parameters(next_token, current_token->_next.get(), move(ty));
	}

	/* 識別子名の後に"["があれば配列 */
	if (current_token->is_equal("["))
	{
		return array_dimensions(next_token, current_token->_next.get(), move(ty));
	}

	/* そうでなければ普通の変数 */
	*next_token = current_token;
	return ty;
}

/**
 * @brief 変数宣言を変数名部分を読み取る
 *
 * @details
 * 変数の型は変数名に辿りつくまで確定できない。 @n
 * 例：int a, *b, **c; aはint型、bはint型へのポインタ、cはint型へのポインタへのポインタ @n
 * 下記のEBNF規則に従う。 @n declarator = pointers ("(" identifier ")" | "(" declarator ")" | identifier) type-suffix
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 変数の型の基準
 * @return 変数の型
 */
shared_ptr<Type> Node::declarator(Token **next_token, Token *current_token, shared_ptr<Type> ty)
{
	/* ポインター指定を読み取る */
	ty = pointers(&current_token, current_token, ty);

	/* ネストした型の場合、外側を先に評価する */
	if (current_token->is_equal("("))
	{
		auto start = current_token;
		auto dummy = make_shared<Type>();
		/* ネスト部分を飛ばす */
		declarator(&current_token, start->_next.get(), dummy);
		current_token = skip(current_token, ")");
		/* ネストの外側の型を決める */
		ty = type_suffix(next_token, current_token, move(ty));
		/* 外側の型をベースの型としてネスト部分の型を決定する */
		return declarator(&current_token, start->_next.get(), ty);
	}

	Token *name = nullptr;
	Token *name_pos = current_token;

	/* 変数名があれば変数名を設定 */
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		name = current_token;
		current_token = current_token->_next.get();
	}

	/* 関数か変数か */
	ty = type_suffix(next_token, current_token, move(ty));

	/* 参照トークンを設定 */
	ty->_name = name;
	ty->_name_pos = name_pos;

	return ty;
}

/**
 * @brief 構造体,共用体の宣言、定義を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 構造体、共用体の型
 * @details 下記のEBNF規則に従う。 @n struct-union-decl = identifier? ( "{" struct-members )?
 */
shared_ptr<Type> Node::struct_union_decl(Token **next_token, Token *current_token)
{
	/* 存在するならば構造体,共用体のタグを読む */
	Token *tag = nullptr;
	if (current_token->_kind == TokenKind::TK_IDENT)
	{
		tag = current_token;
		current_token = current_token->_next.get();
	}

	/* 構造体,共用体のタグが存在し、かつ定義ではない場合 */
	/* 例 struct foo hoge; */
	if (tag && !current_token->is_equal("{"))
	{
		auto ty = Object::find_tag(tag);
		*next_token = current_token;
		/* タグが登録済みならばタグの型をそのまま返す */
		if (ty)
		{
			return ty;
		}
		/* タグが未登録の場合はサイズ-1の構造体を作成して登録する */
		ty = Type::struct_type();
		ty->_size = -1;
		Object::push_tag_scope(tag, ty);
		return ty;
	}

	current_token = skip(current_token, "{");

	/* 構造体の情報を読み込む */
	auto ty = Type::struct_type();
	struct_members(next_token, current_token, ty.get());

	if (tag)
	{
		auto ty2 = Object::find_tag_in_internal_scope(tag);

		/* タグが現在のスコープに存在している場合は上書きする。 */
		if (ty2)
		{
			*ty2 = *ty;
			return ty2;
		}
		/* 存在しない場合は現在のスコープに登録 */
		Object::push_tag_scope(tag, ty);
	}

	return ty;
}

/**
 * @brief 構造体の宣言、定義を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 構造体の型
 * @details 下記のEBNF規則に従う。 @n struct-decl = struct-union-decl
 */
shared_ptr<Type> Node::struct_decl(Token **next_token, Token *current_token)
{
	/* 構造体の情報を読み込む */
	auto ty = struct_union_decl(next_token, current_token);
	ty->_kind = TypeKind::TY_STRUCT;

	/* 宣言のみならば型情報だけ返す */
	if (ty->_size < 0)
	{
		return ty;
	}

	/* 構造体のメンバのオフセットを計算する */
	int offset = 0;
	for (auto mem = ty->_members.get(); mem; mem = mem->_next.get())
	{
		offset = Object::align_to(offset, mem->_align);
		mem->_offset = offset;
		offset += mem->_ty->_size;
		/* アライメントの基数はメンバのアライメントのうち最大値に合わせる */
		if (ty->_align < mem->_align)
		{
			ty->_align = mem->_align;
		}
	}
	ty->_size = Object::align_to(offset, ty->_align);
	return ty;
}

/**
 * @brief 共用体の宣言、定義を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 共用体の型
 * @details 下記のEBNF規則に従う。 @n union-decl = struct-union-decl
 */
shared_ptr<Type> Node::union_decl(Token **next_token, Token *current_token)
{
	auto ty = struct_union_decl(next_token, current_token);
	ty->_kind = TypeKind::TY_UNION;

	/* 宣言のみならば型情報だけ返す */
	if (ty->_size < 0)
	{
		return ty;
	}

	/* 共用体の場合、全てのoffsetは0で共通である。 */
	/* アライメントと全体のサイズの計算だけ行う */
	for (auto mem = ty->_members.get(); mem; mem = mem->_next.get())
	{
		if (ty->_align < mem->_align)
		{
			ty->_align = mem->_align;
		}
		if (ty->_size < mem->_ty->_size)
		{
			ty->_size = mem->_ty->_size;
		}
	}
	ty->_size = Object::align_to(ty->_size, ty->_align);
	return ty;
}

/**
 * @brief 構造体定義のメンバの定義を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 構造体の型
 * @details 下記のEBNF規則に従う。 @n struct-members = (declspec declarator ("," declarator)* ";")* "}"
 */
void Node::struct_members(Token **next_token, Token *current_token, Type *ty)
{
	/* ダミーの先頭 */
	auto head = make_shared<Member>();
	auto cur = head.get();
	int idx = 0;

	/* '}'が出てくるまで読み込み続ける */
	while (!current_token->is_equal("}"))
	{
		VarAttr attr = {};
		auto base = declspec(&current_token, current_token, &attr);
		bool first = true;

		/* ';'が出てくるまで読み込み続ける */
		while (!consume(&current_token, current_token, ";"))
		{
			if (!first)
			{
				/* 2個目以降はカンマ区切りが必要 */
				current_token = skip(current_token, ",");
			}
			first = false;
			auto mem = make_shared<Member>();
			/* メンバ名部分を読み込む */
			mem->_ty = declarator(&current_token, current_token, base);

			/* メンバ名がなければエラー */
			if (!mem->_ty->_name)
			{
				error_token("メンバ名がありません", mem->_ty->_name_pos);
			}

			mem->_token = mem->_ty->_name;
			mem->_idx = idx++;
			/* アライン指定がある場合はそちらを優先 */
			mem->_align = attr._align ? attr._align : mem->_ty->_align;
			/* リストの先頭に繋ぐ */
			cur->_next = move(mem);
			cur = cur->_next.get();
		}
	}

	/* 構造体の最後の要素が不完全配列型である場合、フレキシブル配列メンバとして扱う。
	 * フレキシブル配列メンバは要素数0の配列として扱う。
	 */
	if (cur != head.get() && TypeKind::TY_ARRAY == cur->_ty->_kind && cur->_ty->_array_length < 0)
	{
		cur->_ty = Type::array_of(cur->_ty->_base, 0);
		ty->_is_flexible = true;
	}

	*next_token = current_token->_next.get();
	ty->_members = move(head->_next);
}

/**
 * @brief 入力トークンと対応する構造体のメンバーを取得する。
 * 見つからなければエラーとする。
 *
 * @param ty 構造体の型情報
 * @param token 取得するメンバのトークン
 * @return 構造体のメンバのポインタ
 */
shared_ptr<Member> Node::get_struct_member(Type *ty, Token *token)
{
	for (auto mem = ty->_members; mem; mem = mem->_next)
	{
		if (mem->_token->_str.size() == token->_str.size() &&
			std::equal(token->_str.begin(), token->_str.end(), mem->_token->_str.begin()))
		{
			return mem;
		}
	}
	/* 見つからなければエラー */
	error_token("存在しないメンバです", token);

	/* コンパイルエラー対策 */
	return nullptr;
}

/**
 * @brief 構造体,共用体のメンバにアクセスするノードを生成する
 *
 * @param lhs 参照される構造体、共用体
 * @param token メンバの呼び出しに対応するトークン
 * @return メンバに対応するノード
 */
unique_ptr<Node> Node::struct_ref(unique_ptr<Node> &&lhs, Token *token)
{
	/* 型を確定させる */
	Type::add_type(lhs.get());

	/* 構造体,共用体でなければエラー */
	if (lhs->_ty->_kind != TypeKind::TY_STRUCT && lhs->_ty->_kind != TypeKind::TY_UNION)
	{
		error_token("構造体または共用体ではありません", token);
	}

	auto mem = get_struct_member(lhs->_ty.get(), token);
	auto node = make_unique<Node>(NodeKind::ND_MEMBER, move(lhs), token);
	node->_member = move(mem);
	return node;
}

/**
 * @brief 変数宣言の型指定子部分を読み取る
 *
 * @details
 * 下記のEBNF規則に従う。 @n
 * declspec =  ("void" | "_BOOL" | "int" | "short" | "long" | "char" @n
 * 				| "typedef" | "static" | "extern" @n
 * 				| "signed" @n
 * 				| struct-decl | union-decl | typedef-name @n
 * 				| enum-specifier)+ @n
 *   		    | "const" | "volatile" | "auto" | "register" | "restrict" @n
 *          	| "__restrict" | "__restrict__" | "_Noreturn")+ @n
 * 型指定子における型名の順番は重要ではない。例えば、`int long static` は `static long int` と同じ意味である。
 * 'long` や `short` が指定されていれば `int` を省略できるので、`static long` と書くこともできる。
 * しかし、`char int` のようなものは有効な型指定子ではなく、型名の組み合わせは限られている。
 * この関数では、それまでの型名が表す「現在の」型オブジェクトを保持したまま、各型名の出現回数を数える。
 * 型名でないトークンに達すると、現在の型オブジェクトを返す。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param attr 読み取った型の属性を返すための参照
 * @return 変数の型
 */
shared_ptr<Type> Node::declspec(Token **next_token, Token *current_token, VarAttr *attr)
{
	constexpr int VOID = 1 << 0;
	constexpr int BOOL = 1 << 2;
	constexpr int CHAR = 1 << 4;
	constexpr int SHORT = 1 << 6;
	constexpr int INT = 1 << 8;
	constexpr int LONG = 1 << 10;
	constexpr int FLOAT = 1 << 12;
	constexpr int DOUBLE = 1 << 14;
	constexpr int OTHER = 1 << 16;
	constexpr int SIGNED = 1 << 17;
	constexpr int UNSIGNED = 1 << 18;

	static const std::unordered_map<string_view, void (*)(int &)> add_counter = {
		{"void", [](int &counter)
		 { counter += VOID; }},
		{"_Bool", [](int &counter)
		 { counter += BOOL; }},
		{"char", [](int &counter)
		 { counter += CHAR; }},
		{"short", [](int &counter)
		 { counter += SHORT; }},
		{"int", [](int &counter)
		 { counter += INT; }},
		{"long", [](int &counter)
		 { counter += LONG; }},
		{"float", [](int &counter)
		 { counter += FLOAT; }},
		{"double", [](int &counter)
		 { counter += DOUBLE; }},
		{"signed", [](int &counter)
		 { counter |= SIGNED; }},
		{"unsigned", [](int &counter)
		 { counter |= UNSIGNED; }},
	};

	static const std::unordered_map<int, shared_ptr<Type>> int_to_type = {
		{VOID, Type::VOID_BASE},
		{BOOL, Type::BOOL_BASE},
		{CHAR, Type::CHAR_BASE},
		{SIGNED + CHAR, Type::CHAR_BASE},
		{UNSIGNED + CHAR, Type::UCHAR_BASE},
		{SHORT, Type::SHORT_BASE},
		{SHORT + INT, Type::SHORT_BASE},
		{SIGNED + SHORT, Type::SHORT_BASE},
		{SIGNED + SHORT + INT, Type::SHORT_BASE},
		{UNSIGNED + SHORT, Type::USHORT_BASE},
		{UNSIGNED + SHORT + INT, Type::USHORT_BASE},
		{INT, Type::INT_BASE},
		{SIGNED, Type::INT_BASE},
		{SIGNED + INT, Type::INT_BASE},
		{UNSIGNED, Type::UINT_BASE},
		{UNSIGNED + INT, Type::UINT_BASE},
		{LONG, Type::LONG_BASE},
		{LONG + INT, Type::LONG_BASE},
		{LONG + LONG, Type::LONG_BASE},
		{LONG + LONG + INT, Type::LONG_BASE},
		{SIGNED + LONG, Type::LONG_BASE},
		{SIGNED + LONG + INT, Type::LONG_BASE},
		{SIGNED + LONG + LONG, Type::LONG_BASE},
		{SIGNED + LONG + LONG + INT, Type::LONG_BASE},
		{UNSIGNED + LONG, Type::ULONG_BASE},
		{UNSIGNED + LONG + INT, Type::ULONG_BASE},
		{UNSIGNED + LONG + LONG, Type::ULONG_BASE},
		{UNSIGNED + LONG + LONG + INT, Type::ULONG_BASE},
		{FLOAT, Type::FLOAT_BASE},
		{DOUBLE, Type::DOUBLE_BASE},
		{LONG + DOUBLE, Type::DOUBLE_BASE},
	};

	/* それぞれの型名の出現回数を表すカウンタ。
	 * 例えばビット0, 1は「void」という型名の出現回数を表す。
	 */
	int counter = 0;
	auto ty = Type::INT_BASE;

	while (current_token->is_typename())
	{
		/* ストレージクラス指定子 */
		if (current_token->is_equal("typedef") || current_token->is_equal("static") || current_token->is_equal("extern"))
		{
			/* ストレージクラス指定子が使用できない箇所である場合エラー 例 function(int i, typedef int INT) */
			if (!attr)
			{
				error_token("ここではストレージクラス指定子は使用できません", current_token);
			}
			if (current_token->is_equal("typedef"))
			{
				attr->_is_typedef = true;
			}
			else if (current_token->is_equal("static"))
			{
				attr->_is_static = true;
			}
			else
			{
				attr->_is_extern = true;
			}

			/* typedef,static, externが同時に指定されている場合 */
			if (attr->_is_static + attr->_is_typedef + attr->_is_extern > 1)
			{
				error_token("typedef, static, externは同時に指定できません", current_token);
			}
			current_token = current_token->_next.get();
			continue;
		}

		/* これらのキーワードは認識するが無視する */
		if (consume(&current_token, current_token, "const") || consume(&current_token, current_token, "volatile") ||
			consume(&current_token, current_token, "auto") || consume(&current_token, current_token, "register") ||
			consume(&current_token, current_token, "restrict") || consume(&current_token, current_token, "__restrict") ||
			consume(&current_token, current_token, "__restrict__") || consume(&current_token, current_token, "_Noreturn"))
		{
			continue;
		}

		/* アライメント指定 */
		if (current_token->is_equal("_Alignas"))
		{
			if (!attr)
			{
				error_token("ここでは_Alignas指定子は使用できません", current_token);
			}
			current_token = skip(current_token->_next.get(), "(");

			if (current_token->is_typename())
			{
				attr->_align = type_name(&current_token, current_token)->_align;
			}
			else
			{
				attr->_align = const_expr(&current_token, current_token);
			}
			current_token = skip(current_token, ")");
			continue;
		}

		/* ユーザー定義の型 */
		auto ty2 = Object::find_typedef(current_token);
		if (current_token->is_equal("struct") || current_token->is_equal("union") || current_token->is_equal("enum") || ty2)
		{
			/* ユーザー定義型が他の型と組み合わさることはない 例 int hoge x; */
			if (counter)
			{
				break;
			}
			if (current_token->is_equal("struct"))
			{
				ty = struct_decl(&current_token, current_token->_next.get());
			}
			else if (current_token->is_equal("union"))
			{
				ty = union_decl(&current_token, current_token->_next.get());
			}
			else if (current_token->is_equal("enum"))
			{
				ty = enum_specifier(&current_token, current_token->_next.get());
			}
			else
			{
				ty = ty2;
				current_token = current_token->_next.get();
			}
			counter += OTHER;
			continue;
		}

		auto it = add_counter.find(current_token->_str);
		if (it != add_counter.end())
		{
			it->second(counter);
		}
		else
		{
			error_token("型名ではありません", current_token);
		}

		auto it2 = int_to_type.find(counter);
		if (it2 != int_to_type.end())
		{
			ty = it2->second;
		}
		else
		{
			error_token("無効な型指定です", current_token);
		}

		current_token = current_token->_next.get();
	}

	*next_token = current_token;
	return ty;
}

/**
 * @brief 列挙型の定義を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応する列挙型の型
 * @details 下記のEBNF規則に従う。 @n
 * enum-specifier = ident? "{" enum-list? "}" | ident ("{" enum-list? "}")? @n
 * enum-list = ident ("=" const-expr)? ("," ident ("=" const-expr)?)* ","?
 *
 */
shared_ptr<Type> Node::enum_specifier(Token **next_token, Token *current_token)
{
	auto ty = Type::enum_type();

	/* 列挙型のタグを読む */
	Token *tag = nullptr;
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		tag = current_token;
		current_token = current_token->_next.get();
	}

	/* 列挙型の変数宣言 */
	if (tag && !current_token->is_equal("{"))
	{
		auto ty = Object::find_tag(tag);
		if (!ty)
		{
			error_token("未定義の列挙型です", tag);
		}
		if (TypeKind::TY_ENUM != ty->_kind)
		{
			error_token("列挙型ではありません", tag);
		}
		*next_token = current_token;
		return ty;
	}

	current_token = skip(current_token, "{");

	/* 列挙型のリストを読む */
	bool first = true;
	int val = 0;

	/* "}"が出てくるまで読み続ける */
	while (!consume_end(next_token, current_token))
	{
		if (!first)
		{
			/* 2個目以降では","区切りが必要 */
			current_token = skip(current_token, ",");
		}
		first = false;
		/* 列挙型の変数名 */
		auto name = current_token->_str;
		current_token = current_token->_next.get();

		/* 数値の指定がある場合 */
		if (current_token->is_equal("="))
		{
			val = const_expr(&current_token, current_token->_next.get());
		}

		/* スコープに登録する。次の変数には+1インクリメントされた数値が対応する */
		auto sc = Object::push_scope(name);
		sc->enum_ty = ty;
		sc->enum_val = val++;
	}

	/* タグが存在するならばタグスコープに追加する */
	if (tag)
	{
		Object::push_tag_scope(tag, ty);
	}
	return ty;
}

/**
 * @brief 単文を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n expression-statement = expression? ';'
 */
unique_ptr<Node> Node::expression_statement(Token **next_token, Token *current_token)
{
	/* 空のstatementに対応 */
	if (current_token->is_equal(";"))
	{
		*next_token = current_token->_next.get();
		return make_unique<Node>(NodeKind::ND_BLOCK, current_token);
	}
	auto node = make_unique<Node>(NodeKind::ND_EXPR_STMT, current_token);
	node->_lhs = expression(&current_token, current_token);

	/* expression-statementは';'で終わるはず */
	*next_token = skip(current_token, ";");
	return node;
}

/**
 * @brief 式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n expression = assign (',' expression)?
 */
unique_ptr<Node> Node::expression(Token **next_token, Token *current_token)
{
	auto node = assign(&current_token, current_token);

	/* 後ろにカンマがあるときは式が続く */
	if (current_token->is_equal(","))
	{
		return make_unique<Node>(NodeKind::ND_COMMA, move(node), expression(next_token, current_token->_next.get()), current_token);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief ノードを定数式として評価
 *
 * @param node 評価するノード
 * @return 評価結果の数値
 */
int64_t Node::evaluate(Node *node)
{
	return evaluate2(node, nullptr);
}

/**
 * @brief ノードを定数式として評価
 *
 * @param node 評価するノード
 * @param label 定数式が変数のポインタの場合、変数の名前
 * @return 評価結果の数値
 */
int64_t Node::evaluate2(Node *node, string *label)
{
	Type::add_type(node);

	if (node->_ty->is_flonum())
	{
		return evaluate_double(node);
	}

	int64_t ret = 0;

	switch (node->_kind)
	{
	case NodeKind::ND_ADD:
		ret = evaluate2(node->_lhs.get(), label) + evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_SUB:
		ret = evaluate2(node->_lhs.get(), label) - evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_MUL:
		ret = evaluate(node->_lhs.get()) * evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_DIV:
		if (node->_ty->_is_unsigned)
		{
			ret = static_cast<uint64_t>(evaluate(node->_lhs.get())) / evaluate(node->_rhs.get());
		}
		else
		{
			ret = evaluate(node->_lhs.get()) / evaluate(node->_rhs.get());
		}
		break;
	case NodeKind::ND_NEG:
		ret = -evaluate(node->_lhs.get());
		break;
	case NodeKind::ND_COND:
		ret = evaluate(node->_condition.get()) ? evaluate2(node->_then.get(), label) : evaluate2(node->_else.get(), label);
		break;
	case NodeKind::ND_COMMA:
		ret = evaluate2(node->_rhs.get(), label);
		break;
	case NodeKind::ND_MEMBER:
		if (!label)
		{
			error_token("コンパイル時に定数ではありません", node->_token);
		}
		if (TypeKind::TY_ARRAY != node->_ty->_kind)
		{
			error_token("無効な初期化式です", node->_token);
		}
		ret = evaluate_rval(node->_lhs.get(), label) + node->_member->_offset;
		break;
	case NodeKind::ND_ADDR:
		ret = evaluate_rval(node->_lhs.get(), label);
		break;
	case NodeKind::ND_NOT:
		ret = !evaluate(node->_lhs.get());
		break;
	case NodeKind::ND_BITNOT:
		ret = ~evaluate(node->_lhs.get());
		break;
	case NodeKind::ND_LOGAND:
		ret = evaluate(node->_lhs.get()) && evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_LOGOR:
		ret = evaluate(node->_lhs.get()) || evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_MOD:
		if (node->_ty->_is_unsigned)
		{
			ret = static_cast<uint64_t>(evaluate(node->_lhs.get())) % evaluate(node->_rhs.get());
		}
		else
		{
			ret = evaluate(node->_lhs.get()) % evaluate(node->_rhs.get());
		}
		break;
	case NodeKind::ND_BITAND:
		ret = evaluate(node->_lhs.get()) & evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_BITOR:
		ret = evaluate(node->_lhs.get()) | evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_BITXOR:
		ret = evaluate(node->_lhs.get()) ^ evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_SHL:
		ret = evaluate(node->_lhs.get()) << evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_SHR:
		if (node->_ty->_is_unsigned && node->_ty->_size == 8)
		{
			ret = static_cast<uint64_t>(evaluate(node->_lhs.get())) >> evaluate(node->_rhs.get());
		}
		else
		{
			ret = evaluate(node->_lhs.get()) >> evaluate(node->_rhs.get());
		}
		break;
	case NodeKind::ND_EQ:
		ret = evaluate(node->_lhs.get()) == evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_NE:
		ret = evaluate(node->_lhs.get()) != evaluate(node->_rhs.get());
		break;
	case NodeKind::ND_LT:
		if (node->_ty->_is_unsigned)
		{
			ret = static_cast<uint64_t>(evaluate(node->_lhs.get())) < evaluate(node->_rhs.get());
		}
		else
		{
			ret = evaluate(node->_lhs.get()) < evaluate(node->_rhs.get());
		}
		break;
	case NodeKind::ND_LE:
		if (node->_ty->_is_unsigned)
		{
			ret = static_cast<uint64_t>(evaluate(node->_lhs.get())) <= evaluate(node->_rhs.get());
		}
		else
		{
			ret = evaluate(node->_lhs.get()) <= evaluate(node->_rhs.get());
		}
		break;
	case NodeKind::ND_NUM:
		ret = node->_val;
		break;
	case NodeKind::ND_VAR:
		if (!label)
		{
			error_token("コンパイル時に定数ではありません", node->_token);
		}
		if (TypeKind::TY_ARRAY != node->_var->_ty->_kind && TypeKind::TY_FUNC != node->_var->_ty->_kind)
		{
			error_token("無効な初期化式です", node->_token);
		}
		*label = node->_var->_name;
		ret = 0;
		break;
	case NodeKind::ND_CAST:
	{
		uint64_t val = evaluate2(node->_lhs.get(), label);
		if (node->_ty->is_integer())
		{

			switch (node->_ty->_size)
			{
			case 1:
				ret = node->_ty->_is_unsigned ? static_cast<uint8_t>(val) : static_cast<int8_t>(val);
				break;
			case 2:
				ret = node->_ty->_is_unsigned ? static_cast<uint16_t>(val) : static_cast<int16_t>(val);
				break;
			case 4:
				ret = node->_ty->_is_unsigned ? static_cast<uint32_t>(val) : static_cast<int32_t>(val);
				break;
			default:
				ret = val;
				break;
			}
		}
		else
		{
			ret = val;
		}
		break;
	}
	default:
		error_token("コンパイル時に定数ではありません", node->_token);
	}
	return ret;
}

/**
 * @brief 浮動小数点数型の定数式を評価する
 *
 * @param 評価対象のノード
 * @return 評価結果
 */
double Node::evaluate_double(Node *node)
{
	Type::add_type(node);

	if (node->_ty->is_integer())
	{
		if (node->_ty->_is_unsigned)
		{
			return static_cast<unsigned long>(evaluate(node));
		}
		return evaluate(node);
	}

	double ret = 0.0;

	switch (node->_kind)
	{
	case NodeKind::ND_ADD:
		ret = evaluate_double(node->_lhs.get()) + evaluate_double(node->_rhs.get());
		break;
	case NodeKind::ND_SUB:
		ret = evaluate_double(node->_lhs.get()) - evaluate_double(node->_rhs.get());
		break;
	case NodeKind::ND_MUL:
		ret = evaluate_double(node->_lhs.get()) * evaluate_double(node->_rhs.get());
		break;
	case NodeKind::ND_DIV:
		ret = evaluate_double(node->_lhs.get()) / evaluate_double(node->_rhs.get());
		break;
	case NodeKind::ND_NEG:
		ret = -evaluate_double(node->_lhs.get());
		break;
	case NodeKind::ND_COND:
		ret = evaluate_double(node->_condition.get()) ? evaluate_double(node->_then.get()) : evaluate_double(node->_else.get());
		break;
	case NodeKind::ND_COMMA:
		ret = evaluate_double(node->_rhs.get());
		break;
	case NodeKind::ND_NUM:
		ret = node->_fval;
		break;
	case NodeKind::ND_CAST:
		if (node->_lhs->_ty->is_flonum())
		{
			ret = evaluate_double(node->_lhs.get());
		}
		else
		{
			ret = evaluate(node->_lhs.get());
		}
		break;
	default:
		error_token("コンパイル時に定数ではありません", node->_token);
	}
	return ret;
}

/**
 * @brief 右辺値を評価する
 *
 * @param node 評価対象のノード
 * @param label 変数名を返すための参照
 * @return 評価結果
 */
int64_t Node::evaluate_rval(Node *node, string *label)
{
	int64_t ret = 0;
	switch (node->_kind)
	{
	case NodeKind::ND_MEMBER:
		ret = evaluate_rval(node->_lhs.get(), label) + node->_member->_offset;
		break;
	case NodeKind::ND_DEREF:
		ret = evaluate2(node->_lhs.get(), label);
		break;
	case NodeKind::ND_VAR:
		if (node->_var->_is_local)
		{
			error_token("コンパイル時に定数ではありません", node->_token);
		}
		*label = node->_var->_name;
		break;
	default:
		error_token("無効な初期化式です", node->_token);
	}

	return ret;
}

/**
 * @brief 定数式を読み取って評価する
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 評価結果の数値
 */
int64_t Node::const_expr(Token **next_token, Token *current_token)
{
	auto node = conditional(next_token, current_token);
	return evaluate(node.get());
}

/**
 * @brief 演算代入式を表すノードを生成する
 *
 * @param binary 演算代入を行う左辺と右辺をもったノード
 * @return 演算代入式を表すノード
 * @details 'A op= B'を'tmp = &A, *tmp = *tmp op B'に変換する
 */
unique_ptr<Node> Node::to_assign(unique_ptr<Node> &&binary)
{
	/* 左辺と右辺の型を決定 */
	Type::add_type(binary->_lhs.get());
	Type::add_type(binary->_rhs.get());

	auto token = binary->_token;

	/* 仮変数tmpの作成 */
	auto var = Object::new_lvar("", Type::pointer_to(binary->_lhs->_ty));

	/* tmp = &A */
	auto expr1 = make_unique<Node>(NodeKind::ND_ASSIGN,
								   make_unique<Node>(var, token),
								   make_unique<Node>(NodeKind::ND_ADDR, move(binary->_lhs), token),
								   token);

	/* *tmp = *tmp op B */
	auto expr2 = make_unique<Node>(NodeKind::ND_ASSIGN,
								   make_unique<Node>(NodeKind::ND_DEREF, make_unique<Node>(var, token), token),
								   make_unique<Node>(move(binary->_kind),
													 make_unique<Node>(NodeKind::ND_DEREF, make_unique<Node>(var, token), token),
													 move(binary->_rhs),
													 token),
								   token);

	return make_unique<Node>(NodeKind::ND_COMMA, move(expr1), move(expr2), token);
}

/**
 * @brief 代入式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * assign = conditional (assign-op assign)? @n
 * assign-op = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "<<=" | ">>="
 */
unique_ptr<Node> Node::assign(Token **next_token, Token *current_token)
{
	static const std::unordered_map<string, NodeKind> str_to_op = {
		{"*=", NodeKind::ND_MUL},
		{"/=", NodeKind::ND_DIV},
		{"%=", NodeKind::ND_MOD},
		{"&=", NodeKind::ND_BITAND},
		{"|=", NodeKind::ND_BITOR},
		{"^=", NodeKind::ND_BITXOR},
		{"<<=", NodeKind::ND_SHL},
		{">>=", NodeKind::ND_SHR},
	};

	auto node = conditional(&current_token, current_token);

	if (current_token->is_equal("="))
	{
		return make_unique<Node>(NodeKind::ND_ASSIGN, move(node), assign(next_token, current_token->_next.get()), current_token);
	}
	if (current_token->is_equal("+="))
	{
		return to_assign(new_add(move(node), assign(next_token, current_token->_next.get()), current_token));
	}

	if (current_token->is_equal("-="))
	{
		return to_assign(new_sub(move(node), assign(next_token, current_token->_next.get()), current_token));
	}

	auto it = str_to_op.find(current_token->_str);
	if (it != str_to_op.end())
	{
		return to_assign(make_unique<Node>(it->second, move(node), assign(next_token, current_token->_next.get()), current_token));
	}

	*next_token = current_token;
	return node;
}

/**
 * @brief 3項演算子を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n conditional = logor ( "?" expr ":" conditional )?
 */
unique_ptr<Node> Node::conditional(Token **next_token, Token *current_token)
{
	auto cond = log_or(&current_token, current_token);

	/* 3項演算子を含まない */
	if (!current_token->is_equal("?"))
	{
		*next_token = current_token;
		return cond;
	}

	auto node = make_unique<Node>(NodeKind::ND_COND, current_token);
	node->_condition = move(cond);
	node->_then = expression(&current_token, current_token->_next.get());
	current_token = skip(current_token, ":");
	node->_else = conditional(next_token, current_token);
	return node;
}

/**
 * @brief log_orを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n logor = logand ("||"  logand)*
 */
unique_ptr<Node> Node::log_or(Token **next_token, Token *current_token)
{
	auto node = log_and(&current_token, current_token);

	/* "||"が出てくる限り読み込み続ける */
	while (current_token->is_equal("||"))
	{
		auto start = current_token;
		node = make_unique<Node>(NodeKind::ND_LOGOR, move(node), log_and(&current_token, current_token->_next.get()), start);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief log_andを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n logand = bitor ("&&"  bitor)*
 */
unique_ptr<Node> Node::log_and(Token **next_token, Token *current_token)
{
	auto node = bit_or(&current_token, current_token);

	/* "||"が出てくる限り読み込み続ける */
	while (current_token->is_equal("&&"))
	{
		auto start = current_token;
		node = make_unique<Node>(NodeKind::ND_LOGAND, move(node), bit_or(&current_token, current_token->_next.get()), start);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief bitorを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n bitor = bitxor ("|" bitxor)*
 */
unique_ptr<Node> Node::bit_or(Token **next_token, Token *current_token)
{
	auto node = bit_xor(&current_token, current_token);

	/* "|"が出てくる限り読み込み続ける */
	while (current_token->is_equal("|"))
	{
		auto start = current_token;
		node = make_unique<Node>(NodeKind::ND_BITOR, move(node), bit_xor(&current_token, current_token->_next.get()), start);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief bitxorを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n bitxor = bitand ("^" bitand)*
 */
unique_ptr<Node> Node::bit_xor(Token **next_token, Token *current_token)
{
	auto node = bit_and(&current_token, current_token);
	/* "^"が出てくる限り読み込み続ける */
	while (current_token->is_equal("^"))
	{
		auto start = current_token;
		node = make_unique<Node>(NodeKind::ND_BITXOR, move(node), bit_and(&current_token, current_token->_next.get()), start);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief bitandを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n bitand = equality ("&" equality)*
 */
unique_ptr<Node> Node::bit_and(Token **next_token, Token *current_token)
{
	auto node = equality(&current_token, current_token);
	/* "&"が出てくる限り読み込み続ける */
	while (current_token->is_equal("&"))
	{
		auto start = current_token;
		node = make_unique<Node>(NodeKind::ND_BITAND, move(node), equality(&current_token, current_token->_next.get()), start);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief 等価比較式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n equality = relational ("==" relational | "!=" relational)*
 */
unique_ptr<Node> Node::equality(Token **next_token, Token *current_token)
{
	auto node = relational(&current_token, current_token);

	for (;;)
	{
		auto start = current_token;

		if (current_token->is_equal("=="))
		{
			node = make_unique<Node>(NodeKind::ND_EQ, move(node), relational(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("!="))
		{
			node = make_unique<Node>(NodeKind::ND_NE, move(node), relational(&current_token, current_token->_next.get()), start);
			continue;
		}

		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 比較式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
 */
unique_ptr<Node> Node::relational(Token **next_token, Token *current_token)
{
	auto node = shift(&current_token, current_token);

	while (true)
	{
		auto start = current_token;

		if (current_token->is_equal("<"))
		{
			node = make_unique<Node>(NodeKind::ND_LT, move(node), shift(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("<="))
		{
			node = make_unique<Node>(NodeKind::ND_LE, move(node), shift(&current_token, current_token->_next.get()), start);
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (current_token->is_equal(">"))
		{
			node = make_unique<Node>(NodeKind::ND_LT, shift(&current_token, current_token->_next.get()), move(node), start);
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (current_token->is_equal(">="))
		{
			node = make_unique<Node>(NodeKind::ND_LE, shift(&current_token, current_token->_next.get()), move(node), start);
			continue;
		}

		/* 比較演算子が出てこなくなったらループを抜ける */
		*next_token = current_token;
		return node;
	}
}

/**
 * @brief シフト演算子を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n shift = add ("<<" add | ">>" add)*
 */
unique_ptr<Node> Node::shift(Token **next_token, Token *current_token)
{
	auto node = add(&current_token, current_token);

	for (;;)
	{
		auto start = current_token;

		if (current_token->is_equal("<<"))
		{
			node = make_unique<Node>(NodeKind::ND_SHL,
									 move(node),
									 add(&current_token, current_token->_next.get()),
									 start);
			continue;
		}

		if (current_token->is_equal(">>"))
		{
			node = make_unique<Node>(NodeKind::ND_SHR,
									 move(node),
									 add(&current_token, current_token->_next.get()),
									 start);
			continue;
		}
		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 和を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n add = mul ("+" mul | "-" mul)*
 */
unique_ptr<Node> Node::add(Token **next_token, Token *current_token)
{
	auto node = mul(&current_token, current_token);

	while (true)
	{
		auto start = current_token;
		if (current_token->is_equal("+"))
		{
			node = new_add(move(node), mul(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("-"))
		{
			node = new_sub(move(node), mul(&current_token, current_token->_next.get()), start);
			continue;
		}

		/* "+", "-"どちらも出てこなくなったらループを抜ける */
		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 型キャストを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n "(" type-name ")" cast | unary
 */
unique_ptr<Node> Node::cast(Token **next_token, Token *current_token)
{
	/* キャストを含む場合 */
	if (current_token->is_equal("(") && current_token->_next->is_typename())
	{
		auto start = current_token;

		/* キャスト先の型を読み取る */
		auto ty = type_name(&current_token, current_token->_next.get());
		current_token = skip(current_token, ")");

		/* 複合リテラル式 */
		if (current_token->is_equal("{"))
		{
			return unary(next_token, start);
		}

		/* 型キャスト */
		auto node = new_cast(cast(next_token, current_token), ty);
		node->_token = start;
		return node;
	}

	return unary(next_token, current_token);
}

/**
 * @brief 積を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n mul = cast ("*" cast | "/" cast | "%" cast)*
 */
unique_ptr<Node> Node::mul(Token **next_token, Token *current_token)
{
	auto node = cast(&current_token, current_token);

	while (true)
	{
		auto start = current_token;
		if (current_token->is_equal("*"))
		{
			node = make_unique<Node>(NodeKind::ND_MUL, move(node), cast(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("/"))
		{
			node = make_unique<Node>(NodeKind::ND_DIV, move(node), cast(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("%"))
		{
			node = make_unique<Node>(NodeKind::ND_MOD, move(node), cast(&current_token, current_token->_next.get()), start);
			continue;
		}

		/* 除積演算子が出てこなくなったらループを抜ける */
		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 符号付の単項を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n unary = ("+" | "-" | "*" | "&" | "!" | "~") cast | ("++" | "--") unary |postfix
 */
unique_ptr<Node> Node::unary(Token **next_token, Token *current_token)
{
	static const std::unordered_map<string, NodeKind> str_to_type = {
		{"-", NodeKind::ND_NEG},
		{"&", NodeKind::ND_ADDR},
		{"*", NodeKind::ND_DEREF},
		{"!", NodeKind::ND_NOT},
		{"~", NodeKind::ND_BITNOT},
	};

	if (current_token->is_equal("+"))
	{
		return cast(next_token, current_token->_next.get());
	}

	if (current_token->is_equal("-"))
	{
		return make_unique<Node>(NodeKind::ND_NEG, cast(next_token, current_token->_next.get()), current_token);
	}

	auto it = str_to_type.find(current_token->_str);
	if (it != str_to_type.end())
	{
		return make_unique<Node>(it->second, cast(next_token, current_token->_next.get()), current_token);
	}

	/* ++iをi+=1と読み替える */
	if (current_token->is_equal("++"))
	{
		return to_assign(new_add(unary(next_token, current_token->_next.get()), make_unique<Node>(1, current_token), current_token));
	}

	/* --iをi+=1と読み替える */
	if (current_token->is_equal("--"))
	{
		return to_assign(new_sub(unary(next_token, current_token->_next.get()), make_unique<Node>(1, current_token), current_token));
	}

	return postfix(next_token, current_token);
}

/**
 * @brief 型指定子の後に続くpostfixを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * postfix = "(" type-name ")" "{" initializer-list "}" @n
 * 		   | identifier "(" func-args ")" postfix-tail* @n
 * 		   | primary postfix-tail* @n
 * postfix-tail = "[" expression "]" | "(" func-args ")" | "." identifier | "->" identifier | "++" | "--"
 */
unique_ptr<Node> Node::postfix(Token **next_token, Token *current_token)
{
	/* 複合リテラル式 */
	if (current_token->is_equal("(") && current_token->_next->is_typename())
	{
		auto start = current_token;
		/* 型名を評価 */
		auto ty = type_name(&current_token, current_token->_next.get());
		current_token = skip(current_token, ")");

		/* グローバルスコープにいる場合、グローバル変数として登録 */
		if (Object::at_outermost_scope())
		{
			auto var = new_anonymous_gvar(ty);
			gvar_initializer(next_token, current_token, var);
			return make_unique<Node>(var, start);
		}

		/* ローカル変数として登録 */
		auto var = Object::new_lvar("", ty);
		auto lhs = lvar_initializer(next_token, current_token, var);
		auto rhs = make_unique<Node>(var, current_token);
		return make_unique<Node>(NodeKind::ND_COMMA, move(lhs), move(rhs), start);
	}

	/* 単項を読む */
	auto node = primary(&current_token, current_token);

	for (;;)
	{
		/* 関数 */
		if (current_token->is_equal("("))
		{
			node = function_call(&current_token, current_token->_next.get(), move(node));
			continue;
		}

		/* 配列 */
		if (current_token->is_equal("["))
		{
			/* x[y] を *(x+y) に置き換える */
			auto start = current_token;
			auto idx = expression(&current_token, current_token->_next.get());
			current_token = skip(current_token, "]");
			node = make_unique<Node>(NodeKind::ND_DEREF, new_add(move(node), move(idx), start), start);
			continue;
		}
		/* 構造体 */
		if (current_token->is_equal("."))
		{
			node = struct_ref(move(node), current_token->_next.get());
			current_token = current_token->_next->_next.get();
			continue;
		}
		/* 構造体のポインタ */
		if (current_token->is_equal("->"))
		{
			/* x->y を(*x).y を読み替える */
			node = make_unique<Node>(NodeKind::ND_DEREF, move(node), current_token);
			node = struct_ref(move(node), current_token->_next.get());
			current_token = current_token->_next->_next.get();
			continue;
		}
		/* 後置インクリメント */
		if (current_token->is_equal("++"))
		{
			node = new_inc_dec(move(node), current_token, 1);
			current_token = current_token->_next.get();
			continue;
		}
		/* 後置デクリメント */
		if (current_token->is_equal("--"))
		{
			node = new_inc_dec(move(node), current_token, -1);
			current_token = current_token->_next.get();
			continue;
		}

		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 単項を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * primary = "(" "{" statement+ "}" ")" | "(" expression ")" | "sizeof" unary | "sizeof" "(" type-name ")" @n
 * 		   | "_Alignof" "(" type-name ")" | "_Alignof" unary  | identifier args? | str | num @n
 * 		   | args = "(" ")" | identifier
 */
unique_ptr<Node> Node::primary(Token **next_token, Token *current_token)
{
	/* トークンが"(" "{"ならステートメント式 */
	if (current_token->is_equal("(") && current_token->_next->is_equal("{"))
	{
		auto node = make_unique<Node>(NodeKind::ND_STMT_EXPR, current_token);
		/* ブロックを読み取って、ブロック内の処理を新しいnodeのbodyに付け替える */
		auto stmt = compound_statement(&current_token, current_token->_next->_next.get());
		node->_body = move(stmt->_body);
		/* ブロックの後は') */
		*next_token = skip(current_token, ")");
		return node;
	}

	/* トークンが"("なら、"(" expression ")"のはず */
	if (current_token->is_equal("("))
	{
		auto node = expression(&current_token, current_token->_next.get());
		*next_token = skip(current_token, ")");
		return node;
	}

	/* 文字列リテラル */
	if (TokenKind::TK_STR == current_token->_kind)
	{
		/* 前後の'"'を取り除く */
		auto str = current_token->_str.substr(1, current_token->_str.size() - 2);
		auto var = new_string_literal(str);
		*next_token = current_token->_next.get();
		return make_unique<Node>(var, current_token);
	}

	/* sizeof演算子（対象が型そのもの） */
	if (current_token->is_equal("sizeof") &&
		current_token->_next->is_equal("(") &&
		current_token->_next->_next->is_typename())
	{
		auto start = current_token;
		/* sizeof演算子の対象の型情報を読む */
		auto ty = type_name(&current_token, current_token->_next->_next.get());
		*next_token = skip(current_token, ")");
		return make_unique<Node>(ty->_size, Type::ULONG_BASE, start);
	}

	/* sizeof演算子（対象が式） */
	if (current_token->is_equal("sizeof"))
	{
		/* sizeofの対象を評価 */
		auto node = unary(next_token, current_token->_next.get());
		/* sizeofの対象の型を決定 */
		Type::add_type(node.get());
		/* 型のサイズの数値ノードを返す */
		return make_unique<Node>(node->_ty->_size, Type::ULONG_BASE, current_token);
	}

	/* _Alignof演算子(型) */
	if (current_token->is_equal("_Alignof") && current_token->_next->is_equal("(") && current_token->_next->_next->is_typename())
	{
		auto ty = type_name(&current_token, current_token->_next->_next.get());
		*next_token = skip(current_token, ")");
		return make_unique<Node>(ty->_align, Type::ULONG_BASE, current_token);
	}

	/* _Alignof演算子(変数) */
	if (current_token->is_equal("_Alignof"))
	{
		auto node = unary(next_token, current_token->_next.get());
		Type::add_type(node.get());
		return make_unique<Node>(node->_ty->_align, Type::ULONG_BASE, current_token);
	}

	/* トークンが識別子の場合 */
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		/* 識別子名が登録済みか検索 */
		const auto sc = Object::find_var(current_token);
		*next_token = current_token->_next.get();

		/* 識別子名が登録済み */
		if (sc)
		{
			/* 変数 or 関数 */
			if (sc->_var)
			{
				return make_unique<Node>(sc->_var, current_token);
			}
			/* 列挙型 */
			if (sc->enum_ty)
			{
				return make_unique<Node>(sc->enum_val, current_token);
			}
		}

		if (current_token->_next->is_equal("("))
		{
			error_token("関数が未宣言です", current_token);
		}
		error_token("未定義の変数です", current_token);
	}

	/* トークンが数値の場合 */
	if (TokenKind::TK_NUM == current_token->_kind)
	{
		unique_ptr<Node> node;
		if (current_token->_ty->is_flonum())
		{
			node = make_unique<Node>(NodeKind::ND_NUM, current_token);
			node->_fval = current_token->_fval;
		}
		else
		{
			node = make_unique<Node>(current_token->_val, current_token);
		}
		node->_ty = current_token->_ty;
		*next_token = current_token->_next.get();
		return node;
	}

	/* どれでもなければエラー */
	error_token("式ではありません", current_token);

	/* コンパイルエラー対策、error_token()内でプログラムは終了するためnullptrが返ることはない */
	return nullptr;
}

/**
 * @brief typedefを読み取る
 *
 * @param token 現在のトークン
 * @param base ベースの型
 * @return 次のトークン
 */
Token *Node::parse_typedef(Token *token, shared_ptr<Type> &base)
{
	bool first = true;

	/* ";"が出てくるまで読み込み続ける */
	while (!consume(&token, token, ";"))
	{
		/* 2個目以降では","区切りが必要 */
		if (!first)
		{
			token = skip(token, ",");
		}
		first = false;

		auto ty = declarator(&token, token, base);
		/* typedefの対象の名前がない場合はエラー */
		if (!ty->_name)
		{
			error_token("typedefする型名がありません", ty->_name_pos);
		}

		Object::push_scope(ty->_name->_str)->type_def = ty;
	}
	return token;
}

/**
 * @brief ポインタ指定と型修飾子を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 型指定子の型
 * @return 読み取った型
 * @details 下記のEBNF規則に従う。 @n pointers = ("*" ("const" | "volatile" | "restrict")*)*
 */
shared_ptr<Type> Node::pointers(Token **next_token, Token *current_token, shared_ptr<Type> ty)
{
	/* "*"の数だけその前までの型に対するポインター */
	while (consume(&current_token, current_token, "*"))
	{
		ty = Type::pointer_to(ty);

		/* これらの修飾子は無視する */
		while (current_token->is_equal("const") || current_token->is_equal("volatile") ||
			   current_token->is_equal("restrict") || current_token->is_equal("__restrict") || current_token->is_equal("__restrict__"))
		{
			current_token = current_token->_next.get();
		}
	}

	*next_token = current_token;
	return ty;
}

/**
 * @brief 識別子をもたない仮想的な変数宣言として型情報を読む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 型指定子の型
 * @return 読み取った型
 * @details 下記のEBNF規則に従う。 @n abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
 */
shared_ptr<Type> Node::abstract_declarator(Token **next_token, Token *current_token, shared_ptr<Type> ty)
{
	/* ポインター指定を読み取る */
	ty = pointers(&current_token, current_token, ty);

	/* ネストしている型 */
	if (current_token->is_equal("("))
	{
		auto start = current_token;
		auto dummy = make_shared<Type>();
		/* ネストの中を飛ばす */
		abstract_declarator(&current_token, start->_next.get(), dummy);
		current_token = skip(current_token, ")");
		/* ネストの外側の型を読み込む */
		ty = type_suffix(next_token, current_token, move(ty));
		/* ネストの外側の型をベースとして内側の型を読む */
		return abstract_declarator(&current_token, start->_next.get(), ty);
	}
	return type_suffix(next_token, current_token, move(ty));
}

/**
 * @brief 型情報を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 読み取った型
 * @details 下記のEBNF規則に従う。 @n type-name = declspec abstract-declarator
 */
shared_ptr<Type> Node::type_name(Token **next_token, Token *current_token)
{
	auto ty = declspec(&current_token, current_token, nullptr);
	return abstract_declarator(next_token, current_token, ty);
}

/**
 * @brief 関数呼び出し
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * function_call = "(" (assign ("," assign)*)? ")"
 */
unique_ptr<Node> Node::function_call(Token **next_token, Token *current_token, unique_ptr<Node> &&fn)
{
	Type::add_type(fn.get());

	/* 関数または関数ポインタではない場合エラー */
	if (TypeKind::TY_FUNC != fn->_ty->_kind &&
		(TypeKind::TY_PTR != fn->_ty->_kind || TypeKind::TY_FUNC != fn->_ty->_base->_kind))
	{
		error_token("関数ではありません", fn->_token);
	}

	/* 戻り値の型 */
	auto ty = (TypeKind::TY_FUNC == fn->_ty->_kind) ? fn->_ty : fn->_ty->_base;

	/* 引数の型 */
	auto param_ty = ty->_params;

	/* ノードリストの先頭としてダミーのノードを生成 */
	auto head = make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* ')'が出てくるまで読み込み続ける */
	while (!current_token->is_equal(")"))
	{
		if (head.get() != cur)
		{
			/* 2個目以降の引数には区切りとして","が必要 */
			current_token = skip(current_token, ",");
		}
		/* 引数を読み取る */
		auto arg = assign(&current_token, current_token);
		/* 引数の型を決定 */
		Type::add_type(arg.get());

		/* 引数が多すぎる場合 */
		if (!param_ty && !ty->_is_variadic)
		{
			error_token("引数が多すぎます", current_token);
		}

		if (param_ty)
		{
			if (TypeKind::TY_STRUCT == param_ty->_kind || TypeKind::TY_UNION == param_ty->_kind)
			{
				error_token("構造体、共用体の値渡しはサポートしていません", arg->_token);
			}
			/* 実引数の型を宣言されている仮引数の型でキャストする */
			arg = new_cast(move(arg), param_ty);
			param_ty = param_ty->_next;
		}
		else if (TypeKind::TY_FLOAT == arg->_ty->_kind)
		{
			/* 仮引数のの型情報がないとき、実引数がfloat型ならdouble型に昇格させる */
			arg = new_cast(move(arg), Type::DOUBLE_BASE);
		}
		cur->_next = move(arg);
		cur = cur->_next.get();
	}

	/* 引数が少なすぎる場合 */
	if (param_ty)
	{
		error_token("引数が少なすぎます", current_token);
	}

	/* 関数呼び出しノードを作成 */
	auto node = make_unique<Node>(NodeKind::ND_FUNCALL, move(fn), current_token);
	/* headの次のノード以降を切り離し返り値用のnodeのargsに繋ぐ */
	node->_args = move(head->_next);
	/* 関数の型をセット */
	node->_func_ty = ty;
	/* 戻り値の型をセット */
	node->_ty = ty->_return_ty;

	/* 最後は")"" */
	*next_token = skip(current_token, ")");

	return node;
}

/**
 * @brief トップレベルに出てくる定義が関数かどうか判定する
 *
 * @param token declarator部分のトークン
 * @return true 関数である
 * @return false 関数ではない
 */
bool Node::is_function(Token *token)
{
	if (token->is_equal(";"))
	{
		return false;
	}

	auto dummy = make_shared<Type>();
	auto ty = declarator(&token, token, dummy);
	return TypeKind::TY_FUNC == ty->_kind;
}

/**
 * @brief グローバル変数を読み取る
 *
 * @param token 読み込むトークン
 * @param base 型宣言の前半部分から読み取れた型
 * @return 次のトークン
 */
Token *Node::global_variable(Token *token, shared_ptr<Type> &&base, const VarAttr *attr)
{
	/* 1個めの変数であるか */
	bool first = true;

	/* ;が現れるまで読み込みを続ける */
	while (!consume(&token, token, ";"))
	{
		if (!first)
		{
			/* 2個目以降の変数定義には","区切りが必要 */
			token = skip(token, ",");
		}
		/* 初回フラグを下す */
		first = false;

		/* 最終的な型を決定する */
		auto ty = declarator(&token, token, base);

		/* 変数名がない場合はエラー */
		if (!ty->_name)
		{
			error_token("変数名がありません", ty->_name_pos);
		}

		auto var = Object::new_gvar(ty->_name->_str, ty);
		var->_is_definition = !attr->_is_extern;
		var->_is_static = attr->_is_static;

		/* アライン指定 */
		if (attr->_align)
		{
			var->_align = attr->_align;
		}

		if (token->is_equal("="))
		{
			gvar_initializer(&token, token->_next.get(), var);
		}
	}

	return token;
}

/**
 * @brief goto文とラベルの対応を解決する
 *
 * @details goto文では現在の位置よりも後に出てくるラベルに
 * ジャンプすることが可能なため、関数全体をパースした後でなければ
 * ラベルとの対応を解決できない。
 */
void Node::resolve_goto_label()
{
	for (auto x = gotos; x; x = x->_goto_next)
	{
		for (auto y = labels; y; y = y->_goto_next)
		{
			if (x->_label == y->_label)
			{
				x->_unique_label = y->_unique_label;
				break;
			}
		}
		if (x->_unique_label.empty())
		{
			error_token("ラベルが定義されていません", x->_token->_next.get());
		}
	}

	gotos = nullptr;
	labels = nullptr;
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタをnext_tokenにセットしtrueを返す。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param op 比較する文字列
 * @return 一致：true, 不一致：false
 */
bool Node::consume(Token **next_token, Token *current_token, string &&str)
{
	if (current_token->is_equal(move(str)))
	{
		*next_token = current_token->_next.get();
		return true;
	}
	*next_token = current_token;
	return false;
}

/**
 * @brief トークンが期待している文字列と一致する場合は次のトークンのポインタを返す。不一致ならエラー報告。
 *
 * @param token 対象のトークン
 * @param op 比較する文字列
 * @return 次のトークン
 */
Token *Node::skip(Token *token, string &&op)
{
	if (!token->is_equal(move(op)))
	{
		error_token(op + "が必要です", token);
	}
	return token->_next.get();
}

/**
 * @brief トークンが配列、構造体、列挙型の末尾と一致する場合は次のトークンのポインタをnext_tokenにセットしtrueを返す。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 一致：true, 不一致：false
 */
bool Node::consume_end(Token **next_token, Token *current_token)
{
	if (current_token->is_equal("}"))
	{
		*next_token = current_token->_next.get();
		return true;
	}

	if (current_token->is_equal(",") && current_token->_next->is_equal("}"))
	{
		*next_token = current_token->_next->_next.get();
		return true;
	}
	return false;
}