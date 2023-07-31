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

using std::endl;
using std::string;
using std::unique_ptr;

/** スタックの深さ */
static int depth = 0;

/** 64ビット整数レジスタ、前から順に関数の引数を格納される */
static const std::vector<string> arg_regs64 = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

/** 整数レジスタの下位8ビットのエイリアス、前から順に関数の引数を格納される */
static const std::vector<string> arg_regs8 = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

/** 現在処理中の関数*/
static Object *current_func = nullptr;

/* 入出力 */
static std::ostream *os = &std::cout;

/*****************/
/* CodeGen Class */
/*****************/

/**
 * @brief raxがさすアドレスの値をロードする。
 *
 * @param ty 読み込む値の型
 */
void CodeGen::load(const std::shared_ptr<Type> &ty)
{
	/* 変数の型が配列型の場合、レジスタへ値をロードをしようとしてはいけない。*/
	/* これは一般的に配列全体をレジスタに読み込むことはできないからである。*/
	/* そのため配列型の変数の評価は配列そのものではなく配列のアドレスとなる。*/
	/* これは配列型が暗黙に配列の最初の要素へのポインタへ返還されることを示している。 */
	if (TypeKind::TY_ARRAY == ty->_kind)
	{
		return;
	}

	/* x86-64では、下位8ビットのエイリアスのレジスタに読み込むときには */
	/* 上位56ビットは0クリアされない。そのため符号拡張してraxにロードする。 */
	if (1 == ty->_size)
	{
		*os << "  movzx rax, BYTE PTR [rax]\n";
	}
	else
	{
		*os << "  mov rax, [rax]\n";
	}
}

/**
 * @brief raxの値をスタックのトップのアドレスが示すメモリにストアする。
 *
 */
void CodeGen::store(const Type *ty)
{
	pop("rdi");

	if (1 == ty->_size)
	{
		*os << "  mov [rdi], al\n";
	}
	else
	{
		*os << "  mov [rdi], rax\n";
	}
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
	*os << "  push rax\n";
	++depth;
}

/**
 * @brief スタックからpopした数値を指定したレジスタにセットする
 *
 * @param reg 数値をセットするレジスタ
 */
void CodeGen::pop(string &&reg)
{
	*os << "  pop " << reg << "\n";
	--depth;
}

/**
 * @brief スタックからpopした数値を指定したレジスタにセットする
 *
 * @param reg 数値をセットするレジスタ
 */
void CodeGen::pop(const std::string &reg)
{
	*os << "  pop " << reg << "\n";
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
			*os << "  lea rax, [rbp - " << node->_var->_offset << "]\n";
		}
		/* グローバル変数。RIP相対アドレッシングを使う */
		else
		{
			*os << "  lea rax, [rip + " << node->_var->_name << "]\n";
		}

		return;
	case NodeKind::ND_DEREF:
		generate_expression(std::move(node->_lhs));
		return;
	default:
		break;
	}
	error_token("左辺値ではありません", node->_token);
}

/**
 * @brief 式をアセンブリに変換
 *
 * @param node 変換対象のAST
 */
void CodeGen::generate_expression(unique_ptr<Node> &&node)
{
	*os << "  .loc 1 " << node->_token->_line_no << "\n";
	switch (node->_kind)
	{
	/* 数値 */
	case NodeKind::ND_NUM:
		/* 数値を'rax'に格納 */
		*os << "  mov rax, " << node->_val << "\n";
		return;
	/* 単項演算子の'-' */
	case NodeKind::ND_NEG:
		/* '-'がかかる式を評価 */
		generate_expression(std::move(node->_lhs));
		/* 符号を反転させる */
		*os << "  neg rax\n";
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
		store(node->_ty.get());
		return;
	/* 重複文 */
	case NodeKind::ND_STMT_EXPR:
	{
		auto cur = std::move(node->_body);
		unique_ptr<Node> next;
		for (; cur; cur = std::move(next))
		{
			next = std::move(cur->_next);
			generate_statement(std::move(cur));
		}
		return;
	}
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
			pop(arg_regs64[i]);
		}
		*os << "  mov rax, 0\n";
		*os << "  call " << node->_func_name << "\n";
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
		*os << "  add rax, rdi\n";
		return;
	case NodeKind::ND_SUB:
		/* 'rax' = 'rax' - 'rdi' */
		*os << "  sub rax, rdi\n";
		return;
	case NodeKind::ND_MUL:
		/* 'rax' = 'rax' * 'rdi' */
		*os << "  imul rax, rdi\n";
		return;
	case NodeKind::ND_DIV:
		/* 'rax'(64ビット)を128ビットに拡張して'rdx'と'rax'にセット */
		*os << "  cqo\n";
		/* 'rdx'と'rax'を合わせた128ビットの値を'rdi'で割って商を'rax', 余りを'rdx'にセット */
		*os << "  idiv rdi\n";
		return;
	case NodeKind::ND_EQ:
		*os << "  cmp rax, rdi\n";
		*os << "  sete al\n";
		*os << "  movzb rax, al\n";
		return;
	case NodeKind::ND_NE:
		*os << "  cmp rax, rdi\n";
		*os << "  setne al\n";
		*os << "  movzb rax, al\n";
		return;
	case NodeKind::ND_LT:
		*os << "  cmp rax, rdi\n";
		*os << "  setl al\n";
		*os << "  movzb rax, al\n";
		return;
	case NodeKind::ND_LE:
		*os << "  cmp rax, rdi\n";
		*os << "  setle al\n";
		*os << "  movzb rax, al\n";
		return;
	default:
		break;
	}

	/* エラー */
	error_token("不正な式です", node->_token);
}

