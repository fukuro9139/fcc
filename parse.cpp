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
 * stmt = expr-stmt
 */
node_ptr Node::stmt(const Token *&cur, const Token *tok)
{
	return std::move(expr_stmt(cur, tok));
}

/**
 * @brief
 * expr-stmt = expr ";"
 */
node_ptr Node::expr_stmt(const Token *&cur, const Token *tok)
{
	node_ptr node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, std::move(expr(tok, tok)));
	/* 文は';'で終わるはず */
	cur = Token::skip(tok, ";");
	return std::move(node);
}

/**
 * @brief
 * expr = assign
 */
node_ptr Node::expr(const Token *&cur, const Token *tok)
{
	return std::move(assign(cur, tok));
}

/**
 * @brief
 * assign = equality ("=" assign)?
 */
node_ptr Node::assign(const Token *&cur, const Token *tok)
{
	node_ptr node = std::move(equality(tok, tok));
	if (Token::is_equal(tok, "="))
	{
		node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), std::move(assign(tok, tok->_next.get())));
	}
	cur = tok;
	return std::move(node);
}

/**
 * @brief
 * equality = relational ("==" relational | "!=" relational)*
 */
node_ptr Node::equality(const Token *&cur, const Token *tok)
{
	node_ptr node = std::move(relational(tok, tok));

	for (;;)
	{
		if (Token::is_equal(tok, "=="))
		{
			node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), std::move(relational(tok, tok->_next.get())));
			continue;
		}

		if (Token::is_equal(tok, "!="))
		{
			node = std::make_unique<Node>(NodeKind::ND_NE, std::move(node), std::move(relational(tok, tok->_next.get())));
			continue;
		}

		cur = tok;
		return std::move(node);
	}
}

/**
 * @brief
 * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 */
node_ptr Node::relational(const Token *&cur, const Token *tok)
{
	node_ptr node = std::move(add(tok, tok));

	for (;;)
	{
		if (Token::is_equal(tok, "<"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), std::move(add(tok, tok->_next.get())));
			continue;
		}

		if (Token::is_equal(tok, "<="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), std::move(add(tok, tok->_next.get())));
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (Token::is_equal(tok, ">"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(add(tok, tok->_next.get())), std::move(node));
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (Token::is_equal(tok, ">="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(add(tok, tok->_next.get())), std::move(node));
			continue;
		}

		cur = tok;
		return std::move(node);
	}
}

/**
 * @brief
 * add = mul ("+" mul | "-" mul)*
 */
node_ptr Node::add(const Token *&cur, const Token *tok)
{
	node_ptr node = std::move(mul(tok, tok));

	for (;;)
	{
		if (Token::is_equal(tok, "+"))
		{
			node = std::make_unique<Node>(NodeKind::ND_ADD, std::move(node), std::move(mul(tok, tok->_next.get())));
			continue;
		}

		if (Token::is_equal(tok, "-"))
		{
			node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(node), std::move(mul(tok, tok->_next.get())));
			continue;
		}

		cur = tok;
		return std::move(node);
	}
}

/**
 * @brief
 * mul = unary ("*" unary | "/" unary)*
 */
node_ptr Node::mul(const Token *&cur, const Token *tok)
{
	node_ptr node = std::move(unary(tok, tok));

	for (;;)
	{
		if (Token::is_equal(tok, "*"))
		{
			node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), std::move(unary(tok, tok->_next.get())));
			continue;
		}

		if (Token::is_equal(tok, "/"))
		{
			node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::move(unary(tok, tok->_next.get())));
			continue;
		}

		cur = tok;
		return std::move(node);
	}
}

/**
 * @brief
 * unary = ("+" | "-") unary
 */
node_ptr Node::unary(const Token *&cur, const Token *tok)
{
	if (Token::is_equal(tok, "+"))
	{
		return std::move(unary(cur, tok->_next.get()));
	}

	if (Token::is_equal(tok, "-"))
	{
		return std::make_unique<Node>(NodeKind::ND_NEG, std::move(unary(cur, tok->_next.get())));
	}

	return std::move(primary(cur, tok));
}


/**
 * @brief
 * primary = "(" expr ")" | ident | num
 */
node_ptr Node::primary(const Token *& cur, const Token *tok)
{
	/* トークンが"("なら、"(" expr ")"のはず */
	if(Token::is_equal(tok, "(")){
		node_ptr node = std::move(expr(tok, tok->_next.get()));
		cur = Token::skip(tok, ")");
		return std::move(node);
	}

	/* トークンが識別子の場合 */
	if( TokenKind::TK_IDENT == tok->_kind){
		const std::string name(tok->_loc, tok->_loc + tok->_len);
		node_ptr node = std::make_unique<Node>(name);
		cur = tok->_next.get();
		return std::move(node);
	}

	/* トークンが数値の場合 */
	if( TokenKind::TK_NUM == tok->_kind){
		node_ptr node = std::make_unique<Node>(tok->_val);
		cur = tok->_next.get();
		return std::move(node);
	}

	/* どれでもなければエラー */
	error_at("式ではありません", tok->_loc);

	return std::make_unique_for_overwrite<Node>();
}

/**
 * @brief
 * program = stmt*
 */
node_const_ptr Node::parse(const std::unique_ptr<const Token> tok)
{
	node_ptr head = std::make_unique_for_overwrite<Node>();
	Node *current_node = head.get();
	const Token* current_token = tok.get();
	while(TokenKind::TK_EOF != current_token->_kind){
		current_node->_next = std::move(stmt(current_token, current_token));
		current_node = current_node->_next.get();
	}
	return std::move(head->_next);
}

