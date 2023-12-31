/**
 * @file type.cpp
 * @author K.Fukunaga
 * @brief 型を表すためのクラス
 * @version 0.1
 * @date 2023-07-11
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "type.hpp"
#include "parse.hpp"

/**************/
/* Type Class */
/**************/

const shared_ptr<Type> Type::VOID_BASE = make_shared<Type>(TypeKind::TY_VOID, 1, 1);
const shared_ptr<Type> Type::CHAR_BASE = make_shared<Type>(TypeKind::TY_CHAR, 1, 1);
const shared_ptr<Type> Type::SHORT_BASE = make_shared<Type>(TypeKind::TY_SHORT, 2, 2);
const shared_ptr<Type> Type::INT_BASE = make_shared<Type>(TypeKind::TY_INT, 4, 4);
const shared_ptr<Type> Type::LONG_BASE = make_shared<Type>(TypeKind::TY_LONG, 8, 8);
const shared_ptr<Type> Type::UCHAR_BASE = make_shared<Type>(TypeKind::TY_CHAR, 1, 1, true);
const shared_ptr<Type> Type::USHORT_BASE = make_shared<Type>(TypeKind::TY_SHORT, 2, 2, true);
const shared_ptr<Type> Type::UINT_BASE = make_shared<Type>(TypeKind::TY_INT, 4, 4, true);
const shared_ptr<Type> Type::ULONG_BASE = make_shared<Type>(TypeKind::TY_LONG, 8, 8, true);
const shared_ptr<Type> Type::FLOAT_BASE = make_shared<Type>(TypeKind::TY_FLOAT, 4, 4);
const shared_ptr<Type> Type::DOUBLE_BASE = make_shared<Type>(TypeKind::TY_DOUBLE, 8, 8);
const shared_ptr<Type> Type::BOOL_BASE = make_shared<Type>(TypeKind::TY_BOOL, 1, 1);

Type::Type() : _kind(TypeKind::TY_INT) {}

Type::Type(const TypeKind &kind, const int &size, const int &align)
	: _kind(kind), _size(size), _align(align) {}

Type::Type(const TypeKind &kind, const int &size, const int &align, bool is_unsigned)
	: _kind(kind), _size(size), _align(align), _is_unsigned(is_unsigned) {}

Type::Type(const TypeKind &kind)
	: _kind(kind) {}

Type::Type(const shared_ptr<Type> &base, const int &size, const int &align)
	: _kind(TypeKind::TY_PTR), _base(base), _size(size), _align(align) {}

Type::Type(Token *token, const shared_ptr<Type> &return_ty)
	: _kind(TypeKind::TY_FUNC), _name(token), _return_ty(return_ty) {}

/**
 * @brief ty1とty2のうちサイズの大きいほうの型を返す。ポインタ型優先
 *
 * @param ty1 左辺の型
 * @param ty2 右辺の型
 * @return 共通の型
 */
