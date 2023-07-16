#include "token.hpp"
using std::string;

Token::_shared_ptr_token Token::_token_cur = nullptr;

/**
 * @brief コンストラクタ
 */
Token::Token() = default;

Token::Token(const TokenKind &kind, const std::string &str)
	: _kind(kind), _str(str)
{}

/**
 * @brief デストラクタ
 */
Token::~Token() = default;

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
	if(_token_cur->_kind != TokenKind::TK_RESERVED || _token_cur->_str[0] != op) {return false;}
	_token_cur = std::move(_token_cur->next);
	return true;
}

/**
 * @brief 
 * 次のトークンが期待している記号の時はトークンを１つ進める。
 * それ以外の場合にはエラーを報告する。
 * @param op 期待している記号
 */
void Token::expect(const char &op){
	if (_token_cur->_kind != TokenKind::TK_RESERVED || _token_cur->_str[0] != op) {error(string{op}+ "ではありません");}
	_token_cur = std::move(_token_cur->next);
}

/**
 * @brief 
 * 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
 * それ以外の場合はエラーを返す。
 * @return int 
 */
int Token::expect_number(){
	if(_token_cur->_kind != TokenKind::TK_NUM){error("数ではありません");}
	int val = _token_cur->_val;
	_token_cur = std::move(_token_cur->next);
    return val;
}

/**
 * @brief 
 * 新しいトークンを生成してcurにつなげる
 * @param kind 新しく生成するトークンの種類
 * @param cur 親となるトークン
 * @param str 新しく生成するトークン文字列
 * @return _shared_ptr_token 生成したトークンのポインタ
 */
Token::_shared_ptr_token Token::new_token(const TokenKind &kind, _shared_ptr_token cur, const std::string &str)
{	
	_shared_ptr_token tok = std::make_shared<Token>(kind, str);
    cur->next = tok;
	return std::move(tok);
}

/**
 * @brief 文字列strをトークナイズしてトークン列を生成。
 * @param str トークナイズする対象文字列
 */
void Token::tokenize(const string &str) {
	_shared_ptr_token head = std::make_shared<Token>();
	_shared_ptr_token cur(head);

	for(size_t i = 0, len = str.length(); i < len; ++i){
		/* 空白文字をスキップ */
		if(std::isspace(str[i])){continue;}
		
		if('+' == str[i] || '-' == str[i]) {
			/* 新しいトークンを生成してcurに繋ぎ、curを1つ進める */
			cur = new_token(TokenKind::TK_RESERVED, std::move(cur), string{str[i]});
			continue;
		}
		if(isdigit(str[i])){
			const string sub_str = str.substr(i);
			/* 新しいトークンを生成してcurに繋ぎ、curを1つ進める */
			cur = new_token(TokenKind::TK_NUM, std::move(cur), sub_str);
			
			/* 数値変換 */
			size_t idx;
			cur->_val = std::stoi(sub_str, &idx);
			i += idx - 1;
			continue;
		}
		error("トークナイズできません");

	}

	cur = new_token(TokenKind::TK_EOF, std::move(cur), "");
	_token_cur = std::move(head->next);
}

/**
 * @brief トークンが入力の終わりかどうか判定
 * 
 * @return true 
 * @return false 
 */
bool Token::at_eof() {
  return _token_cur->_kind == TokenKind::TK_EOF;
}

