#include "postprocessor.hpp"

#ifndef WINDOWS

/**
 * @brief 'as'コマンドでアセンブルする
 *
 * @param input_path 入力先ファイルのパス
 * @param output_path 出力先ファイルのパス
 */
void Postprocessor::assemble(const string &input_path, const string &output_path)
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
void Postprocessor::run_linker(const vector<string> &inputs, const string &output)
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
string Postprocessor::create_tmpfile()
{
    char *path = "/tmp/fcc-XXXXXX";
    int fd = mkstemp(path);
    if (fd == -1)
    {
        std::cerr << "\'mkstemp\'コマンドに失敗しました" << endl;
        exit(1);
    }
    close(fd);

    return string(path);
}

/**
 * @brief forkした子プロセスでargvを引数としてexecvpを起動
 *
 * @param argv 引数リスト
 */
void Postprocessor::run_subprocess(const vector<string> &argv)
{
    /* 引数の数 + 1 (NULL終端)*/
    size_t capacity = argv.size() + 1;
    char **cmd = new char *[capacity];

    /* 引数を一つずつコピーしていく */
    for (size_t i = 0; i < argv.size(); ++i)
    {
        cmd[i] = new char[argv[i].size() + 1];
        std::char_traits<char>::copy(cmd[i], argv[i].c_str(), argv[i].size() + 1);
    }
    cmd[capacity - 1] = nullptr;

    if (fork() == 0)
    {
        /* 子プロセスでコマンドを実行する */
        execvp(cmd[0], cmd);
        std::cerr << "\'exec\'コマンドが失敗しました" << endl;
        _exit(1);
    }

    /* 子プロセスが終了するのを待機 */
    int status;
    while (wait(&status) > 0)
    {
    }

    if (status != 0)
    {
        exit(1);
    }

    /* 解放 */
    for (size_t i = 0; i < argv.size(); ++i)
    {
        delete[] cmd[i];
    }

    delete[] cmd;
}

/**
 * @brief ファイルパスを正規表現でパターンマッチングして検索
 *
 * @param pattern 検索パターン
 * @return パターンと一致したパス
 * @note strdup内でmallocで文字列のメモリ領域を確保している。戻り値は戻り先で開放すること。
 */
char *Postprocessor::find_file(const char *pattern)
{
    char *path = nullptr;
    /* 検索結果を格納するバッファ */
    glob_t buf = {};
    /* 検索 */
    glob(pattern, 0, nullptr, &buf);
    /* 最後に見つかったパスをコピー */
    if (buf.gl_pathc > 0)
        path = strdup(buf.gl_pathv[buf.gl_pathc - 1]);
    globfree(&buf);
    return path;
}

/**
 * @brief ファイルが存在しているか判定
 *
 * @param path 判定するファイルのパス
 * @return true 存在している
 * @return false 存在しない
 */
bool Postprocessor::file_exists(const char *path)
{
    struct stat st;
    return !stat(path, &st);
}

/**
 * @brief リンク時に必要なライブラリのパスを検索する。見つからなければエラー。
 *
 * @return ライブラリのパス
 */
string Postprocessor::find_libpath()
{
    if (file_exists("/usr/lib/x86_64-linux-gnu/crti.o"))
    {
        return "/usr/lib/x86_64-linux-gnu";
    }

    if (file_exists("/usr/lib64/crti.o"))
    {
        return "/usr/lib64";
    }

    std::cerr << "ライブラリが見つかりません" << endl;
    exit(1);
}

/**
 * @brief gccのライブラリのパスを検索する。見つからなければエラー。
 *
 * @return ライブラリのパス
 */
string Postprocessor::find_gcc_libpath()
{
    constexpr char *paths[] = {
        "/usr/lib/gcc/x86_64-linux-gnu/*/crtbegin.o",
        "/usr/lib/gcc/x86_64-pc-linux-gnu/*/crtbegin.o", /* For Gentoo */
        "/usr/lib/gcc/x86_64-redhat-linux/*/crtbegin.o", /* For Fedora */
        "/usr/lib/gcc/x86_64-amazon-linux/*/crtbegin.o", /* For Amazon Linux */
    };

    for (auto &path : paths)
    {
        char *path_found = find_file(path);
        if (path_found)
        {
            string libpath = dirname(path_found);
            free(path_found);
            return libpath;
        }
    }
    std::cerr << "gccライブラリが見つかりません" << endl;
    exit(1);
}

#endif /* WINDOWS */