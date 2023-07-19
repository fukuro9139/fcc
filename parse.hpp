#pragma once

#include "tokenize.hpp"
#include <memory>
#include <string>

/** @brief ノードの種類 */
enum class NodeKind
{
	ND_ADD,		  /* + */
	ND_SUB,		  /* - */
	ND_MUL,		  /* * */
	ND_DIV,		  /* / */
	ND_NEG,		  /* 負の単項 */
	ND_ASSIGN,	  /* = */
	ND_EQ,		  /* == */
	ND_NE,		  /* != */
	ND_LT,		  /* < */
	ND_LE,		  /* <= */
	ND_NUM,		  /* 整数 */
	ND_EXPR_STMT, /* 文 */
	ND_VAR,		  /* 変数 */
};

/**
 * @brief
 * 抽象構文木を構成するノード
 */
class Node
{
public:
	NodeKind _kind = NodeKind::ND_EXPR_STMT; /* ノードの種類*/
	std::unique_ptr<Node> _lhs = nullptr;	 /* 左辺 */
	std::unique_ptr<Node> _rhs = nullptr;	 /* 右辺 */
	std::unique_ptr<Node> _next = nullptr;	 /* ノードが木のrootである場合、次の木のrootノード */
	int _val = 0;							 /* kindがND_NUMの場合のみ使う、数値の値 */
	std::string _name = "";					 /* kindがND_VARの場合のみ使う、 変数の名前*/

	Node();
	Node(const NodeKind &kind);
	Node(const NodeKind &kind, std::unique_ptr<Node> lhs, std::unique_ptr<Node> rhs);
	Node(const NodeKind &kind, std::unique_ptr<Node> lhs);
	Node(const int &val);
	Node(const std::string &name);

	~Node();

	static std::unique_ptr<const Node> parse(const std::unique_ptr<const Token> token);

private:
	static std::unique_ptr<Node> statement(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> expression(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> expr_stmt(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> assign(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> equality(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> relational(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> add(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> mul(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> unary(const Token *&current_token, const Token *token);
	static std::unique_ptr<Node> primary(const Token *&current_token, const Token *token);
};