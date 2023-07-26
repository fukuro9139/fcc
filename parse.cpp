/**
 * @file parse.cpp
 * @author K.Fukunaga
 * @brief
 * C言語の再帰下降構文解析を行うパーサーの定義
 *
 * このファイルのほとんどの関数の名前は、入力トークン・リストから読み取る対象に対応している。 @n
 * 例えば、statement() ははトークン・リストからステートメントを読み取ってステートメントに対応する @n
 * 抽象構文木（abstract syntax tree、AST）ノードを返す。 @n
 * 各関数はASTノードと入力トークンの残りの部分を返すように設計してある。 @n
 * C言語は複数の戻り値をサポートしていないので、残りのトークンはポインタ引数を通じて呼び出し元に返される。 @n
 *
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "parse.hpp"

using std::shared_ptr;
using std::unique_ptr;

/** 変数や関数オブジェクトのリスト。パース中に生成される全てのローカル変数はこのリストに連結される。 */
static unique_ptr<Object> locals = nullptr;

/** 現在処理中の関数*/
static Function *current_func = nullptr;

/****************/
/* Object Class */
/****************/

/**
 * @brief 新しい変数を生成してlocalsの先頭に追加する。
 *
 * @param  オブジェクトの型
 * @return 生成した変数へのポインタ
 */
const Object *Object::new_lvar(std::shared_ptr<Type> &&ty)
{
	locals = std::make_unique<Object>(std::move(ty->_name), std::move(locals), std::move(ty));
	return locals.get();
}

/**
 * @brief 変数を名前で検索する。見つからなかった場合はNULLを返す。
 *
 * @param token 検索対象のトークン
 * @return 既出の変数であればその変数オブジェクトへのポインタ
 */
const Object *Object::find_var(const unique_ptr<Token> &token)
{
	/* 引数 */
	for (const Object *var = current_func->_params.get(); var; var = var->_next.get())
	{
		if (var->_name.size() == token->_str.size() && std::equal(var->_name.begin(), var->_name.end(), token->_str.begin()))
		{
			return var;
		}
	}
	/* ローカル変数 */
	for (const Object *var = locals.get(); var; var = var->_next.get())
	{
		if (var->_name.size() == token->_str.size() && std::equal(var->_name.begin(), var->_name.end(), token->_str.begin()))
		{
			return var;
		}
	}
	return nullptr;
}

/******************/
/* Function Class */
/******************/

/** @brief 関数に必要なスタックサイズを計算してstack_sizeにセットする。
 *
 * @param prog スタックサイズをセットする関数
 */
void Function::assign_lvar_offsets(const std::unique_ptr<Function> &prog)
{
	for (Function *fn = prog.get(); fn; fn = fn->_next.get())
	{
		int offset = 0;
		/* 引数 */
		for (Object *var = fn->_params.get(); var; var = var->_next.get())
		{
			offset += 8;
			var->_offset = offset;
		}
		/* ローカル変数 */
		for (Object *var = fn->_locals.get(); var; var = var->_next.get())
		{
			offset += 8;
			var->_offset = offset;
		}
		fn->_stack_size = align_to(std::move(offset), 16);
	}
}

/**
 * @brief 引数をローカル変数としてローカル変数のリストに繋ぐ
 *
 * @param param 引数のリスト
 */
void Function::create_params_lvars(shared_ptr<Type> &&param)
{
	if (param)
	{
		create_params_lvars(std::move(param->_next));
		Object::new_lvar(std::move(param));
	}
}

/**************/
/* Node Class */
/**************/

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
 * 			 | "{" compound-statement @n
 * 			 | expression-statement
 */
