#pragma once

#include <memory>
#include <string>
#include "type.hpp"
#include "tokenize.hpp"

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

	/* グローバル変数用 */
	bool is_function = false; /*!< 関数であるか */

	/* 関数用 */

	std::unique_ptr<Object> _params; /*!< 引数 */
	std::unique_ptr<Node> _body;	 /*!< 関数の表す内容を抽象構文木で表す。根のノードを持つ */
	std::unique_ptr<Object> _locals; /*!< 関数内で使うローカル変数 */
	int _stack_size = 0;			 /*!< 使用するスタックの深さ */

	/* コンストラクタ */

	Object();
	Object(std::string &&name);
	Object(std::string &&name, std::unique_ptr<Object> &&next, std::shared_ptr<Type> &&ty);
	Object(std::unique_ptr<Node> &&body, std::unique_ptr<Object> &&locals);

	/* デストラクタ */
	//~Object();

	/* 静的メンバ関数 (public) */

	static Object *new_lvar(std::shared_ptr<Type> &&ty);
	static Object *new_gvar(std::shared_ptr<Type> &&ty);
	static Object *new_func(std::shared_ptr<Type> &&ty);
	static const Object *find_var(const std::unique_ptr<Token> &token);
	static int align_to(const int &n, const int &align);
	static void assign_lvar_offsets(const std::unique_ptr<Object> &prog);
	static void create_params_lvars(std::shared_ptr<Type> &&param);

	/* 静的メンバ変数 */

	/** 変数や関数オブジェクトのリスト。パース中に生成される全てのローカル変数はこのリストに連結される。 */
	static std::unique_ptr<Object> locals;
	static std::unique_ptr<Object> globals;
	/** 現在処理中の関数 */
	static Object *current_function;
};