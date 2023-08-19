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

class Input
{
public:
	/* メンバ変数(public) */
	vector<string> _inputs;	  /*!< インプットファイルパス */
	vector<string> _include;  /*!< インクルードパス */
	string _output_path = ""; /*!< アウトプットファイルパス */
	string _fcc_input = "";	  /*!< -fccオプションが指定されている時の入力先 */
	string _fcc_output = "";  /*!< -fccオプションが指定されている時の出力先 */

	bool _opt_g = false;   /*!< -gオプションが指定されているか */
	bool _opt_S = false;   /*!< -Sオプションが指定されているか */
	bool _opt_c = false;   /*!< -cオプションが指定されているか */
	bool _opt_E = false;   /*!< -Eオプションが指定されているか */
	bool _opt_fcc = false; /*!< -fccオプションが指定されているか */

	/* 静的メンバ関数(public) */
	static unique_ptr<Input> parse_args(const std::vector<string> &args);
	static string replace_extension(const string &path, const string &extn);

private:
	/* 静的メンバ関数(input) */
	static void usage(int status);
	static bool take_arg(const string &arg);
};