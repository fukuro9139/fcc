/**
 * @file common.hpp
 * @author K.Fukunaga
 * @brief 共通のインクルード、using宣言、汎用で使う関数の宣言
 * @version 0.1
 * @date 2023-07-30
 *
 * @copyright Copyright (c) 2023　MIT License
 *
 */

#pragma once

/* Windows環境で実行したい場合は定義 */
//#define WINDOWS

#include <iostream>
#include <fstream>
#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <locale>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;
using Hideset = std::unordered_set<std::string>;

using std::endl;
using std::make_shared;
using std::make_unique;
using std::make_unique_for_overwrite;
using std::move;
using std::shared_ptr;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::vector;

class Token;

/* 汎用関数 */
void error(string &&msg);
void error_at(string &&msg, const int &location);
void verror_at(const string &filename, const string &input, string &&msg, const int &location, const int &line_no);
void error_token(string &&msg, Token *token);
void warn_token(string &&msg, Token *token);
void run_subprocess(const vector<string> &argv);
std::ostream *open_file(const string &path);

#define unreachable() error("エラー: " + string(__FILE__) + " : " + std::to_string(__LINE__))