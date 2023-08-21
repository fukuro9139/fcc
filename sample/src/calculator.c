/**
 * @file EightQueen.c
 * @author Kengo Fukunaga
 * @brief 四則演算ができる電卓
 * @version 0.1
 * @date 2023-08-05
 *
 * @copyright Copyright (c) 2023
 */

#include "calculator.h"

int main()
{
	struct sigaction sa;
	/* シグナルマスクのクリア */
	if (-1 == sigemptyset(&sa.sa_mask))
	{
		exit(1);
	}
	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;

	/* シグナルハンドラの登録 */
	if (-1 == sigaction(SIGINT, &sa, NULL))
	{
		exit(1);
	}

	while (!end_flg)
	{
		/* 初期化 */
		initialize();

		/* 入力 */
		printf("Enter the formula to be calculated.\n");
		printf("Press Ctrl+C to exit.\n");
		printf("Input: ");
		if (fgets(input_str, MAX_LENGTH, stdin) == (void *)0)
		{
			break;
		}

		/* 終了フラグがたったら抜ける */
		if (end_flg)
		{
			break;
		}

		/* スペースを削除 */
		remove_space(input_str);

		/* 入力が空文字列の場合、次のループへ */
		if (strlen(input_str) == 0)
		{
			printf("\n");
			continue;
		}

		/* 入力された数式を解析 */
		if (parse_input() == false)
		{
			report_error();
			continue;
		}

		/* 計算を実行 */
		if (perform_calculation() == false)
		{
			report_error();
			continue;
		}

		/* 出力 */
		printf("Result: %ld\n\n", nums[0]);
	}

	printf("\nThank you!\n");
	return 0;
}

/**
 * @brief 初期化を行う
 *
 */
void initialize()
{
	input_str[0] = '\0';
	numSize = 0;
	opSize = 0;
	priorityBase = 0;
	error_pos = 0;
}

void remove_space(char *str)
{
	/* 読み込み用ポインタ */
	char *read_ptr = str;
	/* 書き込み用ポインタ */
	char *write_ptr = str;

	/* 読み取り用ポインタがNULL文字に到達するまで繰り返す */
	while (*read_ptr)
	{
		/* 読み取り用ポインタがスペースでない場合、書き込み用ポインタに書き込む */
		if (*read_ptr != ' ' && *read_ptr != '\n')
		{
			*write_ptr = *read_ptr;
			write_ptr++;
		}
		/* 読み取り用ポインタを次に進める */
		read_ptr++;
	}

	/* 書き込み用ポインタの位置にNULL文字を追加して、文字列を終了させる */
	*write_ptr = '\0';
}

/**
 * @brief 入力文字列を字句解析する
 *
 * @return true 字句解析が正常に終了した場合
 * @return false 字句解析が途中で失敗した場合
 */
