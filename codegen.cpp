/**
 * @file codegen.cpp
 * @author K.fukunaga
 * @brief 抽象構文木(AST)を意味解析してアセンブリを出力するコードジェネレーターの定義。
 *
 * x86-64用のアセンブリをIntel記法で出力する。
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "codegen.hpp"

using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;

/** スタックの深さ */
static int depth = 0;

/**
 * @brief 新しいラベルの通し番号を返す。
 *
 * @return これまでに用意したラベルの数+1
 */
int CodeGen::label_count()
{
	static int i = 1;
	return i++;
}

/** @brief 'rax'の数値をスタックにpushする */
void CodeGen::push()
{
	cout << " push rax\n";
	++depth;
}

/**
 * @brief スタックからpopした数値を指定したレジスタにセットする
 *
 * @param reg 数値をセットするレジスタ
 */
void CodeGen::pop(string &&reg)
{
	cout << " pop " << reg << "\n";
	--depth;
}

/**
 * @brief 変数ノードの絶対アドレスを計算して'rax'にセットするコード
 *
 * @note ノードが変数ではないときエラーとする。
 * @param node 対象ノード
 */
void CodeGen::generate_address(unique_ptr<Node> &&node)
{
	switch (node->_kind)
	{
	case NodeKind::ND_VAR:
		cout << " mov rax, rbp\n";
		cout << " sub rax, " << node->_var->_offset << "\n";
		return;
	case NodeKind::ND_DEREF:
		generate_expression(std::move(node->_lhs));
		return;
	default:
		break;
	}
	error_at("左辺値ではありません", std::move(node->_location));
}

/**
 * @brief 式をアセンブリに変換
 *
 * @param node 変換対象のAST
 */
void CodeGen::generate_expression(unique_ptr<Node> &&node)
{
	switch (node->_kind)
	{
	/* 数値 */
	case NodeKind::ND_NUM:
		/* 数値を'rax'に格納 */
		cout << " mov rax, " << node->_val << "\n";
		return;
	/* 単項演算子の'-' */
	case NodeKind::ND_NEG:
		/* '-'がかかる式を評価 */
		generate_expression(std::move(node->_lhs));
		/* 符号を反転させる */
		cout << " neg rax\n";
		return;
	/* 変数 */
	case NodeKind::ND_VAR:
		/* 変数のアドレスを計算 */
		generate_address(std::move(node));
		/* 変数のアドレスから値を'rax'に読み込む */
		cout << " mov rax, [rax]\n";
		return;
	/* 参照外し */
	case NodeKind::ND_DEREF:
		/* 参照先のアドレスを計算 */
		generate_expression(std::move(node->_lhs));
		/* 参照先のアドレスの数値をセット */
		cout << " mov rax, [rax]\n";
		return;
	/* 参照 */
	case NodeKind::ND_ADDR:
		/* アドレスを計算 */
		generate_address(std::move(node->_lhs));
		return;
	/* 代入 */
	case NodeKind::ND_ASSIGN:
		/* 左辺の代入先の変数のアドレスを計算 */
		generate_address(std::move(node->_lhs));
		/* 変数のアドレスをスタックにpush */
		push();
		/* 右辺を評価する。評価結果は'rax' */
		generate_expression(std::move(node->_rhs));
		/* 変数のアドレスを'rdi にpop*/
		pop("rdi");
		/* 'rax'の値を'rdi'のアドレスのメモリに格納 */
		cout << " mov [rdi], rax\n";
		return;
	default:
		break;
	}

	/* 右辺を計算 */
	generate_expression(std::move(node->_rhs));
	/* 計算結果をスタックにpush */
	push();
	/* 左辺を計算 */
	generate_expression(std::move(node->_lhs));
	/* 右辺の計算結果を'rdi'にpop */
	pop("rdi");

	/* 演算子ノードなら演算命令を出力する */
	switch (node->_kind)
	{
	case NodeKind::ND_ADD:
		/* 'rax' = 'rax' + 'rdi' */
		std::cout << " add rax, rdi\n";
		return;
	case NodeKind::ND_SUB:
		/* 'rax' = 'rax' - 'rdi' */
		std::cout << " sub rax, rdi\n";
		return;
	case NodeKind::ND_MUL:
		/* 'rax' = 'rax' * 'rdi' */
		std::cout << " imul rax, rdi\n";
		return;
	case NodeKind::ND_DIV:
		/* 'rax'(64ビット)を128ビットに拡張して'rdx'と'rax'にセット */
		std::cout << " cqo\n";
		/* 'rdx'と'rax'を合わせた128ビットの値を'rdi'で割って商を'rax', 余りを'rdx'にセット */
		std::cout << " idiv rdi\n";
		return;
	case NodeKind::ND_EQ:
		std::cout << " cmp rax, rdi\n";
		std::cout << " sete al\n";
		std::cout << " movzb rax, al\n";
		return;
	case NodeKind::ND_NE:
		std::cout << " cmp rax, rdi\n";
		std::cout << " setne al\n";
		std::cout << " movzb rax, al\n";
		return;
	case NodeKind::ND_LT:
		std::cout << " cmp rax, rdi\n";
		std::cout << " setl al\n";
		std::cout << " movzb rax, al\n";
		return;
	case NodeKind::ND_LE:
		std::cout << " cmp rax, rdi\n";
		std::cout << " setle al\n";
		std::cout << " movzb rax, al\n";
		return;
	default:
		break;
	}

	/* エラー */
	error_at("不正な式です", std::move(node->_location));
}

