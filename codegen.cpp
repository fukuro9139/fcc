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

#include <vector>
#include "codegen.hpp"

using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;

/** スタックの深さ */
static int depth = 0;

/** 関数の引数を格納するレジスタ */
static const std::vector<string> arg_regs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

/** 現在処理中の関数*/
static Object *current_func = nullptr;

/*****************/
/* CodeGen Class */
/*****************/

/**
 * @brief raxがさすアドレスの値をロードする。
 *
 * @details
 * 変数の型が配列型の場合、レジスタへ値をロードをしようとしてはいけない。
 * これは一般的に配列全体をレジスタに読み込むことはできないからである。
 * そのため配列型の変数の評価は配列そのものではなく配列のアドレスとなる。
 * これは配列型が暗黙に配列の最初の要素へのポインタへ返還されることを示している。
 * @param ty 読み込む値の型
 */
void CodeGen::load(const std::shared_ptr<Type> &ty)
{
	if (TypeKind::TY_ARRAY == ty->_kind)
	{
		return;
	}
	std::cout << "  mov rax, [rax]\n";
}

/**
 * @brief raxの値をスタックのトップのアドレスが示すメモリにストアする。
 *
 */
void CodeGen::store()
{
	pop("rdi");
	std::cout << "  mov [rdi], rax\n";
}

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
	cout << "  push rax\n";
	++depth;
}

/**
 * @brief スタックからpopした数値を指定したレジスタにセットする
 *
 * @param reg 数値をセットするレジスタ
 */
void CodeGen::pop(string &&reg)
{
	cout << "  pop " << reg << "\n";
	--depth;
}

/**
 * @brief スタックからpopした数値を指定したレジスタにセットする
 *
 * @param reg 数値をセットするレジスタ
 */
void CodeGen::pop(const std::string &reg)
{
	cout << "  pop " << reg << "\n";
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
		/* ローカル変数 */
		if (node->_var->is_local)
		{
			cout << "  lea rax, [rbp - " << node->_var->_offset << "]\n";
		}
		/* グローバル変数。RIP相対アドレッシングを使う */
		else
		{
			cout << "  lea rax, [rip + " << node->_var->_name << "]\n";
		}

		return;
	case NodeKind::ND_DEREF:
		generate_expression(std::move(node->_lhs));
		return;
	default:
		break;
	}
	error_at("左辺値ではありません", node->_location);
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
		cout << "  mov rax, " << node->_val << "\n";
		return;
	/* 単項演算子の'-' */
	case NodeKind::ND_NEG:
		/* '-'がかかる式を評価 */
		generate_expression(std::move(node->_lhs));
		/* 符号を反転させる */
		cout << "  neg rax\n";
		return;
	/* 変数 */
	case NodeKind::ND_VAR:
		/* 変数のアドレスを計算 */
		generate_address(std::move(node));
		/* 変数のアドレスから値をロードする */
		load(node->_ty);
		return;
	/* 参照外し */
	case NodeKind::ND_DEREF:
		/* 参照先のアドレスを計算 */
		generate_expression(std::move(node->_lhs));
		/* 参照先のアドレスの値をロード */
		load(node->_ty);
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
		/* raxの値をストアする */
		store();
		return;
		/* 関数呼び出し */
	case NodeKind::ND_FUNCALL:
	{
		/* 引数の数 */
		int nargs = 0;

		auto arg = std::move(node->_args);
		unique_ptr<Node> next_arg;

		for (; arg; arg = std::move(next_arg))
		{
			next_arg = std::move(arg->_next);
			/* 引数の値を評価 */
			generate_expression(std::move(arg));
			/* スタックに入れる */
			push();
			++nargs;
		}
		/* 引数の値を対応するレジスタにセット */
		for (int i = nargs - 1; i >= 0; --i)
		{
			pop(arg_regs[i]);
		}
		cout << "  mov rax, 0\n";
		cout << "  call " << node->_func_name << "\n";
		return;
	}

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
		std::cout << "  add rax, rdi\n";
		return;
	case NodeKind::ND_SUB:
		/* 'rax' = 'rax' - 'rdi' */
		std::cout << "  sub rax, rdi\n";
		return;
	case NodeKind::ND_MUL:
		/* 'rax' = 'rax' * 'rdi' */
		std::cout << "  imul rax, rdi\n";
		return;
	case NodeKind::ND_DIV:
		/* 'rax'(64ビット)を128ビットに拡張して'rdx'と'rax'にセット */
		std::cout << "  cqo\n";
		/* 'rdx'と'rax'を合わせた128ビットの値を'rdi'で割って商を'rax', 余りを'rdx'にセット */
		std::cout << "  idiv rdi\n";
		return;
	case NodeKind::ND_EQ:
		std::cout << "  cmp rax, rdi\n";
		std::cout << "  sete al\n";
		std::cout << "  movzb rax, al\n";
		return;
	case NodeKind::ND_NE:
		std::cout << "  cmp rax, rdi\n";
		std::cout << "  setne al\n";
		std::cout << "  movzb rax, al\n";
		return;
	case NodeKind::ND_LT:
		std::cout << "  cmp rax, rdi\n";
		std::cout << "  setl al\n";
		std::cout << "  movzb rax, al\n";
		return;
	case NodeKind::ND_LE:
		std::cout << "  cmp rax, rdi\n";
		std::cout << "  setle al\n";
		std::cout << "  movzb rax, al\n";
		return;
	default:
		break;
	}

	/* エラー */
	error_at("不正な式です", node->_location);
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
		cout << "  jmp .L.return." << current_func->_name << "\n";
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
		cout << "  cmp rax, 0\n";
		/* 条件がfalseなら.L.else.cラベルに飛ぶ */
		cout << "  je .L.else." << c << "\n";
		/* trueのときに実行 */
		generate_statement(std::move(node->_then));
		/* elseは実行しない */
		cout << "  jmp .L.end." << c << "\n";

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
			cout << "  cmp rax, 0\n";
			cout << "  je .L.end." << c << "\n";
		}
		generate_statement(std::move(node->_then));
		/* 加算処理 */
		if (node->_inc)
		{
			generate_expression(std::move(node->_inc));
		}
		cout << "  jmp .L.begin." << c << "\n";
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
		unique_ptr<Node> next_node;
		/* Bodyに含まれるノードをすべて評価する */
		for (; current_node;)
		{
			next_node = std::move(current_node->_next);
			generate_statement(std::move(current_node));
			current_node = std::move(next_node);
		}
		return;
	}
	default:
		break;
	}
	error_at("不正な構文です", node->_location);
}

