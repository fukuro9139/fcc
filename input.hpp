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

#include <fstream>
#include <sstream>
#include <iterator>
#include "common.hpp"

class Input
{
public:
	/* メンバ変数(public) */
	const string _input_path = "";
	const string _output_path = "";

	/* コンストラクタ */
	Input(const string &in, const string &out);

	/* メンバ関数(public) */
	string read_file() const;

	/* 静的メンバ関数(public) */
	static unique_ptr<Input> parse_args(const std::vector<std::string> &args);

private:
	/* 静的メンバ関数(input) */
	static void usage(int status);
};