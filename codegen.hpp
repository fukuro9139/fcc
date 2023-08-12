/**
 * @file codegen.hpp
 * @author K.Fukunaga
 * @brief 抽象構文木(AST)を意味解析してアセンブリを出力するコードジェネレーターの定義。
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include "parse.hpp"
#include "common.hpp"

/* サイズにより型を分類したID */
enum class TypeID
{
	I8,
	I16,
	I32,
	I64,
	U8,
	U16,
	U32,
	U64,
};

/** @brief　アセンブリを生成 */
class CodeGen
{
public:
	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static void generate_code(const unique_ptr<Object> &program, const string &input_path, const string &output_path);

private:
	/* このクラスのインスタンス化は禁止 */
	CodeGen();

	/**************************/
	/* 静的メンバ関数 (private) */
	/**************************/

	static void push();
	static void pop(string &&reg);
	static void pop(const string &reg);
	static void load(const Type *ty);
	static void store(const Type *ty);
	static void cmp_zero(const Type *ty);
	static void generate_address(Node *node);
	static void generate_expression(Node *node);
	static void generate_statement(Node *node);
	static void store_gp(const int &r, const int &offset, const int &sz);
	static void emit_data(const unique_ptr<Object> &program);
	static void emit_text(const unique_ptr<Object> &program);
	static int label_count();
	static void cast(Type *from, Type *to);
	static TypeID get_TypeId(Type *ty);
};
