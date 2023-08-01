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

#include <iostream>
#include <fstream>
#include <cassert>
#include <memory>
#include <vector>
#include "parse.hpp"

/** @brief　アセンブリを生成 */
class CodeGen
{
public:
	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static void generate_code(const std::unique_ptr<Object> &program, const std::string &input_path, const std::string &output_path);

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
	static void store(const std::shared_ptr<Type> &ty);
	static void generate_address(Node *node);
	static void generate_expression(Node *node);
	static void generate_statement(Node *node);
	static void store_gp(const int &r, const int &offset, const int &sz);
	static void emit_data(const std::unique_ptr<Object> &program);
	static void emit_text(const std::unique_ptr<Object> &program);
	static int label_count();
};
