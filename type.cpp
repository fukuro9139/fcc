#include "type.hpp"
#include "parse.hpp"

using std::shared_ptr;

/**************/
/* Type Class */
/**************/

const shared_ptr<Type> Type::INT_BASE = std::make_shared<Type>(TypeKind::TY_INT, 8);

Type::Type() : _kind(TypeKind::TY_INT) {}

Type::Type(const TypeKind &kind, const int &size) : _kind(kind), _size(size) {}

Type::Type(const TypeKind &kind) : _kind(kind) {}

Type::Type(const shared_ptr<Type> &base, const int &size) : _kind(TypeKind::TY_PTR), _base(base), _size(size) {}

Type::Type(const std::string &name, const int &location, const shared_ptr<Type> &return_ty)
	: _kind(TypeKind::TY_FUNC), _name(name), _location(location), _return_ty(return_ty) {}

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
			error_at("左辺値ではありません", node->_lhs->_location);
		}
		node->_ty = node->_lhs->_ty;
		return;

	/* 比較演算の結果はint型 */
	case NodeKind::ND_EQ:
	case NodeKind::ND_NE:
	case NodeKind::ND_LT:
	case NodeKind::ND_LE:
	case NodeKind::ND_NUM:
	case NodeKind::ND_FUNCALL:
		node->_ty = Type::INT_BASE;
		return;

	case NodeKind::ND_VAR:
		/* オブジェクトの型に一致させる */
		node->_ty = node->_var->_ty;
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
			error_at("デリファレンスできるのはポインタ型のみです", node->_location);
		else
			/* ポインタのベース型 */
			node->_ty = node->_lhs->_ty->_base;

		return;

	default:
		break;
	}
}

/**
 * @brief 入力された型がint型かどうか判定
 *
 * @param ty 対象の型
 * @return int型である:true, int型でない:false
 */
bool Type::is_integer() const
{
	return TypeKind::TY_INT == this->_kind;
}

/**
 * @brief base型へのポインター型を生成して返す
 *
 * @param base 参照する型
 * @return baseを参照するポインター型
 */
shared_ptr<Type> Type::pointer_to(const shared_ptr<Type> &base)
{
	return std::make_shared<Type>(base, 8);
}

/**
 * @brief 戻り値の型がreturn_ty型である関数型を生成し返す
 *
 * @param return_ty 戻り値の型
 * @return 戻り値がreturn_ty型の関数型
 */
shared_ptr<Type> Type::func_type(const shared_ptr<Type> &return_ty)
{
	return std::make_shared<Type>(return_ty->_name, return_ty->_location, return_ty);
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
	auto ret = std::make_shared<Type>(TypeKind::TY_ARRAY);
	ret->_size = base->_size * length;
	ret->_base = base;
	ret->_array_length = length;
	return ret;
}
