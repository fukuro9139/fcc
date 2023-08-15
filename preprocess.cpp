/**
 * @file preprocess.cpp
 * @author K.Fukunaga
 * @brief プリプロセスを行う
 * @version 0.1
 * @date 2023-08-15
 * 
 * @copyright Copyright (c) 2023 MIT License
 * 
 */

#include "preprocess.hpp"

/**
 * @brief プリプロセスを行う
 *
 * @param token トークンリストの先頭
 * @return unique_ptr<Token>
 */
unique_ptr<Token> PreProcess::preprocess(unique_ptr<Token> &&token)
{
    convert_keywords(token.get());
    return token;
}

/**
 * @brief トークンを順番にみていってキーワードと一致していれば種類をキーワードに帰る
 *
 * @param token トークン列
 */
void PreProcess::convert_keywords(Token *token)
{
    for (Token *t = token; TokenKind::TK_EOF != t->_kind; t = t->_next.get())
    {
        if (TokenKind::TK_IDENT == t->_kind && is_keyword(t))
        {
            t->_kind = TokenKind::TK_KEYWORD;
        }
    }
}

/**
 * @brief トークンの識別子がキーワードかどうか判定する
 *
 * @param token 対象のトークン
 * @return true キーワードである
 * @return false キーワードではない
 */
bool PreProcess::is_keyword(const Token *token)
{
    for (auto &kw : keywords)
    {
        if (kw == token->_str)
        {
            return true;
        }
    }
    return false;
}