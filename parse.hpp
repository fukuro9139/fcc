/**
 * @file parse.hpp
 * @author K.Fukunaga
 * @brief C言語の再帰下降構文解析を行うパーサーの定義
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include "tokenize.hpp"
#include "object.hpp"
#include "type.hpp"
#include "common.hpp"

/**
 * @brief 構造体のメンバーを表すクラス
 *
 */
struct Member
{
	shared_ptr<Member> _next; /*!< 次のメンバ */
	shared_ptr<Type> _ty;	  /*!< 型情報 e.g. int or pointer to int */
	Token *_token = nullptr;  /*!< 対応するトークン */
	int _idx = 0;			  /*!< 何番目の要素か */
	int _offset = 0;		  /*!< RBPからのオフセット  */
};

/**
 * @brief ノードの種類
 *
 */
enum class NodeKind
{
	ND_NULL_EXPR, /*!< 何もしない */
	ND_ADD,		  /*!< + */
	ND_SUB,		  /*!< - */
	ND_MUL,		  /*!< * */
	ND_DIV,		  /*!< / */
	ND_NEG,		  /*!< 負の単項 */
	ND_ASSIGN,	  /*!< = */
	ND_COND,	  /*!< 三項演算子 */
	ND_COMMA,	  /*!< , */
	ND_MEMBER,	  /*!< 構造体のメンバ  */
	ND_ADDR,	  /* 単項 & */
	ND_DEREF,	  /* 単項* */
	ND_NOT,		  /*!< ! */
	ND_BITNOT,	  /*!< ~ */
	ND_LOGAND,	  /*!< && */
	ND_LOGOR,	  /*!< || */
	ND_RETURN,	  /*!< return */
	ND_IF,		  /*!< if */
	ND_FOR,		  /*!< for or while*/
	ND_SWITCH,	  /*!< switch文 */
	ND_CASE,	  /*!< case */
	ND_BLOCK,	  /*!< {...} */
	ND_GOTO,	  /*!< goto */
	ND_LABEL,	  /*!< ラベル */
	ND_FUNCALL,	  /*!< 関数呼び出し */
	ND_MOD,		  /*!< % */
	ND_BITAND,	  /*!< & */
	ND_BITOR,	  /*!< | */
	ND_BITXOR,	  /*!< ^ */
	ND_SHL,		  /*!< << */
	ND_SHR,		  /*!< >> */
	ND_EQ,		  /*!< == */
	ND_NE,		  /*!< != */
	ND_LT,		  /*!< < */
	ND_LE,		  /*!< <= */
	ND_NUM,		  /*!< 整数 */
	ND_EXPR_STMT, /*!< 文 */
	ND_STMT_EXPR, /*!< ステートメント式  */
	ND_VAR,		  /*!< 変数 */
	ND_CAST,	  /*!< 型キャスト */
	ND_MEMZERO,	  /*!< スタック上の変数のゼロクリア */
};

/**
 * @brief 抽象構文木(AST)を構成するノード
 *
 */
class Node
{
public:
	/* メンバ変数 (public) */

	NodeKind _kind = NodeKind::ND_EXPR_STMT; /*!< ノードの種類*/
	unique_ptr<Node> _next;					 /*!< ノードが木のrootである場合、次の木のrootノード */
	shared_ptr<Type> _ty;					 /*!< 型情報 e.g. int or pointer to int */

	unique_ptr<Node> _lhs; /*!< 左辺 */
	unique_ptr<Node> _rhs; /*!< 右辺 */

	/* if or for */
	unique_ptr<Node> _condition; /*!< if文の条件 */
	unique_ptr<Node> _then;		 /*!< trueのときに行う式 */
	unique_ptr<Node> _else;		 /*!< falseのとき行う式 */
	unique_ptr<Node> _init;		 /*!< 初期化処理 */
	unique_ptr<Node> _inc;		 /*!< 加算処理 */

	/* ブロック */
	unique_ptr<Node> _body; /*!< ブロック内{...}またはステートメント式({...})には複数の式を入れられる */

	/* 構造体 */
	shared_ptr<Member> _member; /*!< 構造体メンバー */

	/* 関数呼び出し */
	string _func_name = "";	   /*!< kindがND_FUNCALLの場合のみ使う、呼び出す関数の名前  */
	shared_ptr<Type> _func_ty; /*!< 関数の型 */
	unique_ptr<Node> _args;	   /*!< 引数  */

	/* 数値 */
	int64_t _val = 0; /*!< kindがND_NUMの場合のみ使う、数値の値 */

	/* 変数 */
	const Object *_var = nullptr; /*!< kindがND_VARの場合のみ使う、 オブジェクトの情報*/

	/* break, continue */
	string _brk_label = "";	 /*!< breakにつけるアセンブリ内で一意なラベル名 */
	string _cont_label = ""; /*!< continueにつけるアセンブリ内で一意なラベル名 */

	/* goto */
	string _label = "";		   /*!< ラベル */
	string _unique_label = ""; /*!< アセンブリ内で使う一意なラベル名 */
	Node *_goto_next;		   /*!< gotoをまとめたリストで次のノード */

	/* switch-case */
	Node *case_next = nullptr;	  /*!< switch文の各ケースのリスト */
	Node *default_case = nullptr; /*!< switch文のdefault */

	/* エラー報告用 */
	Token *_token = nullptr; /* ノードと対応するトークン */

	/* コンストラクタ */

	Node();
	Node(const NodeKind &kind, Token *token);
	Node(const NodeKind &kind, unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token);
	Node(const NodeKind &kind, unique_ptr<Node> &&lhs, Token *token);
	Node(const int64_t &val, Token *token);
	Node(const Object *var, Token *token);

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static unique_ptr<Object> parse(Token *token);
	static unique_ptr<Node> new_cast(unique_ptr<Node> &&expr, shared_ptr<Type> &ty);

private:
	/***************************/
	/* 静的メンバ関数 (private) */
	/***************************/

