#include <iostream>
#include<vector>
#include<string>

using std::cout, std::vector, std::string;

/* トークンの種類 */
enum class TokenKind {
	TK_RESERVED,	/* 記号 */
	TK_NUM,			/* 整数トークン */
	TK_EOF,			/* 入力の終わりを表すトークン */
};

/* トークン型 */
struct Token {
	TokenKind kind;	/* トークンの型 */
	Token *next;	/* 次の入力トークン */
	int val;		/* kindがTK_NUMの場合、その数値 */
	string str;		/* トークン文字列 */
};

/* 現在着目しているトークン */
Token *token;

/* エラーを報告するための関数 */
void error(const string &err) {
	std::cerr << err << "\n";
	exit(1);
}

/**
 *  次のトークンが期待している記号の時には、トークンを1つ進めてtrueを返す。
 * それ以外の場合には偽を返す
 */
bool consume(const char &op) {
	if(token->kind != TokenKind::TK_RESERVED || token->str[0] != op) {return false;}
	token = token->next;
	return true;
}

/**
 * 次のトークンが期待している記号の時はトークンを１つ進める
 * それ以外の場合にはエラーを報告する
*/
void expect(const char &op){
	if (token->kind != TokenKind::TK_RESERVED || token->str[0] != op) {error(string{op}+ "ではありません");}
	token = token->next;
}

/**
 * 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
 * それ以外の場合はエラーを返す
*/
int expect_number(){
	if(token->kind != TokenKind::TK_NUM){error("数ではありません");}
	int val = token->val;
	token = token->next;
}



int main(int argc, char **argv){

	if(argc != 2){
		std::cerr << "引数の個数が正しくありません\n";
		return 1;
	}

	vector<string> args(argv, argv + argc);
	std::size_t idx;

	cout << ".intel_syntax noprefix\n";
	cout << ".globl main\n";
	cout << "main:\n";
	cout << " mov rax, " << std::stol(args[1], &idx, 10) << "\n";

	while(args[1].length() != idx){
		if('+' == args[1][idx]){
			++idx;
			args[1] = args[1].substr(idx);
			cout << " add rax, " << std::stol(args[1], &idx, 10) << "\n";
			continue;
		}else if('-' == args[1][idx]){
			++idx;
			args[1] = args[1].substr(idx);
			cout << " sub rax, " << std::stol(args[1], &idx, 10) << "\n";
			continue;
		}
		std::cerr << "予期しない文字です: " << args[1][idx] << "\n";
		return 1;
	}

	cout << " ret\n";
	return 0;
}
