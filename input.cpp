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

/**
 * @brief コマンドライン引数を解析する。
 *
 * @param args コマンドライン引数を格納したvector<string>
 * @return 読み取ったインプット情報を格納したInputオブジェクトのポインタ
 */
unique_ptr<Input> Input::parse_args(const std::vector<std::string> &args)
{
	auto in = make_unique<Input>();
	bool stdin_flg = false;

	/* args[0]は実行ファイルのパス */
	for (size_t i = 1, sz = args.size(); i < sz; ++i)
	{
		if (take_arg(args[i]))
		{
			if (i + 1 == sz)
			{
				usage(1);
			}
		}

		if ("--help" == args[i])
		{
			usage(0);
		}

		if ("-o" == args[i])
		{
			in->_output_path = args[++i];
			continue;
		}

		if (args[i].starts_with("-o"))
		{
			in->_output_path = args[i].substr(2);
			continue;
		}

		if ("-S" == args[i])
		{
			in->_opt_S = true;
			continue;
		}

		if ("-c" == args[i])
		{
			in->_opt_c = true;
			continue;
		}

		if ("-E" == args[i])
		{
			in->_opt_E = true;
			continue;
		}

		if (args[i].starts_with("-I"))
		{
			in->_include.emplace_back(args[i].substr(2));
			continue;
		}

		if ("-fcc" == args[i])
		{
			in->_opt_fcc = true;
			continue;
		}

		if ("-fcc-input" == args[i])
		{
			in->_fcc_input = args[++i];
			continue;
		}

		if ("-fcc-output" == args[i])
		{
			in->_fcc_output = args[++i];
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
			if (stdin_flg)
			{
				std::cerr << "標準入力を指定する'-'は１つのみ有効です\n";
				usage(1);
			}
			stdin_flg = true;
		}

		in->_inputs.emplace_back(args[i]);
	}

	if (in->_inputs.empty())
	{
		error("入力ファイルが指定されていません\n");
	}

	return in;
}

/**
 * @brief helpを表示して終了する
 *
 * @param status 終了ステータス
 */
void Input::usage(int status)
{
	std::cerr << "fcc [ -o <path> ] [ -S ] <file>\n";
	std::cerr << "fcc [ -S ] <file1> <file2> ..." << std::endl;
	exit(status);
}

/**
 * @brief ファイル名の拡張子を変更して返す
 *
 * @param path 入力ファイルのパス
 * @param extn 変更する拡張子
 * @return ファイルの拡張子を変更したパス
 */
string Input::replace_extension(const string &path, const string &extn)
{
	auto dot_pos = path.find_last_of('.');

	if (dot_pos == string::npos)
	{
		return path + extn;
	}

	return path.substr(0, dot_pos) + extn;
}

/**
 * @brief 引数が必要なオプションであるか（-o, -I）
 *
 * @param arg 入力引数
 * @return true 引数が必要なオプションである
 * @return false 引数が必要なオプションではない
 */
bool Input::take_arg(const string &arg)
{
	constexpr string_view ops[] = {"-o", "-I"};

	for (auto &x : ops)
	{
		if (arg == x)
		{
			return true;
		}
	}

	return false;
}