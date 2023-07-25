#include "type.hpp"
#include "parse.hpp"

using std::shared_ptr;

/**************/
/* Type Class */
/**************/

const std::shared_ptr<Type> Type::INT_BASE = std::make_shared<Type>(TypeKind::TY_INT);

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
			error_at("デリファレンスできるのはポインタ型のみです", std::move(node->_location));
		else
			/* ポインタのベース型 */
			node->_ty = node->_lhs->_ty->_base;

		return;

	default:
		break;
	}
}

/**
 * @brief base型へのポインター型を生成して返す
 *
 * @param base 参照する型
 * @return baseを参照するポインター型
 */
shared_ptr<Type> Type::pointer_to(shared_ptr<Type> &&base)
{
	auto ty = std::make_shared<Type>(TypeKind::TY_PTR);
	ty->_base = std::move(base);
	return ty;
}

/**
 * @brief base型へのポインター型を生成して返す
 *
 * @param base 参照する型
 * @return baseを参照するポインター型
 */
shared_ptr<Type> Type::pointer_to(const shared_ptr<Type> &base)
{
	auto ty = std::make_shared<Type>(TypeKind::TY_PTR);
	ty->_base = base;
	return ty;
}

/**
 * @brief 戻り値の型がreturn_ty型である関数型を生成し返す
 *
 * @param return_ty 戻り値の型
 * @return 戻り値がreturn_ty型の関数型
 */
shared_ptr<Type> Type::func_type(shared_ptr<Type> &&return_ty)
{
	auto ty = std::make_shared<Type>(TypeKind::TY_FUNC);
	ty->_location = return_ty->_location;
	ty->_length = return_ty->_length;
	ty->_return_ty = std::move(return_ty);
	return ty;
}