unique_ptr<Node> Node::statement(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	/* return */
	if (current_token->is_equal("return"))
	{
		/* "return"の次はexpresionがくる */
		auto node = std::make_unique<Node>(NodeKind::ND_RETURN, current_token->_location);
		node->_lhs = expression(current_token, std::move(current_token->_next));

		/* 最後は';'で終わるはず */
		next_token = Token::skip(std::move(current_token), ";");
		return node;
	}

	/* if */
	if (current_token->is_equal("if"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_IF, current_token->_location);

		/* ifの次は'('がくる */
		current_token = Token::skip(std::move(current_token->_next), "(");

		/* 条件文 */
		node->_condition = expression(current_token, std::move(current_token));

		/* 条件文のは')'がくる */
		current_token = Token::skip(std::move(current_token), ")");
		node->_then = statement(current_token, std::move(current_token));

		/* else節が存在する */
		if (current_token->is_equal("else"))
		{
			node->_else = statement(current_token, std::move(current_token->_next));
		}
		next_token = std::move(current_token);
		return node;
	}

	/* for */
	if (current_token->is_equal("for"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_FOR, current_token->_location);

		/* forの次は'('がくる */
		current_token = Token::skip(std::move(current_token->_next), "(");

		/* 初期化処理 */
		node->_init = expression_statement(current_token, std::move(current_token));

		/* 次のトークンが';'でなければ条件が存在する */
		if (!current_token->is_equal(";"))
		{
			node->_condition = expression(current_token, std::move(current_token));
		}
		current_token = Token::skip(std::move(current_token), ";");

		/* 次のトークンが')'でなければ加算処理が存在する */
		if (!current_token->is_equal(")"))
		{
			node->_inc = expression(current_token, std::move(current_token));
		}
		current_token = Token::skip(std::move(current_token), ")");
		/* forの中の処理 */
		node->_then = statement(next_token, std::move(current_token));
		return node;
	}

	/* while */
	if (current_token->is_equal("while"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_FOR, current_token->_location);

		/* whileの次は'('がくる */
		current_token = Token::skip(std::move(current_token->_next), "(");
		node->_condition = expression(current_token, std::move(current_token));

		/* 条件文のは')'がくる */
		current_token = Token::skip(std::move(current_token), ")");
		node->_then = statement(next_token, std::move(current_token));
		return node;
	}

	/* ブロック */
	if (current_token->is_equal("{"))
	{
		return compound_statement(next_token, std::move(current_token->_next));
	}

	/* 式 */
	return expression_statement(next_token, std::move(current_token));
}

/**
 * @brief ブロックを読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n compound-statement = (declaration | statement)* "}"
 */
unique_ptr<Node> Node::compound_statement(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{

	auto node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token->_location);

	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* '}'が出てくるまでstatementをパースする */
	while (!current_token->is_equal("}"))
	{
		/* 変数宣言 */
		if (current_token->is_equal("int"))
		{
			cur->_next = declaration(current_token, std::move(current_token));
		}
		/* 宣言以外の文 */
		else
		{
			cur->_next = statement(current_token, std::move(current_token));
		}

		cur = cur->_next.get();
		Type::add_type(cur);
	}

	/* ダミーのheadからの次のトークン以降を切り離し、新しいノードのbodyに繋ぐ*/
	node->_body = std::move(head->_next);
	/* '}' の次のトークンからパースを続ける*/
	next_token = std::move(current_token->_next);
	return node;
}

/**
 * @brief 関数宣言を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 読み取った関数のオブジェクト
 * @details 下記のEBNF規則に従う。 @n function-definition = declspec declarator "{" compound-statement
 */
unique_ptr<Function> Node::function_definition(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	/* 念のため再初期化 */
	locals = nullptr;

	auto ty = declspec(current_token, std::move(current_token));
	ty = declarator(current_token, std::move(current_token), ty);
	Function::create_params_lvars(std::move(ty->_params));

	auto fn = std::make_unique_for_overwrite<Function>();
	current_func = fn.get();
	fn->_name = ty->_name;
	fn->_params = std::move(locals);

	current_token = Token::skip(std::move(current_token), "{");
	fn->_body = compound_statement(current_token, std::move(current_token));
	fn->_locals = std::move(locals);
	return fn;
}

/**
 * @brief 変数宣言を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
 */
unique_ptr<Node> Node::declaration(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	const auto base = declspec(current_token, std::move(current_token));

	/* ノードリストの先頭としてダミーのノードを作成 */
	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* この文で宣言している変数の個数 */
	int cnt = 0;

	/* ";"が出てくるまで読み取りを続ける */
	while (!current_token->is_equal(";"))
	{
		/* 2個目以降の宣言には",""区切りが必要 */
		if (cnt++ > 0)
		{
			current_token = Token::skip(std::move(current_token), ",");
		}
		/* 変数の最終的な型を決定 */
		auto ty = declarator(current_token, std::move(current_token), base);
		const auto var = Object::new_lvar(std::move(ty));

		/* 宣言の後に初期化式がない場合は次のループへ */
		if (!current_token->is_equal("="))
		{
			continue;
		};

		/* 変数を表すノードを生成 */
		auto lhs = std::make_unique<Node>(var, var->_ty->_location);
		/* 変数の初期化値を表すノードを生成 */
		auto rhs = assign(current_token, std::move(current_token->_next));
		/* 初期化を代入式として表すノードを生成 */
		auto node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(lhs), std::move(rhs), current_token->_location);
		/* ノードリストの末尾に単文ノードとして追加 */
		cur->_next = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, std::move(node), current_token->_location);
		/* ノードリストの末尾を更新 */
		cur = cur->_next.get();
	}
	auto node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token->_location);
	/* ヘッダの次のノード以降を切り離してnodeのbodyに繋ぐ */
	node->_body = std::move(head->_next);
	next_token = std::move(current_token->_next);
	return node;
}

