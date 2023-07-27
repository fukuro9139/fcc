#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

struct Options
{
	std::string opt_o = "";
	std::string input = "";
};

class Input
{
public:
	static void parse_args(const std::vector<std::string> &args);
	static Options opt;

private:
	static void usage(int status);
};