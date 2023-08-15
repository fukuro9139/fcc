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
#include "postprocess.hpp"
#include "preprocess.hpp"
#include "common.hpp"
#include "error.hpp"

void run_fcc(const string &input_path, const string &output_path)
{
	/* 入力ファイルをトークナイズする */
	auto token = Token::tokenize_file(input_path);
	if (!token)
	{
		error("トークナイズに失敗しました: \"" + input_path + "\"");
	}

	/* プリプロセス */
	token = PreProcess::preprocess(move(token));

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

	/* リンクを行うファイル */
	vector<string> ld_args;

	/* 入力ファイルが複数存在するとき出力先は指定できない */
	if (in->_inputs.size() > 1 && !in->_output_path.empty() && (in->_opt_c || in->_opt_S))
	{
		error("入力ファイルが複数ある時に-oオプションは-cまたは-Sオプションと併用できません");
	}

	for (const auto &input_path : in->_inputs)
	{

		/* 出力先 */
		string output_path;

		/* 出力先の指定があれば指定先 */
		if (!in->_output_path.empty())
		{
			output_path = in->_output_path;
		}
		/* 入力が標準入力なら標準出力から出力 */
		else if (input_path == "-")
		{
			output_path = "-";
		}

#ifdef WINDOWS

		else
		{
			output_path = Input::replace_extension(input_path, ".s");
		}

#else /* WINDOWS */

		/* ファイル名は入力ファイルと同じにする */
		else if (in->_opt_S)
		{
			output_path = Input::replace_extension(input_path, ".s");
		}
		else
		{
			output_path = Input::replace_extension(input_path, ".o");
		}

#endif /* WINDOWS */

#ifdef WINDOWS

		run_fcc(input_path, output_path);

#else
		/* 入力ファイルの拡張子が".o"の場合 */
		if (input_path.ends_with(".o"))
		{
			ld_args.emplace_back(input_path);
			continue;
		}

		/* 入力ファイルの拡張子が".s"の場合 */
		if (input_path.ends_with(".s"))
		{
			/* -Sオプションが入っていなければアセンブルする */
			if (!in->_opt_S)
			{
				PostProcess::assemble(input_path, output_path);
			}
			continue;
		}

		/* 入力ファイルの拡張子が".c"以外の場合 */
		if (!input_path.ends_with(".c") && input_path != "-")
		{
			error("不明な拡張子です: " + input_path);
		}

		/* -Sオプションが指定されていれば単にコンパイルするだけ */
		if (in->_opt_S)
		{
			run_fcc(input_path, output_path);
			continue;
		}

		/* -cオプションが指定されていればコンパイル後アセンブル */
		if (in->_opt_c)
		{
			/* 一時ファイルを作成 */
			auto tmpfile = PostProcess::create_tmpfile();
			/* アセンブリコードを生成 */
			run_fcc(input_path, tmpfile);
			/* アセンブル */
			PostProcess::assemble(tmpfile, output_path);
			continue;
		}

		/* それ以外はコンパイル、アセンブル、リンクしたファイルを最終生成物とする */

		/* 一時ファイルを作成 */
		auto tmpfile1 = PostProcess::create_tmpfile();
		auto tmpfile2 = PostProcess::create_tmpfile();
		/* アセンブリコードを生成 */
		run_fcc(input_path, tmpfile1);
		/* アセンブル */
		PostProcess::assemble(tmpfile1, tmpfile2);
		/* リンク対象のリストに追加 */
		ld_args.emplace_back(tmpfile2);
#endif /* WINDOWS */
	}

#ifndef WINDOWS
	/* リンク */
	if (!ld_args.empty())
	{
		PostProcess::run_linker(ld_args, in->_output_path.empty() ? "a.out" : in->_output_path);
	}

#endif /* WINDOWS */

	return 0;
}