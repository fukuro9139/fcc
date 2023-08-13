/**
 * @file common.hpp
 * @author K.Fukunaga
 * @brief 共通のインクルード、using宣言
 * @version 0.1
 * @date 2023-07-30
 * 
 * @copyright Copyright (c) 2023　MIT License
 * 
 */

#pragma once

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

using std::unique_ptr;
using std::shared_ptr;
using std::make_unique;
using std::make_unique_for_overwrite;
using std::make_shared;
using std::move;
using std::string;
using std::string_view;
using std::vector;
using std::endl;

#define unreachable() error("エラー: " + string(__FILE__) + " : " + std::to_string(__LINE__))