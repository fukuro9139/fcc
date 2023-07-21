#include "parse.hpp"

/* 変数や関数オブジェクトのリスト */
/* パース中に生成される全てのローカル変数はこのリストに連結される。 */
static std::unique_ptr<Object> locals = nullptr;

/****************************** Object class ******************************/

/* コンストラクタ */
Object::Object() = default;

Object::Object(std::string &&name) : _name(std::move(name))
{
}

/******************************************************************************************************
 * @brief
 * nameを名前として持つ新しい変数を生成してlocalsの先頭に追加する。
 * 生成した変数へのポインタを返す。
 ******************************************************************************************************/
const Object *Object::new_lvar(std::string &&name)
{
	std::unique_ptr<Object> var = std::make_unique<Object>(std::move(name));
	var->_next = std::move(locals);
	locals = std::move(var);
	return locals.get();
}

/******************************************************************************************************
 * @brief
 * 変数を名前で検索する。見つからなかった場合はNULLを返す。
 ******************************************************************************************************/
const Object *Object::find_var(const std::unique_ptr<Token> &token)
{
	for (const Object *val = locals.get(); val; val = val->_next.get())
	{
		if (val->_name.length() == token->_length && std::equal(val->_name.begin(), val->_name.end(), token->_location))
		{
			return val;
		}
	}
	return nullptr;
}

/****************************** Function class ******************************/

/* コンストラクタ */
Function::Function() = default;

Function::Function(std::unique_ptr<Node> &&body, std::unique_ptr<Object> &&locals)
	: _body(std::move(body)), _locals(std::move(locals))
{
}

/******************************************************************************************************
 * @brief
 * 'n'を切り上げて最も近い'align'の倍数にする。
 * 例：align_to(5,8) = 8, align_to(11,8) = 16
 *****************************************************************************************************/
int Function::align_to(int &&n, int &&align)
{
	return (n + align - 1) / align * align;
}

/* プログラムに必要なスタックサイズを計算する */
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

/****************************** Node class ******************************/

Node::Node() = default;

Node::Node(NodeKind &&kind)
	: _kind(std::move(kind))
{
}

Node::Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs)
	: _kind(std::move(kind)), _lhs(std::move(lhs))
{
}

Node::Node(int &&val)
	: _kind(NodeKind::ND_NUM), _val(std::move(val))
{
}

Node::Node(const Object *var)
	: _kind(NodeKind::ND_VAR), _var(std::move(var))
{
}

Node::Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _rhs(std::move(rhs))
{
}

/******************************************************************************************************
 * @brief
 * statement ノードを生成する。
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 下記のEBNF規則に従う。 @n
 * statement = "return" expression ";" | "{" compound-stmt | expression-statement
 *****************************************************************************************************/
std::unique_ptr<Node> Node::statement(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	if (Token::is_equal(token, "return"))
	{
		/* "return"の次はexpresionがくる */
		std::unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_RETURN, expression(token, std::move(token->_next)));
		/* 最後は';'で終わるはず */
		current_token = Token::skip(std::move(token), ";");
		return node;
	}

	if (Token::is_equal(token, "{"))
	{
		return compound_statement(current_token, std::move(token->_next));
	}

	return expr_stmt(current_token, std::move(token));
}

/******************************************************************************************************
 * @brief
 * compound_statementノードを生成する。
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 下記のEBNF規則に従う。 @n
 * compound-stmt = stmt* "}"
 *****************************************************************************************************/
std::unique_ptr<Node> Node::compound_statement(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	std::unique_ptr<Node> head = std::make_unique_for_overwrite<Node>();
	Node *cur = head.get();
	/* '}'が出てくるまでstatementをパースする */
	while (!Token::is_equal(token, "}"))
	{
		cur->_next = statement(token, std::move(token));
		cur = cur->_next.get();
	}

	/* ダミーのheadからの次のトークン以降を切り離し、新しいノードのbodyに繋ぐ*/
	std::unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_BLOCK);
	node->_body = std::move(head->_next);
	/* '}' の次のトークンからパースを続ける*/
	current_token = std::move(token->_next);
	return node;
}

/******************************************************************************************************
 * @brief
 * expression-statementノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 下記のEBNF規則に従う。 @n
 * expression-statement = expression ';'
 *******************************************************************************************************/
std::unique_ptr<Node> Node::expr_stmt(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	std::unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, expression(token, std::move(token)));
	/* expression-statementは';'で終わるはず */
	current_token = Token::skip(std::move(token), ";");
	return node;
}

/******************************************************************************************************
 * @brief
 * expressionノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * expression = assign
 *****************************************************************************************************/
std::unique_ptr<Node> Node::expression(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	return assign(current_token, std::move(token));
}

/******************************************************************************************************
 * @brief
 * assignノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * assign = equality ("=" assign)?
 *****************************************************************************************************/
std::unique_ptr<Node> Node::assign(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	std::unique_ptr<Node> node = equality(token, std::move(token));
	if (Token::is_equal(token, "="))
	{
		node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), assign(token, std::move(token->_next)));
	}
	current_token = std::move(token);
	return node;
}

