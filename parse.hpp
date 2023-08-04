/**
 * @file parse.hpp
 * @author K.Fukunaga
 * @brief C言語の再帰下降構文解析を行うパーサーの定義
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "tokenize.hpp"
#include "object.hpp"
#include "type.hpp"

/**
 * @brief 構造体のメンバーを表すクラス
 *
 */
struct Member
{
	std::shared_ptr<Member> _next; /*!< 次のメンバ */
	std::shared_ptr<Type> _ty;	   /*!< 型情報 e.g. int or pointer to int */
	Token *_token = nullptr;	   /*!< 対応するトークン */
	int _offset = 0;			   /*!< RBPからのオフセット  */
};

/**
 * @brief ノードの種類
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
	ND_BLOCK,	  /*!< {...} */
	ND_GOTO,	  /*!< goto */
	ND_LABEL,	  /*!< ラベル */
	ND_FUNCALL,	  /*!< 関数呼び出し */
	ND_MOD,		  /*!< % */
	ND_BITAND,	  /*!< & */
	ND_BITOR,	  /*!< | */
	ND_BITXOR,	  /*!< ^ */
	ND_EQ,		  /*!< == */
	ND_NE,		  /*!< != */
	ND_LT,		  /*!< < */
	ND_LE,		  /*!< <= */
	ND_NUM,		  /*!< 整数 */
	ND_EXPR_STMT, /*!< 文 */
	ND_STMT_EXPR, /*!< ステートメント式  */
	ND_VAR,		  /*!< 変数 */
	ND_CAST,	  /*!< 型キャスト */
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
	std::unique_ptr<Node> _next;			 /*!< ノードが木のrootである場合、次の木のrootノード */
	std::shared_ptr<Type> _ty;				 /*!< 型情報 e.g. int or pointer to int */

	std::unique_ptr<Node> _lhs; /*!< 左辺 */
	std::unique_ptr<Node> _rhs; /*!< 右辺 */

	/* if or for */
	std::unique_ptr<Node> _condition; /*!< if文の条件 */
	std::unique_ptr<Node> _then;	  /*!< trueのときに行う式 */
	std::unique_ptr<Node> _else;	  /*!< falseのとき行う式 */
	std::unique_ptr<Node> _init;	  /*!< 初期化処理 */
	std::unique_ptr<Node> _inc;		  /*!< 加算処理 */

	/* ブロック */
	std::unique_ptr<Node> _body; /*!< ブロック内{...}またはステートメント式({...})には複数の式を入れられる */

	/* 構造体 */
	std::shared_ptr<Member> _member; /*!< 構造体メンバー */

	/* 関数呼び出し */
	std::string _func_name = "";	/*!< kindがND_FUNCALLの場合のみ使う、呼び出す関数の名前  */
	std::shared_ptr<Type> _func_ty; /*!< 関数の型 */
	std::unique_ptr<Node> _args;	/*!< 引数  */

	/* 数値 */
	int64_t _val = 0; /*!< kindがND_NUMの場合のみ使う、数値の値 */

	/* 変数 */
	const Object *_var = nullptr; /*!< kindがND_VARの場合のみ使う、 オブジェクトの情報*/

	/* break */
	std::string _brk_label = ""; /*!< forの終わりにつけるアセンブリ内で使う一意なラベル名 */

	/* goto */
	std::string _label = "";		/*!< ラベル */
	std::string _unique_label = ""; /*!< アセンブリ内で使う一意なラベル名 */
	Node *_goto_next;				/*!< gotoをまとめたリストで次のノード */

	/* エラー報告用 */
	Token *_token = nullptr; /* ノードと対応するトークン */

	/* コンストラクタ */

	Node();
	Node(const NodeKind &kind, Token *token);
	Node(const NodeKind &kind, std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, Token *token);
	Node(const NodeKind &kind, std::unique_ptr<Node> &&lhs, Token *token);
	Node(const int64_t &val, Token *token);
	Node(const Object *var, Token *token);

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static std::unique_ptr<Object> parse(Token *token);
	static std::unique_ptr<Node> new_cast(std::unique_ptr<Node> &&expr, std::shared_ptr<Type> &ty);

private:
	/***************************/
	/* 静的メンバ関数 (private) */
	/***************************/

	static std::unique_ptr<Node> new_add(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, Token *token);
	static std::unique_ptr<Node> new_sub(std::unique_ptr<Node> &&lhs, std::unique_ptr<Node> &&rhs, Token *token);
	static Object *new_string_literal(const std::string &str);
	static Object *new_anonymous_gvar(std::shared_ptr<Type> &&ty);
	static std::string new_unique_name();
	static std::unique_ptr<Node> new_long(const int64_t &val, Token *token);
	static std::unique_ptr<Node> new_inc_dec(std::unique_ptr<Node> &&node, Token *token, int addend);
	static std::unique_ptr<Node> statement(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> compound_statement(Token **next_token, Token *current_token);
	static Token *function_definition(Token *token, std::shared_ptr<Type> &&base, VarAttr *attr);
	static std::shared_ptr<Type> struct_decl(Token **next_token, Token *current_token);
	static std::shared_ptr<Type> union_decl(Token **next_token, Token *current_token);
	static std::shared_ptr<Type> struct_union_decl(Token **next_token, Token *current_token);
	static void struct_members(Token **next_token, Token *current_token, Type *ty);
	static std::shared_ptr<Member> get_struct_member(Type *ty, Token *token);
	static std::unique_ptr<Node> struct_ref(std::unique_ptr<Node> &&lhs, Token *token);
	static std::shared_ptr<Type> declspec(Token **next_token, Token *current_token, VarAttr *attr);
	static std::shared_ptr<Type> enum_specifier(Token **next_token, Token *current_token);
	static std::shared_ptr<Type> declarator(Token **next_token, Token *current_token, std::shared_ptr<Type> ty);
	static std::unique_ptr<Node> declaration(Token **next_token, Token *current_token, std::shared_ptr<Type> &base);
	static std::shared_ptr<Type> function_parameters(Token **next_token, Token *current_token, std::shared_ptr<Type> &&ty);
	static std::shared_ptr<Type> array_dimensions(Token **next_token, Token *current_token, std::shared_ptr<Type> &&ty);
	static std::shared_ptr<Type> type_suffix(Token **next_token, Token *current_token, std::shared_ptr<Type> &&ty);
	static std::unique_ptr<Node> expression(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> expression_statement(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> to_assign(std::unique_ptr<Node> &&binary);
	static std::unique_ptr<Node> log_or(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> log_and(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> assign(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> bit_or(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> bit_xor(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> bit_and(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> equality(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> relational(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> add(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> cast(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> mul(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> unary(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> postfix(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> primary(Token **next_token, Token *current_token);
	static Token *parse_typedef(Token *token, std::shared_ptr<Type> &base);
	static std::shared_ptr<Type> abstract_declarator(Token **next_token, Token *current_token, std::shared_ptr<Type> &&ty);
	static std::shared_ptr<Type> type_name(Token **next_token, Token *current_token);
	static std::unique_ptr<Node> function_call(Token **next_token, Token *current_token);
	static Token *global_variable(Token *token, std::shared_ptr<Type> &&base);
	static void resolve_goto_label();
	static bool is_function(Token *token);
};