bool parse_input()
{
	long tmp = 0;				// 読み取った数値
	int sz = strlen(input_str); // 入力文字列の長さ

	for (int i = 0; i < sz; i++)
	{
		char c = input_str[i];

		/* 数字 */
		if (isdigit(c))
		{
			/* 直前が')'なら無効な数式 */
			if (i != 0 && input_str[i - 1] == ')')
			{
				error_pos = i;
				return false;
			}

			/* 数値に変換 */
			int n = c - '0';
			tmp = 10 * tmp + n;

			/* 数式の末尾なら現在の数字を格納 */
			if (i == sz - 1)
			{
				nums[numSize] = tmp;
				numSize++;
			}
			continue;
		}

		/* 括弧開き */
		if (c == '(')
		{
			/* 直前の文字が数字または')'の場合無効な数式 */
			if (i != 0 && (isdigit(input_str[i - 1]) || input_str[i - 1] == ')'))
			{
				error_pos = i;
				return false;
			}

			priorityBase += 10;
			continue;
		}
		/* 括弧閉じ */
		if (c == ')')
		{
			priorityBase -= 10;

			/* ')'の数がこれまでに出てきた'('の数より多い場合閉じられない括弧が存在する */
			if (priorityBase < 0)
			{
				error_pos = i;
				return false;
			}

			/* 直前が数字または')'でないなら無効な数式 */
			if (!isdigit(input_str[i - 1]) && input_str[i - 1] != ')')
			{
				error_pos = i;
				return false;
			}

			/* 数式の末尾なら現在の数字を格納 */
			if (i == sz - 1)
			{
				nums[numSize] = tmp;
				numSize++;
			}
			continue;
		}

		/**
		 * +演算子、-演算子については符号を表す単項演算子の場合もある。
		 * 直前の文字が存在しないまたは'('の場合、単項演算子と解釈して0を演算子の前に補完する。
		 * 例: 1 + (-2) -> 1 + 0 - 2
		 */

		/* +演算子 */
		if (c == '+')
		{
			/* 直前が数字または'('または')'ではないときも存在しないため無効な数式 */
			if (i != 0 && !isdigit(input_str[i - 1]) && input_str[i - 1] != '(' && input_str[i - 1] != ')')
			{
				error_pos = i;
				return false;
			}
			setOP(OP_PLUS, tmp, i);
			tmp = 0;
			continue;
		}

		/* -演算子 */
		if (c == '-')
		{
			/* 直前が数字または'('または')'ではないときも存在しないため無効な数式 */
			if (i != 0 && !isdigit(input_str[i - 1]) && input_str[i - 1] != '(' && input_str[i - 1] != ')')
			{
				error_pos = i;
				return false;
			}
			setOP(OP_MINUS, tmp, i);
			tmp = 0;
			continue;
		}

		/* /演算子 */
		if (c == '/')
		{
			/* 割り算の被除数が存在しないため無効な数式 */
			if (i == 0)
			{
				error_pos = i;
				return false;
			}
			/* 直前が数字または')'ではないときも存在しないため無効な数式 */
			if (!isdigit(input_str[i - 1]) && input_str[i - 1] != ')')
			{
				error_pos = i;
				return false;
			}

			/* 演算子と演算対象の直前の数値を配列にセットする */
			setOP(OP_DIV, tmp, i);

			/* 読み取った数値をクリアする */
			tmp = 0;
			continue;
		}

		/* *演算子 */
		if (c == '*')
		{
			/* 積の被積数が存在しないため無効な数式 */
			if (i == 0)
			{
				error_pos = i;
				return false;
			}
			/* 直前が数字または')'ではないときも存在しないため無効な数式 */
			if (!isdigit(input_str[i - 1]) && input_str[i - 1] != ')')
			{
				error_pos = i;
				return false;
			}

			/* 演算子と演算対象の直前の数値を配列にセットする */
			setOP(OP_MUL, tmp, i);

			/* 読み取った数値をクリアする */
			tmp = 0;
			continue;
		}

		/* 上記のどれでもないときはエラー */
		error_pos = i;
		return false;
	}

	/* 入力された数式に問題がないか確認 */
	/* 括弧の数があっているか */
	if (priorityBase != 0)
	{
		error_pos = sz - 1;
		return false;
	}
	/* 演算子の数があっているか */
	if (numSize != opSize + 1)
	{
		error_pos = sz - 1;
		return false;
	}
	return true;
}

/**
 * @brief 演算子を配列にセットする
 *
 * @param type 演算子の種類
 * @param val 直前の数値
 * @param pos 演算子の位置
 */
void setOP(OperatorType type, long val, int pos)
{
	nums[numSize] = val;

	/* 演算子の種類の設定 */
	ops[opSize].kind = type;

	/* 優先度の設定 */
	if (type == OP_PLUS || type == OP_MINUS)
	{
		ops[opSize].priority = priorityBase + 1;
	}
	else
	{
		ops[opSize].priority = priorityBase + 2;
	}

	/* エラー報告用の位置の設定 */
	ops[opSize].pos = pos;
	numSize++;
	opSize++;
}

/**
 * @brief 最も右にあって最も優先度が高い演算子を探す
 *
 * @return 最も優先度の高い演算子のインデックス
 */
int find_highest_priority_op()
{
	int ret = 0;
	for (int i = 1; i < opSize; i++)
	{
		if (ops[i].priority > ops[ret].priority)
		{
			ret = i;
		}
	}
	return ret;
}

/**
 * @brief 字句解析した式を意味解析して演算を行う
 *
 * @return true 演算が正常に終了
 * @return false 演算が途中で失敗
 */
bool perform_calculation()
{
	while (numSize > 1)
	{
		/* 最も優先度の高い演算子を探索 */
		int idx = find_highest_priority_op();
		/* 計算を実行 */
		if (calc(idx) == false)
		{
			return false;
		}
		/* 数字配列の演算を終えた要素の部分を前につめる */
		for (int i = idx + 1; i < numSize - 1; i++)
		{
			nums[i] = nums[i + 1];
		}
		numSize--;
		/* 演算子配列の演算を終えた要素の部分を前につめる */
		for (int i = idx; i < opSize - 1; i++)
		{
			ops[i] = ops[i + 1];
		}
		opSize--;
	}

	return true;
}

