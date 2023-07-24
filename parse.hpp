/**
 * @file parse.hpp
 * @author K.Fukunaga
 * @brief C言語の再帰下降構文解析を行うパーサーの定義
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <memory>
#include <string>
#include "tokenize.hpp"
#include "type.hpp"

/** @brief 変数を表すオブジェクトリスト、各変数は名前によって区別する */
struct Object
{
	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	std::unique_ptr<Object> _next = nullptr; /*!< 次のオブジェクト */
	std::string _name = "";					 /*!< 名前 */
	std::shared_ptr<Type> _ty = nullptr;	 /* 型 */
	int _offset = 0;						 /*!< RBPからのオフセット */

	/*****************/
	/* コンストラクタ */
	/*****************/

	Object();
	Object(std::string &&name);

	/**********************/
	/* メンバ関数 (public) */
	/**********************/

	static const Object *new_lvar(std::shared_ptr<Type> &&ty);
	static const Object *find_var(const std::unique_ptr<Token> &token);
};

/* Functionクラスの中でNodeクラスを使いたいので先に宣言 */
class Node;

/** @brief 関数を表す */
struct Function
{
	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	std::unique_ptr<Node> _body = nullptr;	   /*!< 関数の表す内容を抽象構文木で表す。根のノードを持つ */
	std::unique_ptr<Object> _locals = nullptr; /*!< オブジェクトとしての情報 */
	int _stack_size = 0;					   /*!< 使用するスタックの深さ */

	/*****************/
	/* コンストラクタ */
	/*****************/

	Function();
	Function(std::unique_ptr<Node> &&body, std::unique_ptr<Object> &&locals);

	/**********************/
	/* メンバ関数 (public) */
	/**********************/

	void assign_lvar_offsets();

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static int align_to(int &&n, int &&align);
};

/** @brief ノードの種類 */
enum class NodeKind
{
	ND_ADD,		  /*!< + */
	ND_SUB,		  /*!< - */
	ND_MUL,		  /*!< * */
	ND_DIV,		  /*!< / */
	ND_NEG,		  /*!< 負の単項 */
	ND_ASSIGN,	  /*!< = */
	ND_ADDR,	  /* 単項 & */
	ND_DEREF,	  /* 単項* */
	ND_RETURN,	  /*!< return */
	ND_IF,		  /*!< if */
	ND_FOR,		  /*!< for or while*/
	ND_BLOCK,	  /*!< {...} */
	ND_FUNCALL,	  /* 関数呼び出し */
	ND_EQ,		  /*!< == */
	ND_NE,		  /*!< != */
	ND_LT,		  /*!< < */
	ND_LE,		  /*!< <= */
	ND_NUM,		  /*!< 整数 */
	ND_EXPR_STMT, /*!< 文 */
	ND_VAR,		  /*!< 変数 */
};

/** @brief 抽象構文木を構成するノード */
class Node
{
public:
	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	NodeKind _kind = NodeKind::ND_EXPR_STMT; /*!< ノードの種類*/
	std::unique_ptr<Node> _next = nullptr;	 /*!< ノードが木のrootである場合、次の木のrootノード */
	std::shared_ptr<Type> _ty = nullptr;	 /*!< 型情報 e.g. int or pointer to int */

	std::unique_ptr<Node> _lhs = nullptr; /*!< 左辺 */
	std::unique_ptr<Node> _rhs = nullptr; /*!< 右辺 */

	/* if or for */
	std::unique_ptr<Node> _condition = nullptr; /*!< if文の条件 */
	std::unique_ptr<Node> _then = nullptr;		/*!< trueのときに行う式 */
	std::unique_ptr<Node> _else = nullptr;		/*!< falseのとき行う式 */
	std::unique_ptr<Node> _init = nullptr;		/*!< 初期化処理 */
	std::unique_ptr<Node> _inc = nullptr;		/*!< 加算処理 */

	/* ブロック */
	std::unique_ptr<Node> _body = nullptr; /*!< ブロック内{...}には複数の式を入れられる */

	/* 関数呼び出し */
	std::string func_name = "";	 /*!< kindがND_FUNCALLの場合のみ使う、呼び出す関数の名前  */
	std::unique_ptr<Node> _args; /*!< 引数  */

	int _val = 0;				  /*!< kindがND_NUMの場合のみ使う、数値の値 */
	const Object *_var = nullptr; /*!< kindがND_VARの場合のみ使う、 オブジェクトの情報*/

	std::string::const_iterator _location; /* ノードと対応する入力文字列の位置 */

	/*****************/
	/* コンストラクタ */
	/*****************/

	Node();
	Node(NodeKind &&kind, const std::string::const_iterator &location);
	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, const std::string::const_iterator &location);
	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs, const std::string::const_iterator &location);
	Node(int &&val, const std::string::const_iterator &location);
	Node(const Object *var, const std::string::const_iterator &location);

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static std::unique_ptr<Function> parse(std::unique_ptr<Token> &&token);

private:
	/***************************/
	/* 静的メンバ関数 (private) */
	/***************************/

	static std::unique_ptr<Node> statement(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> compound_statement(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::shared_ptr<Type> declspec(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::shared_ptr<Type> declarator(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token, std::shared_ptr<Type> ty);
	static std::unique_ptr<Node> declaration(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> expression(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> expression_statement(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> assign(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> equality(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> relational(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> add(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> mul(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> unary(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> primary(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::unique_ptr<Node> function_call(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	static std::unique_ptr<Node> new_add(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, std::string::const_iterator &location);
	static std::unique_ptr<Node> new_sub(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, std::string::const_iterator &location);
};