#pragma once

#include <memory>
#include <string>

/* 型の種類 */
enum class TypeKind
{
	TY_INT,	  /*!< int型 */
	TY_PTR,	  /*!< ポインター型 */
	TY_FUNC,  /*!< 関数 */
	TY_ARRAY, /*!< 関数  */
};

/* Nodeクラスを使いたいので先に宣言 */
class Node;

/**
 * @brief 型を表すクラス
 *
 */
class Type
{
public:
	/* メンバ変数 (public) */

	const TypeKind _kind; /*!< 型の種類 */
	int _size = 0;		  /* 型のサイズ */

	/**
	 * @brief kindがTY_PTRまたはTY_ARRAYのとき、参照先の型
	 *
	 * ポインタと配列の二重性を表すために同じメンバを使う。
	 * "kind"の代わりに"base"を調べることで型がポインタかどうか判断する。
	 */
	std::shared_ptr<Type> _base;

	std::string _name = ""; /*!< 変数の名前 */
	int _location = 0;		/*!< 変数に対応する入力文字列の位置 */

	/* 配列 */
	int _array_length = 0; /*!< 配列の長さ */

	/* 関数 */
	std::shared_ptr<Type> _return_ty; /*!< kindがTY_FUNCのとき、戻り値の型 */
	std::shared_ptr<Type> _params;	  /*!< 引数の型  */
	std::shared_ptr<Type> _next;	  /*!< リストの次の型 */

	/* コンストラクタ */
	Type();
	Type(const TypeKind &kind, const int &size);
	Type(const TypeKind &kind);
	Type(const std::shared_ptr<Type> &base, const int &size);
	Type(const std::string &name, const int &location, const std::shared_ptr<Type> &return_ty);

	/* メンバ関数 (public) */

	bool is_integer() const;

	/* 静的メンバ関数 (public) */

	static void add_type(Node *node);
	static std::shared_ptr<Type> pointer_to(const std::shared_ptr<Type> &base);
	static std::shared_ptr<Type> array_of(std::shared_ptr<Type> base, int length);
	static std::shared_ptr<Type> func_type(const std::shared_ptr<Type> &return_ty);

	/* 静的メンバ変数 (public) */

	static const std::shared_ptr<Type> INT_BASE; /*!< int型 */
};
