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

/**
 * @brief
 * 変数や関数オブジェクトのリスト。パース中に生成される全てのローカル変数はこのリストに連結される。
 */
static unique_ptr<Object> locals = nullptr;

/****************/
/* Object Class */
/****************/

/** @brief デフォルトコンストラクタ */
Object::Object() = default;

/**
 * @brief 名前を付けてオブジェクトを生成
 *
 * @param name オブジェクトの名前
 */
Object::Object(std::string &&name) : _name(std::move(name))
{
}

/**
 * @brief nameを名前として持つ新しい変数を生成してlocalsの先頭に追加する。
 *
 * @param name オブジェクトの名前
 * @return 生成した変数へのポインタ
 */
const Object *Object::new_lvar(std::shared_ptr<Type> &&ty)
{
	unique_ptr<Object> var = std::make_unique<Object>(std::string(ty->_location, ty->_location + ty->_length));
	var->_next = std::move(locals);
	var->_ty = std::move(ty);
	locals = std::move(var);
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
	for (const Object *var = locals.get(); var; var = var->_next.get())
	{
		if (var->_name.length() == token->_length && std::equal(var->_name.begin(), var->_name.end(), token->_location))
		{
			return var;
		}
	}
	return nullptr;
}

/******************/
/* Function Class */
/******************/

/** @brief デフォルトコンストラクタ */
Function::Function() = default;

/**
 * @brief 関数の中身を定義してオブジェクトを生成
 *
 * @param body 関数内で行う処理
 * @param locals 関数内で使うローカル変数
 */
Function::Function(unique_ptr<Node> &&body, unique_ptr<Object> &&locals)
	: _body(std::move(body)), _locals(std::move(locals))
{
}

/**
 * @brief 'n'を切り上げて最も近い'align'の倍数にする。
 *
 * @param n 切り上げ対象
 * @param align 基数
 * @return 切り上げた結果
 * @details 例：align_to(5,8) = 8, align_to(11,8) = 16
 */
int Function::align_to(int &&n, int &&align)
{
	return (n + align - 1) / align * align;
}

/** @brief 関数に必要なスタックサイズを計算してstack_sizeにセットする。 */
void Function::assign_lvar_offsets()
{
	int offset = 0;
	for (Object *var = this->_locals.get(); var; var = var->_next.get())
	{
		offset += 8;
		var->_offset = offset;
	}
	this->_stack_size = align_to(std::move(offset), 16);
}

/**************/
/* Node Class */
/**************/

/** @brief デフォルトコンストラクタ */
Node::Node() = default;

/**
 * @brief 右辺と左辺を持たないノードオブジェクトを生成。
 *
 * @param kind ノードの種類
 * @param location ノードと対応する入力文字列の位置
 */
Node::Node(NodeKind &&kind, const std::string::const_iterator &location)
	: _kind(std::move(kind)), _location(location)
{
}

/**
 * @brief 左辺を持つオブジェクトを生成。
 *
 * @param kind ノードの種類
 * @param lhs 左辺のノード
 * @param location ノードと対応する入力文字列の位置
 */
Node::Node(NodeKind &&kind, unique_ptr<Node> &&lhs, const std::string::const_iterator &location)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _location(location)
{
}

/**
 * @brief 数値を表すノードのオブジェクトを生成。
 *
 * @param val ノードが表す数値
 * @param location ノードと対応する入力文字列の位置
 */
Node::Node(int &&val, const std::string::const_iterator &location)
	: _kind(NodeKind::ND_NUM), _val(std::move(val)), _location(location)
{
}

/**
 * @brief 変数を表すノードのオブジェクトを生成。
 *
 * @param var ノードが表す変数
 * @param location ノードと対応する入力文字列の位置
 */
Node::Node(const Object *var, const std::string::const_iterator &location)
	: _kind(NodeKind::ND_VAR), _var(std::move(var)), _location(location)
{
}

/**
 * @brief 右辺と左辺を持つオブジェクトを生成。
 *
 * @param kind ノードの種類
 * @param lhs 左辺のノード
 * @param rhs 右辺のノード
 * @param location ノードと対応する入力文字列の位置
 */
