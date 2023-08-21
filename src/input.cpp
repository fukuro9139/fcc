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

/** 現在の入力ファイルの種類の指定 */
FileType Input::opt_x = FileType::FILE_NONE;

/* 文字列とファイルタイプの対応テーブル */
const std::unordered_map<string, FileType> Input::filetype_table = {
	{"c", FileType::FILE_C},
	{"assembler", FileType::FILE_ASM},
	{"none", FileType::FILE_NONE},
};

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
				std::cerr << "オプション指定が正しくありません\n";
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

		if ("-g" == args[i])
		{
			in->_opt_g = true;
			continue;
		}

		if ("-w" == args[i])
		{
			in->_opt_w = true;
			continue;
		}

		if ("-x" == args[i])
		{
			if (filetype_table.contains(args[++i]))
			{
				opt_x = filetype_table.at(args[i]);
			}
			else
			{
				std::cerr << "オプション指定が正しくありません\n";
				usage(1);
			}
			continue;
		}

		if (args[i].starts_with("-x"))
		{
			auto t = args[i].substr(2);
			if (filetype_table.contains(t))
			{
				opt_x = filetype_table.at(t);
			}
			else
			{
				std::cerr << "オプション指定が正しくありません\n";
				usage(1);
			}
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

		if ("-I" == args[i])
		{
			in->_include.emplace_back(args[++i]);
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

		in->_inputs.emplace_back(args[i], get_file_type(args[i]));
	}

	if (in->_inputs.empty())
	{
		error("入力ファイルが指定されていません\n");
	}

	/* デフォルトのインクルードパスを設定する */
	auto path = fs::path(args[0]).parent_path() / "../include";
	in->_include.emplace_back(path.string());

	return in;
}

/**
 * @brief helpを表示して終了する
 *
 * @param status 終了ステータス
 */
void Input::usage(int status)
{
	std::cerr << "Usage: fcc [options] files...\n";
	std::cerr << "Options:\n";
	std::cerr << "  --help  ヘルプを表示します。\n";
	std::cerr << "  -o      出力ファイルの名前を指定します。\n";
	std::cerr << "  -g      オブジェクト・ファイルにデバッグ情報を生成します。\n";
	std::cerr << "  -w      すべての警告メッセージを無効にします。\n";
	std::cerr << "  -I      インクルード・ファイルの検索先に追加するディレクトリーを指定します。\n";
	std::cerr << "  -E      プリプロセスのみを行いコンパイル、アセンブル、リンクを行いません。\n";
	std::cerr << "  -S      コンパイルまでを行いアセンブル、リンクを行いません。\n";
	std::cerr << "  -c      リンクを抑止します。\n";
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
	constexpr string_view ops[] = {"-o", "-x", "-I"};

	for (auto &x : ops)
	{
		if (arg == x)
		{
			return true;
		}
	}

	return false;
}


/**
 * @brief ファイルの種類を判定する
 * 
 * @param filename ファイル名
 * @return 判定結果
 */
FileType Input::get_file_type(const string & filename)
{
	if(filename.ends_with(".o")){
		return FileType::FILE_OBJ;
	}

	if(FileType::FILE_NONE != opt_x){
		return opt_x;
	}

	if(filename.ends_with(".c") || filename.ends_with("h")){
		return FileType::FILE_C;
	}

	if(filename.ends_with(".s")){
		return FileType::FILE_ASM;
	}

	error("不明なファイル拡張子です: " + filename);
}