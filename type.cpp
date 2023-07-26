#include "type.hpp"
#include "parse.hpp"

using std::shared_ptr;

/**************/
/* Type Class */
/**************/

const std::shared_ptr<Type> Type::INT_BASE = std::make_shared<Type>(TypeKind::TY_INT);

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
	for(auto n = node->_args.get(); n; n = n->_next.get()){
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
		node->_ty = Type::pointer_to(node->_lhs->_ty);
		return;

	/* デリファレンス */
	case NodeKind::ND_DEREF:
		/* デリファレンスできるのはポインタ型のみ */
		if (TypeKind::TY_PTR != node->_lhs->_ty->_kind)
			error_at("デリファレンスできるのはポインタ型のみです", node->_location);
		else
			/* ポインタのベース型 */
			node->_ty = node->_lhs->_ty->_base;

		return;

	default:
		break;
	}
}
