/**
 * @file EightQueen.c
 * @author Kengo Fukunaga
 * @brief エイトクイーン問題をバックトラック法で解くソルバー
 * @version 0.1
 * @date 2023-08-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "eightqueen.h"

/* 発見した買いの総数 */
static int count;

void init_board(int board[WIDTH][WIDTH])
{
	for (int i = 0; i < WIDTH; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			board[i][j] = 0;
		}
	}
}

/* 盤面を出力する */
void print_board(int queen_position[WIDTH])
{
	printf("No.%d\n", count);
	for (int i = 0; i < WIDTH; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			/* 位置(i, j)にクイーンが置いてある */
			if (j == queen_position[i])
			{
				printf("Q");
			}
			/* 位置(i, j)にクイーンが置いてない */
			else
			{
				printf("*");
			}
		}
		printf("\n");
	}
	printf("\n");
}

/* クイーンが(row, col)の位置にいるとしてクイーンが動ける範囲のボードの値を
 * +val する。
 */
void change_board(int board[WIDTH][WIDTH], int row, int col, int val)
{
	/* クイーンが動ける方向 */
	int delta[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

	for (int d = 0; d < 8; d++)
	{
		for (int k = 1; k < (WIDTH > WIDTH ? WIDTH : WIDTH); k++)
		{
			int next_row = row + k * delta[d][0];
			int next_col = col + k * delta[d][1];

			if (next_row < 0 || next_row >= WIDTH || next_col < 0 || next_col >= WIDTH)
				continue;

			board[next_row][next_col] += val;
		}
	}
}

void set_queen(int queen_position[WIDTH], int board[WIDTH][WIDTH], int row)
{
	/* 全てのクイーンを置き終わった */
	if (row == WIDTH)
	{
		count++;
		print_board(queen_position);
		return 0;
	}

	/* 新しいクイーンを置く場所を決める */
	for (int col = 0; col < WIDTH; col++)
	{
		/* board[row][col]の位置に配置可能 */
		if (board[row][col] == 0)
		{
			/* board[row][col]の位置にクイーンを配置 */
			queen_position[row] = col;
			/* 縦横斜めを使用禁止にするためおけない場所に座布団を+1 */
			change_board(board, row, col, 1);
			/* 次の行のクイーンの位置を決める */
			set_queen(queen_position, board, row + 1);
			/* 重ねた座布団を-1 */
			change_board(board, row, col, -1);
		}
	}
}

int main(void)
{
	/* 発見した個数の初期化 */
	count = 0;

	/* クイーンの位置 */
	/* 解となる配置ではクイーンは必ず各列に1個ずつ存在する。なぜならばクイーンが同じ列に
	 * 複数存在すればお互いに相手を取ることが可能なため条件を満たさない。
	 * したがって、各列に存在するクイーンの数は1個以下であり、クイーンをおかない列が存在すると
	 * ８個のクイーンを置ききることができない。
	 */
	int queen_position[WIDTH];

	/* 現在のボードの状態 */
	int board[WIDTH][WIDTH];

	/* ボードを初期化する */
	init_board(board);

	/* クイーンの配置を決めて出力する */
	set_queen(queen_position, board, 0);

	printf("%d solutions were found.\n", count);

	printf("Press the Key to exit.");
	getchar();

	return 0;
}