/**
 * @brief 具体的な演算を行う
 *
 * @param idx 演算を行う演算子のインデックス
 * @return true 正常に演算が終了
 * @return false 途中で演算が失敗
 */
bool calc(int idx)
{
	long lhs = nums[idx];
	long rhs = nums[idx + 1];
	switch (ops[idx].kind)
	{
	/* plus */
	case OP_PLUS:
		if (check_add(lhs, rhs))
		{
			nums[idx] = lhs + rhs;
			break;
		}
		else
		{
			error_pos = ops[idx].pos;
			return false;
		}

	/* minus */
	case OP_MINUS:
		if (check_sub(lhs, rhs))
		{
			nums[idx] = lhs - rhs;
			break;
		}
		else
		{
			error_pos = ops[idx].pos;
			return false;
		}

	/* divide */
	case OP_DIV:
		if (check_div(lhs, rhs))
		{
			nums[idx] = lhs / rhs;
			break;
		}
		else
		{
			error_pos = ops[idx].pos;
			return false;
		}

	/* multiple */
	case OP_MUL:
		if (check_mul(lhs, rhs))
		{
			nums[idx] = lhs * rhs;
			break;
		}
		else
		{
			error_pos = ops[idx].pos;
			return false;
		}
	default:
		/* 演算子以外が入力されたらエラー */
		return false;
	}
	return true;
}

/**
 * @brief エラー報告
 * 次のような形式でエラー報告する
 * 1 + 2 / 0 *3
 *       ^ Invalid expression. Please try again.
 */
void report_error()
{
	printf("%s\n", input_str);
	printf("%*s", error_pos, "");
	printf("^");
	printf(" Invalid expression. Please try again.\n\n");
}

/**
 * @brief a+bがオーバーフローしないかチェック
 *
 * @param a
 * @param b
 * @return true オーバーフローしない
 * @return false オーバーフローする
 */
bool check_add(long a, long b)
{
	/* aが非負のとき */
	if (a >= 0)
	{
		return b <= INT_MAX - a; // 最小値側に超えることはないので最大値以下であることをチェック
	}

	/* aが負のとき */
	return b >= INT_MIN - a; // 最大値側に超えることはないので最小値以上であることをチェック
}

/**
 * @brief a-bがオーバーフローしないかチェック
 *
 * @param a
 * @param b
 * @return true オーバーフローしない
 * @return false オーバーフローする
 */
bool check_sub(long a, long b)
{
	/* bが最小値であるとき-bは最大値を超えることに注意 */
	if (b == INT_MIN)
	{
		return a < 0;
	}
	return check_add(a, -b);
}

/**
 * @brief a/bがオーバーフローしないかチェック
 *
 * @param a
 * @param b
 * @return true オーバーフローしない
 * @return false オーバーフローする
 */
bool check_div(long a, long b)
{
	/* 割り算はbが０でなければ問題なし */
	return b != 0;
}
/**
 * @brief a*bがオーバーフローしないかチェック
 *
 * @param a
 * @param b
 * @return true オーバーフローしない
 * @return false オーバーフローする
 */
bool check_mul(long a, long b)
{
	/* どちらかが0なら問題なし */
	if (a == 0 || b == 0)
	{
		return true;
	}

	/* ともに正 */
	if (a > 0 && b > 0)
	{
		return b <= INT_MAX / a;
	}

	/* ともに負 */
	if (a < 0 && b < 0)
	{
		/* どちらかが最小値ならオーバーフロー */
		if (a == INT_MIN || b == INT_MIN)
		{
			return false;
		}
		return -b <= INT_MAX / (-a);
	}
	/* a,bの符号が異なる */
	/* 片方が最小値のときは相手が1でなければオーバーフロー */
	if (a == INT_MIN)
	{
		return b == 1;
	}
	if (b == INT_MIN)
	{
		return a == 1;
	}
	/* 最小値でなければ符号を反転させることができる */
	if (a < 0)
	{
		a *= -1;
	}

	if (b < 0)
	{
		b *= -1;
	}

	return b <= INT_MAX / a;
}

void signal_handler(int signum)
{
	end_flg = true;
}