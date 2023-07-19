#pragma once
#include "parse.hpp"
#include <cassert>
#include <memory>

/**
 * @brief
 * Intel記法でアセンブラを出力する
 */
class CodeGen
{
public:
	static void generate_code(std::unique_ptr<Node> &&node);

private:
	static void push();
	static void pop(std::string &&reg);

	static void generate_address(std::unique_ptr<Node> &&node);
	static void generate_expression(std::unique_ptr<Node> &&node);
	static void generate_statement(std::unique_ptr<Node> &&node);
};