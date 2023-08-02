/**
 * @file object.cpp
 * @author K.Fukunaga
 * @brief 関数や変数を表すクラス
 * @version 0.1
 * @date 2023-08-01
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "object.hpp"
#include "parse.hpp"

using std::shared_ptr;
using std::unique_ptr;

/* 静的メンバ変数 */

std::unique_ptr<Object> Object::locals = nullptr;
std::unique_ptr<Object> Object::globals = nullptr;
std::unique_ptr<Scope> Object::scope = std::make_unique<Scope>();

/* コンストラクタ */

Object::Object() = default;

Object::Object(const std::string &name) : _name(name) {}

Object::Object(const std::string &name, shared_ptr<Type> &&ty)
	: _name(name), _ty(std::move(ty)) {}

Object::Object(unique_ptr<Node> &&body, unique_ptr<Object> &&locs) : _body(std::move(body)), _locals(std::move(locs)) {}

/* メンバ関数 */

/**
 * @brief 新しい変数オブジェクトを作成する
 *
 * @param name 変数の名前
 * @param ty 変数の型
 * @return 作成したオブジェクトのポインタ
 */
unique_ptr<Object> Object::new_var(const std::string &name, shared_ptr<Type> &&ty)
{
	auto var = std::make_unique<Object>(name, std::move(ty));
	push_scope(name)->_var = var.get();
	return var;
}

/**
 * @brief 新しいローカル変数を生成してObject::localsの先頭に追加する。
 *
 * @param  オブジェクトの型
 * @return 生成した変数へのポインタ
 */
Object *Object::new_lvar(const std::string &name, shared_ptr<Type> &&ty)
{
	auto var = new_var(name, std::move(ty));
	var->is_local = true;
	var->_next = std::move(locals);
	locals = std::move(var);
	return locals.get();
}

/**
 * @brief 新しいグローバル変数を生成してObject::globalsの先頭に追加する。
 *
 * @param  ty オブジェクトの型
 * @return 生成した変数へのポインタ
 */
Object *Object::new_gvar(const std::string &name, shared_ptr<Type> &&ty)
{
	auto var = new_var(name, std::move(ty));
	var->_next = std::move(globals);
	globals = std::move(var);
	return globals.get();
}

/**
 * @brief 変数を名前で検索する。見つからなかった場合はnullptrを返す。
 *
 * @param token 検索対象のトークン
 * @return 既出の変数であればその変数が属するスコープ
 */
VarScope *Object::find_var(const Token *token)
{
	/* スコープを内側から探していく */
	for (auto sc = scope.get(); sc; sc = sc->_next.get())
	{
		for (auto sc2 = sc->_vars.get(); sc2; sc2 = sc2->_next.get())
		{
			if (token->is_equal(sc2->_name))
			{
				return sc2;
			}
		}
	}

	return nullptr;
}

/**
 * @brief typedefされた型を検索する。見つからなければnullptrを返す
 *
 * @param token 検索するトークン
 * @return typedefされた型
 */
shared_ptr<Type> Object::find_typedef(const Token *token)
{
	if (TokenKind::TK_IDENT == token->_kind)
	{
		auto sc = find_var(token);
		if (sc)
		{
			return sc->type_def;
		}
	}
	return nullptr;
}

/**
 * @brief 構造体のタグを名前で検索する。見つからなかった場合はnullptrを返す。
 *
 * @param token 検索対象のトークン
 * @return 既出の構造体であればその型へのポインタ
 */
shared_ptr<Type> Object::find_tag(const Token *token)
{
	/* スコープを内側から探していく */
	for (auto sc = scope.get(); sc; sc = sc->_next.get())
	{
		for (auto sc2 = sc->_tags.get(); sc2; sc2 = sc2->_next.get())
		{
			if (token->is_equal(sc2->_name))
			{
				return sc2->_ty;
			}
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
			offset = align_to(offset, var->_ty->_align);
			var->_offset = offset;
		}
		/* ローカル変数 */
		for (Object *var = fn->_locals.get(); var; var = var->_next.get())
		{
			offset += var->_ty->_size;
			offset = align_to(offset, var->_ty->_align);
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
		new_lvar(param->_token->_str, std::move(param));
	}
}

/**
 * @brief 新しいスコープに入る
 *
 */
void Object::enter_scope()
{
	scope = std::make_unique<Scope>(std::move(scope));
}

/**
 * @brief 現在のスコープから抜ける
 *
 */
void Object::leave_scope()
{
	scope = std::move(scope->_next);
}

/**
 * @brief 現在のスコープに変数を追加する
 *
 * @param name 追加する変数の名前
 * @param obj 追加する変数のオブジェクト
 */
VarScope *Object::push_scope(const std::string &name)
{
	scope->_vars = std::make_unique<VarScope>(std::move(scope->_vars), name);
	return scope->_vars.get();
}

/**
 * @brief 現在のスコープに構造体のタグを追加する
 *
 * @param token 追加する構造体のトークン
 * @param ty 追加する構造体の型
 */
void Object::push_tag_scope(Token *token, const std::shared_ptr<Type> &ty)
{
	scope->_tags = std::make_unique<TagScope>(token->_str, ty, std::move(scope->_tags));
}