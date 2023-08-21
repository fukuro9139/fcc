/**
 * @file input.hpp
 * @author K.Fukunaga
 * @brief 入力された引数を解析するためのクラス
 * @version 0.1
 * @date 2023-08-01
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include "common.hpp"

/**
 * @brief 入力に関連する操作をまとめたクラス
 *
 */
class Input
{
public:
	/** 入力ファイルの種類 */
	enum class FileType
	{
		FILE_NONE, /*!< 指定なし */
		FILE_C,	   /*!< Cソースファイル */
		FILE_ASM,  /*!< アセンブリファイル */
		FILE_OBJ   /*!< オブジェクトファイル */
	};

	/** 入力ファイルを表す構造体 */
	struct InputFile
	{
		string _name = "";					  /*!< ファイル名 */
		FileType _type = FileType::FILE_NONE; /*!<  ファイルの種類*/
		InputFile(const string &name, const FileType &type) : _name(name), _type(type) {}
	};

	/* メンバ変数(public) */
	vector<InputFile> _inputs; /*!< インプットファイルパス */
	vector<string> _include;   /*!< インクルードパス */
	string _output_path = "";  /*!< アウトプットファイルパス */
	string _fcc_input = "";	   /*!< -fccオプションが指定されている時の入力先 */
	string _fcc_output = "";   /*!< -fccオプションが指定されている時の出力先 */

	bool _opt_g = false;   /*!< -gオプションが指定されているか */
	bool _opt_S = false;   /*!< -Sオプションが指定されているか */
	bool _opt_c = false;   /*!< -cオプションが指定されているか */
	bool _opt_E = false;   /*!< -Eオプションが指定されているか */
	bool _opt_fcc = false; /*!< -fccオプションが指定されているか */
	bool _opt_w = false;   /*!< -wオプションが指定されているか */

	/* 静的メンバ関数(public) */
	static unique_ptr<Input> parse_args(const std::vector<string> &args);
	static string replace_extension(const string &path, const string &extn);

private:
	/* 静的メンバ関数(input) */
	static void usage(int status);
	static bool take_arg(const string &arg);
	static FileType get_file_type(const string &filename);

	static FileType opt_x;
	static const std::unordered_map<string, FileType> filetype_table;
};

using FileType = Input::FileType;