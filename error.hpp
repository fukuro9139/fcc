#pragma onec

#include "tokenize.hpp"

/* エラー報告用関数 */

void error(string &&msg);
void error_at(string &&msg, const int &location);
void verror_at(const string &filename, const string &input, string &&msg, const int &location, const int &line_no);
void error_token(string &&msg, Token *token);