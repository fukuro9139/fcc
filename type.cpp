#include "type.hpp"
#include "parse.hpp"

/** @brief int型を表す型クラスのオブジェクトを生成 */
static std::shared_ptr<Type> ty_int = std::make_shared<Type>(TypeKind::TY_INT);

/**************/
/* Type Class */
/**************/

/** @brief デフォルトコンストラクタ  */
Type::Type() = default;

/**
 * @brief 型の種類を指定してオブジェクトを生成。
 *
 * @param kind 型
 */
Type::Type(TypeKind &&kind)
	: _kind(std::move(kind))
{
}

Type::Type(const std::shared_ptr<Type> &base)
	: _kind(TypeKind::TY_PTR), _base(base)
{
}

bool Type::is_integer(const std::shared_ptr<Type> &ty)
{
	return TypeKind::TY_INT == ty->_kind;
}

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

	for (Node *n = node->_body.get(); n; n = n->_next.get())
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
	case NodeKind::ND_ASSIGN:
		node->_ty = node->_lhs->_ty;
		return;

	/* 比較演算の結果はint型 */
	case NodeKind::ND_EQ:
	case NodeKind::ND_NE:
	case NodeKind::ND_LT:
	case NodeKind::ND_LE:
	case NodeKind::ND_VAR:
	case NodeKind::ND_NUM:
		node->_ty = ty_int;
		return;
	/* 参照は参照先へのポインタ型 */
	case NodeKind::ND_ADDR:
		node->_ty = std::make_shared<Type>(node->_lhs->_ty);
		return;
	/* デリファレンス */
	case NodeKind::ND_DEREF:
		if (TypeKind::TY_PTR == node->_lhs->_ty->_kind)
			/* デリファレンスの対象がポインタ型ならポインタのベース型 */
			node->_ty = node->_lhs->_ty->_base;
		else
			/* デリファレンスの対象がポインタでないならint型を設定 */
			node->_ty = ty_int;
		return;
	default:
		break;
	}
}
