#include "parse.hpp"

using node_ptr = std::unique_ptr<Node>;
using token_ptr = std::unique_ptr<Token>;

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

Node::Node(std::string &&name)
	: _kind(NodeKind::ND_VAR), _name(std::move(name))
{
}

Node::Node(NodeKind &&kind, node_ptr &&lhs, node_ptr &&rhs)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _rhs(std::move(rhs))
{
}

/**
 * @brief
 * statement = expression-statement
 */
node_ptr Node::statement(token_ptr &current_token, token_ptr &&token)
{
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
		node_ptr node = std::make_unique<Node>(std::string(token->_location, token->_location + token->_length));
		current_token = std::move(token->_next);
		return node;
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
node_ptr Node::parse(std::unique_ptr<Token> &&token)
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
	/* ダミーの次のノード以降を切り離して返す */
	return std::move(head->_next);
}
