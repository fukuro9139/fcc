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

#include "common.hpp"

class Token;

/** #if関連のブロックの種類 */
enum class BlockKind
{
	IN_THEN,
	IN_ELIF,
	IN_ELSE,
};

/** #if関連を表す構造体 */
struct CondIncl
{
	unique_ptr<Token> _token /*!< 参照用のトークン */;
	BlockKind _ctx;			/*!< ブロックの種類 */
	bool _included = false; /*!< ブロックが有効であるか */

	CondIncl();
	CondIncl(unique_ptr<Token> &&token, const BlockKind &ctx, bool included);
};

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
	static unique_ptr<Token> skip_line(unique_ptr<Token> &&token);
	static void convert_keywords(unique_ptr<Token> &token);
	static bool is_keyword(const Token *token);
	static bool is_hash(const unique_ptr<Token> &token);
	static unique_ptr<Token> new_eof_token(const unique_ptr<Token> &src);
	static unique_ptr<Token> skip_cond_incl(unique_ptr<Token> &&token);
	static unique_ptr<Token> skip_cond_incl2(unique_ptr<Token> &&token);
	static unique_ptr<Token> copy_line(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static Token *find_macro(const unique_ptr<Token> &token);
	static Token *add_macro(const unique_ptr<Token> &token, unique_ptr<Token> &&body);
    static void delete_macro(const string &name);
	static bool expand_macro(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
    static void add_hideset(Hideset &hs, const string &name);
	static long evaluate_const_expr(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static CondIncl *push_cond_incl(unique_ptr<Token> &&token, bool included);
	static vector<unique_ptr<CondIncl>> cond_incl;
	static std::unordered_map<string, unique_ptr<Token>> macros;

	/** 識別子一覧 */
	static constexpr string_view keywords[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char", "float", "double",
											   "struct", "union", "short", "long", "void", "typedef", "_Bool",
											   "enum", "static", "goto", "break", "continue", "switch", "case",
											   "default", "extern", "_Alignof", "_Alignas", "do", "signed", "unsigned",
											   "const", "volatile", "auto", "register", "restrict", "__restrict", "__restrict__", "_Noreturn"};
};