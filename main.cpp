/**
 * @file main.cpp
 * @author K.fukunaga
 * @brief C言語をコンパイルしてアセンブリを出力するx86-64用のコンパイラです。
 *
 * 以下の、順序に従ってコンパイルを行う。
 * 	- 字句解析：文字列をトークンに変換
 * 	- 構文解析：トークン・リストを抽象構文木に変換
 * 	- コード生成：アセンブリを生成
 * 	.
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "codegen.hpp"
#include "parse.hpp"
#include "tokenize.hpp"
#include "input.hpp"

int main(int argc, char **argv)
{
	/* 入力をvectorに変換 */
	std::vector<std::string> args(argv, argv + argc);
	/* 引数を解析してオプションを判断 */
	Input::parse_args(args);

	/* 入力ファイルをトークナイズする */
	auto token = Token::tokenize_file(Input::opt.input_path);

	/* トークン列をパースし抽象構文木を構築する */
	auto program = Node::parse(token.get());

	/* 抽象構文木を巡回しながらコード生成 */
	CodeGen::generate_code(program, Input::opt.input_path, Input::opt.opt_o);

	return 0;
}