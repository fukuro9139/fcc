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
			in->_output_path = args[i];
			continue;
		}

		if ("-S" == args[i])
		{
			in->_opt_S = true;
			continue;
		}

		if (args[i].starts_with("-o"))
		{
			in->_output_path = args[i].substr(2);
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

		in->_input_path = args[i];
	}

	if (in->_input_path.empty())
	{
		std::cerr << "入力ファイルが指定されていません\n";
		exit(1);
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
	std::cerr << "fcc [ -o <path> ] <file>" << std::endl;
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
	
	if(dot_pos == string::npos){
		return path + extn;
	}

	return path.substr(0, dot_pos) + extn;
    
}