shared_ptr<Type> Type::get_common_type(shared_ptr<Type> ty1, shared_ptr<Type> ty2)
{
	/* 左辺がポインタなら同じ型へのポインタを返す。 */
	if (ty1->_base)
	{
		return pointer_to(ty1->_base);
	}

	/* 演算の中に出てくる関数は関数ポインタ */
	if (TypeKind::TY_FUNC == ty1->_kind)
	{
		return pointer_to(ty1);
	}
	if (TypeKind::TY_FUNC == ty2->_kind)
	{
		return pointer_to(ty2);
	}

	/* 片方がdouble型ならdouble型 */
	if (TypeKind::TY_DOUBLE == ty1->_kind || TypeKind::TY_DOUBLE == ty2->_kind)
	{
		return Type::DOUBLE_BASE;
	}

	/* 片方がfloat型ならfloat型 */
	if (TypeKind::TY_FLOAT == ty1->_kind || TypeKind::TY_FLOAT == ty2->_kind)
	{
		return Type::FLOAT_BASE;
	}

	/* 算術演算の結果はint型以上とする */
	if (ty1->_size < 4)
	{
		ty1 = Type::INT_BASE;
	}
	if (ty2->_size < 4)
	{
		ty2 = Type::INT_BASE;
	}

	/* サイズが違えばサイズが大きい方を返す */
	if (ty1->_size != ty2->_size)
	{
		return (ty1->_size < ty2->_size) ? ty2 : ty1;
	}

	/* サイズが同じ場合は右オペランドがunsignedならunsignedを返す */
	if (ty2->_is_unsigned)
	{
		return ty2;
	}

	return ty1;
}

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
	/* 演算結果の型は大きいほうの型にあわせる */
	case NodeKind::ND_ADD:
	case NodeKind::ND_SUB:
	case NodeKind::ND_MUL:
	case NodeKind::ND_DIV:
		usual_arith_conv(node->_lhs, node->_rhs);
		node->_ty = node->_lhs->_ty;
		break;

	case NodeKind::ND_NEG:
	{
		auto ty = get_common_type(INT_BASE, node->_lhs->_ty);
		node->_lhs = Node::new_cast(move(node->_lhs), ty);
		node->_ty = ty;
		break;
	}

	case NodeKind::ND_ASSIGN:
		if (TypeKind::TY_ARRAY == node->_lhs->_ty->_kind)
		{
			error_token("左辺値ではありません", node->_lhs->_token);
		}
		/* 左辺に合わせてキャストする */
		if (TypeKind::TY_STRUCT != node->_lhs->_ty->_kind)
		{
			node->_rhs = Node::new_cast(move(node->_rhs), node->_lhs->_ty);
		}
		node->_ty = node->_lhs->_ty;
		break;

	case NodeKind::ND_COND:
		/* どちらかがvoid型の場合はvoid型 */
		if (TypeKind::TY_VOID == node->_then->_ty->_kind ||
			TypeKind::TY_VOID == node->_else->_ty->_kind)
		{
			node->_ty = Type::VOID_BASE;
		}
		/* 大きい方の型に合わせる */
		else
		{
			usual_arith_conv(node->_then, node->_else);
			node->_ty = node->_then->_ty;
		}
		break;

	/* カンマ区切りの型は右辺に一致させる */
	case NodeKind::ND_COMMA:
		node->_ty = node->_rhs->_ty;
		break;

	/* 構造体の型はメンバの型 */
	case NodeKind::ND_MEMBER:
		node->_ty = node->_member->_ty;
		break;

	/* 参照は参照先へのポインタ型 */
	case NodeKind::ND_ADDR:
	{
		auto ty = node->_lhs->_ty;

		/* 配列型の変数への参照はポインタ型としてみた配列の型と同じ */
		if (TypeKind::TY_ARRAY == ty->_kind)
		{
			node->_ty = pointer_to(node->_lhs->_ty->_base);
		}
		/* そうでなければ左辺の型へのポインタ */
		else
		{
			node->_ty = pointer_to(ty);
		}
		break;
	}

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
		break;

	case NodeKind::ND_NOT:
		node->_ty = INT_BASE;
		break;

	case NodeKind::ND_BITNOT:
		node->_ty = node->_lhs->_ty;
		break;

	case NodeKind::ND_LOGAND:
	case NodeKind::ND_LOGOR:
		node->_ty = INT_BASE;
		break;

	case NodeKind::ND_FUNCALL:
		node->_ty = Type::LONG_BASE;
		break;

	case NodeKind::ND_MOD:
	case NodeKind::ND_BITAND:
	case NodeKind::ND_BITOR:
	case NodeKind::ND_BITXOR:
		usual_arith_conv(node->_lhs, node->_rhs);
		node->_ty = node->_lhs->_ty;
		break;

	case NodeKind::ND_SHL:
	case NodeKind::ND_SHR:
		node->_ty = node->_lhs->_ty;
		break;

	/* 比較演算は大きい方の型に合わせて行う、結果はint型とする */
	case NodeKind::ND_EQ:
	case NodeKind::ND_NE:
	case NodeKind::ND_LT:
	case NodeKind::ND_LE:
		/* 比較の前に大きいほうの型に合わせる */
		usual_arith_conv(node->_lhs, node->_rhs);
		node->_ty = Type::INT_BASE;
		break;

	case NodeKind::ND_NUM:
		node->_ty = INT_BASE;
		break;

	/* ステートメント式 */
	case NodeKind::ND_STMT_EXPR:
	{
		/* 中身がなければ何もしない */
		if (!node->_body)
		{
			break;
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
			break;
		}
		error_token("ステートメント式はvoid型の戻り値をサポートしていません", node->_token);
	}

	case NodeKind::ND_VAR:
		/* オブジェクトの型に一致させる */
		node->_ty = node->_var->_ty;
		break;

	default:
		break;
	}
}

