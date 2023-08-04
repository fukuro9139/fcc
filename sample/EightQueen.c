/**
 * @file EightQueen.c
 * @author Kengo Fukunaga
 * @brief エイトクイーン問題をバックトラック法で解くソルバー
 * @version 0.1
 * @date 2023-08-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdio.h>

/* 発見した買いの総数 */
static int count = 0;

/* 現在のボードの状態 */
int board[8][8];

/* クイーンが動ける方向 */
int delta[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

void init_board()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			board[i][j] = 0;
		}
	}
}

/* クイーンが(row, col)の位置にいるとしてクイーンが動ける範囲のボードの値を
 * +d する。
 */
void change_board(int row, int col, int d)
{
	/* 縦と横 */
	for (int i = 0; i < 8; i++)
	{
		board[row][i] += d;
		board[i][col] += d;
	}

	i = row;
	j = col;
	while (i < 8 && j < 8)
	{
		board[i][j] += d;
		i++;
		j++;
	}

	i = row;
	j = col;
	while (i >= 0 && j >= 0)
	{
		board[i][j] += d;
		i--;
		j--;
	}

	i = row;
	j = col;
	while (i < 8 && j >= 0)
	{
		board[i][j] += d;
		i++;
		j--;
	}

	i = row;
	j = col;
	while (i >= 0 && j < 8)
	{
		board[i][j] += d;
		i--;
		j++;
	}
}

void set_queen(int queen_position[8], int board[8][8], int row)
{
	int col;

	if (row == 8)
	{
		count++;
		return;
	}

	/* チェスの盤上に座布団を重ねていくようなイメージ、三次元的な処理 */
	for (col = 0; col < 8; col++)
	{
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
	int queen_position[8];
	int board[8][8];
	init_board(board);
	set_queen(queen_position, board, 0);

	printf("%d\n", count);

	return 0;
}