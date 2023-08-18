/**
 * @file postprocess.hpp
 * @author K.Fukunaga
 * @brief コンパイル後の処理を行う
 * @version 0.1
 * @date 2023-08-15
 * 
 * @copyright Copyright (c) 2023  MIT Licence
 * 
 */

#pragma once

#include "common.hpp"

#ifndef WINDOWS

class PostProcess
{
public:
    static void assemble(const string &input_path, const string &output_path);
    static void run_linker(const vector<string> &inputs, const string &output);
    static string create_tmpfile();

private:
    PostProcess();
    static string find_file(const string &patern);
    static string find_libpath();
    static string find_gcc_libpath();
};

#endif /* WINDOWS */