/**
 * @brief 文をアセンブリに変換
 *
 * @param node 変換対象のAST
 */
void CodeGen::generate_statement(unique_ptr<Node> &&node)
{
	*os << "  .loc 1 " << node->_token->_line_no << "\n";

	switch (node->_kind)
	{
	case NodeKind::ND_RETURN:
		/* return の後の式を評価 */
		generate_expression(std::move(node->_lhs));
		/* エピローグまでjmpする */
		*os << "  jmp .L.return." << current_func->_name << "\n";
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
		*os << "  cmp rax, 0\n";
		/* 条件がfalseなら.L.else.cラベルに飛ぶ */
		*os << "  je .L.else." << c << "\n";
		/* trueのときに実行 */
		generate_statement(std::move(node->_then));
		/* elseは実行しない */
		*os << "  jmp .L.end." << c << "\n";

		/* falseのとき実行 */
		*os << ".L.else." << c << ":\n";
		if (node->_else)
		{
			generate_statement(std::move(node->_else));
		}
		*os << ".L.end." << c << ":\n";
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

		*os << ".L.begin." << c << ":\n";
		/* 終了条件判定 */
		if (node->_condition)
		{
			generate_expression(std::move(node->_condition));
			*os << "  cmp rax, 0\n";
			*os << "  je .L.end." << c << "\n";
		}
		generate_statement(std::move(node->_then));
		/* 加算処理 */
		if (node->_inc)
		{
			generate_expression(std::move(node->_inc));
		}
		*os << "  jmp .L.begin." << c << "\n";
		*os << ".L.end." << c << ":\n";
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
	error_token("不正な構文です", node->_token);
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
		*os << "  .data\n";
		*os << "  .globl " << var->_name << "\n";
		*os << var->_name << ":\n";

		if (var->is_str_literal)
		{
			for (int i = 0; i < var->_init_data.size(); ++i)
			{
				*os << "  .byte " << static_cast<int>(var->_init_data[i]) << "\n";
			}
		}
		else
		{
			*os << "  .zero " << var->_ty->_size << "\n";
		}
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
		*os << "  .globl " << fn->_name << "\n";
		*os << "  .text\n";
		*os << fn->_name << ":\n";

		/* 現在の関数をセット */
		current_func = fn;

		/* プロローグ */
		/* スタックサイズの領域を確保する */
		*os << "  push rbp\n";
		*os << "  mov rbp, rsp\n";
		*os << "  sub rsp, " << fn->_stack_size << "\n";

		/* 関数の場合、レジスタから引数を受け取って確保してあるスタック領域にローカル変数と同様にストアする */
		int cnt = 0;
		for (auto var = fn->_params.get(); var; var = var->_next.get())
		{
			if (1 == var->_ty->_size)
			{
				*os << "  mov [rbp - " << var->_offset << "], " << arg_regs8[cnt++] << "\n";
			}
			else
			{
				*os << "  mov [rbp - " << var->_offset << "], " << arg_regs64[cnt++] << "\n";
			}
		}

		/* コードを出力 */
		generate_statement(std::move(fn->_body));

		/* 関数終了時にスタックの深さが0 (popし残し、popし過ぎがない) */
		assert(depth == 0);

		/* エピローグ */
		/* 最後の結果がraxに残っているのでそれが返り値になる */
		*os << ".L.return." << fn->_name << ":\n";
		*os << "  mov rsp, rbp\n";
		*os << "  pop rbp\n";
		*os << "  ret\n";
	}
}

/**
 * @brief 関数ごとにASTを意味解析し、Intel記法でアセンブリを出力する
 *
 * @param program アセンブリを出力する対象関数
 */
void CodeGen::generate_code(unique_ptr<Object> &&program, const std::string &input_path, const std::string &output_path)
{
	/* ファイルを開くのに成功したら出力先をファイルに変更する */
	/* ファイルを開くのに失敗したら出力先は標準出力のまま */
	unique_ptr<std::ostream> ofs(new std::ofstream(output_path));
	if (!ofs->fail())
	{
		os = ofs.get();
	}

	/* intel記法であることを宣言 */
	*os << ".intel_syntax noprefix\n";

	/* .fileディレクティブを出力 */
	*os << ".file 1 \"" << input_path << "\"\n";

	/* スタックサイズを計算してセット */
	Object::assign_lvar_offsets(program);

	/* .data部を出力 */
	emit_data(program);

	/* text部を出力 */
	emit_text(program);
}
