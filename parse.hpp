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

/* 関数、変数を表すオブジェクト、名前によって区別する */
/* 単方向リストの形で全オブジェクトを管理する */
struct Object
{
	std::unique_ptr<Object> _next = nullptr; /* 次のオブジェクト */
	std::string _name = "";					 /* 名前 */
	int _offset = 0;						 /* RBPからのオフセット */

	Object();
	Object(std::string && name);

	static const Object *new_lvar(std::string &&name);

	static const Object *find_var(const std::unique_ptr<Token> &token);
};

/* Functionクラスの中でNodeクラスを使いたいので先に宣言 */
class Node;

/* 関数 */
struct Function
{
	std::unique_ptr<Node> _body = nullptr;	/* 関数の表す内容を抽象構文木で表す。根のノードを持つ */
	std::unique_ptr<Object> _locals = nullptr; /* オブジェクトとしての情報 */
	int _stack_size = 0;					/* 使用するスタックの深さ */

	Function();
	Function(std::unique_ptr<Node> &&body, std::unique_ptr<Object> &&locals);

	static int align_to(int &&n, int &&align);
	void assign_lvar_offsets();
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
	const Object *_var = nullptr;	 /* kindがND_VARの場合のみ使う、 オブジェクトの情報*/

	Node();
	Node(NodeKind &&kind);
	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs);
	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs);
	Node(int &&val);
	Node(const Object *var);

	static std::unique_ptr<Function> parse(std::unique_ptr<Token> &&token);

private:
	static std::unique_ptr<Node> statement(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> expression(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> expr_stmt(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> assign(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> equality(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> relational(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> add(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> mul(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> unary(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
	static std::unique_ptr<Node> primary(std::unique_ptr<Token> &current_token, std::unique_ptr<Token> &&token);
};