/**
 * @file EightQueen.c
 * @author Kengo Fukunaga
 * @brief 四則演算ができる電卓
 * @version 0.1
 * @date 2023-08-05
 *
 * @copyright Copyright (c) 2023
 */

/** TRUE, FALSEの定義 */
typedef enum
{
	FALSE = 0,
	TRUE = 1
} bool;

/** 演算子の種類  */
typedef enum
{
	OP_PLUS,  // '+'
	OP_MINUS, // '-'
	OP_DIV,	  // '/'
	OP_MUL	  // '*'
} OperatorType;

/** 演算子を表現する構造体 */
typedef struct
{
	OperatorType kind; // 演算子の種類
	int priority;	   // 演算子の優先度
	int pos;		   // エラー報告用の演算子の位置
} Operator;

/** 入力文字列 */
char input_str[128];

/** 数式内の数値 */
int nums[128];

/** 数式内の演算子 */
Operator ops[128];

/** 演算子の優先順位の基数。括弧の内側に入るほど高くなる */
int priorityBase;

/** パースした数値の数 */
int numSize;

/** パースした演算子の数 */
int opSize;

/** エラーが見つかった個所 */
int error_pos;

/* プロトタイプ宣言 */
void remove_space(char *str);
void initialize();
bool parse_input();
int find_highest_priority_op();
bool perform_calculation();
bool calc(int idx);
void setOP(OperatorType type, int val, int pos);
bool check_add(int a, int b);
bool check_sub(int a, int b);
bool check_div(int a, int b);
bool check_mul(int a, int b);

void report_error();

/* FCCはヘッダをインクルードできないので標準ライブラリの関数を宣言しておく */
int printf(char *str);
int strlen(char *str);
int isdigit(char c);

/* FCCはファイルポインタがまだ実装できていないのでfgets()をラッピング */
char *receive_input(char *str);

int main()
{
	while (1)
	{
		/* 初期化 */
		initialize();

		/* 入力 */
		printf("Enter the formula to be calculated.\n");
		printf("Press Ctrl+C to exit.\n");
		printf("Input: ");
		if (receive_input(input_str) == (void *)0)
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
		if (parse_input() == FALSE)
		{
			report_error();
			continue;
		}

		/* 計算を実行 */
		if (perform_calculation() == FALSE)
		{
			report_error();
			continue;
		}

		/* 出力 */
		printf("Result: %d\n\n", nums[0]);
	}

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
	int tmp = 0;				// 読み取った数値
	int sz = strlen(input_str); // 入力文字列の長さ

	for (int i = 0; i < sz; i++)
	{
		char c = input_str[i];

		/* 数字 */
		if (isdigit(c))
		{
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
				return FALSE;
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
				return FALSE;
			}

			/* 直前が数字または')'でないなら無効な数式 */
			if (!isdigit(input_str[i - 1]) && input_str[i - 1] != ')')
			{
				error_pos = i;
				return FALSE;
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
				return FALSE;
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
				return FALSE;
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
				return FALSE;
			}
			/* 直前が数字または')'ではないときも存在しないため無効な数式 */
			if (!isdigit(input_str[i - 1]) && input_str[i - 1] != ')')
			{
				error_pos = i;
				return FALSE;
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
				return FALSE;
			}
			/* 直前が数字または')'ではないときも存在しないため無効な数式 */
			if (!isdigit(input_str[i - 1]) && input_str[i - 1] != ')')
			{
				error_pos = i;
				return FALSE;
			}

			/* 演算子と演算対象の直前の数値を配列にセットする */
			setOP(OP_MUL, tmp, i);

			/* 読み取った数値をクリアする */
			tmp = 0;
			continue;
		}

		/* 上記のどれでもないときはエラー */
		error_pos = i;
		return FALSE;
	}

	/* 入力された数式に問題がないか確認 */
	/* 括弧の数があっているか */
	if (priorityBase != 0)
	{
		error_pos = sz - 1;
		return FALSE;
	}
	/* 演算子の数があっているか */
	if (numSize != opSize + 1)
	{
		error_pos = sz - 1;
		return FALSE;
	}
	return TRUE;
}

/**
 * @brief 演算子を配列にセットする
 *
 * @param type 演算子の種類
 * @param val 直前の数値
 * @param pos 演算子の位置
 */
void setOP(OperatorType type, int val, int pos)
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
		if (calc(idx) == FALSE)
		{
			return FALSE;
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

	return TRUE;
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
	int lhs = nums[idx];
	int rhs = nums[idx + 1];
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
			return FALSE;
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
			return FALSE;
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
			return FALSE;
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
			return FALSE;
		}
	default:
		/* 演算子以外が入力されたらエラー */
		return FALSE;
	}
	return TRUE;
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
bool check_add(int a, int b)
{
	int max = (1 << 31) - 1;
	int min = -(1 << 31);

	/* aが非負のとき */
	if (a >= 0)
	{
		return b <= max - a; // 最小値側に超えることはないので最大値以下であることをチェック
	}

	/* aが負のとき */
	return b >= min - a; // 最大値側に超えることはないので最小値以上であることをチェック
}

/**
 * @brief a-bがオーバーフローしないかチェック
 *
 * @param a
 * @param b
 * @return true オーバーフローしない
 * @return false オーバーフローする
 */
bool check_sub(int a, int b)
{
	int min = -(1 << 31);
	/* bが最小値であるとき-bは最大値を超えることに注意 */
	if (b == min)
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
bool check_div(int a, int b)
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
bool check_mul(int a, int b)
{
	int max = (1 << 31) - 1;
	int min = -(1 << 31);

	/* どちらかが0なら問題なし */
	if (a == 0 || b == 0)
	{
		return TRUE;
	}

	/* ともに正 */
	if (a > 0 && b > 0)
	{
		return b <= max / a;
	}

	/* ともに負 */
	if (a < 0 && b < 0)
	{
		/* どちらかが最小値ならオーバーフロー */
		if (a == min || b == min)
		{
			return FALSE;
		}
		return -b <= max / (-a);
	}
	/* a,bの符号が異なる */
	/* 片方が最小値のときは相手が1でなければオーバーフロー */
	if (a == min)
	{
		return b == 1;
	}
	if (b == min)
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

	return b <= max / a;
}
