#include <iostream>
#include<vector>
#include<string>

using std::cout, std::vector, std::string;

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
