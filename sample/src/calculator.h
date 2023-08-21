#ifndef __CALCULATOR_H
#define __CALCULATOR_H

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>

extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream);

#define INT_MAX ((1L << 31) - 1)
#define INT_MIN (- (1L << 31))
#define MAX_LENGTH 128

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
char input_str[MAX_LENGTH];

/** 数式内の数値 */
long nums[MAX_LENGTH];

/** 数式内の演算子 */
Operator ops[MAX_LENGTH];

/** 演算子の優先順位の基数。括弧の内側に入るほど高くなる */
int priorityBase;

/** パースした数値の数 */
int numSize;

/** パースした演算子の数 */
int opSize;

/** エラーが見つかった個所 */
int error_pos;

bool end_flg = false;

/* プロトタイプ宣言 */
void remove_space(char *str);
void initialize(void);
bool parse_input(void);
int find_highest_priority_op(void);
bool perform_calculation(void);
bool calc(int idx);
void setOP(OperatorType type, long val, int pos);
bool check_add(long a, long b);
bool check_sub(long a, long b);
bool check_div(long a, long b);
bool check_mul(long a, long b);
void report_error(void);
void signal_handler(int signum);

#endif