/**
 * @brief プログラムの.data部を出力する
 *
 * @param program 入力プログラム
 */
void CodeGen::emit_data(const unique_ptr<Object> &program)
{
	for (auto var = program.get(); var; var = var->_next.get())
	{
		/* 関数の場合は何もしない */
		if (var->is_function)
		{
			continue;
		}

		/* グローバル変数のサイズだけ領域を確保する */
		cout << "  .data\n";
		cout << "  .globl " << var->_name << "\n";
		cout << var->_name << ":\n";
		cout << "  .zero " << var->_ty->_size << "\n";
	}
}

/**
 * @brief プログラムの.text部を出力する
 *
 * @param program 入力プログラム
 */
void CodeGen::emit_text(const std::unique_ptr<Object> &program)
{
	for (auto fn = program.get(); fn; fn = fn->_next.get())
	{
		/* 関数でなければ何もしない */
		if (!fn->is_function)
		{
			continue;
		}

		/* 関数のラベル部分を出力 */
		cout << "  .globl " << fn->_name << "\n";
		cout << "  .text\n";
		cout << fn->_name << ":\n";

		/* 現在の関数をセット */
		current_func = fn;

		/* プロローグ */
		/* スタックサイズの領域を確保する */
		cout << "  push rbp\n";
		cout << "  mov rbp, rsp\n";
		cout << "  sub rsp, " << fn->_stack_size << "\n";

		/* 関数の場合、レジスタから引数を受け取って確保してあるスタック領域にローカル変数と同様にストアする */
		int cnt = 0;
		for (auto var = fn->_params.get(); var; var = var->_next.get())
		{
			cout << "  mov [rbp - " << var->_offset << "], " << arg_regs[cnt++] << "\n";
		}

		/* コードを出力 */
		generate_statement(std::move(fn->_body));

		/* 関数終了時にスタックの深さが0 (popし残し、popし過ぎがない) */
		assert(depth == 0);

		/* エピローグ */
		/* 最後の結果がraxに残っているのでそれが返り値になる */
		cout << ".L.return." << fn->_name << ":\n";
		cout << "  mov rsp, rbp\n";
		cout << "  pop rbp\n";
		cout << "  ret\n";
	}
}

/**
 * @brief 関数ごとにASTを意味解析し、Intel記法でアセンブリを出力する
 *
 * @param program アセンブリを出力する対象関数
 */
void CodeGen::generate_code(unique_ptr<Object> &&program)
{

	/* スタックサイズを計算してセット */
	Object::assign_lvar_offsets(program);

	/* intel記法であることを宣言 */
	cout << ".intel_syntax noprefix\n";

	/* .data部を出力 */
	emit_data(program);

	/* text部を出力 */
	emit_text(program);
}
