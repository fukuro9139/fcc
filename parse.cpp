#include "parse.hpp"

using node_ptr = std::unique_ptr<Node>;
using node_const_ptr = std::unique_ptr<const Node>;

Node::Node() = default;

Node::Node(const NodeKind &kind)
	: _kind(kind)
{
}

Node::Node(const NodeKind &kind, node_ptr lhs)
	: _kind(kind), _lhs(std::move(lhs))
{
}

Node::Node(const int &val)
	: _kind(NodeKind::ND_NUM), _val(val)
{
}

Node::Node(const std::string &name)
	: _kind(NodeKind::ND_VAR), _name(name)
{
}

Node::Node(const NodeKind &kind, node_ptr lhs, node_ptr rhs)
	: _kind(kind), _lhs(std::move(lhs)), _rhs(std::move(rhs))
{
}

Node::~Node() = default;

/**
 * @brief
 * statement = expression-statement
 */
node_ptr Node::statement(const Token *&current_token, const Token *token)
{
	return std::move(expr_stmt(current_token, token));
}

/**
 * @brief
 * expression-statement = expression ";"
 */
node_ptr Node::expr_stmt(const Token *&current_token, const Token *token)
{
	node_ptr node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, std::move(expression(token, token)));
	/* 文は';'で終わるはず */
	current_token = Token::skip(token, ";");
	return std::move(node);
}

/**
 * @brief
 * expression = assign
 */
node_ptr Node::expression(const Token *&current_token, const Token *token)
{
	return std::move(assign(current_token, token));
}

/**
 * @brief
 * assign = equality ("=" assign)?
 */
node_ptr Node::assign(const Token *&current_token, const Token *token)
{
	node_ptr node = std::move(equality(token, token));
	if (Token::is_equal(token, "="))
	{
		node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), std::move(assign(token, token->_next.get())));
	}
	current_token = token;
	return std::move(node);
}

/**
 * @brief
 * equality = relational ("==" relational | "!=" relational)*
 */
node_ptr Node::equality(const Token *&current_token, const Token *token)
{
	node_ptr node = std::move(relational(token, token));

	for (;;)
	{
		if (Token::is_equal(token, "=="))
		{
			node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), std::move(relational(token, token->_next.get())));
			continue;
		}

		if (Token::is_equal(token, "!="))
		{
			node = std::make_unique<Node>(NodeKind::ND_NE, std::move(node), std::move(relational(token, token->_next.get())));
			continue;
		}

		current_token = token;
		return std::move(node);
	}
}

/**
 * @brief
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 */
node_ptr Node::relational(const Token *&current_token, const Token *token)
{
	node_ptr node = std::move(add(token, token));

	for (;;)
	{
		if (Token::is_equal(token, "<"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), std::move(add(token, token->_next.get())));
			continue;
		}

		if (Token::is_equal(token, "<="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), std::move(add(token, token->_next.get())));
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (Token::is_equal(token, ">"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(add(token, token->_next.get())), std::move(node));
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (Token::is_equal(token, ">="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(add(token, token->_next.get())), std::move(node));
			continue;
		}

		current_token = token;
		return std::move(node);
	}
}

/**
 * @brief
 * add = mul ("+" mul | "-" mul)*
 */
node_ptr Node::add(const Token *&current_token, const Token *token)
{
	node_ptr node = std::move(mul(token, token));

	for (;;)
	{
		if (Token::is_equal(token, "+"))
		{
			node = std::make_unique<Node>(NodeKind::ND_ADD, std::move(node), std::move(mul(token, token->_next.get())));
			continue;
		}

		if (Token::is_equal(token, "-"))
		{
			node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(node), std::move(mul(token, token->_next.get())));
			continue;
		}

		current_token = token;
		return std::move(node);
	}
}

/**
 * @brief
 * mul = unary ("*" unary | "/" unary)*
 */
node_ptr Node::mul(const Token *&current_token, const Token *token)
{
	node_ptr node = std::move(unary(token, token));

	for (;;)
	{
		if (Token::is_equal(token, "*"))
		{
			node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), std::move(unary(token, token->_next.get())));
			continue;
		}

		if (Token::is_equal(token, "/"))
		{
			node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::move(unary(token, token->_next.get())));
			continue;
		}

		current_token = token;
		return std::move(node);
	}
}

/**
 * @brief
 * unary = ("+" | "-") unary
 */
node_ptr Node::unary(const Token *&current_token, const Token *token)
{
	if (Token::is_equal(token, "+"))
	{
		return std::move(unary(current_token, token->_next.get()));
	}

	if (Token::is_equal(token, "-"))
	{
		return std::make_unique<Node>(NodeKind::ND_NEG, std::move(unary(current_token, token->_next.get())));
	}

	return std::move(primary(current_token, token));
}

/**
 * @brief
 * primary = "(" expression ")" | ident | num
 */
node_ptr Node::primary(const Token *&current_token, const Token *token)
{
	/* トークンが"("なら、"(" expression ")"のはず */
	if (Token::is_equal(token, "("))
	{
		node_ptr node = std::move(expression(token, token->_next.get()));
		current_token = Token::skip(token, ")");
		return std::move(node);
	}

	/* トークンが識別子の場合 */
	if (TokenKind::TK_IDENT == token->_kind)
	{
		const std::string name(token->_location, token->_location + token->_length);
		node_ptr node = std::make_unique<Node>(name);
		current_token = token->_next.get();
		return std::move(node);
	}

	/* トークンが数値の場合 */
	if (TokenKind::TK_NUM == token->_kind)
	{
		node_ptr node = std::make_unique<Node>(token->_value);
		current_token = token->_next.get();
		return std::move(node);
	}

	/* どれでもなければエラー */
	error_at("式ではありません", token->_location);

	return std::make_unique_for_overwrite<Node>();
}

/**
 * @brief
 * program = statement*
 */
node_const_ptr Node::parse(const std::unique_ptr<const Token> token)
{
	/* スタート地点としてダミーのノードを作る */
	node_ptr head = std::make_unique_for_overwrite<Node>();
	Node *current_node = head.get();
	const Token *current_token = token.get();
	while (TokenKind::TK_EOF != current_token->_kind)
	{
		/* 現在の文から新しく抽象構文木を構築して木の根を現在のノードにに繋ぐ */
		current_node->_next = std::move(statement(current_token, current_token));
		/* 現在のノードを進める */
		current_node = current_node->_next.get();
	}
	/* ダミーの次のノード以降を切り離して返す */
	return std::move(head->_next);
}
