/**
 * @file type.cpp
 * @author K.Fukunaga
 * @brief 型を表すためのクラス
 * @version 0.1
 * @date 2023-08-01
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "type.hpp"
#include "parse.hpp"

using std::shared_ptr;

/**************/
/* Type Class */
/**************/

const shared_ptr<Type> Type::VOID_BASE = std::make_shared<Type>(TypeKind::TY_VOID, 1, 1);
const shared_ptr<Type> Type::SHORT_BASE = std::make_shared<Type>(TypeKind::TY_SHORT, 2, 2);
const shared_ptr<Type> Type::INT_BASE = std::make_shared<Type>(TypeKind::TY_INT, 4, 4);
const shared_ptr<Type> Type::LONG_BASE = std::make_shared<Type>(TypeKind::TY_LONG, 8, 8);
const shared_ptr<Type> Type::CHAR_BASE = std::make_shared<Type>(TypeKind::TY_CHAR, 1, 1);

Type::Type() : _kind(TypeKind::TY_INT) {}

Type::Type(const TypeKind &kind, const int &size, const int &align) : _kind(kind), _size(size), _align(align) {}

Type::Type(const TypeKind &kind) : _kind(kind) {}

Type::Type(const shared_ptr<Type> &base, const int &size, const int &align) : _kind(TypeKind::TY_PTR), _base(base), _size(size), _align(align) {}

Type::Type(Token *token, const shared_ptr<Type> &return_ty)
	: _kind(TypeKind::TY_FUNC), _token(token), _return_ty(return_ty) {}

/**
 * @brief 抽象構文木(AST)を巡回しながら型情報を設定する。
 *
 * @param node ASTのノード
 */
void Type::add_type(Node *node)
{
	/* ノードが存在しない or 既に型が設定されている場合は何もしない */
	if (!node || node->_ty)
	{
		return;
	}

	add_type(node->_lhs.get());
	add_type(node->_rhs.get());
	add_type(node->_condition.get());
	add_type(node->_then.get());
	add_type(node->_else.get());
	add_type(node->_init.get());
	add_type(node->_inc.get());

	for (auto n = node->_body.get(); n; n = n->_next.get())
	{
		add_type(n);
	}
	for (auto n = node->_args.get(); n; n = n->_next.get())
	{
		add_type(n);
	}

	switch (node->_kind)
	{
	/* 演算結果の型は左辺の型にあわせる */
	case NodeKind::ND_ADD:
	case NodeKind::ND_SUB:
	case NodeKind::ND_MUL:
	case NodeKind::ND_DIV:
	case NodeKind::ND_NEG:
		node->_ty = node->_lhs->_ty;
		return;

	case NodeKind::ND_ASSIGN:
		if (TypeKind::TY_ARRAY == node->_lhs->_ty->_kind)
		{
			error_token("左辺値ではありません", node->_lhs->_token);
		}
		node->_ty = node->_lhs->_ty;
		return;

	/* 比較演算の結果はlong型(一番大きい数値型へ合わせてしまう) */
	case NodeKind::ND_EQ:
	case NodeKind::ND_NE:
	case NodeKind::ND_LT:
	case NodeKind::ND_LE:
	case NodeKind::ND_NUM:
	case NodeKind::ND_FUNCALL:
		node->_ty = Type::LONG_BASE;
		return;

	case NodeKind::ND_VAR:
		/* オブジェクトの型に一致させる */
		node->_ty = node->_var->_ty;
		return;

	/* カンマ区切りの型は右辺に一致させる */
	case NodeKind::ND_COMMA:
		node->_ty = node->_rhs->_ty;
		return;

	/* 構造体の型はメンバの型 */
	case NodeKind::ND_MEMBER:
		node->_ty = node->_member->_ty;
		return;

	/* 参照は参照先へのポインタ型 */
	case NodeKind::ND_ADDR:
		/* 配列型の変数への参照はポインタ型としてみた配列の型と同じ */
		if (TypeKind::TY_ARRAY == node->_lhs->_ty->_kind)
		{
			node->_ty = pointer_to(node->_lhs->_ty->_base);
		}
		/* そうでなければ左辺の型へのポインタ */
		else
		{
			node->_ty = pointer_to(node->_lhs->_ty);
		}
		return;

	/* デリファレンス */
	case NodeKind::ND_DEREF:
		/* デリファレンスできるのはポインタ型のみ */
		if (!node->_lhs->_ty->_base)
		{
			error_token("デリファレンスできるのはポインタ型のみです", node->_token);
		}
		/* void型のポインタはデリファレンス不可 */
		if (TypeKind::TY_VOID == node->_lhs->_ty->_base->_kind)
		{
			error_token("void型のポインタはデリファレンスできません", node->_token);
		}
		/* ポインタのベース型 */
		node->_ty = node->_lhs->_ty->_base;

		return;

	/* ステートメント式 */
	case NodeKind::ND_STMT_EXPR:
	{
		/* 中身がなければ何もしない */
		if (!node->_body)
		{
			return;
		}
		auto stmt = node->_body.get();
		/* 複文の中を最後まで辿っていく */
		while (stmt->_next)
		{
			stmt = stmt->_next.get();
		}
		if (NodeKind::ND_EXPR_STMT == stmt->_kind)
		{
			node->_ty = stmt->_lhs->_ty;
			return;
		}
		error_token("ステートメント式はvoid型の戻り値をサポートしていません", node->_token);
	}
	default:
		break;
	}
}

/**
 * @brief 入力された型が整数型かどうか判定
 *
 * @param ty 対象の型
 * @return 整数型である:true, 整数型でない:false
 */
bool Type::is_integer() const
{
	const auto k = this->_kind;
	return TypeKind::TY_CHAR == k || TypeKind::TY_SHORT == k || TypeKind::TY_INT == k ||
		   TypeKind::TY_LONG == k;
}

/**
 * @brief base型へのポインター型を生成して返す
 *
 * @param base 参照する型
 * @return baseを参照するポインター型
 */
shared_ptr<Type> Type::pointer_to(const shared_ptr<Type> &base)
{
	return std::make_shared<Type>(base, 8, 8);
}

/**
 * @brief 戻り値の型がreturn_ty型である関数型を生成し返す
 *
 * @param return_ty 戻り値の型
 * @return 戻り値がreturn_ty型の関数型
 */
shared_ptr<Type> Type::func_type(const shared_ptr<Type> &return_ty)
{
	return std::make_shared<Type>(return_ty->_token, return_ty);
}

/**
 * @brief base型の要素を持つ配列型を生成し返す
 *
 * @param base 要素の型
 * @param length 配列の長さ
 * @return base型の要素を持つ配列型
 */
shared_ptr<Type> Type::array_of(shared_ptr<Type> base, int length)
{
	auto ret = std::make_shared<Type>(TypeKind::TY_ARRAY, base->_size * length, base->_align);
	ret->_base = base;
	ret->_array_length = length;
	return ret;
}
