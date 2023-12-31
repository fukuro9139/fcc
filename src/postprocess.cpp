/**
 * @file postprocess.cpp
 * @author K.Fukunaga
 * @brief コンパイル後の処理を行う
 * @version 0.1
 * @date 2023-08-15
 *
 * @copyright Copyright (c) 2023  MIT Licence
 *
 */

#include "postprocess.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <string.h>
#include <glob.h>
#include <sys/stat.h>

/**
 * @brief 'as'コマンドでアセンブルする
 *
 * @param input_path 入力先ファイルのパス
 * @param output_path 出力先ファイルのパス
 */
void PostProcess::assemble(const string &input_path, const string &output_path)
{
	vector<string> cmd = {"as", "--noexecstack", "-c", input_path, "-o", output_path};
	run_subprocess(cmd);
}

/**
 * @brief 'ld'コマンドでリンクする
 *
 * @param input_path 入力先ファイルのパス
 * @param output_path 出力先ファイルのパス
 */
void PostProcess::run_linker(const vector<string> &inputs, const string &output)
{
	vector<string> cmd;
	cmd.reserve(50);

	cmd.emplace_back("ld");
	cmd.emplace_back("-o");
	cmd.emplace_back(output);
	cmd.emplace_back("-m");
	cmd.emplace_back("elf_x86_64");
	cmd.emplace_back("-dynamic-linker");
	cmd.emplace_back("/lib64/ld-linux-x86-64.so.2");

	string libpath = find_libpath();
	string gcc_libpath = find_gcc_libpath();

	cmd.emplace_back(libpath + "/crt1.o");
	cmd.emplace_back(libpath + "/crti.o");
	cmd.emplace_back(gcc_libpath + "/crtbegin.o");
	cmd.emplace_back("-L" + gcc_libpath);
	cmd.emplace_back("-L" + libpath);
	cmd.emplace_back("-L" + libpath + "/..");
	cmd.emplace_back("-L/usr/lib64");
	cmd.emplace_back("-L/lib64");
	cmd.emplace_back("-L/usr/lib/x86_64-linux-gnu");
	cmd.emplace_back("-L/usr/lib/x86_64-pc-linux-gnu");
	cmd.emplace_back("-L/usr/lib/x86_64-redhat-linux");
	cmd.emplace_back("-L/usr/lib");
	cmd.emplace_back("-L/lib");

	for (auto &input : inputs)
	{
		cmd.emplace_back(input);
	}

	cmd.emplace_back("-lc");
	cmd.emplace_back("-lgcc");
	cmd.emplace_back("--as-needed");
	cmd.emplace_back("-lgcc_s");
	cmd.emplace_back("--no-as-needed");
	cmd.emplace_back(gcc_libpath + "/crtend.o");
	cmd.emplace_back(libpath + "/crtn.o");

	run_subprocess(cmd);
}

/**
 * @brief 一時ファイルを作成する
 *
 * @return 作成したファイルのパス
 */
string PostProcess::create_tmpfile()
{
	char *path = strdup("/tmp/fcc-XXXXXX");
	int fd = mkstemp(path);
	if (fd == -1)
	{
		error("\'mkstemp\'コマンドに失敗しました");
	}
	close(fd);

	auto tmp_path = string(path);
	free(path);
	return tmp_path;
}

/**
 * @brief ファイルパスを正規表現でパターンマッチングして検索
 *
 * @param pattern 検索パターン
 * @return パターンと一致したパス
 * @note strdup内でmallocで文字列のメモリ領域を確保している。戻り値は戻り先で開放すること。
 */
string PostProcess::find_file(const string &pattern)
{
	string path;
	/* 検索結果を格納するバッファ */
	glob_t buf = {};
	/* 検索 */
	glob(pattern.c_str(), 0, nullptr, &buf);
	/* 最後に見つかったパスをコピー */
	if (buf.gl_pathc > 0)
		path = buf.gl_pathv[buf.gl_pathc - 1];
	globfree(&buf);
	return path;
}

/**
 * @brief リンク時に必要なライブラリのパスを検索する。見つからなければエラー。
 *
 * @return ライブラリのパス
 */
string PostProcess::find_libpath()
{
	if (fs::is_regular_file("/usr/lib/x86_64-linux-gnu/crti.o"))
	{
		return "/usr/lib/x86_64-linux-gnu";
	}

	if (fs::is_regular_file("/usr/lib64/crti.o"))
	{
		return "/usr/lib64";
	}

	error("ライブラリが見つかりません");
}

/**
 * @brief gccのライブラリのパスを検索する。見つからなければエラー。
 *
 * @return ライブラリのパス
 */
string PostProcess::find_gcc_libpath()
{
	static const string paths[] = {
		"/usr/lib/gcc/x86_64-linux-gnu/*/crtbegin.o",
		"/usr/lib/gcc/x86_64-pc-linux-gnu/*/crtbegin.o", /* For Gentoo */
		"/usr/lib/gcc/x86_64-redhat-linux/*/crtbegin.o", /* For Fedora */
		"/usr/lib/gcc/x86_64-amazon-linux/*/crtbegin.o", /* For Amazon Linux */
	};

	for (auto &path : paths)
	{
		auto path_found = find_file(path);
		if (!path_found.empty())
		{
			fs::path p = path_found;
			return p.parent_path().string();
		}
	}
	error("gccライブラリが見つかりません");
}