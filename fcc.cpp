#include <iostream>

int main(int argc, char **argv){
	using std::cout;

	if(argc != 2){
		std::cerr << "引数の個数が正しくありません\n";
		return 1;
	}

	cout << ".intel_syntax noprefix\n";
	cout << ".globl main\n";
	cout << "main:\n";
	cout << " mov rax, " << argv[1] << "\n";
	cout << " ret\n";
}
