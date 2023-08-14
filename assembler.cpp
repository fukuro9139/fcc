#include "assembler.hpp"

#ifndef WINDOWS

void Assembler::assemble(const string &input_path, const string &output_path)
{
    char *cinput = new char[input_path.size() + 1];
    char *coutput = new char[output_path.size() + 1];

    std::char_traits<char>::copy(cinput, input_path.c_str(), input_path.size() + 1);
    std::char_traits<char>::copy(coutput, output_path.c_str(), output_path.size() + 1);

    char *const cmd[] = {"as", "-c", cinput, "-o", coutput, NULL};

    if (fork() == 0)
    {
        /* 子プロセスで'as'コマンドを実行する */
        execvp("as", cmd);
        std::cerr << "\'exec\'コマンドが失敗しました\n";
        _exit(1);
    }

    /* 子プロセスが終了するのを待機 */
    int status;
    while (wait(&status) > 0);
    if (status != 0){
        exit(1);
    }

    delete[] cinput;
    delete[] coutput;
}

/**
 * @brief 一時ファイルを作成する
 *
 * @return 作成したファイルのパス
 */
string Assembler::create_tmpfile()
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

#endif /* WINDOWS */