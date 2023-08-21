/**
 * @file main.cpp
 * @author K.Fukunaga
 * @brief x86-64用で動作するC言語コンパイラです。
 *
 * 以下の、順序に従ってコンパイル、アセンブル、リンクを行う。
 * 	- 字句解析：文字列をトークンに変換
 *  - プリプロセス
 * 	- 構文解析：トークン・リストを抽象構文木に変換
 * 	- コード生成：アセンブリを生成
 *  - 'as'コマンドを呼び出しアセンブルする
 *  - 'ld'コマンドを呼び出しリンクする
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

/**
 * @brief -fccオプションを引数に追加した上でで子プロセスとしてfccを起動する。
 * input_pathのファイルをコンパイルしoutput_pathに出力する。
 *
 * @param args もともとの引数
 * @param input_path 入力先
 * @param output_path 出力先
 */
void run_fcc(const vector<string> &args, const string &input_path, const string &output_path)
{
	auto cmd(args);
	cmd.emplace_back("-fcc");
	cmd.emplace_back("-fcc-input");
	cmd.emplace_back(input_path);
	cmd.emplace_back("-fcc-output");
	cmd.emplace_back(output_path);

	run_subprocess(cmd);
}

/**
 * @brief 初期化を行う
 * @param in 入力引数
 */
void initialize(const unique_ptr<Input> &in)
{
	init_warning_level(in->_opt_w ? 0 : 1);
}

/**
 * @brief コンパイルを実行する
 *
 * @param input_path 入力先
 * @param output_path 出力先
 */
void fcc(const unique_ptr<Input> &in, const string &input_path, const string &output_path)
{
	/* 初期化 */
	initialize(in);

	/* 入力ファイルをトークナイズする */
	auto token = Token::tokenize_file(input_path);

	/* プリプロセス */
	token = PreProcess::preprocess(move(token), in);

	/* -Eオプションが指定されている場合はプリプロセス済ファイルを出力 */
	if (in->_opt_E)
	{
		Token::print_token(token, in->_output_path.empty() ? "-" : in->_output_path);
		return;
	}

	/* トークン列をパースし抽象構文木を構築する */
	auto program = Node::parse(token);

	/* 抽象構文木を巡回しながらコード生成 */
	CodeGen::generate_code(program, input_path, output_path, in->_opt_g);
}

/**
 * @brief メイン処理
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
	/* 入力をvectorに変換 */
	vector<string> args(argv, argv + argc);
	/* 引数を解析してオプションを判断 */
	auto in = Input::parse_args(args);
	/* リンクを行うファイル */
	vector<string> ld_args;

	/* -fccオプションが指定されている場合は-fcc_input, -fcc_outputを入力、出力先としてコンパイルを実行 */
	if (in->_opt_fcc)
	{
		fcc(in, in->_fcc_input, in->_fcc_output);
		return 0;
	}

	/* 入力ファイルが複数存在するとき出力先は指定できない */
	if (in->_inputs.size() > 1 && !in->_output_path.empty() && (in->_opt_c || in->_opt_S || in->_opt_E))
	{
		error("入力ファイルが複数ある時に-oオプションは-c, -S, -Eオプションと併用できません");
	}

	for (const auto &input : in->_inputs)
	{
		/* 出力先 */
		string output_path;

		/* 出力先の指定があれば指定先 */
		if (!in->_output_path.empty())
		{
			output_path = in->_output_path;
		}
		/* 入力が標準入力なら標準出力から出力 */
		else if (input._name == "-")
		{
			output_path = "-";
		}

#if __linux__

		/* ファイル名は入力ファイルと同じにする */
		else if (in->_opt_S)
		{
			output_path = Input::replace_extension(input._name, ".s");
		}
		else
		{
			output_path = Input::replace_extension(input._name, ".o");
		}

#else /* __linux__ */

		else
		{
			output_path = Input::replace_extension(input._name, ".s");
		}

#endif /* __linux__ */

#if __linux__

		/* 入力ファイルの拡張子が".o"の場合 */
		if (input._type == FileType::FILE_OBJ)
		{
			ld_args.emplace_back(input._name);
			continue;
		}

		/* アセンブリファイルとして指定されているか入力ファイルの拡張子が".s"の場合 */
		if (input._type == FileType::FILE_ASM)
		{
			/* -Sオプションが入っていなければアセンブルする */
			if (!in->_opt_S)
			{
				PostProcess::assemble(input._name, output_path);
			}
			continue;
		}

		assert(input._type == FileType::FILE_C);

		if (in->_opt_E)
		{
			run_fcc(args, input._name, output_path);
			continue;
		}

		/* -Sオプションが指定されていれば単にコンパイルするだけ */
		if (in->_opt_S)
		{
			run_fcc(args, input._name, output_path);
			continue;
		}

		/* -cオプションが指定されていればコンパイル後アセンブル */
		if (in->_opt_c)
		{
			/* 一時ファイルを作成 */
			auto tmpfile = PostProcess::create_tmpfile();
			/* アセンブリコードを生成 */
			run_fcc(args, input._name, tmpfile);
			/* アセンブル */
			PostProcess::assemble(tmpfile, output_path);
			continue;
		}

		/* それ以外はコンパイル、アセンブル、リンクしたファイルを最終生成物とする */

		/* 一時ファイルを作成 */
		auto tmpfile1 = PostProcess::create_tmpfile();
		auto tmpfile2 = PostProcess::create_tmpfile();
		/* アセンブリコードを生成 */
		run_fcc(args, input._name, tmpfile1);
		/* アセンブル */
		PostProcess::assemble(tmpfile1, tmpfile2);
		/* リンク対象のリストに追加 */
		ld_args.emplace_back(tmpfile2);

#else

		run_fcc(args, input._name, output_path);

#endif /* __linux__ */
	}

#if __linux__
	/* リンク */
	if (!ld_args.empty())
	{
		PostProcess::run_linker(ld_args, in->_output_path.empty() ? "a.out" : in->_output_path);
	}

#endif /* __linux__ */

	return 0;
}