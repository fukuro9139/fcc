/**
 * @file preprocess.hpp
 * @author K.Fukunaga
 * @brief プリプロセスを行う
 * @version 0.1
 * @date 2023-08-15
 *
 * @copyright Copyright (c) 2023  MIT Licence
 *
 */

#pragma once

#include "tokenize.hpp"
#include "common.hpp"

class PreProcess
{
public:
    /* 静的メンバ関数(public) */
    static unique_ptr<Token> preprocess(unique_ptr<Token> &&token);

private:
    PreProcess();
    /* 静的メンバ関数(private) */
    static unique_ptr<Token> preprocess2(unique_ptr<Token> &&token);
    static unique_ptr<Token> append(unique_ptr<Token> &&token1, unique_ptr<Token> &&token2);
    static void convert_keywords(Token *token);
    static bool is_keyword(const Token *token);
    static bool is_hash(const Token *token);

    /** 識別子一覧 */
    static constexpr string_view keywords[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char", "float", "double",
                                               "struct", "union", "short", "long", "void", "typedef", "_Bool",
                                               "enum", "static", "goto", "break", "continue", "switch", "case",
                                               "default", "extern", "_Alignof", "_Alignas", "do", "signed", "unsigned",
                                               "const", "volatile", "auto", "register", "restrict", "__restrict", "__restrict__", "_Noreturn"};
};