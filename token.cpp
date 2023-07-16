#include "token.hpp"

namespace Parser{

	using std::string;
	using _ptr_token = Token::_shared_ptr_token;
	using _itr_str = Token::_itr_str;

	_ptr_token Token::_token_cur = nullptr;

	/** @brief コンストラクタ */
	Token::Token() = default;

	Token::Token(const TokenKind &kind, const _itr_str &itr)
		: _kind(kind), _str(itr)
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
	bool Token::consume(const char &op) {
		if(_token_cur->_kind != TokenKind::TK_RESERVED || *(_token_cur->_str) != op) {return false;}
		_token_cur = std::move(_token_cur->next);
		return true;
	}

	/**
	 * @brief 
	 * 次のトークンが期待している記号の時はトークンを１つ進める。 \n 
	 * それ以外の場合にはエラーを報告する。
	 * @param op 期待している記号
	 */
	void Token::expect(const char &op){
		if (_token_cur->_kind != TokenKind::TK_RESERVED || *(_token_cur->_str) != op) {
			error_at(string{op}+ "ではありません", _token_cur->_str);
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
	 * @return _shared_ptr_token 生成したトークンのポインタ
	 */
	_ptr_token Token::new_token(const TokenKind &kind, _shared_ptr_token cur, const _itr_str &itr)
	{	
		_shared_ptr_token tok = std::make_shared<Token>(kind, itr);
		cur->next = tok;
		return std::move(tok);
	}

	/**
	 * @brief 文字列strをトークナイズしてトークン列を生成。
	 * @param str トークナイズする対象文字列
	 */
	void Token::tokenize(const string &str) {
		_ptr_token head = std::make_shared<Token>();
		_ptr_token cur(head);

		for(_itr_str it = str.begin(), end = str.end(); it != end; ++it){
			/* 空白文字をスキップ */
			if(std::isspace(*it)){continue;}
			
			if('+' == *it || '-' == *it) {
				/* 新しいトークンを生成してcurに繋ぎ、curを1つ進める */
				cur = new_token(TokenKind::TK_RESERVED, std::move(cur), it);
				continue;
			}
			if(isdigit(*it)){
				const string sub_str(it, str.end());
				/* 新しいトークンを生成してcurに繋ぎ、curを1つ進める */
				cur = new_token(TokenKind::TK_NUM, std::move(cur), it);
				
				/* 数値変換 */
				size_t idx;
				cur->_val = std::stoi(sub_str, &idx);
				it += idx - 1;
				continue;
			}

			error_at("トークナイズできません", it);

		}

		cur = new_token(TokenKind::TK_EOF, std::move(cur), str.end());
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

}