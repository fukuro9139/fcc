#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

struct Options
{
	std::string opt_o = "";
	std::string input_path = "";
};

class Input
{
public:
	static std::string read_file(const std::string &path);
	static void parse_args(const std::vector<std::string> &args);
	
	static Options opt;

private:
	/* インスタンス化禁止 */
	Input();
	static void usage(int status);
};