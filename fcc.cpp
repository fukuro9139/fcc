#include <iostream>
#include<vector>
#include<string>
#include "token.hpp"
#include "node.hpp"

int main(int argc, char **argv){

	using std::cout;
	using Parser::Token;
	using Parser::Node;

	if(argc != 2){
		std::cerr << "引数の個数が正しくありません\n";
		return 1;
	}

	std::vector<std::string> args(argv, argv + argc);
	user_input = args[1];

	/* トークナイズしてパースする */
	Token::tokenize(user_input);
	Node::program();

	/* アセンブリの前半部分を出力 */
	cout << ".intel_syntax noprefix\n";
	cout << ".globl main\n";
	cout << "main:\n";

	/* プロローグ */
	/* 変数２６個分の領域を確保する */
	/* 1変数につき8バイト */
	cout << " push rbp\n";
	cout << " mov rbp, rsp\n";
	cout << " sub rsp, 208\n";

	/* 先頭の式から順に抽象構文木を下りながらコード生成 */
	for(int i = 0; Node::code[i]; ++i){
		Node::gen(std::move(Node::code[i]));

		/* 式の評価結果としてスタックに１つ値が残っている */
		/* はずなので、スタックが溢れないようにポップしておく */
		cout << " pop rax\n";
	}

	/* エピローグ */
	/* 最後の結果がRAXに残っているのでそれが返り値になる */
	cout << " mov rsp, rbp\n";
	cout << " pop rbp\n";
	cout << " ret\n";
	return 0;
}