/**
 * @brief 文をアセンブリに変換
 *
 * @param node 変換対象のAST
 */
void CodeGen::generate_statement(unique_ptr<Node> &&node)
{

	switch (node->_kind)
	{
	case NodeKind::ND_RETURN:
		/* return の後の式を評価 */
		generate_expression(std::move(node->_lhs));
		/* エピローグまでjmpする */
		cout << " jmp .L.return\n";
		return;
	case NodeKind::ND_EXPR_STMT:
		generate_expression(std::move(node->_lhs));
		return;
	case NodeKind::ND_IF:
	{
		/* 通し番号を取得 */
		const int c = label_count();

		/* 条件を評価 */
		generate_expression(std::move(node->_condition));
		/* 条件を比較 */
		cout << " cmp rax, 0\n";
		/* 条件がfalseなら.L.else.cラベルに飛ぶ */
		cout << " je .L.else." << c << "\n";
		/* trueのときに実行 */
		generate_statement(std::move(node->_then));
		/* elseは実行しない */
		cout << " jmp .L.end." << c << "\n";

		/* falseのとき実行 */
		cout << ".L.else." << c << ":\n";
		if (node->_else)
		{
			generate_statement(std::move(node->_else));
		}
		cout << ".L.end." << c << ":\n";
		return;
	}

	/* for or while */
	case NodeKind::ND_FOR:
	{
		/* 通し番号を取得 */
		const int c = label_count();

		/* forの場合、初期化処理 */
		if (node->_init)
		{
			generate_statement(std::move(node->_init));
		}

		cout << ".L.begin." << c << ":\n";
		/* 終了条件判定 */
		if (node->_condition)
		{
			generate_expression(std::move(node->_condition));
			cout << " cmp rax, 0\n";
			cout << " je .L.end." << c << "\n";
		}
		generate_statement(std::move(node->_then));
		/* 加算処理 */
		if (node->_inc)
		{
			generate_expression(std::move(node->_inc));
		}
		cout << " jmp .L.begin." << c << "\n";
		cout << ".L.end." << c << ":\n";
		return;
	}

	case NodeKind::ND_BLOCK:
	{
		/* これから処理を進めていくノード */
		unique_ptr<Node> current_node = std::move(node->_body);

		/* ブロックの中が空なら何もしない */
		if (!current_node)
		{
			return;
		}
		/* Bodyに含まれるノードをすべて評価する */
		for (unique_ptr<Node> next_node = std::move(current_node->_next); current_node;)
		{
			generate_statement(std::move(current_node));
			current_node = std::move(next_node);
			if (current_node)
			{
				next_node = std::move(current_node->_next);
			}
		}
		return;
	}
	default:
		break;
	}
	error_at("不正な構文です", std::move(node->_location));
}

/**
 * @brief 関数ごとにASTを意味解析し、Intel記法でアセンブリを出力する
 *
 * @param program アセンブリを出力する対象関数
 */
void CodeGen::generate_code(unique_ptr<Function> &&program)
{
	/* アセンブリの前半部分を出力 */
	cout << ".intel_syntax noprefix\n";
	cout << ".globl main\n";
	cout << "main:\n";

	/* スタックサイズを計算 */
	program->assign_lvar_offsets();

	/* プロローグ */
	/* スタックサイズの領域を確保する */
	/* 1変数につき8バイト */
	cout << " push rbp\n";
	cout << " mov rbp, rsp\n";
	cout << " sub rsp, " << program->_stack_size << "\n";

	generate_statement(std::move(program->_body));
	assert(0 == depth);

	/* エピローグ */
	/* 最後の結果がRAXに残っているのでそれが返り値になる */
	cout << ".L.return:\n";
	cout << " mov rsp, rbp\n";
	cout << " pop rbp\n";
	cout << " ret\n";
}