/**
 * @brief 宣言を変数 or 関数か判断し結果の型を返す
 *
 * @details
 * 例：int a; int型の変数 int fn(); 戻り値がint型の関数。次のEBNF規則に従う。 @n
 * type-suffix = ("(" function-parameters? ")")? @n
 * function-parameters = parameter ("," parameter)* @n
 * parameter = declspec declarator
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 宣言されている型
 * @return 変数 or 関数の型
 */
shared_ptr<Type> Node::type_suffix(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token, shared_ptr<Type> &&ty)
{
	/* 識別子名の後に"("があれば関数 */
	if (current_token->is_equal("("))
	{
		current_token = std::move(current_token->_next);

		auto head = std::make_unique_for_overwrite<Type>();
		auto cur = head.get();

		/* ")"が出てくるまで読み取りを続ける */
		while (!current_token->is_equal(")"))
		{
			if (head.get() != cur)
			{
				/* 2個目以降の引数では","区切りが必要 */
				current_token = Token::skip(std::move(current_token), ",");
			}
			auto base = declspec(current_token, std::move(current_token));
			cur->_next = declarator(current_token, std::move(current_token), base);
			cur = cur->_next.get();
		}
		ty = Type::func_type(ty);
		ty->_params = std::move(head->_next);
		next_token = std::move(current_token->_next);
		return ty;
	}
	/* そうでなければ普通の変数 */
	next_token = std::move(current_token);
	return ty;
}

/**
 * @brief 変数宣言を変数名部分を読み取る
 *
 * @details
 * 変数の型は変数名に辿りつくまで確定できない。 @n
 * 例：int a, *b, **c; aはint型、bはint型へのポインタ、cはint型へのポインタへのポインタ @n
 * 下記のEBNF規則に従う。 @n declarator = "*"* ident
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 変数の型の基準
 * @return 変数の型
 */
shared_ptr<Type> Node::declarator(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token, shared_ptr<Type> ty)
{
	/* "*"の数だけ直前の型へのポインタになる */
	while (Token::consume(current_token, std::move(current_token), "*"))
	{
		ty = Type::pointer_to(ty);
	}
	/* トークンの種類が識別子でないときエラー */
	if (TokenKind::TK_IDENT != current_token->_kind)
	{
		error_at("識別子の名前ではありません", current_token->_location);
	}

	/* 名前を設定 */
	ty->_name = current_token->_str;
	ty->_location = current_token->_location;

	/* 関数か変数か */
	ty = type_suffix(next_token, std::move(current_token->_next), std::move(ty));

	return std::move(ty);
}

/**
 * @brief 変数宣言の型宣言部分を読み取る
 *
 * @details 下記のEBNF規則に従う。 @n declspec = "int"
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 変数の型
 */
shared_ptr<Type> Node::declspec(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	next_token = Token::skip(std::move(current_token), "int");
	return Type::INT_BASE;
}

/**
 * @brief 単文を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n expression-statement = expression? ';'
 */
unique_ptr<Node> Node::expression_statement(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	/* 空のstatementに対応 */
	if (current_token->is_equal(";"))
	{
		next_token = std::move(current_token->_next);
		return std::make_unique<Node>(NodeKind::ND_BLOCK, current_token->_location);
	}
	unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, expression(current_token, std::move(current_token)), current_token->_location);

	/* expression-statementは';'で終わるはず */
	next_token = Token::skip(std::move(current_token), ";");
	return node;
}

/**
 * @brief 式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n expression = assign
 */
unique_ptr<Node> Node::expression(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	return assign(next_token, std::move(current_token));
}

/**
 * @brief 代入式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n assign = equality ("=" assign)?
 */
