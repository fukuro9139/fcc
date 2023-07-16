#include <iostream>
#include<vector>
#include<string>
#include "token.hpp"

using std::cout;
using std::vector;
using std::string;

int main(int argc, char **argv){

	if(argc != 2){
		std::cerr << "引数の個数が正しくありません\n";
		return 1;
	}

	vector<string> args(argv, argv + argc);
	std::size_t idx;

	/* トークナイズする */
	Token::tokenize(args[1]);

	/* アセンブリの前半部分を出力 */
	cout << ".intel_syntax noprefix\n";
	cout << ".globl main\n";
	cout << "main:\n";

	/* 式の最初は数でなければならないので、それをチェックして */
	/* 最初のmove命令を出力 */
	cout << " mov rax, " << Token::expect_number() << "\n";

	/* '+ <数>'または'- <数>'というトークンの並びを消費しつつ */
	/* アセンブリを出力 */
	while(!Token::at_eof()){
		if(Token::consume('+')){
			cout << " add rax, " << Token::expect_number() << "\n";
			continue;
		}
		Token::expect('-');
		cout << " sub rax, " << Token::expect_number() << "\n";
	}

	cout << " ret\n";
	return 0;
}