Node::Node(NodeKind &&kind, unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, const std::string::const_iterator &location)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _rhs(std::move(rhs)), _location(location)
{
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
 * 			 | "{" compound-statement @n
 * 			 | expression-statement
 */
unique_ptr<Node> Node::statement(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	/* return */
	if (Token::is_equal(current_token, "return"))
	{
		/* "return"の次はexpresionがくる */
		unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_RETURN, current_token->_location);
		node->_lhs = expression(current_token, std::move(current_token->_next));

		/* 最後は';'で終わるはず */
		next_token = Token::skip(std::move(current_token), ";");
		return node;
	}

	/* if */
	if (Token::is_equal(current_token, "if"))
	{
		unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_IF, current_token->_location);

		/* ifの次は'('がくる */
		current_token = Token::skip(std::move(current_token->_next), "(");

		/* 条件文 */
		node->_condition = expression(current_token, std::move(current_token));

		/* 条件文のは')'がくる */
		current_token = Token::skip(std::move(current_token), ")");
		node->_then = statement(current_token, std::move(current_token));

		/* else節が存在する */
		if (Token::is_equal(current_token, "else"))
		{
			node->_else = statement(current_token, std::move(current_token->_next));
		}
		next_token = std::move(current_token);
		return node;
	}

	/* for */
	if (Token::is_equal(current_token, "for"))
	{
		unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_FOR, current_token->_location);

		/* forの次は'('がくる */
		current_token = Token::skip(std::move(current_token->_next), "(");

		/* 初期化処理 */
		node->_init = expression_statement(current_token, std::move(current_token));

		/* 次のトークンが';'でなければ条件が存在する */
		if (!Token::is_equal(current_token, ";"))
		{
			node->_condition = expression(current_token, std::move(current_token));
		}
		current_token = Token::skip(std::move(current_token), ";");

		/* 次のトークンが')'でなければ加算処理が存在する */
		if (!Token::is_equal(current_token, ")"))
		{
			node->_inc = expression(current_token, std::move(current_token));
		}
		current_token = Token::skip(std::move(current_token), ")");
		/* forの中の処理 */
		node->_then = statement(next_token, std::move(current_token));
		return node;
	}

	/* while */
	if (Token::is_equal(current_token, "while"))
	{
		unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_FOR, current_token->_location);

		/* whileの次は'('がくる */
		current_token = Token::skip(std::move(current_token->_next), "(");
		node->_condition = expression(current_token, std::move(current_token));

		/* 条件文のは')'がくる */
		current_token = Token::skip(std::move(current_token), ")");
		node->_then = statement(next_token, std::move(current_token));
		return node;
	}

	/* ブロック */
	if (Token::is_equal(current_token, "{"))
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

	unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token->_location);

	unique_ptr<Node> head = std::make_unique_for_overwrite<Node>();
	Node *cur = head.get();

	/* '}'が出てくるまでstatementをパースする */
	while (!Token::is_equal(current_token, "}"))
	{
		/* 変数宣言 */
		if (Token::is_equal(current_token, "int"))
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
 * @brief 変数宣言を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
 */
unique_ptr<Node> Node::declaration(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	const shared_ptr<Type> base = declspec(current_token, std::move(current_token));

	/* ノードリストの先頭としてダミーのノードを作成 */
	unique_ptr<Node> head = std::make_unique_for_overwrite<Node>();
	Node *cur = head.get();

	/* この文で宣言している変数の個数 */
	int cnt = 0;

	/* ";"が出てくるまで読み取りを続ける */
	while (!Token::is_equal(current_token, ";"))
	{
		/* 2個目以降の宣言には",""区切りが必要 */
		if (cnt++ > 0)
		{
			current_token = Token::skip(std::move(current_token), ",");
		}
		/* 変数の最終的な型を決定 */
		shared_ptr<Type> ty = declarator(current_token, std::move(current_token), base);
		const Object *var = Object::new_lvar(std::move(ty));

		/* 宣言の後に初期化式がない場合は次のループへ */
		if (!Token::is_equal(current_token, "="))
		{
			continue;
		};

		/* 変数を表すノードを生成 */
		unique_ptr<Node> lhs = std::make_unique<Node>(var, var->_ty->_location);
		/* 変数の初期化値を表すノードを生成 */
		unique_ptr<Node> rhs = assign(current_token, std::move(current_token->_next));
		/* 初期化を代入式として表すノードを生成 */
		unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(lhs), std::move(rhs), current_token->_location);
		/* ノードリストの末尾に単文ノードとして追加 */
		cur->_next = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, std::move(node), current_token->_location);
		/* ノードリストの末尾を更新 */
		cur = cur->_next.get();
	}
	unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token->_location);
	/* ヘッダの次のノード以降を切り離してnodeのbodyに繋ぐ */
	node->_body = std::move(head->_next);
	next_token = std::move(current_token->_next);
	return node;
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
		ty = std::make_shared<Type>(std::move(ty));
	}
	/* トークンの種類が識別子でないときエラー */
	if (TokenKind::TK_IDENT != current_token->_kind)
	{
		error_at("識別子の名前ではありません", std::move(current_token->_location));
	}
	ty->_length = std::move(current_token->_length);
	ty->_location = std::move(current_token->_location);
	next_token = std::move(current_token->_next);
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
	return std::move(ty_int());
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
	if (Token::is_equal(current_token, ";"))
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
	unique_ptr<Node> node = equality(current_token, std::move(current_token));
	if (Token::is_equal(current_token, "="))
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
	unique_ptr<Node> node = relational(current_token, std::move(current_token));

	for (;;)
	{
		auto location = current_token->_location;
		if (Token::is_equal(current_token, "=="))
		{
			node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), relational(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (Token::is_equal(current_token, "!="))
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

	for (;;)
	{

		auto location = current_token->_location;
		if (Token::is_equal(current_token, "<"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), add(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (Token::is_equal(current_token, "<="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), add(current_token, std::move(current_token->_next)), location);
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (Token::is_equal(current_token, ">"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, add(current_token, std::move(current_token->_next)), std::move(node), location);
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (Token::is_equal(current_token, ">="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, add(current_token, std::move(current_token->_next)), std::move(node), location);
			continue;
		}

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

	for (;;)
	{
		auto location = current_token->_location;
		if (Token::is_equal(current_token, "+"))
		{
			node = new_add(std::move(node), mul(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (Token::is_equal(current_token, "-"))
		{
			node = new_sub(std::move(node), mul(current_token, std::move(current_token->_next)), location);
			continue;
		}

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

	for (;;)
	{
		auto location = current_token->_location;
		if (Token::is_equal(current_token, "*"))
		{
			node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), unary(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (Token::is_equal(current_token, "/"))
		{
			node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), unary(current_token, std::move(current_token->_next)), location);
			continue;
		}

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
	if (Token::is_equal(current_token, "+"))
	{
		return unary(next_token, std::move(current_token->_next));
	}

	if (Token::is_equal(current_token, "-"))
	{
		return std::make_unique<Node>(NodeKind::ND_NEG, unary(next_token, std::move(current_token->_next)), current_token->_location);
	}

	if (Token::is_equal(current_token, "&"))
	{
		return std::make_unique<Node>(NodeKind::ND_ADDR, unary(next_token, std::move(current_token->_next)), current_token->_location);
	}

	if (Token::is_equal(current_token, "*"))
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
	if (Token::is_equal(current_token, "("))
	{
		unique_ptr<Node> node = expression(current_token, std::move(current_token->_next));
		next_token = Token::skip(std::move(current_token), ")");
		return node;
	}

	/* トークンが識別子の場合 */
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		/* 識別子の後に()がついていたら関数呼び出し */
		if(Token::is_equal(current_token->_next, "(")){
			unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_FUNCALL, current_token->_location);
			node->func_name = std::string(current_token->_location, current_token->_location + current_token->_length);
			next_token = Token::skip(std::move(current_token->_next->_next), ")");
			return node;
		}

		/* それ以外なら普通の変数 */
		const Object *var = Object::find_var(current_token);

		/* 変数が宣言されていない場合はエラー */
		if (!var)
		{
			error_at("未宣言の変数です", std::move(current_token->_location));
		}
		unique_ptr<Node> node = std::make_unique<Node>(var, current_token->_location);
		next_token = std::move(current_token->_next);
		return node;
	}

	/* トークンが数値の場合 */
	if (TokenKind::TK_NUM == current_token->_kind)
	{
		unique_ptr<Node> node = std::make_unique<Node>(std::move(current_token->_value), current_token->_location);
		next_token = std::move(current_token->_next);
		return node;
	}

	/* どれでもなければエラー */
	error_at("式ではありません", std::move(current_token->_location));

	/* コンパイルエラー対策、error_at()内でプログラムは終了するためnullptrが返ることはない */
	return nullptr;
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
unique_ptr<Node> Node::new_add(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, std::string::const_iterator &location)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 + 数 */
	if (Type::is_integer(lhs->_ty) && Type::is_integer(rhs->_ty))
	{
		return std::make_unique<Node>(NodeKind::ND_ADD, std::move(lhs), std::move(rhs), location);
	}

	/* ptr + ptr は無効な演算 */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		error_at("無効な演算です", std::move(location));
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
std::unique_ptr<Node> Node::new_sub(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, std::string::const_iterator &location)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 - 数 */
	if (Type::is_integer(lhs->_ty) && Type::is_integer(rhs->_ty))
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
		node->_ty = ty_int();
		return std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::make_unique<Node>(8, location), location);
	}

	/* 数 - ptr はエラー */
	error_at("無効な演算です", std::move(location));

	/* コンパイルエラー対策。直前のerror_at()で終了されるのでnullptrが返ることはない */
	return nullptr;
}

/**
 * @brief トークン・リストを構文解析して関数ごとにASTを構築する
 *
 * @param token トークン・リストの先頭
 * @return 構文解析結果
 * @details program = statement*
 */
unique_ptr<Function> Node::parse(unique_ptr<Token> &&token)
{
	/* 最初の'{'を飛ばす */
	token = std::move(Token::skip(std::move(token), "{"));

	unique_ptr<Function> prog = std::make_unique<Function>(compound_statement(token, std::move(token)), std::move(locals));
	return std::move(prog);
}
