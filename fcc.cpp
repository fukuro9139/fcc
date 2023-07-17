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
	Token::user_input = args[1];

	/* トークナイズしてパースする */
	Token::tokenize(Token::user_input);
	std::unique_ptr<Node> root = std::move(Node::expr());

	/* アセンブリの前半部分を出力 */
	cout << ".intel_syntax noprefix\n";
	cout << ".globl main\n";
	cout << "main:\n";

	/* 抽象構文木を下りながらコード生成 */
	Node::gen(std::move(root));

	/* スタックトップに式全体の値が残っているはずなので */
	/* それをRAXにロードして関数からの返り値とする */
	cout << " pop rax\n";

	cout << " ret\n";
	return 0;
}
