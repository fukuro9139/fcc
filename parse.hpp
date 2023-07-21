#pragma once

#include "tokenize.hpp"
#include <memory>
#include <string>

/**
 * @brief 関数、変数を表すオブジェクト、名前によって区別する
 */
struct Object
{

	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	std::unique_ptr<Object> _next = nullptr; /*!< 次のオブジェクト */
	std::string _name = "";					 /*!< 名前 */
	int _offset = 0;						 /*!< RBPからのオフセット */

	/*****************/
	/* コンストラクタ */
	/*****************/

	/**
	 * @brief Construct a new Object object
	 *
	 */
	Object();

	/**
	 * @brief Construct a new Object object
	 *
	 * @param name オブジェクトの名前
	 */
	Object(std::string &&name);

	/**********************/
	/* メンバ関数 (public) */
	/**********************/

	/**
	 * @brief nameを名前として持つ新しい変数を生成してlocalsの先頭に追加する。
	 *
	 * @param name オブジェクトの名前
	 * @return const Object* 生成した変数へのポインタを返す。
	 */
	static const Object *new_lvar(std::string &&name);

	/**
	 * @brief 変数を名前で検索する。見つからなかった場合はNULLを返す。
	 *
	 * @param token 検索対象のトークン
	 * @return const Object* 既出の変数であればその変数オブジェクトへのポインタ。新規の場合はNULL。
	 */
	static const Object *find_var(const std::unique_ptr<Token> &token);
};

/* Functionクラスの中でNodeクラスを使いたいので先に宣言 */
class Node;

/**
 * @brief 関数を表す
 *
 */
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

	/**
	 * @brief Construct a new Function object
	 *
	 */
	Function();

	/**
	 * @brief Construct a new Function object
	 *
	 * @param body 関数内で行う処理
	 * @param locals 関数内で使うローカル変数
	 */
	Function(std::unique_ptr<Node> &&body, std::unique_ptr<Object> &&locals);

	/**********************/
	/* メンバ関数 (public) */
	/**********************/

	/**
	 * @brief プログラムに必要なスタックサイズを計算する
	 *
	 */
	void assign_lvar_offsets();

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	/**
	 * @brief 'n'を切り上げて最も近い'align'の倍数にする。
	 *
	 * @param n 切り上げ対象
	 * @param align 基数
	 * @return int 切り上げた結果
	 * @details 例：align_to(5,8) = 8, align_to(11,8) = 16
	 */
	static int align_to(int &&n, int &&align);
};

/** @brief ノードの種類
 *
 */
enum class NodeKind
{
	ND_ADD,		  /*!< + */
	ND_SUB,		  /*!< - */
	ND_MUL,		  /*!< * */
	ND_DIV,		  /*!< / */
	ND_NEG,		  /*!< 負の単項 */
	ND_ASSIGN,	  /*!< = */
	ND_RETURN,	  /*!< return */
	ND_IF,		  /*!< if */
	ND_FOR,		  /*!< for or while*/
	ND_BLOCK,	  /*!< {...} */
	ND_EQ,		  /*!< == */
	ND_NE,		  /*!< != */
	ND_LT,		  /*!< < */
	ND_LE,		  /*!< <= */
	ND_NUM,		  /*!< 整数 */
	ND_EXPR_STMT, /*!< 文 */
	ND_VAR,		  /*!< 変数 */
};

/**
 * @brief 抽象構文木を構成するノード
 *
 */
class Node
{
public:
	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	NodeKind _kind = NodeKind::ND_EXPR_STMT; /*!< ノードの種類*/
	std::unique_ptr<Node> _next = nullptr;	 /*!< ノードが木のrootである場合、次の木のrootノード */

	std::unique_ptr<Node> _lhs = nullptr; /*!< 左辺 */
	std::unique_ptr<Node> _rhs = nullptr; /*!< 右辺 */

