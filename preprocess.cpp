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
#include "error.hpp"

/**
 * @brief プリプロセスを行う
 *
 * @param token トークンリストの先頭
 * @return unique_ptr<Token>
 */
unique_ptr<Token> PreProcess::preprocess(unique_ptr<Token> &&token)
{
    token = preprocess2(move(token));
    convert_keywords(token.get());
    return token;
}

/**
 * @brief トークンリストを先頭から巡回してプリプロセスマクロとディレクティブを処理する
 *
 * @param トークンリスト
 * @return 処理後のトークンリスト
 */
unique_ptr<Token> PreProcess::preprocess2(unique_ptr<Token> &&token)
{
    auto head = make_unique_for_overwrite<Token>();
    auto cur = head.get();

    while (TokenKind::TK_EOF != token->_kind)
    {
        /* 行頭'#'でなければそのまま */
        if (!is_hash(token.get()))
        {
            cur->_next = move(token);
            cur = cur->_next.get();
            token = move(cur->_next);
            continue;
        }

        token = move(token->_next);

        /* '#'のみの行は無視する */
        if (token->_at_begining)
        {
            continue;
        }

        if (token->is_equal("include"))
        {
            token = move(token->_next);

            /* '#include'の次はファイル名 */
            if(TokenKind::TK_STR != token->_kind){
                error_token("ファイル名ではありません", token.get());
            }

            /* 現在のファイル */
            fs::path src_path = token->_file->_name;
            /* 文字列リテラルなので前後の'"'を取り除く */
            const auto name = token->_str.substr(1, token->_str.size() -2);
            /* includeするファイルのパスを生成、現在のファイルからの相対パス */
            const auto inc_path = src_path.replace_filename(name).string();
            auto token2 = Token::tokenize_file(inc_path);

            token = append(move(token2), move(token->_next));
            continue;
        }

        error_token("無効なプリプロセッサディレクティブです", token.get());
    }

    cur->_next = move(token);
    return move(head->_next);
}


/**
 * @brief トークン1の末尾にトークン2を付け加える
 * 
 * @param token1 
 * @param token2 
 * @return トークン1の末尾にトークン2を付け加えたトークン
 */
unique_ptr<Token> PreProcess::append(unique_ptr<Token>&& token1, unique_ptr<Token>&& token2)
{
    if(!token1 || TokenKind::TK_EOF == token1->_kind){
        return token2;
    }

    auto tok = token1.get();
    /* トークン1を末尾まで辿る */
    while(TokenKind::TK_EOF != tok->_next->_kind){
        tok = tok->_next.get();
    }
    tok->_next = move(token2);
    return token1;
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

/**
 * @brief トークンが行頭で'#'であるか
 *
 * @param token 対象トークン
 * @return true 行頭'#'である
 * @return false 行頭'#'ではない
 */
bool PreProcess::is_hash(const Token *token)
{
    return token->_at_begining && token->is_equal("#");
}