/******************************************************************************************************
 * @brief
 * equalityノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * equality = relational ("==" relational | "!=" relational)*
 *****************************************************************************************************/
std::unique_ptr<Node> Node::equality(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	std::unique_ptr<Node> node = relational(token, std::move(token));

	for (;;)
	{
		if (Token::is_equal(token, "=="))
		{
			node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), relational(token, std::move(token->_next)));
			continue;
		}

		if (Token::is_equal(token, "!="))
		{
			node = std::make_unique<Node>(NodeKind::ND_NE, std::move(node), relational(token, std::move(token->_next)));
			continue;
		}

		current_token = std::move(token);
		return node;
	}
}

/******************************************************************************************************
 * @brief
 * relationalノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 *****************************************************************************************************/
std::unique_ptr<Node> Node::relational(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	std::unique_ptr<Node> node = add(token, std::move(token));

	for (;;)
	{
		if (Token::is_equal(token, "<"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), add(token, std::move(token->_next)));
			continue;
		}

		if (Token::is_equal(token, "<="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), add(token, std::move(token->_next)));
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (Token::is_equal(token, ">"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, add(token, std::move(token->_next)), std::move(node));
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (Token::is_equal(token, ">="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, add(token, std::move(token->_next)), std::move(node));
			continue;
		}

		current_token = std::move(token);
		return node;
	}
}

/**
 * @brief
 * addノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * add = mul ("+" mul | "-" mul)*
 */
std::unique_ptr<Node> Node::add(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	std::unique_ptr<Node> node = mul(token, std::move(token));

	for (;;)
	{
		if (Token::is_equal(token, "+"))
		{
			node = std::make_unique<Node>(NodeKind::ND_ADD, std::move(node), mul(token, std::move(token->_next)));
			continue;
		}

		if (Token::is_equal(token, "-"))
		{
			node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(node), mul(token, std::move(token->_next)));
			continue;
		}

		current_token = std::move(token);
		return node;
	}
}

/******************************************************************************************************
 * @brief
 *
 *****************************************************************************************************/

/**
 * @brief
 * mulノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * mul = unary ("*" unary | "/" unary)*
 */
std::unique_ptr<Node> Node::mul(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	std::unique_ptr<Node> node = unary(token, std::move(token));

	for (;;)
	{
		if (Token::is_equal(token, "*"))
		{
			node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), unary(token, std::move(token->_next)));
			continue;
		}

		if (Token::is_equal(token, "/"))
		{
			node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), unary(token, std::move(token->_next)));
			continue;
		}

		current_token = std::move(token);
		return node;
	}
}

/**
 * @brief
 * unaryノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * unary = ("+" | "-") unary
 */
std::unique_ptr<Node> Node::unary(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	if (Token::is_equal(token, "+"))
	{
		return unary(current_token, std::move(token->_next));
	}

	if (Token::is_equal(token, "-"))
	{
		return std::make_unique<Node>(NodeKind::ND_NEG, unary(current_token, std::move(token->_next)));
	}

	return primary(current_token, std::move(token));
}

/**
 * @brief
 * primaryノードを生成する
 * @param current_token
 * @param token
 * @return std::unique_ptr<Node>
 * @details
 * 次のEBNF規則に従う
 * primary = "(" expression ")" | ident | num
 */
std::unique_ptr<Node> Node::primary(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token)
{
	/* トークンが"("なら、"(" expression ")"のはず */
	if (Token::is_equal(token, "("))
	{
		std::unique_ptr<Node> node = expression(token, std::move(token->_next));
		current_token = Token::skip(std::move(token), ")");
		return node;
	}

	/* トークンが識別子の場合 */
	if (TokenKind::TK_IDENT == token->_kind)
	{
		const Object *var = Object::find_var(token);
		if (!var)
		{
			var = Object::new_lvar(std::string(token->_location, token->_location + token->_length));
		}
		current_token = std::move(token->_next);
		return std::make_unique<Node>(var);
	}

	/* トークンが数値の場合 */
	if (TokenKind::TK_NUM == token->_kind)
	{
		std::unique_ptr<Node> node = std::make_unique<Node>(std::move(token->_value));
		current_token = std::move(token->_next);
		return node;
	}

	/* どれでもなければエラー */
	error_at("式ではありません", std::move(token->_location));

	/* コンパイルエラー対策、error_at()内でプログラムは終了するためnullptrが返ることはない */
	return nullptr;
}



/**
 * @brief 
 * トークン列をパースして抽象構文木を構築する
 * @param token 
 * @return std::unique_ptr<Function> 
 * @details
 * 次のEBNF規則に従う
 * program = statement*
 */
std::unique_ptr<Function> Node::parse(std::unique_ptr<Token> &&token)
{
	/* 最初の'{'を飛ばす */
	token = std::move(Token::skip(std::move(token), "{"));

	std::unique_ptr<Function> prog = std::make_unique<Function>(std::move(compound_statement(token, std::move(token))), std::move(locals));
	return std::move(prog);
}
