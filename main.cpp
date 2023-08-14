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
#include "assembler.hpp"
#include "common.hpp"

void run_fcc(const string &input_path, const string &output_path)
{
	/* 入力ファイルをトークナイズする */
	auto token = Token::tokenize_file(input_path);

	/* トークン列をパースし抽象構文木を構築する */
	auto program = Node::parse(token);

	/* 抽象構文木を巡回しながらコード生成 */
	CodeGen::generate_code(program, input_path, output_path);
}

int main(int argc, char **argv)
{
	/* 入力をvectorに変換 */
	vector<string> args(argv, argv + argc);

	/* 引数を解析してオプションを判断 */
	auto in = Input::parse_args(args);

	/* 出力先 */
	string output_path;

	/* 出力先の指定があれば指定先 */
	if (!in->_output_path.empty())
	{
		output_path = in->_output_path;
	}
	/* 入力が標準入力なら標準出力から出力 */
	else if (in->_input_path == "-")
	{
		output_path = "-";
	}

#ifdef WINDOWS

	else
	{
		output_path = Input::replace_extension(in->_input_path, ".s");
	}

#else /* WINDOWS */

	/* ファイル名は入力ファイルと同じにする */
	else if (in->_opt_S)
	{
		output_path = Input::replace_extension(in->_input_path, ".s");
	}
	else
	{
		output_path = Input::replace_extension(in->_input_path, ".o");
	}

#endif /* WINDOWS */

#ifdef WINDOWS

	run_fcc(in->_input_path, output_path);

#else
	/* -Sオプションがついていれば最終生成物はアセンブリコード */
	if (in->_opt_S)
	{
		run_fcc(in->_input_path, output_path);
	}

	/* それ以外はアセンブルしたファイルを最終生成物とする */
	else
	{
		/* 一時ファイルを作成 */
		auto tmpfile = Assembler::create_tmpfile();

		/* アセンブリコードを生成 */
		run_fcc(in->_input_path, tmpfile);

		/* アセンブル */
		Assembler::assemble(tmpfile, output_path);
	}

#endif /* WINDOWS */

	return 0;
}