/**
 * @brief 整数型かどうか判定
 *
 * @param ty 対象の型
 * @return 整数型である:true, 整数型でない:false
 */
bool Type::is_integer() const
{
	return TypeKind::TY_CHAR == _kind || TypeKind::TY_SHORT == _kind || TypeKind::TY_INT == _kind ||
		   TypeKind::TY_LONG == _kind || TypeKind::TY_BOOL == _kind || TypeKind::TY_ENUM == _kind;
}

/**
 * @brief 浮動小数点型であるか判定
 *
 * @return true 浮動小数点型である
 * @return false 浮動小数点型ではない
 */
bool Type::is_flonum() const
{
	return TypeKind::TY_FLOAT == _kind || TypeKind::TY_DOUBLE == _kind;
}

/**
 * @brief 数値であるかどうか
 *
 * @return true 数値である
 * @return false 数値ではない
 */
bool Type::is_numeric() const
{
	return is_flonum() || is_integer();
}

/**
 * @brief base型へのポインター型を生成して返す
 *
 * @param base 参照する型
 * @return baseを参照するポインター型
 */
shared_ptr<Type> Type::pointer_to(const shared_ptr<Type> &base)
{
	auto ty = make_shared<Type>(base, 8, 8);
	ty->_is_unsigned = true;
	return ty;
}

/**
 * @brief 戻り値の型がreturn_ty型である関数型を生成し返す
 *
 * @param return_ty 戻り値の型
 * @return 戻り値がreturn_ty型の関数型
 */
shared_ptr<Type> Type::func_type(const shared_ptr<Type> &return_ty)
{
	return make_shared<Type>(return_ty->_name, return_ty);
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
	auto ret = make_shared<Type>(TypeKind::TY_ARRAY, base->_size * length, base->_align);
	ret->_base = base;
	ret->_array_length = length;
	return ret;
}

/**
 * @brief 算術演算の暗黙の型変換を行う
 *
 * @details 多くの二幸演算子ではオペランドを暗黙のうちに昇格させて
 * 両方のオペランドが同じ型になるようにしている。int型よりも小さい方は常に
 * int型に昇格する。一方のオペランドが他方より大きい場合（int型とlog型など）
 * 小さいほうの型が大きいほうの型に昇格する。
 * @param lhs 左辺のノード
 * @param rhs 右辺のノード
 */
void Type::usual_arith_conv(unique_ptr<Node> &lhs, unique_ptr<Node> &rhs)
{
	auto ty = Type::get_common_type(lhs->_ty, rhs->_ty);
	lhs = Node::new_cast(move(lhs), ty);
	rhs = Node::new_cast(move(rhs), ty);
}

/**
 * @brief enum型の型を返す
 *
 * @return enum型の型
 */
shared_ptr<Type> Type::enum_type()
{
	return make_shared<Type>(TypeKind::TY_ENUM, 4, 4);
}

/**
 * @brief 構造体型の型を生成して返す
 *
 * @return 構造体型の型
 */
shared_ptr<Type> Type::struct_type()
{
	return make_shared<Type>(TypeKind::TY_STRUCT, 0, 1);
}

/**
 * @brief 構造体型の型の実体を新しく複製して返す
 *
 * @param ty 複製元
 * @return 複製した構造体型の型
 */
shared_ptr<Type> Type::copy_struct_type(shared_ptr<Type> ty)
{
	/* 実体をコピー */
	ty = make_shared<Type>(*ty);

	auto head = make_shared<Member>();
	auto cur = head.get();

	/* メンバの実体をコピー */
	for (auto mem = ty->_members.get(); mem; mem = mem->_next.get())
	{
		cur->_next = make_shared<Member>(*mem);
		cur = cur->_next.get();
	}

	ty->_members = head->_next;
	return ty;
}