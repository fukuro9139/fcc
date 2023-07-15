#include "token.hpp"

using std::string;
/**
 * @brief 
 * エラー報告用の関数
 * @param err 
 */
void Token::error(const string &err) {
	std::cerr << err << "\n";
	exit(1);
}

/**
 * @brief 
 * 次のトークンが期待している記号の時には、トークンを1つ進めてtrueを返す。
 * それ以外の場合には偽を返す。
 * @param op 期待している記号
 * @return true 次のトークンが期待している記号
 * @return false それ以外
 */
bool Token::consume(const char &op) {
	if(token->kind != TokenKind::TK_RESERVED || token->str[0] != op) {return false;}
	token = std::move(token->next);
	return true;
}

/**
 * @brief 
 * 次のトークンが期待している記号の時はトークンを１つ進める。
 * それ以外の場合にはエラーを報告する。
 * @param op 期待している記号
 */
void Token::expect(const char &op){
	if (token->kind != TokenKind::TK_RESERVED || token->str[0] != op) {error(string{op}+ "ではありません");}
	token = std::move(token->next);
}

/**
 * @brief 
 * 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
 * それ以外の場合はエラーを返す。
 * @return int 
 */
int Token::expect_number(){
	if(token->kind != TokenKind::TK_NUM){error("数ではありません");}
	int val = token->val;
	token = std::move(token->next);
    return val;
}

/**
 * @brief トークンが入力の終わりかどうか判定
 * 
 * @return true 
 * @return false 
 */
bool Token::at_eof() {
  return token->kind == TokenKind::TK_EOF;
}