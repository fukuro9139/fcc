/**
 * @file object.hpp
 * @author K.Fukunaga
 * @brief 関数や変数を表すクラス
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include "type.hpp"
#include "tokenize.hpp"
#include "common.hpp"

/* 先に宣言 */
struct Scope;
struct VarScope;
struct Initializer;
struct Relocation;

/**
 * @brief 変数または関数を表す。各オブジェクトは名前によって区別する
 *
 */
class Object
{
public:
	/* メンバ変数 (public) */
	/* 共通 */

	unique_ptr<Object> _next; /*!< 次のオブジェクト */
	string _name = "";		  /*!< 名前 */
	shared_ptr<Type> _ty;	  /* 型 */
	bool _is_local = false;	  /*!< ローカル変数であるか */
	int _align = 0;			  /*!< アライメント */

	/* ローカル変数用 */

	int _offset = 0; /*!< RBPからのオフセット */

	/* グローバル変数 or 関数用 */
	bool _is_function = false;	 /*!< 関数であるか */
	bool _is_definition = false; /*!< 宣言であるか */
	bool _is_static = false;	 /*!< ファイルスコープか */

	/* グローバル変数 */
	unique_ptr<unsigned char[]> _init_data; /*!< グローバル変数の初期値 */
	unique_ptr<Relocation> _rel;			/*!< 他のグローバル変数のポインタによる初期化 */

	/* 関数用 */

	unique_ptr<Object> _params; /*!< 引数 */
	unique_ptr<Node> _body;		/*!< 関数の表す内容を抽象構文木で表す。根のノードを持つ */
	unique_ptr<Object> _locals; /*!< 関数内で使うローカル変数 */
	int _stack_size = 0;		/*!< 使用するスタックの深さ */

	/* コンストラクタ */

	Object();
	Object(const string &name);
	Object(const string &name, shared_ptr<Type> &ty);
	Object(unique_ptr<Node> &&body, unique_ptr<Object> &&locs);

	/* 静的メンバ関数 (public) */

	static Object *new_lvar(const string &name, shared_ptr<Type> ty);
	static Object *new_gvar(const string &name, shared_ptr<Type> ty);
	static unique_ptr<Initializer> new_initializer(const shared_ptr<Type> &ty, bool is_flexible);
	static VarScope *find_var(const Token *token);
	static shared_ptr<Type> find_typedef(const Token *token);
	static shared_ptr<Type> find_tag(const Token *token);
	static shared_ptr<Type> find_tag_in_internal_scope(const Token *token);
	static int align_to(const int &n, const int &align);
	static void assign_lvar_offsets(const unique_ptr<Object> &prog);
	static void create_params_lvars(shared_ptr<Type> &param);
	static void enter_scope();
	static void leave_scope();
	static VarScope *push_scope(const string &name);
	static void push_tag_scope(Token *token, const shared_ptr<Type> &ty);

	/* 静的メンバ変数 */

	/** ローカル変数オブジェクトのリスト。パース中に生成される全てのローカル変数はこのリストに連結される。 */
	static unique_ptr<Object> locals;
	/** グローバル変数オブジェクトのリスト。パース中に生成される全てのグローバル変数はこのリストに連結される。 */
	static unique_ptr<Object> globals;

private:
	/* 静的メンバ関数 (private) */

	static unique_ptr<Object> new_var(const string &name, shared_ptr<Type> &ty);

	/** 変数のスコープ */
	static unique_ptr<Scope> scope;
};

/**
 * @brief 構造体、共用体、列挙型のタグのスコープ
 *
 */
struct TagScope
{
	unique_ptr<TagScope> _next; /*!< スコープ内の次のタグ */
	string _name = "";			/*!< 構造体の名前 */
	shared_ptr<Type> _ty;		/*!< 構造体の型 */

	TagScope(const string &name, const shared_ptr<Type> &ty, unique_ptr<TagScope> &&next) : _name(name), _ty(ty), _next(std::move(next)) {}
};

/**
 * @brief ローカル変数、グローバル変数、typedef, 列挙型のスコープ
 *
 */
struct VarScope
{
	unique_ptr<VarScope> _next;	  /*!< 次の変数  */
	const string _name = "";	  /*!< 変数名 */
	const Object *_var = nullptr; /*!< 対応する変数のオブジェクト */
	shared_ptr<Type> type_def;	  /*!< typedefされた型  */
	shared_ptr<Type> enum_ty;	  /*!< 列挙型の型 */
	int enum_val = 0;			  /*!< 列挙型が表す数値 */

	VarScope(unique_ptr<VarScope> &&next, const string &name) : _next(std::move(next)), _name(name) {}
};

/**
 * @brief 変数/typedef、構造体/共用体/列挙型のスコープを表す
 *
 */
struct Scope
{
	unique_ptr<Scope> _next;	/*!< 次のスコープ  */
	unique_ptr<VarScope> _vars; /*!< 変数のスコープ */
	unique_ptr<TagScope> _tags; /*!< 構造体のタグのスコープ */

	Scope() {}
	Scope(unique_ptr<Scope> &&next) : _next(std::move(next)) {}
};

/**
 * @brief 変数がもつ属性、例：typedef, extern
 *
 */
struct VarAttr
{
	bool _is_typedef = false; /*!< typedefされた型か*/
	bool _is_static = false;  /*!< ファイルスコープか */
	bool _is_extern = false;  /*!< 外部宣言か */
	int _align = 0;			  /*!< アライメント */
};

/**
 * @brief 変数の初期化を表す。初期化は入れ子になる
 * ことがあるため（例：x[2][2] ={{1,1},{2,2}};）
 * この構造体は木構造を形成する
 *
 */
struct Initializer
{
	Initializer();
	Initializer(const shared_ptr<Type> &ty);

	unique_ptr<Initializer> _next; /*!< 次の初期化式 */
	shared_ptr<Type> _ty;		   /*!< 初期化式の型 */
	Token *_token = nullptr;	   /*!< エラー報告用 */
	bool _is_flexible = false;	   /*!< 要素数が指定されているか */

	/** ネストした初期化式でなければ初期化式の内容の式 */
	unique_ptr<Node> _expr;

	/** ネストしている場合、各要素の初期化式 */
	unique_ptr<unique_ptr<Initializer>[]> _children;
};

/**
 * @brief 配列型の変数の場合,初期化式において配列を構成する
 * 各要素を表す。そうでなければ変数そのものを表す。
 *
 */
struct InitDesg
{
	InitDesg *_next;			  /*!< 自身が配列,構造体の場合、親の要素 */
	int _idx = 0;				  /*!< 自身を表す配列のインデックス */
	shared_ptr<Member> _member;	  /*!< 構造体のメンバ */
	const Object *_var = nullptr; /*!< 変数を表すオブジェクト */
};

/**
 * @brief グローバル変数は定数式または他のグローバル変数のポインタで初期化される。
 * この構造体は後者の初期化法をサポートする。
 * 例: int x=1; int *y = &x; int main(){...}
 */
struct Relocation
{
	unique_ptr<Relocation> _next; /*!< 次のポインタによる初期化式 */
	int _offset = 0;			  /*!< オフセット */
	string _label;				  /*!< 変数名 */
	long _addend = 0;			  /*!<  ポインタが最終的に指し示すアドレスのオフセット  */
};