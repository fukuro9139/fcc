#include<algorithm>
#include "token.hpp"

std::string user_input;

namespace Parser{

	using std::string;
	using _ptr_token = Token::_unique_ptr_token;
	using _itr_str = Token::_itr_str;

	_ptr_token Token::_token_cur = nullptr;
	const string Token::_ops = "+-*/()<>";

	/** @brief コンストラクタ */
	constexpr Token::Token() = default;

	Token::Token(const TokenKind &kind, const _itr_str &itr, const size_t &len)
		: _kind(kind), _str(itr), _len(len)
	{}

    /** @brief デストラクタ */
	Token::~Token() = default;

	/**
	 * @brief エラー報告用の関数
	 * @param err
	 */
	void Token::error(const string &msg) {
		std::cerr << msg << "\n";
		exit(1);
	}

	/**
	 * @brief エラー箇所を報告する
	 * @param pos エラー箇所
	 */
	void Token::error_at(const string &msg, const _itr_str &loc){
		_itr_str first = user_input.begin();
		size_t pos = std::distance(first, loc);
		std::cerr << user_input << "\n";
		std::cerr << string(pos, ' ') << "^ ";
		std::cerr << msg << "\n";
		exit(1);
	}

	/**
	 * @brief
	 * 次のトークンが期待している記号の時には、トークンを1つ進めてtrueを返す。 \n
	 * それ以外の場合には偽を返す。
	 * @param op 期待している記号
	 * @return true 次のトークンが期待している記号
	 * @return false それ以外
	 */
	bool Token::consume(const string &op) {
		if( TokenKind::TK_RESERVED != _token_cur->_kind || !Token::_comp_op(op)) {return false;}
		_token_cur = std::move(_token_cur->next);
		return true;
	}
	/**
	 * @brief
	 * 次のトークンが期待している識別子の時には、識別子トークンのポインタを返し \n
	 * トークンを1つ進める。 \n
	 * それ以外の場合には偽を返す。
	 * @return _unique_ptr_toke 次のトークンが識別子
	 * @return nullptr それ以外
	 */
	_ptr_token Token::consume_ident()
	{
		if(TokenKind::TK_IDENT != _token_cur->_kind) {return nullptr;}
		_ptr_token ret = std::move(_token_cur);
		_token_cur = std::move(ret->next);
		return std::move(ret);
	}

	/**
	 * @brief
	 * 次のトークンが期待している記号の時はトークンを１つ進める。 \n
	 * それ以外の場合にはエラーを報告する。
	 * @param op 期待している記号
	 */
	void Token::expect(const std::string &op){
		if (_token_cur->_kind != TokenKind::TK_RESERVED || !Token::_comp_op(op)) {
			error_at(op + "ではありません", _token_cur->_str);
		}
		_token_cur = std::move(_token_cur->next);
	}

	/**
	 * @brief 
	 * 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。 \n 
	 * それ以外の場合はエラーを返す。
	 * @return int 
	 */
	int Token::expect_number(){
		if(_token_cur->_kind != TokenKind::TK_NUM){
			error_at("数ではありません", _token_cur->_str);
		}
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
	 * @param len トークン文字列の文字数
	 * @return _unique_ptr_token 生成したトークンのポインタ
	 */
	Token* Token::new_token(const TokenKind &kind, Token *cur, const _itr_str &itr, const size_t &len)
	{	
		_unique_ptr_token tok = std::make_unique<Token>(kind, itr, len);
		cur->next = std::move(tok);
		return cur->next.get();
	}

	/**
	 * @brief 文字列strをトークナイズしてトークン列を生成。
	 * @param str トークナイズする対象文字列
	 */
	void Token::tokenize(const string &str) {
		_ptr_token head = std::make_unique<Token>();
		Token *cur = head.get();

		for(_itr_str it = str.begin(), last = str.end(); it != last;){
			/* 空白文字をスキップ */
			if(std::isspace(*it)){
				++it;
				continue;
			}
			
			/* 2文字演算子 */
			if( _start_with(it, last, "==") || _start_with(it, last, "!=") ||
			    _start_with(it, last, "<=") || _start_with(it, last, ">=")) {
					cur = new_token(TokenKind::TK_RESERVED, cur, it, 2);
					it += 2;
					continue;
			}

			/* 1文字演算子 */
			if(Token::_ops.find(*it) != string::npos) {
				/* 新しいトークンを生成してcurに繋ぎ、curを1つ進める */
				cur = new_token(TokenKind::TK_RESERVED, cur, it, 1);
				++it;
				continue;
			}
			
			/* 変数 */
			if('a' <= *it && *it <= 'z'){
				cur = new_token(TokenKind::TK_IDENT, cur, it, 1);
				++it;
				continue;
			}

			if(isdigit(*it)){
				/* 新しいトークンを生成してcurに繋ぎ、curを1つ進める */
				cur = new_token(TokenKind::TK_NUM, cur, it, 0);

				/* 数値変換 */
				size_t idx;
				const string sub_str(it, str.end());
				cur->_val = std::stoi(sub_str, &idx);
				it += idx;
				continue;
			}

			error_at("トークナイズできません", it);

		}

		cur = new_token(TokenKind::TK_EOF, std::move(cur), str.end(), 0);
		_token_cur = std::move(head->next);
	}

	/**
	 * @brief 
	 * 現在のトークンが終端かどうか
	 * @return true 終端である
	 * @return false 終端ではない
	 */
	bool Token::at_eof()
	{
		return TokenKind::TK_EOF == _token_cur->_kind;
	}

	/**
	 * @brief 
	 * 文字列と現在のトークン文字列の比較
	 * @return true 一致
	 * @return false 不一致
	 */
	bool Token::_comp_op(const std::string &op)
	{
		/* 長さの比較 */
		if(op.length() != _token_cur->_len) { return false; }
		return std::equal(op.begin(), op.end(), _token_cur->_str);
	}

	/**
	 * @brief
	 * 文字列の先頭がopと一致するか
	 * @return true 一致
	 * @return false 不一致
	 */
	bool Token::_start_with(const _itr_str &first, const _itr_str &last, const std::string &op)
	{
		const size_t len = std::distance(first, last);
		return (len >= op.length() && std::equal(op.begin(), op.end(), first));
	}
}