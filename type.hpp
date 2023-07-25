#pragma once

#include <memory>
#include <string>

/* 型の種類 */
enum class TypeKind
{
	TY_INT,	 /*!< int型 */
	TY_PTR,	 /*!< ポインター型 */
	TY_FUNC, /*!< 関数 */
};

/* Nodeクラスを使いたいので先に宣言 */
class Node;

/** @brief 型を表すクラス */
class Type
{
public:
	/* メンバ変数 (public) */

	const TypeKind _kind;					/*!< 型の種類 */
	const std::shared_ptr<Type> _base;		/*!< kindがTY_PTRのとき、参照先の型 */
	std::string _name;						/*!< 変数の名前 */
	int _location;							/*!< 変数に対応する入力文字列の位置 */
	const std::shared_ptr<Type> _return_ty; /*!< kindがTY_FUNCのとき、戻り値の型 */

	/* コンストラクタ */

	Type(const TypeKind &kind = TypeKind::TY_INT,
		 const std::shared_ptr<Type> &base = nullptr,
		 const std::string &name = "",
		 const int &location = 0,
		 const std::shared_ptr<Type> &return_ty = nullptr);

	/* メンバ関数 (public) */

	bool is_integer() const;

	/* 静的メンバ関数 (public) */

	static void add_type(Node *node);
	static std::shared_ptr<Type> pointer_to(const std::shared_ptr<Type> &base);
	static std::shared_ptr<Type> func_type(const std::shared_ptr<Type> &return_ty);

	/* 静的メンバ変数 (public) */

	static const std::shared_ptr<Type> INT_BASE; /*!< int型 */
};

/**
 * @brief 入力された型がint型かどうか判定
 *
 * @param ty 対象の型
 * @return int型である:true, int型でない:false
 */
inline bool Type::is_integer() const
{
	return TypeKind::TY_INT == this->_kind;
}

/**
 * @brief base型へのポインター型を生成して返す
 *
 * @param base 参照する型
 * @return baseを参照するポインター型
 */
inline std::shared_ptr<Type> Type::pointer_to(const std::shared_ptr<Type> &base)
{
	return std::make_shared<Type>(TypeKind::TY_PTR, base);
}

/**
 * @brief 戻り値の型がreturn_ty型である関数型を生成し返す
 *
 * @param return_ty 戻り値の型
 * @return 戻り値がreturn_ty型の関数型
 */
inline std::shared_ptr<Type> Type::func_type(const std::shared_ptr<Type> &return_ty)
{
	return std::make_shared<Type>(TypeKind::TY_FUNC, nullptr, return_ty->_name, return_ty->_location, return_ty);
}