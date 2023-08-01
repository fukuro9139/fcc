/**
 * @file object.hpp
 * @author K.Fukunaga
 * @brief 関数や変数を表すクラス
 * @version 0.1
 * @date 2023-08-01
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include <memory>
#include <string>
#include "type.hpp"
#include "tokenize.hpp"

/* 先に宣言 */
struct Scope;
struct VarScope;

/**
 * @brief 変数または関数を表す。各オブジェクトは名前によって区別する
 *
 */
class Object
{
public:
	/* メンバ変数 (public) */
	/* 共通 */

	std::unique_ptr<Object> _next; /*!< 次のオブジェクト */
	std::string _name = "";		   /*!< 名前 */
	std::shared_ptr<Type> _ty;	   /* 型 */
	bool is_local = false;		   /*!< ローカル変数であるか */

	/* ローカル変数用 */

	int _offset = 0; /*!< RBPからのオフセット */

	/* グローバル変数 or 関数用 */
	bool is_function = false;	/*!< 関数であるか */
	bool is_definition = false; /*!< 宣言であるか */

	/* グローバル変数 */
	std::string _init_data = ""; /*!< 文字列リテラル */
	bool is_str_literal = false; /*!< 文字列リテラルか*/

	/* 関数用 */

	std::unique_ptr<Object> _params; /*!< 引数 */
	std::unique_ptr<Node> _body;	 /*!< 関数の表す内容を抽象構文木で表す。根のノードを持つ */
	std::unique_ptr<Object> _locals; /*!< 関数内で使うローカル変数 */
	int _stack_size = 0;			 /*!< 使用するスタックの深さ */

	/* コンストラクタ */

	Object();
	Object(const std::string &name);
	Object(const std::string &name, std::unique_ptr<Object> &&next, std::shared_ptr<Type> &&ty);
	Object(std::unique_ptr<Node> &&body, std::unique_ptr<Object> &&locs);

	/* デストラクタ */
	//~Object();

	/* 静的メンバ関数 (public) */

	static Object *new_lvar(const std::string &name, std::shared_ptr<Type> &&ty);
	static Object *new_gvar(const std::string &name, std::shared_ptr<Type> &&ty);
	static VarScope *find_var(const Token *token);
	static std::shared_ptr<Type> find_typedef(const Token *token);
	static std::shared_ptr<Type> find_tag(const Token *token);
	static int align_to(const int &n, const int &align);
	static void assign_lvar_offsets(const std::unique_ptr<Object> &prog);
	static void create_params_lvars(std::shared_ptr<Type> &&param);
	static void enter_scope();
	static void leave_scope();
	static VarScope *push_scope(const std::string &name);
	static void push_tag_scope(Token *token, const std::shared_ptr<Type> &ty);

	/* 静的メンバ変数 */

	/** ローカル変数オブジェクトのリスト。パース中に生成される全てのローカル変数はこのリストに連結される。 */
	static std::unique_ptr<Object> locals;
	/** グローバル変数オブジェクトのリスト。パース中に生成される全てのグローバル変数はこのリストに連結される。 */
	static std::unique_ptr<Object> globals;

private:
	/** 変数のスコープ */
	static std::unique_ptr<Scope> scope;
};

/**
 * @brief 構造体、共用体のタグのスコープ
 *
 */
struct TagScope
{
	std::unique_ptr<TagScope> _next; /*!< スコープ内の次のタグ */
	std::string _name = "";			 /*!< 構造体の名前 */
	std::shared_ptr<Type> _ty;		 /*!< 構造体の型 */

	TagScope(const std::string &name, const std::shared_ptr<Type> &ty, std::unique_ptr<TagScope> &&next) : _name(name), _ty(ty), _next(std::move(next)) {}
};

/**
 * @brief ローカル変数、グローバル変数、typedefのスコープ
 *
 */
struct VarScope
{
	std::unique_ptr<VarScope> _next; /*!< 次の変数  */
	const std::string _name = "";	 /*!< 変数名 */
	const Object *_obj = nullptr;	 /*!< 対応する変数のオブジェクト */
	std::shared_ptr<Type> type_def;	 /*!< typedefされた型  */

	VarScope(std::unique_ptr<VarScope> &&next, const std::string &name) : _next(std::move(next)), _name(name) {}
};

/**
 * @brief ローカル変数、構造体のスコープを表す
 *
 */
struct Scope
{
	std::unique_ptr<Scope> _next;	 /*!< 次のスコープ  */
	std::unique_ptr<VarScope> _vars; /*!< 変数のスコープ */
	std::unique_ptr<TagScope> _tags; /*!< 構造体のタグのスコープ */

	Scope() {}
	Scope(std::unique_ptr<Scope> &&next) : _next(std::move(next)) {}
};

/**
 * @brief 変数がもつ属性、例：typedef, extern
 *
 */
struct VarAttr
{
	bool is_typedef = false;
};