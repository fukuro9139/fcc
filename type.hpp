#pragma once

#include <memory>
#include <string>

/* 型の種類 */
enum class TypeKind
{
	TY_INT, /*!< int型 */
	TY_PTR, /*!< ポインター型 */
};

/* Node, Tokenクラスを使いたいので先に宣言 */
class Node;
class Token;

/** @brief 型を表すクラス */
class Type
{
public:
	/**********************/
	/* メンバ変数 (public) */
	/**********************/

	TypeKind _kind;							/*!< 型の種類 */
	std::shared_ptr<Type> _base = nullptr;	/*!< kindがTY_PTRのとき、参照先の型 */
	std::unique_ptr<Token> _name = nullptr; /* 宣言しているトークン */

	/*****************/
	/* コンストラクタ */
	/*****************/

	Type();
	Type(TypeKind &&kind);
	Type(const std::shared_ptr<Type> &base);
	Type(std::shared_ptr<Type> &&base);

	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	/**
	 * @brief 入力された型がint型かどうか判定
	 *
	 * @param ty 対象の型
	 * @return int型である:true, int型でない:false
	 */
	static bool is_integer(const std::shared_ptr<Type> &ty);

	/**
	 * @brief 抽象構文木(AST)を巡回しながら型情報を設定する。
	 *
	 * @param node ASTのノード
	 */
	static void add_type(Node *node);
};

std::shared_ptr<Type> ty_int();