	/* if or for */
	std::unique_ptr<Node> _condition = nullptr; /*!< if文の条件 */
	std::unique_ptr<Node> _then = nullptr;		/*!< trueのときに行う式 */
	std::unique_ptr<Node> _else = nullptr;		/*!< falseのとき行う式 */
	std::unique_ptr<Node> _init = nullptr;		/* 初期化処理 */
	std::unique_ptr<Node> _inc = nullptr;		/* 加算処理 */

	/* ブロック */
	std::unique_ptr<Node> _body = nullptr; /*!< ブロック内{...}には複数の式を入れられる */

	int _val = 0;				  /*!< kindがND_NUMの場合のみ使う、数値の値 */
	const Object *_var = nullptr; /*!< kindがND_VARの場合のみ使う、 オブジェクトの情報*/

	/*****************/
	/* コンストラクタ */
	/*****************/

	/**
	 * @brief Construct a new Node object
	 *
	 */
	Node();

	/**
	 * @brief Construct a new Node object
	 *
	 * @param kind ノードの種類
	 */
	Node(NodeKind &&kind);

	/**
	 * @brief Construct a new Node object
	 *
	 * @param kind ノードの種類
	 * @param lhs 左辺のノード
	 * @param rhs 右辺のノード
	 */
	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs);

	/**
	 * @brief Construct a new Node object
	 *
	 * @param kind ノードの種類
	 * @param lhs 左辺のノード
	 */
	Node(NodeKind &&kind, std::unique_ptr<Node> &&lhs);

	/**
	 * @brief Construct a new Node object
	 *
	 * @param val ノードが表す数値
	 */
	Node(int &&val);

	/**
	 * @brief Construct a new Node object
	 *
	 * @param var ノードが表す変数
	 */
	Node(const Object *var);

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	/**
	 * @brief トークン列をパースして抽象構文木を構築する
	 *
	 * @param token 先頭のトークン
	 * @return std::unique_ptr<Function>
	 * @details program = statement*
	 */
	static std::unique_ptr<Function> parse(std::unique_ptr<Token> &&token);

private:
	/***************************/
	/* 静的メンバ関数 (private) */
	/***************************/

	/**
	 * @brief statement ノードを生成する。
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n
	 * statement = "return" expression ";"
	 * 			 | "if" "(" expression ")" statement ("else" statement)?
	 * 			 | "for" "(" expression-statement expression? ";" expression? ")" stmt
	 * 			 | "while" "(" expr ")" stmt
	 * 			 | "{" compound-stmt
	 * 			 | expression-statement
	 */
	static std::unique_ptr<Node> statement(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief compound_statementノードを生成する。
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n compound-stmt = stmt* "}"
	 */
	static std::unique_ptr<Node> compound_statement(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief expressionノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n expression = assign
	 */
	static std::unique_ptr<Node> expression(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief expression-statementノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n expression-statement = expression? ';'
	 */
	static std::unique_ptr<Node> expr_stmt(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief assignノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n assign = equality ("=" assign)?
	 */
	static std::unique_ptr<Node> assign(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief equalityノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n equality = relational ("==" relational | "!=" relational)*
	 */
	static std::unique_ptr<Node> equality(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief relationalノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n relational = add ("<" add | "<=" add | ">" add | ">=" add)*
	 */
	static std::unique_ptr<Node> relational(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief addノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n add = mul ("+" mul | "-" mul)*
	 */
	static std::unique_ptr<Node> add(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief mulノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n mul = unary ("*" unary | "/" unary)*
	 */
	static std::unique_ptr<Node> mul(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief unaryノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n unary = ("+" | "-") unary
	 */
	static std::unique_ptr<Node> unary(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);

	/**
	 * @brief primaryノードを生成する
	 *
	 * @param next_token 次に処理するべきトークンを返すための参照
	 * @param current_token 現在処理しているトークン
	 * @return std::unique_ptr<Node>
	 * @details 下記のEBNF規則に従う。 @n primary = "(" expression ")" | ident | num
	 */
	static std::unique_ptr<Node> primary(std::unique_ptr<Token> &next_token, std::unique_ptr<Token> &&current_token);
};