unique_ptr<Node> Node::assign(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	auto node = equality(current_token, std::move(current_token));
	if (current_token->is_equal("="))
	{
		return std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), assign(next_token, std::move(current_token->_next)), current_token->_location);
	}
	next_token = std::move(current_token);
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
unique_ptr<Node> Node::equality(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	auto node = relational(current_token, std::move(current_token));

	for (;;)
	{
		auto location = current_token->_location;
		if (current_token->is_equal("=="))
		{
			node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), relational(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (current_token->is_equal("!="))
		{
			node = std::make_unique<Node>(NodeKind::ND_NE, std::move(node), relational(current_token, std::move(current_token->_next)), location);
			continue;
		}

		next_token = std::move(current_token);
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
unique_ptr<Node> Node::relational(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	unique_ptr<Node> node = add(current_token, std::move(current_token));

	while (true)
	{

		auto location = current_token->_location;
		if (current_token->is_equal("<"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), add(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (current_token->is_equal("<="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), add(current_token, std::move(current_token->_next)), location);
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (current_token->is_equal(">"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, add(current_token, std::move(current_token->_next)), std::move(node), location);
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (current_token->is_equal(">="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, add(current_token, std::move(current_token->_next)), std::move(node), location);
			continue;
		}

		/* 比較演算子が出てこなくなったらループを抜ける */
		next_token = std::move(current_token);
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
unique_ptr<Node> Node::add(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	unique_ptr<Node> node = mul(current_token, std::move(current_token));

	while (true)
	{
		auto location = current_token->_location;
		if (current_token->is_equal("+"))
		{
			node = new_add(std::move(node), mul(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (current_token->is_equal("-"))
		{
			node = new_sub(std::move(node), mul(current_token, std::move(current_token->_next)), location);
			continue;
		}

		/* "+", "-"どちらも出てこなくなったらループを抜ける */
		next_token = std::move(current_token);
		return node;
	}
}

/**
 * @brief 積を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n mul = unary ("*" unary | "/" unary)*
 */
unique_ptr<Node> Node::mul(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	unique_ptr<Node> node = unary(current_token, std::move(current_token));

	while (true)
	{
		auto location = current_token->_location;
		if (current_token->is_equal("*"))
		{
			node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), unary(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (current_token->is_equal("/"))
		{
			node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), unary(current_token, std::move(current_token->_next)), location);
			continue;
		}

		/* "*", "/"どちらも出てこなくなったらループを抜ける */
		next_token = std::move(current_token);
		return node;
	}
}

/**
 * @brief 符号付の単項を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n unary = ("+" | "-" | "*" | "&") unary
 */
unique_ptr<Node> Node::unary(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	if (current_token->is_equal("+"))
	{
		return unary(next_token, std::move(current_token->_next));
	}

	if (current_token->is_equal("-"))
	{
		return std::make_unique<Node>(NodeKind::ND_NEG, unary(next_token, std::move(current_token->_next)), current_token->_location);
	}

	if (current_token->is_equal("&"))
	{
		return std::make_unique<Node>(NodeKind::ND_ADDR, unary(next_token, std::move(current_token->_next)), current_token->_location);
	}

	if (current_token->is_equal("*"))
	{
		return std::make_unique<Node>(NodeKind::ND_DEREF, unary(next_token, std::move(current_token->_next)), current_token->_location);
	}

	return primary(next_token, std::move(current_token));
}

/**
 * @brief 単項を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * primary = "(" expression ")" | identifier args? | num @n
 * args = "(" ")"
 */
unique_ptr<Node> Node::primary(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	/* トークンが"("なら、"(" expression ")"のはず */
	if (current_token->is_equal("("))
	{
		auto node = expression(current_token, std::move(current_token->_next));
		next_token = Token::skip(std::move(current_token), ")");
		return node;
	}

	/* トークンが識別子の場合 */
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		/* 識別子の後に()がついていたら関数呼び出し */
		if (current_token->_next->is_equal("("))
		{
			return function_call(next_token, std::move(current_token));
		}

		/* それ以外なら普通の変数 */
		const auto var = Object::find_var(current_token);

		/* 変数が宣言されていない場合はエラー */
		if (!var)
		{
			error_at("未宣言の変数です", current_token->_location);
		}
		auto node = std::make_unique<Node>(var, current_token->_location);
		next_token = std::move(current_token->_next);
		return node;
	}

	/* トークンが数値の場合 */
	if (TokenKind::TK_NUM == current_token->_kind)
	{
		auto node = std::make_unique<Node>(std::move(current_token->_value), current_token->_location);
		next_token = std::move(current_token->_next);
		return node;
	}

	/* どれでもなければエラー */
	error_at("式ではありません", current_token->_location);

	/* コンパイルエラー対策、error_at()内でプログラムは終了するためnullptrが返ることはない */
	return nullptr;
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
unique_ptr<Node> Node::function_call(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	/* 関数呼び出しノードを作成 */
	auto node = std::make_unique<Node>(NodeKind::ND_FUNCALL, current_token->_location);
	/* 関数の名前をセット */
	node->_func_name = std::move(current_token->_str);

	current_token = std::move(current_token->_next->_next);

	/* ノードリストの先頭としてダミーのノードを生成 */
	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* ')'が出てくるまで読み込み続ける */
	while (!current_token->is_equal(")"))
	{
		if (head.get() != cur)
		{
			/* 2個目以降の引数には区切りとして","が必要 */
			current_token = Token::skip(std::move(current_token), ",");
		}
		cur->_next = assign(current_token, std::move(current_token));
		cur = cur->_next.get();
	}

	/* 最後は")"" */
	next_token = Token::skip(std::move(current_token), ")");
	/* headの次のノード以降を切り離し返り値用のnodeのargsに繋ぐ */
	node->_args = std::move(head->_next);
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
 * @param location ノードと対応する入力文字列の位置
 * @return 対応するASTノード
 */
unique_ptr<Node> Node::new_add(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, const int &location)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 + 数 */
	if (lhs->_ty->is_integer() && rhs->_ty->is_integer())
	{
		return std::make_unique<Node>(NodeKind::ND_ADD, std::move(lhs), std::move(rhs), location);
	}

	/* ptr + ptr は無効な演算 */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		error_at("無効な演算です", location);
	}

	/* "数 + ptr" を "ptr + 数" に変換する*/
	if (!lhs->_ty->_base && rhs->_ty->_base)
	{
		std::swap(lhs, rhs);
	}

	/* ptr + 数 */
	rhs = std::make_unique<Node>(NodeKind::ND_MUL, std::move(rhs), std::make_unique<Node>(8, location), location);
	return std::make_unique<Node>(NodeKind::ND_ADD, std::move(lhs), std::move(rhs), location);
}

/**
 * @brief 左辺 - 右辺の計算結果を表すノードを生成する。
 *
 * @details
 * C言語では、-演算子も'+'演算子と同様にポインタ演算を行うためにオーバーロードされている。
 * ポインタ - ポインタは2つのポインタ間にある要素の数を返す。
 * @param lhs 左辺
 * @param rhs 右辺
 * @param location ノードと対応する入力文字列の位置
 * @return 対応するASTノード
 */
std::unique_ptr<Node> Node::new_sub(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, const int &location)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 - 数 */
	if (lhs->_ty->is_integer() && rhs->_ty->is_integer())
	{
		return std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), location);
	}

	/* ptr - 数 */
	if (lhs->_ty->_base && !rhs->_ty->_base)
	{
		rhs = std::make_unique<Node>(NodeKind::ND_MUL, std::move(rhs), std::make_unique<Node>(8, location), location);
		Type::add_type(rhs.get());
		unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), location);
		return node;
	}

	/* ptr - ptr */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), location);
		node->_ty = Type::INT_BASE;
		return std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::make_unique<Node>(8, location), location);
	}

	/* 数 - ptr はエラー */
	error_at("無効な演算です", location);

	/* コンパイルエラー対策。直前のerror_at()で終了されるのでnullptrが返ることはない */
	return nullptr;
}

/**
 * @brief トークン・リストを構文解析して関数ごとにASTを構築する
 *
 * @param token トークン・リストの先頭
 * @return 構文解析結果
 * @details program = function-definition*
 */
unique_ptr<Function> Node::parse(unique_ptr<Token> &&token)
{
	/* リストの先頭としてダミーのheadを生成 */
	auto head = std::make_unique_for_overwrite<Function>();
	auto cur = head.get();

	/* トークンリストを最後まで辿る*/
	while (TokenKind::TK_EOF != token->_kind)
	{
		cur->_next = function_definition(token, std::move(token));
		cur = cur->_next.get();
	}

	return std::move(head->_next);
}
