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
	static void generate_code(std::unique_ptr<const Node> node);

private:
	static void push();
	static void pop(const std::string &reg);

	static void generate_address(const Node *node);
	static void generate_expression(const Node *node);
	static void generate_statement(const Node *node);
};