#include "object.hpp"
#include "parse.hpp"

using std::shared_ptr;
using std::unique_ptr;

/* 静的メンバ変数 */

std::unique_ptr<Object> Object::locals = nullptr;
std::unique_ptr<Object> Object::globals = nullptr;
Object *Object::current_function = nullptr;

/* コンストラクタ */

Object::Object() = default;

Object::Object(std::string &&name) : _name(std::move(name)) {}

Object::Object(std::string &&name, unique_ptr<Object> &&next, shared_ptr<Type> &&ty)
	: _name(std::move(name)), _next(std::move(next)), _ty(std::move(ty)) {}

Object::Object(unique_ptr<Node> &&body, unique_ptr<Object> &&locals) : _body(std::move(body)), _locals(std::move(locals)) {}

/* メンバ関数 */

/**
 * @brief 新しいローカル変数を生成してObject::localsの先頭に追加する。
 *
 * @param  オブジェクトの型
 * @return 生成した変数へのポインタ
 */
Object *Object::new_lvar(shared_ptr<Type> &&ty)
{
	locals = std::make_unique<Object>(std::move(ty->_name), std::move(locals), std::move(ty));
	locals->is_local = true;
	return locals.get();
}

/**
 * @brief 新しいグローバル変数を生成してObject::globalsの先頭に追加する。
 *
 * @param  ty オブジェクトの型
 * @return 生成した変数へのポインタ
 */
Object *Object::new_gvar(shared_ptr<Type> &&ty)
{
	globals = std::make_unique<Object>(std::move(ty->_name), std::move(globals), std::move(ty));
	return globals.get();
}

/**
 * @brief 新しい関数を生成してObject::globalsの先頭に追加する。
 *
 * @param  オブジェクトの型
 * @return 生成した関数へのポインタ
 */
Object *Object::new_func(std::shared_ptr<Type> &&ty)
{
	auto parameters = ty->_params;

	auto fn = std::make_unique<Object>(std::move(ty->_name), std::move(globals), std::move(ty));

	/* 関数であるフラグをセット */
	fn->is_function = true;

	/* 引数をローカル変数として作成 */
	create_params_lvars(std::move(parameters));
	fn->_params = std::move(locals);

	/* 作成した関数オブジェクトをObject::globalにセット */
	globals = std::move(fn);

	return globals.get();
}

/**
 * @brief 変数を名前で検索する。見つからなかった場合はNULLを返す。
 *
 * @param token 検索対象のトークン
 * @return 既出の変数であればその変数オブジェクトへのポインタ
 */
const Object *Object::find_var(const unique_ptr<Token> &token)
{
	auto current_function = Object::current_function;

	/* 引数 */
	for (const Object *var = current_function->_params.get(); var; var = var->_next.get())
	{
		if (var->_name.size() == token->_str.size() && std::equal(var->_name.begin(), var->_name.end(), token->_str.begin()))
		{
			return var;
		}
	}
	/* ローカル変数 */
	for (const Object *var = locals.get(); var; var = var->_next.get())
	{
		if (var->_name.size() == token->_str.size() && std::equal(var->_name.begin(), var->_name.end(), token->_str.begin()))
		{
			return var;
		}
	}

	/* グローバル変数 */
	for (const Object *var = globals.get(); var; var = var->_next.get())
	{
		if (var->_name.size() == token->_str.size() && std::equal(var->_name.begin(), var->_name.end(), token->_str.begin()))
		{
			return var;
		}
	}
	return nullptr;
}

/**
 * @brief 'n'を切り上げて最も近い'align'の倍数にする。
 *
 * @param n 切り上げ対象
 * @param align 基数
 * @return 切り上げた結果
 * @details 例：align_to(5,8) = 8, align_to(11,8) = 16
 */
int Object::align_to(const int &n, const int &align)
{
	return (n + align - 1) / align * align;
}

/** @brief 関数に必要なスタックサイズを計算してstack_sizeにセットする。
 *
 * @param prog スタックサイズをセットする関数
 */
void Object::assign_lvar_offsets(const unique_ptr<Object> &prog)
{
	for (auto fn = prog.get(); fn; fn = fn->_next.get())
	{
		/* 関数でなければスキップ */
		if (!fn->is_function)
		{
			continue;
		}

		int offset = 0;
		/* 引数 */
		for (auto *var = fn->_params.get(); var; var = var->_next.get())
		{
			offset += var->_ty->_size;
			var->_offset = offset;
		}
		/* ローカル変数 */
		for (Object *var = fn->_locals.get(); var; var = var->_next.get())
		{
			offset += var->_ty->_size;
			var->_offset = offset;
		}
		/* スタックサイズが16の倍数になるようにアライメントする */
		fn->_stack_size = align_to(std::move(offset), 16);
	}
}

/**
 * @brief 引数をローカル変数としてローカル変数のリストに繋ぐ
 *
 * @param param 引数のリスト
 */
void Object::create_params_lvars(shared_ptr<Type> &&param)
{
	if (param)
	{
		create_params_lvars(std::move(param->_next));
		Object::new_lvar(std::move(param));
	}
}