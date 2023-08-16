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
#include "tokenize.hpp"
#include "parse.hpp"

CondIncl::CondIncl() = default;

/** #if関連の条件リスト */
vector<unique_ptr<CondIncl>> PreProcess::cond_incl;

/**
 * @brief プリプロセスを行う
 *
 * @param token トークンリストの先頭
 * @return unique_ptr<Token>
 */
unique_ptr<Token> PreProcess::preprocess(unique_ptr<Token> &&token)
{
    /* プリプロセスマクロとディレクティブを処理 */
    token = preprocess2(move(token));

    /* #ifと#endifの対応を確認 */
    if (!cond_incl.empty())
    {
        error_token("対応する#endifが存在しません", cond_incl.back()->token.get());
    }

    /* 識別子を認識 */
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

        auto start = move(token);
        token = move(start->_next);

        /* '#'のみの行は無視する */
        if (token->_at_begining)
        {
            continue;
        }

        if (token->is_equal("include"))
        {
            token = move(token->_next);

            /* '#include'の次はファイル名 */
            if (TokenKind::TK_STR != token->_kind)
            {
                error_token("ファイル名ではありません", token.get());
            }

            string inc_path;
            if (token->_str[0] == '/')
            {
                inc_path = token->_str;
            }
            else
            {
                /* 現在のファイル */
                fs::path src_path = token->_file->_name;
                /* 文字列リテラルなので前後の'"'を取り除く */
                const auto name = token->_str.substr(1, token->_str.size() - 2);
                /* includeするファイルのパスを生成、現在のファイルからの相対パス */
                inc_path = src_path.replace_filename(name).string();
            }
            auto token2 = Token::tokenize_file(inc_path);

            /* 次の行頭までスキップする */
            token = skip_line(move(token->_next));
            /* includeしたトークンを繋ぐ */
            token = append(move(token2), move(token));
            continue;
        }

        if (token->is_equal("if"))
        {
            auto val = evaluate_const_expr(token, move(token));
            push_cond_incl(move(start));
            /* #if の後の条件式が0（偽）であった場合は#endifまでスキップ */
            if (!val)
            {
                token = skip_cond_incl(move(token));
            }
            continue;
        }

        if (token->is_equal("endif"))
        {
            /* 対になる#ifが存在しないとき */
            if (cond_incl.empty())
            {
                error_token("対応する#ifが存在しません", start.get());
            }
            cond_incl.pop_back();
            token = skip_line(move(token->_next));
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
unique_ptr<Token> PreProcess::append(unique_ptr<Token> &&token1, unique_ptr<Token> &&token2)
{
    if (!token1 || TokenKind::TK_EOF == token1->_kind)
    {
        return token2;
    }

    auto tok = token1.get();
    /* トークン1を末尾まで辿る */
    while (TokenKind::TK_EOF != tok->_next->_kind)
    {
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

/**
 * @brief 一部のプリプロセッサディレクティブは次の行の前に余分なトークンを許容する。
 * この関数では次の行頭までトークンをスキップする
 *
 * @param token 確認するトークン
 * @return 直後の行頭のトークン
 */
unique_ptr<Token> PreProcess::skip_line(unique_ptr<Token> &&token)
{
    if (token->_at_begining)
    {
        return token;
    }

    warn_token("このトークンは無視されます", token.get());
    while (!token->_at_begining)
    {
        token = move(token->_next);
    }
    return token;
}

/**
 * @brief トークンの内容をコピーしてkindをTK_EOF, strを""に変更する
 *
 * @param src コピー元
 * @return 生成したトークン
 */
unique_ptr<Token> PreProcess::new_eof_token(const Token *src)
{
    auto t = Token::copy_token(src);
    t->_kind = TokenKind::TK_EOF;
    t->_str = "";
    return t;
}

/**
 * @brief #endifが出てくるまでトークンをスキップする
 *
 * @param token スキップを開始するトークン
 * @return #endifまたは末尾にあたるトークン
 */
unique_ptr<Token> PreProcess::skip_cond_incl(unique_ptr<Token> &&token)
{
    while (TokenKind::TK_EOF != token->_kind)
    {
        if (is_hash(token.get()) && token->_next->is_equal("endif"))
        {
            break;
        }
        token = move(token->_next);
    }
    return token;
}

/**
 * @brief 次の行頭または末尾までのトークンを全てコピーする。
 * この関数は#if文を評価するためのトークンリストを作成するために用いる。
 *
 * @param next_token 次の文頭のトークンを返すための参照
 * @param current_token 開始位置のトークン
 * @return 生成したトークンリスト
 */
unique_ptr<Token> PreProcess::copy_line(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
    auto head = make_unique_for_overwrite<Token>();
    auto cur = head.get();

    for (; !current_token->_at_begining; current_token = move(current_token->_next))
    {
        cur->_next = Token::copy_token(current_token.get());
        cur = cur->_next.get();
    }
    cur->_next = new_eof_token(current_token.get());
    next_token = move(current_token);
    return move(head->_next);
}

/**
 * @brief 次の文末までを定数式として評価する
 *
 * @param next_token 次の文頭のトークンを返すための参照
 * @param current_token 開始位置のトークン
 * @return 評価結果
 */
long PreProcess::evaluate_const_expr(unique_ptr<Token> &next_token, unique_ptr<Token> &&current_token)
{
    auto start = current_token.get();
    auto expr = copy_line(next_token, move(current_token->_next));

    /* #ifの後に条件式がなければエラー */
    if (TokenKind::TK_EOF == expr->_kind)
    {
        error_token("条件式が存在しません", start);
    }

    Token *rest;
    /* 定数式を評価 */
    auto val = Node::const_expr(&rest, expr.get());

    /* 定数式の評価後に余ったトークンがあればエラー */
    if (TokenKind::TK_EOF != rest->_kind)
    {
        error_token("余分なトークンが存在します", rest);
    }

    return val;
}

/**
 * @brief #ifをリストに追加する
 *
 * @param token #ifに対応するトークン
 * @return 追加したCondIncl構造体のポインタ
 */
CondIncl *PreProcess::push_cond_incl(unique_ptr<Token> &&token)
{
    auto ci = make_unique<CondIncl>();
    ci->token = move(token);
    cond_incl.emplace_back(move(ci));
    return cond_incl.back().get();
}