#include "input.hpp"

Options Input::opt;

void Input::parse_args(const std::vector<std::string> &args)
{
	for (size_t i = 1, sz = args.size(); i < sz; ++i)
	{
		if ("--help" == args[i])
		{
			usage(0);
		}

		if ("-o" == args[i])
		{
			/* "-o" オプションの後に何も引数がなければエラー終了 */
			if (++i == sz)
			{
				usage(1);
			}
			opt.opt_o = args[i];
			continue;
		}

		if (args[i].starts_with("-o"))
		{
			opt.opt_o = args[i].substr(2);
			continue;
		}

		if (args[i][0] == '-')
		{
			if (rgs[i].size() >= 2)
			{
				std::cerr << "不明なオプションです: " << args[i] << "\n";
				std::cerr << "fccでは下記のオプションが使えます\n";
				usage(1);
			}
			continue;
		}

		opt.input = args[i];
	}

	// if(!input_path){
	// 	std::cerr << "入力ファイルが存在しません\n";
	// 	exit(1);
	// }
}

void Input::usage(int status)
{
	std::cerr << "fcc [ -o <path> ] <file>\n";
	exit(status);
}
