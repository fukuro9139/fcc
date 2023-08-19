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
class Input;
using MacroArgs = std::unordered_map<string, unique_ptr<Token>>;
using Macro_handler_fn = unique_ptr<Token> (*)(const Token *);

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

/** マクロ */
struct Macro
{
	unique_ptr<Token> _body;			 /*!< マクロの展開先 */
	unique_ptr<vector<string>> _params;	 /*!< 関数マクロの引数 */
	bool _is_objlike = true;			 /*!< オブジェクトマクロであるか */
	Macro_handler_fn _handler = nullptr; /*!< 動的な事前定義マクロの動作(__LINE__など) */

	Macro(unique_ptr<Token> &&body, const bool &objlike);
};

class PreProcess
{
public:
	/* 静的メンバ関数(public) */
	static unique_ptr<Token> preprocess(unique_ptr<Token> &&token, const unique_ptr<Input> &in);

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
	static void read_macro_definition(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static Macro *find_macro(const unique_ptr<Token> &token);
	static Macro *add_macro(const unique_ptr<Token> &token, const bool &is_objlike, unique_ptr<Token> &&body);
	static void delete_macro(const string &name);
	static bool expand_macro(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static unique_ptr<Token> substitute_obj_macro(const unique_ptr<Token> &dst, const unique_ptr<Token> &macro);
	static unique_ptr<Token> substitute_func_macro(const unique_ptr<Token> &dst, const unique_ptr<Token> &macro, const MacroArgs &args);
	static unique_ptr<vector<string>> read_macro_params(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static unique_ptr<Token> resd_macro_arg_one(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static unique_ptr<MacroArgs> read_macro_args(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token, const vector<string> &params);
	static void add_hideset(unique_ptr<Hideset> &hs, const string &name);
	static long evaluate_const_expr(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static unique_ptr<Token> read_const_expr(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token);
	static CondIncl *push_cond_incl(unique_ptr<Token> &&token, bool included);
	static unique_ptr<Token> skip(unique_ptr<Token> &&token, const string &op);
	static void copy_macro_token(Token *dst, const Token *src, const string &name, const unique_ptr<Hideset> &hs);
	static string quate_string(const string &str);
	static unique_ptr<Token> new_str_token(const string &str, const Token *ref);
	static unique_ptr<Token> new_num_token(const int &val, const Token *ref);
	static string join_tokens(const Token *start, const Token *end);
	static unique_ptr<Token> stringize(const Token *ref, const Token *arg);
	static unique_ptr<Token> paste(const Token *lhs, const Token *rhs);
	static unique_ptr<Token> vir_file_tokenize(const string &str, const string &file_name, const int &file_no);
	static string read_include_filename(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token, bool &is_dquote);
	static unique_ptr<Token> include_file(unique_ptr<Token> &&follow_token, const string &path);
	static string search_include_path(const string &current_path, const string &filename, const bool &dquote);
	static void define_macro(const string &name, const string &buf);
	static void add_builtin(const string &name, const Macro_handler_fn &fn);
	static void init_macros();
	static unique_ptr<Token> file_macro(const Token *macro_token);
	static unique_ptr<Token> line_macro(const Token *macro_token);

	static vector<unique_ptr<CondIncl>> cond_incl;
	static std::unordered_map<string, unique_ptr<Macro>> macros;
	static const Input *input_options;

	/** 識別子一覧 */
	static constexpr string_view keywords[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char", "float", "double",
											   "struct", "union", "short", "long", "void", "typedef", "_Bool",
											   "enum", "static", "goto", "break", "continue", "switch", "case",
											   "default", "extern", "_Alignof", "_Alignas", "do", "signed", "unsigned",
											   "const", "volatile", "auto", "register", "restrict", "__restrict", "__restrict__", "_Noreturn"};
};