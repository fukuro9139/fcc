#include "common.hpp"
#include "tokenize.hpp"

#ifndef WINDOWS

#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#else

#include <windows.h>

#endif /* WINDOWS */

/**
 * @brief エラーを報告して終了する
 *
 * @param msg エラーメッセージ
 */
void error(string &&msg)
{
    std::cerr << msg << std::endl;
    exit(1);
}

/**
 * @brief エラー箇所の位置を受け取ってエラー出力して終了
 *
 * @param msg エラーメッセージ
 * @param location エラー箇所の位置
 */
void error_at(string &&msg, const int &location)
{
    auto current_file = Token::get_current_file();

    /* エラー箇所が含まれる行の行数を取得 */
    int line_no = 1;
    for (int l = 0; l < location; ++l)
    {
        if (current_file->_contents[l] == '\n')
        {
            ++line_no;
        }
    }

    verror_at(current_file->_name, current_file->_contents, move(msg), location, line_no);
    exit(1);
}

/**
 * @brief 下記のフォーマットでエラー箇所を報告する
 * foo.c:10: x = y + 1;
 *               ^ <error message here>
 * @param msg エラーメッセージ
 * @param location エラー箇所
 * @param line_no エラー箇所の行数
 */
void verror_at(const string &filename, const string &input, string &&msg, const int &location, const int &line_no)
{
    int line_start = location;
    /* エラー箇所が含まれる行の先頭位置を探す */
    while (line_start > 0 && input[line_start - 1] != '\n')
    {
        --line_start;
    }

    int line_end = location;
    int max = input.end() - input.begin();
    /* エラー箇所が含まれる行の末尾の位置を探す */
    while (line_end < max - 1 && input[line_end + 1] != '\n')
    {
        ++line_end;
    }

    /* 行頭の空白をスキップ */
    while (std::isspace(input[line_start]) && line_start < line_end)
    {
        ++line_start;
    }

    /* ファイル名 */
    string loc_info = filename + ":" + std::to_string(line_no) + ": ";
    int indent = loc_info.size();
    std::cerr << loc_info;

    /* エラー箇所が含まれる行を出力 */
    std::cerr << input.substr(line_start, line_end - line_start + 1) << "\n";

    /* エラーメッセージを出力 */
    std::cerr << string(indent + location - line_start, ' ') << "^ ";
    std::cerr << msg << std::endl;
}

/**
 * @brief トークンを受け取ってエラーメッセージを出力
 *
 * @param msg エラーメッセージ
 * @param token エラー箇所を含むトークン
 */
void error_token(string &&msg, Token *token)
{
    verror_at(token->_file->_name, token->_file->_contents, move(msg), token->_location, token->_line_no);
    exit(1);
}

/**
 * @brief 警告を出力する
 *
 * @param msg 警告メッセージ
 * @param token 対象メッセージ
 */
void warn_token(string &&msg, Token *token)
{
    verror_at(token->_file->_name, token->_file->_contents, move(msg), token->_location, token->_line_no);
}

/**
 * @brief forkした子プロセスでargvを引数としてexecvpを起動
 *
 * @param argv 引数リスト
 */
void run_subprocess(const vector<string> &argv)
#ifndef WINDOWS
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
#else
{
    string cmd_str;
    for (int i = 0; i < argv.size(); ++i)
    {
        cmd_str += argv[i];
        if (i != argv.size() - 1)
        {
            cmd_str.push_back(' ');
        }
    }
    char *cmd = new char[cmd_str.size() + 1];
    std::char_traits<char>::copy(cmd, cmd_str.c_str(), cmd_str.size() + 1);

    // wchar_t *wcmd = new wchar_t[cmd.size() + 1];
    // mbstowcs(wcmd, cmd.c_str(), cmd.size() + 1);

    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};

    si.cb = sizeof(si);

    /* プロセスの実行 */
    if (CreateProcess(nullptr, cmd, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi))
    {
        /* 子プロセスの終了まで待つ */
        WaitForSingleObject(pi.hProcess, INFINITE);

        /* アプリケーションの終了コードの取得 */
        unsigned long exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        /* 終了コードが負の値になる場合もあるので、signedにキャストする */
        long ec = static_cast<long>(exitCode);

        /* ハンドルを閉じる */
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (ec != 0)
        {
            exit(1);
        }
    }
    else
    {
        /* 起動失敗 */
        error("プロセス起動に失敗しました");
    }
    /* 解放 */
    // delete[] wcmd;
}
#endif /* WINDOWS */

/**
 * @brief ファイルを開いてファイルストリームのポインタを返す。
 * アウトプットパスとして"-"が指定されているまたはファイルを開くのに失敗したら出力先は標準出力。
 *
 * @param path 出力先ファイルパス
 * @return 出力用ostreamのポインタ
 */
std::ostream *open_file(const string &path)
{
    static unique_ptr<std::ofstream> os;
    os = nullptr;
    /* 出力先が標準出力ではない */
    if (output_path != "-")
    {
        /* ファイルを開く */
        os = make_unique<std::ofstream>(output_path);
        if (!os->fail())
        {
            return os.get();
        }
    }
    return &std::cout;
}