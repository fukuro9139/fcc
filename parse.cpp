#include "parse.hpp"

using std::unique_ptr;

/** 変数や関数オブジェクトのリスト。 @n パース中に生成される全てのローカル変数はこのリストに連結される。 */
static unique_ptr<Object> locals = nullptr;

/****************/
/* Object Class */
/****************/

Object::Object() = default;

Object::Object(std::string &&name) : _name(std::move(name))
{
}

const Object *Object::new_lvar(std::string &&name)
{
	unique_ptr<Object> var = std::make_unique<Object>(std::move(name));
	var->_next = std::move(locals);
	locals = std::move(var);
	return locals.get();
}

const Object *Object::find_var(const unique_ptr<Token> &token)
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

/******************/
/* Function Class */
/******************/

Function::Function() = default;

Function::Function(unique_ptr<Node> &&body, unique_ptr<Object> &&locals)
	: _body(std::move(body)), _locals(std::move(locals))
{
}

int Function::align_to(int &&n, int &&align)
{
	return (n + align - 1) / align * align;
}

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

Node::Node() = default;

Node::Node(NodeKind &&kind, const std::string::const_iterator &location)
	: _kind(std::move(kind)), _location(location)
{
}

Node::Node(NodeKind &&kind, unique_ptr<Node> &&lhs, const std::string::const_iterator &location)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _location(location)
{
}

Node::Node(int &&val, const std::string::const_iterator &location)
	: _kind(NodeKind::ND_NUM), _val(std::move(val)), _location(location)
{
}

Node::Node(const Object *var, const std::string::const_iterator &location)
	: _kind(NodeKind::ND_VAR), _var(std::move(var)), _location(location)
{
}

Node::Node(NodeKind &&kind, unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, const std::string::const_iterator &location)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _rhs(std::move(rhs)), _location(location)
{
}

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
		current_token = std::move(Token::skip(std::move(current_token), ")"));
		node->_then = statement(current_token, std::move(current_token));
		/* else節が存在する */
		if (Token::is_equal(current_token, "else"))
		{
			node->_else = std::move(statement(current_token, std::move(current_token->_next)));
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
		node->_init = expr_stmt(current_token, std::move(current_token));

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
		current_token = std::move(Token::skip(std::move(current_token), ")"));
		node->_then = statement(next_token, std::move(current_token));
		return node;
	}

	/* ブロック */
	if (Token::is_equal(current_token, "{"))
	{
		return compound_statement(next_token, std::move(current_token->_next));
	}

	/* 式 */
	return expr_stmt(next_token, std::move(current_token));
}

unique_ptr<Node> Node::compound_statement(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{

	unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token->_location);

	unique_ptr<Node> head = std::make_unique_for_overwrite<Node>();
	Node *cur = head.get();
	/* '}'が出てくるまでstatementをパースする */
	while (!Token::is_equal(current_token, "}"))
	{
		cur->_next = statement(current_token, std::move(current_token));
		cur = cur->_next.get();
	}

	/* ダミーのheadからの次のトークン以降を切り離し、新しいノードのbodyに繋ぐ*/
	node->_body = std::move(head->_next);
	/* '}' の次のトークンからパースを続ける*/
	next_token = std::move(current_token->_next);
	return node;
}

unique_ptr<Node> Node::expr_stmt(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	/* 空のstatementに対応 */
	if (Token::is_equal(current_token, ";"))
	{
		next_token = std::move(current_token->_next);
		return std::make_unique<Node>(NodeKind::ND_BLOCK, current_token->_location);
	}
	unique_ptr<Node> node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, current_token->_location);
	node->_lhs = expression(current_token, std::move(current_token));

	/* expression-statementは';'で終わるはず */
	next_token = Token::skip(std::move(current_token), ";");
	return node;
}

unique_ptr<Node> Node::expression(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	return assign(next_token, std::move(current_token));
}

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

unique_ptr<Node> Node::add(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
	unique_ptr<Node> node = mul(current_token, std::move(current_token));

	for (;;)
	{
		auto location = current_token->_location;
		if (Token::is_equal(current_token, "+"))
		{
			node = std::make_unique<Node>(NodeKind::ND_ADD, std::move(node), mul(current_token, std::move(current_token->_next)), location);
			continue;
		}

		if (Token::is_equal(current_token, "-"))
		{
			node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(node), mul(current_token, std::move(current_token->_next)), location);
			continue;
		}

		next_token = std::move(current_token);
		return node;
	}
}

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

	return primary(next_token, std::move(current_token));
}

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
		const Object *var = Object::find_var(current_token);
		if (!var)
		{
			var = Object::new_lvar(std::string(current_token->_location, current_token->_location + current_token->_length));
		}
		next_token = std::move(current_token->_next);
		return std::make_unique<Node>(var, current_token->_location);
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

unique_ptr<Function> Node::parse(unique_ptr<Token> &&token)
{
	/* 最初の'{'を飛ばす */
	token = std::move(Token::skip(std::move(token), "{"));

	unique_ptr<Function> prog = std::make_unique<Function>(std::move(compound_statement(token, std::move(token))), std::move(locals));
	return std::move(prog);
}