	static unique_ptr<Node> new_add(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token);
	static unique_ptr<Node> new_sub(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token);
	static Object *new_string_literal(const string &str);
	static Object *new_anonymous_gvar(shared_ptr<Type> &&ty);
	static string new_unique_name();
	static unique_ptr<Node> new_long(const int64_t &val, Token *token);
	static unique_ptr<Node> new_inc_dec(unique_ptr<Node> &&node, Token *token, int addend);
	static unique_ptr<Node> statement(Token **next_token, Token *current_token);
	static unique_ptr<Initializer> initializer(Token **next_token, Token *current_token, shared_ptr<Type> ty, shared_ptr<Type> &new_ty);
	static void string_initializer(Token **next_token, Token *current_token, Initializer *init);
	static int count_array_init_element(Token *token, const Type *ty);
	static void array_initializer1(Token **next_token, Token *current_token, Initializer *init);
	static void array_initializer2(Token **next_token, Token *current_token, Initializer *init);
	static void struct_initializer1(Token **next_token, Token *current_token, Initializer *init);
	static void struct_initializer2(Token **next_token, Token *current_token, Initializer *init);
	static void union_initializer(Token **next_token, Token *current_token, Initializer *init);
	static void initializer2(Token **next_token, Token *current_token, Initializer *init);
	static Token *skip_excess_element(Token *token);
	static unique_ptr<Node> init_desg_expr(InitDesg *desg, Token *token);
	static unique_ptr<Node> create_lvar_init(Initializer *init, Type *ty, InitDesg *desg, Token *token);
	static unique_ptr<Node> lvar_initializer(Token **next_token, Token *current_token, Object *var);
	static void gvar_initializer(Token **next_token, Token *current_token, Object *var);
	static void write_buf(unsigned char buf[], int64_t val, int sz, int offset);
	static Relocation *write_gvar_data(Relocation *cur, Initializer *init, Type *ty, unsigned char buf[], int offset);
	static unique_ptr<Node> compound_statement(Token **next_token, Token *current_token);
	static Token *function_definition(Token *token, shared_ptr<Type> &&base, VarAttr *attr);
	static shared_ptr<Type> struct_decl(Token **next_token, Token *current_token);
	static shared_ptr<Type> union_decl(Token **next_token, Token *current_token);
	static shared_ptr<Type> struct_union_decl(Token **next_token, Token *current_token);
	static void struct_members(Token **next_token, Token *current_token, Type *ty);
	static shared_ptr<Member> get_struct_member(Type *ty, Token *token);
	static unique_ptr<Node> struct_ref(unique_ptr<Node> &&lhs, Token *token);
	static shared_ptr<Type> declspec(Token **next_token, Token *current_token, VarAttr *attr);
	static shared_ptr<Type> enum_specifier(Token **next_token, Token *current_token);
	static shared_ptr<Type> declarator(Token **next_token, Token *current_token, shared_ptr<Type> ty);
	static unique_ptr<Node> declaration(Token **next_token, Token *current_token, shared_ptr<Type> &base);
	static shared_ptr<Type> function_parameters(Token **next_token, Token *current_token, shared_ptr<Type> &&ty);
	static shared_ptr<Type> array_dimensions(Token **next_token, Token *current_token, shared_ptr<Type> &&ty);
	static shared_ptr<Type> type_suffix(Token **next_token, Token *current_token, shared_ptr<Type> &&ty);
	static unique_ptr<Node> expression(Token **next_token, Token *current_token);
	static unique_ptr<Node> expression_statement(Token **next_token, Token *current_token);
	static int64_t const_expr(Token **next_token, Token *current_token);
	static int64_t evaluate(Node *node);
	static int64_t evaluate2(Node *node, string *label);
	static int64_t evaluate_rval(Node *node, string *label);
	static unique_ptr<Node> to_assign(unique_ptr<Node> &&binary);
	static unique_ptr<Node> log_or(Token **next_token, Token *current_token);
	static unique_ptr<Node> conditional(Token **next_token, Token *current_token);
	static unique_ptr<Node> log_and(Token **next_token, Token *current_token);
	static unique_ptr<Node> assign(Token **next_token, Token *current_token);
	static unique_ptr<Node> bit_or(Token **next_token, Token *current_token);
	static unique_ptr<Node> bit_xor(Token **next_token, Token *current_token);
	static unique_ptr<Node> bit_and(Token **next_token, Token *current_token);
	static unique_ptr<Node> equality(Token **next_token, Token *current_token);
	static unique_ptr<Node> relational(Token **next_token, Token *current_token);
	static unique_ptr<Node> shift(Token **next_token, Token *current_token);
	static unique_ptr<Node> add(Token **next_token, Token *current_token);
	static unique_ptr<Node> cast(Token **next_token, Token *current_token);
	static unique_ptr<Node> mul(Token **next_token, Token *current_token);
	static unique_ptr<Node> unary(Token **next_token, Token *current_token);
	static unique_ptr<Node> postfix(Token **next_token, Token *current_token);
	static unique_ptr<Node> primary(Token **next_token, Token *current_token);
	static Token *parse_typedef(Token *token, shared_ptr<Type> &base);
	static shared_ptr<Type> abstract_declarator(Token **next_token, Token *current_token, shared_ptr<Type> &&ty);
	static shared_ptr<Type> type_name(Token **next_token, Token *current_token);
	static unique_ptr<Node> function_call(Token **next_token, Token *current_token);
	static Token *global_variable(Token *token, shared_ptr<Type> &&base);
	static void resolve_goto_label();
	static bool is_function(Token *token);
};
