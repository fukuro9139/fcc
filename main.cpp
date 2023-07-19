#include "codegen.hpp"
#include "parse.hpp"
#include "tokenize.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		error(std::string(argv[0]) + ": 引数の個数が正しくありません");
	}

	/* 入力文字列をトークナイズする */
	std::unique_ptr<Token> tok = Token::tokenize(std::string(argv[1]));

	/* トークン列をパースし抽象構文木を構築する */
	std::unique_ptr<Node> node = Node::parse(std::move(tok));

	/*　抽象構文木を下りながらコード生成　*/
	CodeGen::generate_code(std::move(node));

	return 0;
}