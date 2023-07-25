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
	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	TypeKind _kind;						   /*!< 型の種類 */
	std::shared_ptr<Type> _base = nullptr; /*!< kindがTY_PTRのとき、参照先の型 */
	size_t _length;						   /*!< 変数の長さ */
	std::string::const_iterator _location; /*!< 変数に対応する入力文字列の位置 */
	std::shared_ptr<Type> _return_ty;	   /*!< kindがTY_FUNCのとき、戻り値の型 */

	/*****************/
	/* コンストラクタ */
	/*****************/

	Type();
	Type(TypeKind &&kind);

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static bool is_integer(const std::shared_ptr<Type> &ty);
	static void add_type(Node *node);
	static std::shared_ptr<Type> pointer_to(std::shared_ptr<Type> &&base);
	static std::shared_ptr<Type> pointer_to(const std::shared_ptr<Type> &base);
	static std::shared_ptr<Type> func_type(std::shared_ptr<Type> &&return_ty);

	/**************************/
	/* 静的メンバ変数 (public) */
	/**************************/

	static const std::shared_ptr<Type> INT_BASE; /*!< int型 */
};