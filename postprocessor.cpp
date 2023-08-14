#include "postprocessor.hpp"

#ifndef WINDOWS

void Postprocessor::assemble(const string &input_path, const string &output_path)
{
    vector<string> cmd = {"as", "-c", input_path, "-o", output_path};
    run_subprocess(cmd);
}

/**
 * @brief 一時ファイルを作成する
 *
 * @return 作成したファイルのパス
 */
string Postprocessor::create_tmpfile()
{
    char *path = strdup("/tmp/fcc-XXXXXX");
    int fd = mkstemp(path);
    if (fd == -1)
    {
        std::cerr << "\'mkstemp\'コマンドに失敗しました\n";
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
        std::cerr << "\'exec\'コマンドが失敗しました\n";
        _exit(1);
    }

    /* 子プロセスが終了するのを待機 */
    int status;
    while (wait(&status) > 0){}
    
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

#endif /* WINDOWS */