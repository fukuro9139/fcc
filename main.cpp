/**
 * @file main.cpp
 * @author K.Fukunaga
 * @brief C言語をコンパイルしてアセンブリを出力するx86-64用のコンパイラです。
 *
 * 以下の、順序に従ってコンパイルを行う。
 * 	- 字句解析：文字列をトークンに変換
 * 	- 構文解析：トークン・リストを抽象構文木に変換
 * 	- コード生成：アセンブリを生成
 * 	.
 * @version 0.1
 * @date 2023-06-28
 *
 * @copyright Copyright (c) 2023 MIT Licence
 *
 */

#include "codegen.hpp"
#include "parse.hpp"
#include "tokenize.hpp"
#include "input.hpp"

int main(int argc, char **argv)
{
	/* 入力をvectorに変換 */
	vector<string> args(argv, argv + argc);
	
	/* 引数を解析してオプションを判断 */
	auto in = Input::parse_args(args);

	/* 入力ファイルをトークナイズする */
	auto token = Token::tokenize_file(in);

	/* トークン列をパースし抽象構文木を構築する */
	auto program = Node::parse(token);

	/* 抽象構文木を巡回しながらコード生成 */
	CodeGen::generate_code(program, in);

	return 0;
}