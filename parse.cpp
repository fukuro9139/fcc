/**
 * @file parse.cpp
 * @author K.Fukunaga
 * @brief
 * C言語の再帰下降構文解析を行うパーサーの定義
 *
 * このファイルのほとんどの関数の名前は、入力トークン・リストから読み取る対象に対応している。
 * 例えば、statement() ははトークン・リストからステートメントを読み取ってステートメントに対応する
 * 抽象構文木（abstract syntax tree、AST）ノードを返す。
 * 各関数はASTノードと入力トークンの残りの部分を返すように設計してある。
 * C言語は複数の戻り値をサポートしていないので、残りのトークンはポインタ引数を通じて呼び出し元に返される。
 *
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "parse.hpp"

using std::shared_ptr;
using std::unique_ptr;

/** 現在パースしている関数 */
static Object *current_function = nullptr;

/** 現在の関数で出てくるgoto文とラベルのリスト */
static Node *gotos = nullptr;
static Node *labels = nullptr;

/* 現在のループのを抜けるラベル */
static std::string brk_label = "";

/**************/
/* Node Class */
/**************/

/* コンストラクタ */

Node::Node() = default;

Node::Node(const NodeKind &kind, Token *token) : _kind(kind), _token(token) {}

Node::Node(const NodeKind &kind, unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token)
	: _kind(kind), _lhs(std::move(lhs)), _rhs(std::move(rhs)), _token(token) {}

Node::Node(const NodeKind &kind, unique_ptr<Node> &&lhs, Token *token)
	: _kind(kind), _lhs(std::move(lhs)), _token(token) {}

Node::Node(const int64_t &val, Token *token) : _kind(NodeKind::ND_NUM), _val(val), _token(token) {}

Node::Node(const Object *var, Token *token) : _kind(NodeKind::ND_VAR), _var(var), _token(token) {}

/**
 * @brief 型キャストに対応するノードを作成する
 *
 * @param expr 型キャストの対象とする式
 * @param ty キャスト後の型
 * @return std::unique_ptr<Node> 型キャストに対応するノード
 */
std::unique_ptr<Node> Node::new_cast(std::unique_ptr<Node> &&expr, std::shared_ptr<Type> &ty)
{
	/* 型を確定 */
	Type::add_type(expr.get());

	auto node = std::make_unique<Node>(NodeKind::ND_CAST, std::move(expr), expr->_token);
	node->_ty = std::make_shared<Type>(*ty);
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
	if (lhs->_ty->is_integer() && rhs->_ty->is_integer())
	{
		return std::make_unique<Node>(NodeKind::ND_ADD, std::move(lhs), std::move(rhs), token);
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
	rhs = std::make_unique<Node>(NodeKind::ND_MUL, std::move(rhs), new_long(lhs->_ty->_base->_size, token), token);
	return std::make_unique<Node>(NodeKind::ND_ADD, std::move(lhs), std::move(rhs), token);
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
	if (lhs->_ty->is_integer() && rhs->_ty->is_integer())
	{
		return std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), token);
	}

	/* ptr - 数 */
	if (lhs->_ty->_base && !rhs->_ty->_base)
	{
		rhs = std::make_unique<Node>(NodeKind::ND_MUL, std::move(rhs), new_long(lhs->_ty->_base->_size, token), token);
		Type::add_type(rhs.get());
		auto node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), token);
		return node;
	}

	/* ptr - ptr */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		int sz = lhs->_ty->_base->_size;
		auto node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), token);
		node->_ty = Type::INT_BASE;
		return std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::make_unique<Node>(sz, token), token);
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
std::string Node::new_unique_name()
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
Object *Node::new_anonymous_gvar(std::shared_ptr<Type> &&ty)
{
	return Object::new_gvar(new_unique_name(), std::move(ty));
}

/**
 * @brief 文字列リテラルとしてstrをもつグローバル変数を生成する。
 *
 * @param str 文字列リテラル
 * @return 生成したグローバル変数オブジェクトへのポインタ
 */
Object *Node::new_string_literal(const std::string &str)
{
	/* 文字列リテラルの型はchar型配列で長さは文字数+'\0'終端 */
	auto ty = Type::array_of(Type::CHAR_BASE, str.size() + 1);

	/* 仮名を使ってオブジェクトを生成 */
	auto obj = new_anonymous_gvar(std::move(ty));

	/* init_dataに文字列を入れて'\0'終端を追加 */
	obj->_init_data = str;
	obj->_init_data.push_back('\0');
	obj->is_str_literal = true;
	return obj;
}

/**
 * @brief long型の数値ノードを作成する
 *
 * @param val 数値
 * @param token 対応するトークン
 * @return long型の数値ノード
 */
