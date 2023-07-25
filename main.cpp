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

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		error(std::string(argv[0]) + ": 引数の個数が正しくありません");
	}

	/* 入力文字列をトークナイズする */
	auto token = Token::tokenize(std::string(argv[1]));

	/* トークン列をパースし抽象構文木を構築する */
	auto program = Node::parse(std::move(token));

	/* 抽象構文木を巡回しながらコード生成 */
	CodeGen::generate_code(std::move(program));

	return 0;
}