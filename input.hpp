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
	vector<string> _inputs;	/*!< インプットファイルパス */
	string _output_path = ""; /*!< アウトプットファイルパス */
	bool _opt_S = false;			/*!< -Sオプションが指定されているか */

	/* 静的メンバ関数(public) */
	static unique_ptr<Input> parse_args(const std::vector<std::string> &args);
	static string replace_extension(const string &path, const string &extn);

private:
	/* 静的メンバ関数(input) */
	static void usage(int status);
};