/**
 * @file input.cpp
 * @author K.Fukunaga
 * @brief 入力された引数を解析するためのクラス
 * @version 0.1
 * @date 2023-08-01
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "input.hpp"

Options Input::opt;

/**
 * @brief コマンドライン引数を解析する。読み取ったオプションはOptions構造体に格納。
 *
 * @param args コマンドライン引数を格納したvector<string>
 */
void Input::parse_args(const std::vector<std::string> &args)
{
	/* args[0]は実行ファイルのパス */
	for (size_t i = 1, sz = args.size(); i < sz; ++i)
	{
		if ("--help" == args[i])
		{
			usage(0);
		}

		if ("-o" == args[i])
		{
			/* "-o" オプションの後に何も引数がなければエラー終了 */
			if (++i == sz)
			{
				usage(1);
			}
			opt.opt_o = args[i];
			continue;
		}

		if (args[i].starts_with("-o"))
		{
			opt.opt_o = args[i].substr(2);
			continue;
		}

		if (args[i][0] == '-')
		{
			if (args[i].size() >= 2)
			{
				std::cerr << "不明なオプションです: " << args[i] << "\n";
				std::cerr << "fccでは下記のオプションが使えます\n";
				usage(1);
			}
		}

		opt.input_path = args[i];
	}

	if (opt.input_path.size() == 0)
	{
		std::cerr << "入力ファイルが存在しません\n";
		exit(1);
	}
}

/**
 * @brief helpを表示して終了する
 *
 * @param status 終了ステータス
 */
void Input::usage(int status)
{
	std::cerr << "fcc [ -o <path> ] <file>" << std::endl;
	exit(status);
}

/**
 * @brief
 *
 * @param path 入力ファイルのパス
 * @return 入力ファイルの内容をまとめた文字列
 */
std::string Input::read_file(const std::string &path)
{
	std::string input_data;
	if (path == "-")
	{
		std::string buf;
		/* 標準入力から読み取れなくなるまで読み込みを続ける */
		while (std::getline(std::cin, buf))
		{
			if (buf.empty())
			{
				break;
			}
			else
			{
				input_data += buf;
			}
		}
	}
	else
	{
		std::ifstream ifs(path);
		if (!ifs)
		{
			std::cerr << "ファイルが開けませんでした： " << path << std::endl;
			exit(1);
		}

		/* ファイルから読み込む */
		input_data = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
	}

	/* ファイルが空または改行で終わっていない場合、'\n'を付け加える */
	if (input_data.size() == 0 || input_data.back() != '\n')
	{
		input_data.push_back('\n');
	}

	return input_data;
}