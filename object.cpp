/**
 * @file object.cpp
 * @author K.Fukunaga
 * @brief 関数や変数を表すクラス
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "object.hpp"
#include "parse.hpp"
#include "error.hpp"

/* Initializerクラス */
Initializer::Initializer() = default;
Initializer::Initializer(const shared_ptr<Type> &ty) : _ty(ty) {}

/* Objectクラス */

/* 静的メンバ変数 */

unique_ptr<Object> Object::locals = nullptr;
unique_ptr<Object> Object::globals = nullptr;
unique_ptr<Scope> Object::scope = make_unique<Scope>();

/* コンストラクタ */

Object::Object() = default;

Object::Object(const string &name) : _name(name) {}

Object::Object(const string &name, shared_ptr<Type> &ty)
	: _name(name), _ty(ty) {}

Object::Object(unique_ptr<Node> &&body, unique_ptr<Object> &&locs) : _body(move(body)), _locals(move(locs)) {}

/* メンバ関数 */

/**
 * @brief 新しい変数オブジェクトを作成する
 *
 * @param name 変数の名前
 * @param ty 変数の型
 * @return 作成したオブジェクトのポインタ
 */
unique_ptr<Object> Object::new_var(const string &name, shared_ptr<Type> &ty)
{
	auto var = make_unique<Object>(name, ty);
	var->_align = ty->_align;
	push_scope(name)->_var = var.get();
	return var;
}

/**
 * @brief 新しいローカル変数を生成してObject::localsの先頭に追加する。
 *
 * @param  オブジェクトの型
 * @return 生成した変数へのポインタ
 */
Object *Object::new_lvar(const string &name, shared_ptr<Type> ty)
{
	auto var = new_var(name, ty);
	var->_is_local = true;
	var->_next = move(locals);
	locals = move(var);
	return locals.get();
}

/**
 * @brief 新しいグローバル変数を生成してObject::globalsの先頭に追加する。
 *
 * @param  ty オブジェクトの型
 * @return 生成した変数へのポインタ
 */
Object *Object::new_gvar(const string &name, shared_ptr<Type> ty)
{
	auto var = new_var(name, ty);
	var->_next = move(globals);
	var->_is_static = true;
	var->_is_definition = true;
	globals = move(var);
	return globals.get();
}

/**
 * @brief 新しい初期化式を作成する
 *
 * @param ty 初期化式の型
 * @param is_flexible
 * @return 作成した初期化式
 */
unique_ptr<Initializer> Object::new_initializer(const shared_ptr<Type> &ty, bool is_flexible)
{
	auto init = make_unique<Initializer>(ty);

	if (TypeKind::TY_ARRAY == ty->_kind)
	{
		/* 要素数が指定されていない */
		if (is_flexible && ty->_size < 0)
		{
			init->_is_flexible = true;
			return init;
		}

		int len_arr = ty->_array_length;
		init->_children = make_unique<unique_ptr<Initializer>[]>(len_arr);
		for (int i = 0; i < len_arr; ++i)
		{
			init->_children[i] = new_initializer(ty->_base, false);
		}
		return init;
	}

	if (TypeKind::TY_STRUCT == ty->_kind || TypeKind::TY_UNION == ty->_kind)
	{
		/* 要素数を数える */
		int len_struct = 0;
		for (auto mem = ty->_members.get(); mem; mem = mem->_next.get())
		{
			++len_struct;
		}

		init->_children = make_unique<unique_ptr<Initializer>[]>(len_struct);

		for (auto mem = ty->_members; mem; mem = mem->_next)
		{
			/* フレキシブル配列メンバをもつとき、最後の配列メンバをフレキシブル配列メンバとする */
			if (is_flexible && ty->_is_flexible && !mem->_next)
			{
				auto child = make_unique<Initializer>();
				child->_ty = mem->_ty;
				child->_is_flexible = true;
				init->_children[mem->_idx] = move(child);
			}
			else
			{
				init->_children[mem->_idx] = Object::new_initializer(mem->_ty, false);
			}
		}
		return init;
	}
	return init;
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
 * @brief 最も内側のスコープ内で構造体のタグを名前で検索する。見つからなかった場合はnullptrを返す。
 *
 * @param token 検索対象のトークン
 * @return 既出の構造体であればその型へのポインタ
 */
shared_ptr<Type> Object::find_tag_in_internal_scope(const Token *token)
{
	for (auto sc = scope->_tags.get(); sc; sc = sc->_next.get())
	{
		if (token->is_equal(sc->_name))
		{
			return sc->_ty;
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
		if (!fn->_is_function)
		{
			continue;
		}

		int offset = 0;

		/* ローカル変数 */
		for (Object *var = fn->_locals.get(); var; var = var->_next.get())
		{
			offset += var->_ty->_size;
			offset = align_to(offset, var->_align);
			var->_offset = offset;
		}
		
		/* 引数 */
		for (auto *var = fn->_params.get(); var; var = var->_next.get())
		{
			offset += var->_ty->_size;
			offset = align_to(offset, var->_align);
			var->_offset = offset;
		}

		/* スタックサイズが16の倍数になるようにアライメントする */
		fn->_stack_size = align_to(move(offset), 16);
	}
}

/**
 * @brief 引数をローカル変数としてローカル変数のリストに繋ぐ
 *
 * @param param 引数のリスト
 */
void Object::create_params_lvars(shared_ptr<Type> &param)
{
	if (param)
	{
		create_params_lvars(param->_next);
		/* 引数名が存在しないときエラー */
		if(!param->_name){
			error_token("引数名がありません", param->_name_pos);
		}
		new_lvar(param->_name->_str, param);
	}
}

/**
 * @brief 新しいスコープに入る
 *
 */
void Object::enter_scope()
{
	scope = make_unique<Scope>(move(scope));
}

/**
 * @brief 現在のスコープから抜ける
 *
 */
void Object::leave_scope()
{
	scope = move(scope->_next);
}

/**
 * @brief 現在のスコープに変数を追加する
 *
 * @param name 追加する変数の名前
 * @param obj 追加する変数のオブジェクト
 */
VarScope *Object::push_scope(const string &name)
{
	scope->_vars = make_unique<VarScope>(move(scope->_vars), name);
	return scope->_vars.get();
}

/**
 * @brief 現在のスコープに構造体のタグを追加する
 *
 * @param token 追加する構造体のトークン
 * @param ty 追加する構造体の型
 */
void Object::push_tag_scope(Token *token, const shared_ptr<Type> &ty)
{
	scope->_tags = make_unique<TagScope>(token->_str, ty, move(scope->_tags));
}

/**
 * @brief 現在のスコープが最も外側のスコープ（グローバルスコープ）であるか
 *
 * @return true グローバルスコープである
 * @return false グローバルスコープではない
 */
bool Object::at_outermost_scope()
{
	return scope->_next == nullptr;
}