std::unique_ptr<Node> Node::new_long(const int64_t &val, Token *token)
{
	auto node = std::make_unique<Node>(val, token);
	node->_ty = Type::LONG_BASE;
	return node;
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
	return new_cast(new_add(to_assign(new_add(std::move(node),
											  std::make_unique<Node>(addend, token),
											  token)),
							std::make_unique<Node>(-addend, token),
							token),
					ty);
}

/**
 * @brief トークン・リストを構文解析して関数ごとにASTを構築する
 *
 * @param token トークン・リストの先頭
 * @return 構文解析結果
 * @details program = (typedef | function-definition | global-variable)*
 */
unique_ptr<Object> Node::parse(Token *token)
{
	/* トークンリストを最後まで辿る*/
	while (TokenKind::TK_EOF != token->_kind)
	{
		VarAttr attr = {};
		auto base = declspec(&token, token, &attr);

		/* typedef */
		if (attr.is_typedef)
		{
			token = parse_typedef(token, base);
			continue;
		}

		/* 関数 */
		if (is_function(token))
		{
			token = function_definition(token, std::move(base), &attr);
			continue;
		}

		/* グローバル変数 */
		token = global_variable(token, std::move(base));
	}

	return std::move(Object::globals);
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
 * statement = "return" expression ";" @n
 * 			 | "if" "(" expression ")" statement ("else" statement)? @n
 * 			 | "for" "(" expression-statement expression? ";" expression? ")" statement @n
 * 			 | "while" "(" expression ")" statement @n
 * 			 | "goto" ident ";" @n
 * 			 |  ident ":" statement @n
 * 			 | "{" compound-statement @n
 * 			 | expression-statement
 */
unique_ptr<Node> Node::statement(Token **next_token, Token *current_token)
{
	/* return */
	if (current_token->is_equal("return"))
	{
		/* "return"の次はexpresionがくる */
		auto node = std::make_unique<Node>(NodeKind::ND_RETURN, current_token);
		auto expr = expression(&current_token, current_token->_next.get());

		/* 最後は';'で終わるはず */
		*next_token = Token::skip(current_token, ";");

		/* 式の型を決定する */
		Type::add_type(expr.get());

		/* return先の型にキャストする */
		node->_lhs = new_cast(std::move(expr), current_function->_ty->_return_ty);
		return node;
	}

	/* if */
	if (current_token->is_equal("if"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_IF, current_token);

		/* ifの次は'('がくる */
		current_token = Token::skip(current_token->_next.get(), "(");

		/* 条件文 */
		node->_condition = expression(&current_token, current_token);

		/* 条件文のは')'がくる */
		current_token = Token::skip(current_token, ")");
		node->_then = statement(&current_token, current_token);

		/* else節が存在する */
		if (current_token->is_equal("else"))
		{
			node->_else = statement(&current_token, current_token->_next.get());
		}
		*next_token = current_token;
		return node;
	}

	/* for */
	if (current_token->is_equal("for"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_FOR, current_token);

		/* forの次は'('がくる */
		current_token = Token::skip(current_token->_next.get(), "(");

		/* for文のブロックスコープに入る */
		Object::enter_scope();

		/* 現在のラベルを保存 */
		auto brk = brk_label;
		/* forを抜けるラベルを設定 */
		brk_label = new_unique_name();
		node->_brk_label = brk_label;

		/* 型指定子がきたら変数が定義されている */
		if (Token::is_typename(current_token))
		{
			auto base = declspec(&current_token, current_token, nullptr);
			node->_init = declaration(&current_token, current_token, base);
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
		current_token = Token::skip(current_token, ";");

		/* 次のトークンが')'でなければ加算処理が存在する */
		if (!current_token->is_equal(")"))
		{
			node->_inc = expression(&current_token, current_token);
		}
		current_token = Token::skip(current_token, ")");
		/* forの中の処理 */
		node->_then = statement(next_token, current_token);
		/* for文のブロックスコープを抜ける */
		Object::leave_scope();

		/* 保存していた値を代入してfor文に入る前のラベルに戻す */
		brk_label = brk;
		return node;
	}

	/* while */
	if (current_token->is_equal("while"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_FOR, current_token);

		/* whileの次は'('がくる */
		current_token = Token::skip(current_token->_next.get(), "(");
		node->_condition = expression(&current_token, current_token);

		/* 条件文のは')'がくる */
		current_token = Token::skip(current_token, ")");

		/* while文を抜けるラベルを設定 */
		auto brk = brk_label;
		brk_label = new_unique_name();
		node->_brk_label = brk_label;

		/* while文の中身 */
		node->_then = statement(next_token, current_token);

		/* ラベルを設定しなおす */
		brk_label = brk;
		return node;
	}

	/* goto */
	if (current_token->is_equal("goto"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_GOTO, current_token);
		/* ソース内に書かれている名前 */
		node->_label = current_token->_next->_str;
		/* リストの先頭に追加 */
		node->_goto_next = gotos;
		gotos = node.get();

		*next_token = Token::skip(current_token->_next->_next.get(), ";");
		return node;
	}

	/* break */
	if (current_token->is_equal("break"))
	{
		if (brk_label.empty())
		{
			error_token("無効なbreak文です", current_token);
		}
		auto node = std::make_unique<Node>(NodeKind::ND_GOTO, current_token);
		node->_unique_label = brk_label;
		*next_token = Token::skip(current_token->_next.get(), ";");
		return node;
	}

	/* ラベル */
	if (TokenKind::TK_IDENT == current_token->_kind && current_token->_next->is_equal(":"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_LABEL, current_token);
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

	auto node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token);

	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* ブロックスコープに入る */
	Object::enter_scope();

	/* '}'が出てくるまでstatementをパースする */
	while (!current_token->is_equal("}"))
	{
		/* 変数宣言、定義 */
		if (Token::is_typename(current_token) && !current_token->_next->is_equal(":"))
		{
			/* 型指定子を読み取る */
			VarAttr attr = {};
			auto base = declspec(&current_token, current_token, &attr);

			/* typedefの場合 */
			if (attr.is_typedef)
			{
				current_token = parse_typedef(current_token, base);
				continue;
			}
			/* それ以外の場合は変数の定義または宣言 */
			cur->_next = declaration(&current_token, current_token, base);
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
	node->_body = std::move(head->_next);
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

	auto parameters = ty->_params;
	/* 新しい関数を生成する。 */
	auto fn = Object::new_gvar(ty->_token->_str, std::move(ty));
	/* 関数であるフラグをセット */
	fn->is_function = true;

	/* 定義か宣言か、後ろに";"がくるなら宣言 */
	fn->is_definition = !Token::consume(&token, token, ";");

	/* staticかどうか */
	fn->is_static = attr->is_static;

	/* 宣言であるなら現在のトークンを返して抜ける */
	if (!fn->is_definition)
	{
		return token;
	}

	current_function = fn;

	/* 関数のブロックスコープに入る */
	Object::enter_scope();

	/* 引数をローカル変数として作成 */
	Object::create_params_lvars(std::move(parameters));
	fn->_params = std::move(Object::locals);

	/* 引数の次は"{"がくる */
	token = Token::skip(token, "{");

	/* 関数の中身を読み取る */
	fn->_body = compound_statement(&token, token);

	/* ローカル変数をセット */
	fn->_locals = std::move(Object::locals);

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
unique_ptr<Node> Node::declaration(Token **next_token, Token *current_token, shared_ptr<Type> &base)
{
	/* ノードリストの先頭としてダミーのノードを作成 */
	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* 初回かどうか */
	bool first = true;

	/* ";"が出てくるまで読み取りを続ける */
	while (!current_token->is_equal(";"))
	{
		/* 2個目以降の宣言には",""区切りが必要 */
		if (!first)
		{
			current_token = Token::skip(current_token, ",");
		}
		/* 初回フラグを下げる */
		first = false;

		/* 変数の最終的な型を決定 */
		auto ty = declarator(&current_token, current_token, base);

		if (ty->_size < 0)
		{
			error_token("変数が不完全な型で宣言されています", current_token);
		}

		/* 変数がvoid型で宣言されていたらエラー */
		if (TypeKind::TY_VOID == ty->_kind)
		{
			error_token("変数がvoid型で宣言されています", current_token);
		}

		const auto var = Object::new_lvar(ty->_token->_str, std::move(ty));

		/* 宣言の後に初期化式がない場合は次のループへ */
		if (!current_token->is_equal("="))
		{
			continue;
		};

		/* 変数を表すノードを生成 */
		auto lhs = std::make_unique<Node>(var, var->_ty->_token);
		/* 変数の初期化値を表すノードを生成 */
		auto rhs = assign(&current_token, current_token->_next.get());
		/* 初期化を代入式として表すノードを生成 */
		auto node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(lhs), std::move(rhs), current_token);
		/* ノードリストの末尾に単文ノードとして追加 */
		cur->_next = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, std::move(node), current_token);
		/* ノードリストの末尾を更新 */
		cur = cur->_next.get();
	}
	auto node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token);
	/* ヘッダの次のノード以降を切り離してnodeのbodyに繋ぐ */
	node->_body = std::move(head->_next);
	*next_token = current_token->_next.get();
	return node;
}

/**
 * @brief 関数の引数を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 宣言されている型
 * @return 引数の情報を含む関数の型
 * @details 以下のEBNF規則に従う。 @n
 * function-parameters = (parameters ("," parameters)*)? ")" @n
 * parameters = declspec declarator
 */
shared_ptr<Type> Node::function_parameters(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	auto head = std::make_unique_for_overwrite<Type>();
	auto cur = head.get();

	/* ")"が出てくるまで読み取りを続ける */
	while (!current_token->is_equal(")"))
	{
		if (head.get() != cur)
		{
			/* 2個目以降の引数では","区切りが必要 */
			current_token = Token::skip(current_token, ",");
		}
		/* 引数の型を決定 */
		auto ty2 = declspec(&current_token, current_token, nullptr);
		ty2 = declarator(&current_token, current_token, ty2);

		/* 関数の引数では、T型の配列はT型へのポインタとして解釈する。例： *argv[] は **argv に変換される */
		if (TypeKind::TY_ARRAY == ty2->_kind)
		{
			auto token = ty2->_token;
			ty2 = Type::pointer_to(ty2->_base);
			ty2->_token = token;
		}
		cur->_next = std::make_shared<Type>(*ty2);
		cur = cur->_next.get();
	}

	ty = Type::func_type(ty);
	ty->_params = std::move(head->_next);
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
 * 次のEBNF規則に従う。 @n array-dimensions = num? "]" type-suffix
 */
shared_ptr<Type> Node::array_dimensions(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	/* 要素数の指定がない場合は長さを-1にする */
	if (current_token->is_equal("]"))
	{
		ty = type_suffix(next_token, current_token->_next.get(), std::move(ty));
		return Type::array_of(ty, -1);
	}

	int sz = current_token->get_number();
	current_token = Token::skip(current_token->_next.get(), "]");
	ty = type_suffix(next_token, current_token, std::move(ty));
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
		return function_parameters(next_token, current_token->_next.get(), std::move(ty));
	}

	/* 識別子名の後に"["があれば配列 */
	if (current_token->is_equal("["))
	{
		return array_dimensions(next_token, current_token->_next.get(), std::move(ty));
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
 * 下記のEBNF規則に従う。 @n declarator = "*"* ("(" identifier ")" | "(" declarator ")" | identifier) type-suffix
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 変数の型の基準
 * @return 変数の型
 */
shared_ptr<Type> Node::declarator(Token **next_token, Token *current_token, shared_ptr<Type> ty)
{
	/* "*"の数だけ直前の型へのポインタになる */
	while (Token::consume(&current_token, current_token, "*"))
	{
		ty = Type::pointer_to(ty);
	}

	/* ネストした型の場合、外側を先に評価する */
	if (current_token->is_equal("("))
	{
		auto start = current_token;
		auto dummy = std::make_shared<Type>();
		/* ネスト部分を飛ばす */
		declarator(&current_token, start->_next.get(), dummy);
		current_token = Token::skip(current_token, ")");
		/* ネストの外側の型を決める */
		ty = type_suffix(next_token, current_token, std::move(ty));
		/* 外側の型をベースの型としてネスト部分の型を決定する */
		return declarator(&current_token, start->_next.get(), ty);
	}

	/* トークンの種類が識別子でないときエラー */
	if (TokenKind::TK_IDENT != current_token->_kind)
	{
		error_token("識別子ではありません", current_token);
	}

	/* 関数か変数か */
	ty = type_suffix(next_token, current_token->_next.get(), std::move(ty));

	/* 参照トークンを設定 */
	ty->_token = current_token;

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

	current_token = Token::skip(current_token, "{");

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
		mem->_offset = offset;
		offset += mem->_ty->_size;
		/* アライメントの基数はメンバの基数のうち最大値に合わせる */
		if (ty->_align < mem->_ty->_align)
		{
			ty->_align = mem->_ty->_align;
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
		if (ty->_align < mem->_ty->_align)
		{
			ty->_align = mem->_ty->_align;
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
	auto head = std::make_shared<Member>();
	auto cur = head.get();

	/* '}'が出てくるまで読み込み続ける */
	while (!current_token->is_equal("}"))
	{
		auto base = declspec(&current_token, current_token, nullptr);
		bool first = true;

		/* ';'が出てくるまで読み込み続ける */
		while (!Token::consume(&current_token, current_token, ";"))
		{
			if (!first)
			{
				/* 2個目以降はカンマ区切りが必要 */
				current_token = Token::skip(current_token, ",");
			}
			first = false;
			auto mem = std::make_shared<Member>();
			/* メンバ名部分を読み込む */
			mem->_ty = declarator(&current_token, current_token, base);
			mem->_token = mem->_ty->_token;
			/* リストの先頭に繋ぐ */
			cur->_next = std::move(mem);
			cur = cur->_next.get();
		}
	}
	*next_token = current_token->_next.get();
	ty->_members = std::move(head->_next);
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
	auto node = std::make_unique<Node>(NodeKind::ND_MEMBER, std::move(lhs), token);
	node->_member = std::move(mem);
	return node;
}

/**
 * @brief 変数宣言の型指定子部分を読み取る
 *
 * @details
 * 下記のEBNF規則に従う。 @n
 * declspec =  ("void" | "_BOOL" | "int" | "short" | "long" | "char" @n
 * 				| "typedef" | "static" @n
 * 				| struct-decl | union-decl | typedef-name @n
 * 				| enum-specifier)+ @n
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
	constexpr int OTHER = 1 << 12;

	static const std::unordered_map<std::string, int> str_to_type = {
		{"void", VOID},
		{"_Bool", BOOL},
		{"char", CHAR},
		{"short", SHORT},
		{"int", INT},
		{"long", LONG},
	};

	static const std::unordered_map<int, shared_ptr<Type>> int_to_type = {
		{VOID, Type::VOID_BASE},
		{BOOL, Type::BOOL_BASE},
		{CHAR, Type::CHAR_BASE},
		{SHORT, Type::SHORT_BASE},
		{SHORT + INT, Type::SHORT_BASE},
		{INT, Type::INT_BASE},
		{LONG, Type::LONG_BASE},
		{LONG + INT, Type::LONG_BASE},
		{LONG + LONG, Type::LONG_BASE},
		{LONG + LONG + INT, Type::LONG_BASE},
	};

	/* それぞれの型名の出現回数を表すカウンタ。
	 * 例えばビット0, 1は「void」という型名の出現回数を表す。
	 */
	int counter = 0;
	auto ty = Type::INT_BASE;

	while (Token::is_typename(current_token))
	{
		/* ストレージクラス指定子 */
		if (current_token->is_equal("typedef") || current_token->is_equal("static"))
		{
			/* ストレージクラス指定子が使用できない箇所である場合エラー 例 function(int i, typedef int INT) */
			if (!attr)
			{
				error_token("ここではストレージクラス指定子は使用できません", current_token);
			}
			if (current_token->is_equal("typedef"))
			{
				attr->is_typedef = true;
			}
			else
			{
				attr->is_static = true;
			}
			/* typedefとstaticが同時に指定されている場合 */
			if (attr->is_static + attr->is_typedef > 1)
			{
				error_token("typedefとstaticは同時に指定できません", current_token);
			}
			current_token = current_token->_next.get();
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

		auto it = str_to_type.find(current_token->_str);
		if (it != str_to_type.end())
		{
			counter += it->second;
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
 * enum-list = ident ("=" num)? ("," ident ("=" num)?)*
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

	current_token = Token::skip(current_token, "{");

	/* 列挙型のリストを読む */
	bool first = true;
	int val = 0;

	/* "}"が出てくるまで読み続ける */
	while (!current_token->is_equal("}"))
	{
		if (!first)
		{
			/* 2個目以降では","区切りが必要 */
			current_token = Token::skip(current_token, ",");
		}
		first = false;
		/* 列挙型の変数名 */
		auto name = current_token->_str;
		current_token = current_token->_next.get();

		/* 数値の指定がある場合 */
		if (current_token->is_equal("="))
		{
			val = current_token->_next->get_number();
			current_token = current_token->_next->_next.get();
		}

		/* スコープに登録する。次の変数には+1インクリメントされた数値が対応する */
		auto sc = Object::push_scope(name);
		sc->enum_ty = ty;
		sc->enum_val = val++;
	}

	*next_token = current_token->_next.get();

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
		return std::make_unique<Node>(NodeKind::ND_BLOCK, current_token);
	}
	auto node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, current_token);
	node->_lhs = expression(&current_token, current_token);

	/* expression-statementは';'で終わるはず */
	*next_token = Token::skip(current_token, ";");
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
		return std::make_unique<Node>(NodeKind::ND_COMMA, std::move(node), expression(next_token, current_token->_next.get()), current_token);
	}
	*next_token = current_token;
	return node;
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
	auto expr1 = std::make_unique<Node>(NodeKind::ND_ASSIGN,
										std::make_unique<Node>(var, token),
										std::make_unique<Node>(NodeKind::ND_ADDR, std::move(binary->_lhs), token),
										token);

	/* *tmp = *tmp op B */
	auto expr2 = std::make_unique<Node>(NodeKind::ND_ASSIGN,
										std::make_unique<Node>(NodeKind::ND_DEREF, std::make_unique<Node>(var, token), token),
										std::make_unique<Node>(std::move(binary->_kind),
															   std::make_unique<Node>(NodeKind::ND_DEREF, std::make_unique<Node>(var, token), token),
															   std::move(binary->_rhs),
															   token),
										token);

	return std::make_unique<Node>(NodeKind::ND_COMMA, std::move(expr1), std::move(expr2), token);
}

/**
 * @brief 代入式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * assign = logor (assign-op assign)? @n
 * assign-op = "=" | "+=" | "-=" | "*=" | "/=" | "%="
 */
unique_ptr<Node> Node::assign(Token **next_token, Token *current_token)
{
	static const std::unordered_map<std::string, NodeKind> str_to_op = {
		{"*=", NodeKind::ND_MUL},
		{"/=", NodeKind::ND_DIV},
		{"%=", NodeKind::ND_MOD},
		{"&=", NodeKind::ND_BITAND},
		{"|=", NodeKind::ND_BITOR},
		{"^=", NodeKind::ND_BITXOR}};

	auto node = log_or(&current_token, current_token);

	if (current_token->is_equal("="))
	{
		return std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), assign(next_token, current_token->_next.get()), current_token);
	}
	if (current_token->is_equal("+="))
	{
		return to_assign(new_add(std::move(node), assign(next_token, current_token->_next.get()), current_token));
	}

	if (current_token->is_equal("-="))
	{
		return to_assign(new_sub(std::move(node), assign(next_token, current_token->_next.get()), current_token));
	}

	auto it = str_to_op.find(current_token->_str);
	if (it != str_to_op.end())
	{
		return to_assign(std::make_unique<Node>(it->second, std::move(node), assign(next_token, current_token->_next.get()), current_token));
	}

	*next_token = current_token;
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
std::unique_ptr<Node> Node::log_or(Token **next_token, Token *current_token)
{
	auto node = log_and(&current_token, current_token);

	/* "||"が出てくる限り読み込み続ける */
	while (current_token->is_equal("||"))
	{
		auto start = current_token;
		node = std::make_unique<Node>(NodeKind::ND_LOGOR, std::move(node), log_and(&current_token, current_token->_next.get()), start);
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
std::unique_ptr<Node> Node::log_and(Token **next_token, Token *current_token)
{
	auto node = bit_or(&current_token, current_token);

	/* "||"が出てくる限り読み込み続ける */
	while (current_token->is_equal("&&"))
	{
		auto start = current_token;
		node = std::make_unique<Node>(NodeKind::ND_LOGAND, std::move(node), bit_or(&current_token, current_token->_next.get()), start);
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
		node = std::make_unique<Node>(NodeKind::ND_BITOR, std::move(node), bit_xor(&current_token, current_token->_next.get()), start);
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
		node = std::make_unique<Node>(NodeKind::ND_BITXOR, std::move(node), bit_and(&current_token, current_token->_next.get()), start);
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
		node = std::make_unique<Node>(NodeKind::ND_BITAND, std::move(node), equality(&current_token, current_token->_next.get()), start);
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
			node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), relational(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("!="))
		{
			node = std::make_unique<Node>(NodeKind::ND_NE, std::move(node), relational(&current_token, current_token->_next.get()), start);
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
 * @details 下記のEBNF規則に従う。 @n relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 */
unique_ptr<Node> Node::relational(Token **next_token, Token *current_token)
{
	auto node = add(&current_token, current_token);

	while (true)
	{
		auto start = current_token;

		if (current_token->is_equal("<"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), add(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("<="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), add(&current_token, current_token->_next.get()), start);
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (current_token->is_equal(">"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, add(&current_token, current_token->_next.get()), std::move(node), start);
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (current_token->is_equal(">="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, add(&current_token, current_token->_next.get()), std::move(node), start);
			continue;
		}

		/* 比較演算子が出てこなくなったらループを抜ける */
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
			node = new_add(std::move(node), mul(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("-"))
		{
			node = new_sub(std::move(node), mul(&current_token, current_token->_next.get()), start);
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
	if (current_token->is_equal("(") && Token::is_typename(current_token->_next.get()))
	{
		auto start = current_token;

		/* キャスト先の型を読み取る */
		auto ty = type_name(&current_token, current_token->_next.get());
		current_token = Token::skip(current_token, ")");

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
			node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), cast(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("/"))
		{
			node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), cast(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("%"))
		{
			node = std::make_unique<Node>(NodeKind::ND_MOD, std::move(node), cast(&current_token, current_token->_next.get()), start);
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
	static const std::unordered_map<std::string, NodeKind> str_to_type = {
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
		return std::make_unique<Node>(NodeKind::ND_NEG, cast(next_token, current_token->_next.get()), current_token);
	}

	auto it = str_to_type.find(current_token->_str);
	if (it != str_to_type.end())
	{
		return std::make_unique<Node>(it->second, cast(next_token, current_token->_next.get()), current_token);
	}

	/* ++iをi+=1と読み替える */
	if (current_token->is_equal("++"))
	{
		return to_assign(new_add(unary(next_token, current_token->_next.get()), std::make_unique<Node>(1, current_token), current_token));
	}

	/* --iをi+=1と読み替える */
	if (current_token->is_equal("--"))
	{
		return to_assign(new_sub(unary(next_token, current_token->_next.get()), std::make_unique<Node>(1, current_token), current_token));
	}

	return postfix(next_token, current_token);
}

/**
 * @brief 配列の添え字[]または構造体のメンバを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n postfix = primary ("[" expression "]" | "." identifier | "->" identifier | "++" | "--")*
 */
unique_ptr<Node> Node::postfix(Token **next_token, Token *current_token)
{
	/* 単項を読む */
	auto node = primary(&current_token, current_token);

	for (;;)
	{
		/* 配列 */
		if (current_token->is_equal("["))
		{
			/* x[y] を *(x+y) に置き換える */
			auto start = current_token;
			auto idx = expression(&current_token, current_token->_next.get());
			current_token = Token::skip(current_token, "]");
			node = std::make_unique<Node>(NodeKind::ND_DEREF, new_add(std::move(node), std::move(idx), start), start);
			continue;
		}
		/* 構造体 */
		if (current_token->is_equal("."))
		{
			node = struct_ref(std::move(node), current_token->_next.get());
			current_token = current_token->_next->_next.get();
			continue;
		}
		/* 構造体のポインタ */
		if (current_token->is_equal("->"))
		{
			/* x->y を(*x).y を読み替える */
			node = std::make_unique<Node>(NodeKind::ND_DEREF, std::move(node), current_token);
			node = struct_ref(std::move(node), current_token->_next.get());
			current_token = current_token->_next->_next.get();
			continue;
		}
		/* 後置インクリメント */
		if (current_token->is_equal("++"))
		{
			node = new_inc_dec(std::move(node), current_token, 1);
			current_token = current_token->_next.get();
			continue;
		}
		/* 後置デクリメント */
		if (current_token->is_equal("--"))
		{
			node = new_inc_dec(std::move(node), current_token, -1);
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
 * primary = "(" "{" statement+ "}" ")" | "(" expression ")" | "sizeof" unary | "sizeof" "(" type-name ")" | identifier args? | str | num @n
 * args = "(" ")"
 */
unique_ptr<Node> Node::primary(Token **next_token, Token *current_token)
{
	/* トークンが"(" "{"ならステートメント式 */
	if (current_token->is_equal("(") && current_token->_next->is_equal("{"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_STMT_EXPR, current_token);
		/* ブロックを読み取って、ブロック内の処理を新しいnodeのbodyに付け替える */
		auto stmt = compound_statement(&current_token, current_token->_next->_next.get());
		node->_body = std::move(stmt->_body);
		/* ブロックの後は') */
		*next_token = Token::skip(current_token, ")");
		return node;
	}

	/* トークンが"("なら、"(" expression ")"のはず */
	if (current_token->is_equal("("))
	{
		auto node = expression(&current_token, current_token->_next.get());
		*next_token = Token::skip(current_token, ")");
		return node;
	}

	/* 文字列リテラル */
	if (TokenKind::TK_STR == current_token->_kind)
	{
		auto var = new_string_literal(current_token->_str);
		*next_token = current_token->_next.get();
		return std::make_unique<Node>(var, current_token);
	}

	/* sizeof演算子（対象が型そのもの） */
	if (current_token->is_equal("sizeof") &&
		current_token->_next->is_equal("(") &&
		Token::is_typename(current_token->_next->_next.get()))
	{
		auto start = current_token;
		/* sizeof演算子の対象の型情報を読む */
		auto ty = type_name(&current_token, current_token->_next->_next.get());
		*next_token = Token::skip(current_token, ")");
		return std::make_unique<Node>(ty->_size, start);
	}

	/* sizeof演算子（対象が式） */
	if (current_token->is_equal("sizeof"))
	{
		/* sizeofの対象を評価 */
		auto node = unary(next_token, current_token->_next.get());
		/* sizeofの対象の型を決定 */
		Type::add_type(node.get());
		/* 型のサイズの数値ノードを返す */
		return std::make_unique<Node>(node->_ty->_size, current_token);
	}

	/* トークンが識別子の場合 */
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		/* 識別子の後に()がついていたら関数呼び出し */
		if (current_token->_next->is_equal("("))
		{
			return function_call(next_token, current_token);
		}

		/* それ以外なら普通の変数か列挙型の定数 */
		const auto sc = Object::find_var(current_token);

		/* 変数が宣言されていない場合はエラー */
		if (!sc || (!sc->_var && !sc->enum_ty))
		{
			error_token("未宣言の変数です", current_token);
		}
		unique_ptr<Node> node;
		/* 変数 */
		if (sc->_var)
		{
			node = std::make_unique<Node>(sc->_var, current_token);
		}
		/* 列挙型の定数 */
		else
		{
			node = std::make_unique<Node>(sc->enum_val, current_token);
		}
		*next_token = current_token->_next.get();
		return node;
	}

	/* トークンが数値の場合 */
	if (TokenKind::TK_NUM == current_token->_kind)
	{
		auto node = std::make_unique<Node>(current_token->_value, current_token);
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
	while (!Token::consume(&token, token, ";"))
	{
		/* 2個目以降では","区切りが必要 */
		if (!first)
		{
			token = Token::skip(token, ",");
		}
		first = false;

		auto ty = declarator(&token, token, base);
		Object::push_scope(ty->_token->_str)->type_def = ty;
	}
	return token;
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
shared_ptr<Type> Node::abstract_declarator(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	/* "*"の数だけその前までの型に対するポインター */
	while (current_token->is_equal("*"))
	{
		ty = Type::pointer_to(ty);
		current_token = current_token->_next.get();
	}

	/* ネストしている型 */
	if (current_token->is_equal("("))
	{
		auto start = current_token;
		auto dummy = std::make_shared<Type>();
		/* ネストの中を飛ばす */
		abstract_declarator(&current_token, start->_next.get(), std::move(dummy));
		current_token = Token::skip(current_token, ")");
		/* ネストの外側の型を読み込む */
		ty = type_suffix(next_token, current_token, std::move(ty));
		/* ネストの外側の型をベースとして内側の型を読む */
		return abstract_declarator(&current_token, start->_next.get(), std::move(ty));
	}
	return type_suffix(next_token, current_token, std::move(ty));
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
	return abstract_declarator(next_token, current_token, std::move(ty));
}

/**
 * @brief 関数呼び出し
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * function_call = identifier "(" (assign ("," assign)*)? ")"
 */
unique_ptr<Node> Node::function_call(Token **next_token, Token *current_token)
{
	auto start = current_token;

	/* 名前を検索 */
	auto sc = Object::find_var(start);
	/* 未宣言または関数として定義されていない場合はエラー */
	if (!sc)
	{
		error_token("未宣言の関数です", start);
	}
	if (!sc->_var || sc->_var->_ty->_kind != TypeKind::TY_FUNC)
	{
		error_token("関数以外として宣言されています", start);
	}
	/* 関数の型 */
	auto ty = sc->_var->_ty;
	/* 引数の型 */
	auto param_ty = ty->_params;

	/* 関数の引数を読む、引数は識別子の次の次のトークンから */
	current_token = current_token->_next->_next.get();

	/* ノードリストの先頭としてダミーのノードを生成 */
	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* ')'が出てくるまで読み込み続ける */
	while (!current_token->is_equal(")"))
	{
		if (head.get() != cur)
		{
			/* 2個目以降の引数には区切りとして","が必要 */
			current_token = Token::skip(current_token, ",");
		}
		/* 引数を読み取る */
		auto arg = assign(&current_token, current_token);
		/* 引数の型を決定 */
		Type::add_type(arg.get());

		if (param_ty)
		{
			if (TypeKind::TY_STRUCT == param_ty->_kind || TypeKind::TY_UNION == param_ty->_kind)
			{
				error_token("構造体、共用体の値渡しはサポートしていません", arg->_token);
			}
			/* 実引数の型を宣言されている仮引数の型でキャストする */
			arg = new_cast(std::move(arg), param_ty);
			param_ty = param_ty->_next;
		}
		cur->_next = std::move(arg);
		cur = cur->_next.get();
	}

	/* 関数呼び出しノードを作成 */
	auto node = std::make_unique<Node>(NodeKind::ND_FUNCALL, start);
	/* 関数の名前をセット */
	node->_func_name = std::move(start->_str);
	/* headの次のノード以降を切り離し返り値用のnodeのargsに繋ぐ */
	node->_args = std::move(head->_next);
	/* 関数の型をセット */
	node->_func_ty = ty;
	/* 戻り値の型をセット */
	node->_ty = ty->_return_ty;

	/* 最後は")"" */
	*next_token = Token::skip(current_token, ")");

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
	/* 識別子にたどり着くまで辿り続ける */
	while (token->is_equal("*"))
	{
		token = token->_next.get();
	}

	/* 識別子が来なければエラー */
	if (TokenKind::TK_IDENT != token->_kind)
	{
		error_token("識別子ではありません", token);
	}

	/* 識別子の次に来るのが"("なら関数 */
	if (token->_next->is_equal("("))
	{
		return true;
	}

	/* そうでないなら変数 */
	return false;
}

/**
 * @brief グローバル変数を読み取る
 *
 * @param token 読み込むトークン
 * @param base 型宣言の前半部分から読み取れた型
 * @return 次のトークン
 */
Token *Node::global_variable(Token *token, shared_ptr<Type> &&base)
{
	/* 1個めの変数であるか */
	bool first = true;

	/* ;が現れるまで読み込みを続ける */
	while (!Token::consume(&token, token, ";"))
	{
		if (!first)
		{
			/* 2個目以降の変数定義には","区切りが必要 */
			token = Token::skip(token, ",");
		}
		/* 初回フラグを下す */
		first = false;

		/* 最終的な型を決定する */
		auto ty = declarator(&token, token, base);
		Object::new_gvar(ty->_token->_str, std::move(ty));
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