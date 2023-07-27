/**
 * @file codegen.hpp
 * @author K.Fukunaga
 * @brief 抽象構文木(AST)を意味解析してアセンブリを出力するコードジェネレーターの定義。
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once
#include "parse.hpp"
#include <cassert>
#include <memory>

/** @brief　アセンブリを生成 */
class CodeGen
{
public:
	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static void generate_code(std::unique_ptr<Object> &&program);

private:

	/* このクラスのインスタンス化は禁止 */
	CodeGen();

	/**************************/
	/* 静的メンバ関数 (private) */
	/**************************/

	static void push();
	static void pop(std::string &&reg);
	static void pop(const std::string &reg);
	static void load(const std::shared_ptr<Type> &ty);
	static void store();
	static void generate_address(std::unique_ptr<Node> &&node);
	static void generate_expression(std::unique_ptr<Node> &&node);
	static void generate_statement(std::unique_ptr<Node> &&node);
	static int label_count();
};
