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
	/* メンバ変数 (public) */

	std::unique_ptr<Object> _next; /*!< 次のオブジェクト */
	std::string _name;			   /*!< 名前 */
	std::shared_ptr<Type> _ty;	   /* 型 */
	int _offset;				   /*!< RBPからのオフセット */

	/* コンストラクタ */

	Object() = default;
	Object(std::string &&name) : _name(std::move(name)){};
	Object(std::string &&name, std::unique_ptr<Object> &&next, std::shared_ptr<Type> &&ty) : _name(std::move(name)), _next(std::move(next)), _ty(std::move(ty)){};

	/* メンバ関数 (public) */

	static const Object *new_lvar(std::shared_ptr<Type> &&ty);
	static const Object *find_var(const std::unique_ptr<Token> &token);
};

/* Functionクラスの中でNodeクラスを使いたいので先に宣言 */
class Node;

/** @brief 関数を表す */
struct Function
{

	/* メンバ変数 (public) */

	std::unique_ptr<Node> _body;	 /*!< 関数の表す内容を抽象構文木で表す。根のノードを持つ */
	std::unique_ptr<Object> _locals; /*!< オブジェクトとしての情報 */
	int _stack_size;				 /*!< 使用するスタックの深さ */

	std::unique_ptr<Function> _next; /*!< 関数リストの次の関数 */
	std::string _name;				 /*!< 関数名 */

	/* コンストラクタ */

	Function() = default;
	Function(std::unique_ptr<Node> &&body, std::unique_ptr<Object> &&locals) : _body(std::move(body)), _locals(std::move(locals)){};

	/* メンバ関数 (public) */

	/* 静的メンバ関数 (public) */

	static int align_to(const int &n, const int &align);
	static void assign_lvar_offsets(const std::unique_ptr<Function> &prog);
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
	/* メンバ変数 (public) */

	NodeKind _kind;				 /*!< ノードの種類*/
	std::unique_ptr<Node> _next; /*!< ノードが木のrootである場合、次の木のrootノード */
	std::shared_ptr<Type> _ty;	 /*!< 型情報 e.g. int or pointer to int */

	std::unique_ptr<Node> _lhs; /*!< 左辺 */
	std::unique_ptr<Node> _rhs; /*!< 右辺 */

	/* if or for */
	std::unique_ptr<Node> _condition; /*!< if文の条件 */
	std::unique_ptr<Node> _then;	  /*!< trueのときに行う式 */
	std::unique_ptr<Node> _else;	  /*!< falseのとき行う式 */
	std::unique_ptr<Node> _init;	  /*!< 初期化処理 */
	std::unique_ptr<Node> _inc;		  /*!< 加算処理 */

	/* ブロック */
	std::unique_ptr<Node> _body; /*!< ブロック内{...}には複数の式を入れられる */

	/* 関数呼び出し */
	std::string _func_name;		 /*!< kindがND_FUNCALLの場合のみ使う、呼び出す関数の名前  */
	std::unique_ptr<Node> _args; /*!< 引数  */

	int _val = 0;				  /*!< kindがND_NUMの場合のみ使う、数値の値 */
	const Object *_var = nullptr; /*!< kindがND_VARの場合のみ使う、 オブジェクトの情報*/

	int _location; /* ノードと対応する入力文字列の位置 */

	/* コンストラクタ */

	Node() = default;

	Node(NodeKind &&kind, const int &location)
		: _kind(std::move(kind)), _location(location){};

	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, const int &location)
		: _kind(std::move(kind)), _lhs(std::move(lhs)), _rhs(std::move(rhs)), _location(location){};

	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs, const int &location)
		: _kind(std::move(kind)), _lhs(std::move(lhs)), _location(location){};

	Node(const int &val, const int &location)
		: _kind(NodeKind::ND_NUM), _val(val), _location(location){};

	Node(const Object *var, const int &location)
		: _kind(NodeKind::ND_VAR), _var(var), _location(location){};

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
	static std::unique_ptr<Function> function_definition(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::shared_ptr<Type> declspec(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::shared_ptr<Type> declarator(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token, std::shared_ptr<Type> ty);
	static std::unique_ptr<Node> declaration(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
	static std::shared_ptr<Type> type_suffix(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token, std::shared_ptr<Type> &&ty);
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

	static std::unique_ptr<Node> new_add(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, const int &location);
	static std::unique_ptr<Node> new_sub(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, const int &location);
};

/**
 * @brief 'n'を切り上げて最も近い'align'の倍数にする。
 *
 * @param n 切り上げ対象
 * @param align 基数
 * @return 切り上げた結果
 * @details 例：align_to(5,8) = 8, align_to(11,8) = 16
 */
inline int Function::align_to(const int &n, const int &align)
{
	return (n + align - 1) / align * align;
}