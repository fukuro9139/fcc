#include "error.hpp"

/***********/
/* 汎用関数 */
/***********/

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
 * @brief エラー箇所の位置を受け取ってエラー出力
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
}

/**
 * @brief 下記のフォーマットでエラー箇所を報告して終了する
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
    exit(1);
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
}
