#include "parse.hpp"

using node_ptr = std::unique_ptr<Node>;
using token_ptr = std::unique_ptr<Token>;
using obj_ptr = std::unique_ptr<Object>;

/* 変数や関数オブジェクトのリスト */
/* パース中に生成される全てのローカル変数はこのリストに連結される。 */
static obj_ptr locals = nullptr;

/****************************** Object class ******************************/

/* コンストラクタ */
Object::Object() = default;

Object::Object(std::string &&name) : _name(std::move(name))
{
}

/**
 * @brief
 * nameを名前として持つ新しい変数を生成してlocalsの先頭に追加する。
 * 生成した変数へのポインタを返す。
 */
const Object *Object::new_lvar(std::string &&name)
{
	obj_ptr var = std::make_unique<Object>(std::move(name));
	var->_next = std::move(locals);
	locals = std::move(var);
	return locals.get();
}

/**
 * @brief
 * 変数を名前で検索する。見つからなかった場合はNULLを返す。
 */
const Object *Object::find_var(const token_ptr &token)
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

/**
 * @brief
 * 'n'を切り上げて最も近い'align'の倍数にする。
 * 例：align_to(5,8) = 8, align_to(11,8) = 16
 */
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

Node::Node(NodeKind &&kind, node_ptr &&lhs)
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

Node::Node(NodeKind &&kind, node_ptr &&lhs, node_ptr &&rhs)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _rhs(std::move(rhs))
{
}

/**
 * @brief
 * statement = "return" expression ";" | expression-statement
 */
node_ptr Node::statement(token_ptr &current_token, token_ptr &&token)
{
	if (Token::is_equal(token, "return"))
	{
		node_ptr node = std::make_unique<Node>(NodeKind::ND_RETURN, expression(token, std::move(token->_next)));
		/* 文は';'で終わるはず */
		current_token = Token::skip(std::move(token), ";");
		return node;
	}

	return expr_stmt(current_token, std::move(token));
}

/**
 * @brief
 * expression-statement = expression ";"
 */
node_ptr Node::expr_stmt(token_ptr &current_token, token_ptr &&token)
{
	node_ptr node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, expression(token, std::move(token)));
	/* 文は';'で終わるはず */
	current_token = Token::skip(std::move(token), ";");
	return node;
}

/**
 * @brief
 * expression = assign
 */
node_ptr Node::expression(token_ptr &current_token, token_ptr &&token)
{
	return assign(current_token, std::move(token));
}

/**
 * @brief
 * assign = equality ("=" assign)?
 */
node_ptr Node::assign(token_ptr &current_token, token_ptr &&token)
{
	node_ptr node = equality(token, std::move(token));
	if (Token::is_equal(token, "="))
	{
		node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), assign(token, std::move(token->_next)));
	}
	current_token = std::move(token);
	return node;
}

/**
 * @brief
 * equality = relational ("==" relational | "!=" relational)*
 */
node_ptr Node::equality(token_ptr &current_token, token_ptr &&token)
{
	node_ptr node = relational(token, std::move(token));

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

/**
 * @brief
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 */
node_ptr Node::relational(token_ptr &current_token, token_ptr &&token)
{
	node_ptr node = add(token, std::move(token));

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
 * add = mul ("+" mul | "-" mul)*
 */
node_ptr Node::add(token_ptr &current_token, token_ptr &&token)
{
	node_ptr node = mul(token, std::move(token));

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

/**
 * @brief
 * mul = unary ("*" unary | "/" unary)*
 */
node_ptr Node::mul(token_ptr &current_token, token_ptr &&token)
{
	node_ptr node = unary(token, std::move(token));

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
 * unary = ("+" | "-") unary
 */
node_ptr Node::unary(token_ptr &current_token, token_ptr &&token)
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
 * primary = "(" expression ")" | ident | num
 */
node_ptr Node::primary(token_ptr &current_token, token_ptr &&token)
{
	/* トークンが"("なら、"(" expression ")"のはず */
	if (Token::is_equal(token, "("))
	{
		node_ptr node = expression(token, std::move(token->_next));
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
		node_ptr node = std::make_unique<Node>(std::move(token->_value));
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
 * program = statement*
 */
std::unique_ptr<Function> Node::parse(std::unique_ptr<Token> &&token)
{
	/* スタート地点としてダミーのノードを作る */
	node_ptr head = std::make_unique_for_overwrite<Node>();
	Node *current_node = head.get();

	while (TokenKind::TK_EOF != token->_kind)
	{
		/* 現在の文から新しく抽象構文木を構築して木の根を現在のノードにに繋ぐ */
		current_node->_next = statement(token, std::move(token));
		/* 現在のノードを進める */
		current_node = current_node->_next.get();
	}
	/* ダミーの次のノード以降を切り離してFunctionのbodyに繋ぐ */
	std::unique_ptr<Function> prog = std::make_unique<Function>(std::move(head->_next), std::move(locals));
	return std::move(prog);
}
