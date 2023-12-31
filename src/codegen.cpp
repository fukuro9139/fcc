/**
 * @file codegen.cpp
 * @author K.Fukunaga
 * @brief 抽象構文木(AST)を意味解析してアセンブリを出力するコードジェネレーターの定義。
 *
 * x86-64用のアセンブリをIntel記法で出力する。
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "codegen.hpp"
#include "tokenize.hpp"
#include "parse.hpp"
#include "object.hpp"
#include "type.hpp"

/** スタックの深さ */
static int depth = 0;

/** 現在処理中の関数*/
static Object *current_func = nullptr;

/* 入出力 */
static std::ostream *os = &std::cout;

/* デバッグ情報を付与するか */
static bool print_dbg_info = false;

/*****************/
/* CodeGen Class */
/*****************/

/**
 * @brief 型をサイズごとに分類したIDを返す
 *
 * @param ty 判定する型
 * @return 型のID
 */
int CodeGen::get_TypeId(Type *ty)
{
	TypeID id = TypeID::U64;

	switch (ty->_kind)
	{
	case TypeKind::TY_CHAR:
		id = ty->_is_unsigned ? TypeID::U8 : TypeID::I8;
		break;
	case TypeKind::TY_SHORT:
		id = ty->_is_unsigned ? TypeID::U16 : TypeID::I16;
		break;
	case TypeKind::TY_INT:
		id = ty->_is_unsigned ? TypeID::U32 : TypeID::I32;
		break;
	case TypeKind::TY_LONG:
		id = ty->_is_unsigned ? TypeID::U64 : TypeID::I64;
		break;
	case TypeKind::TY_FLOAT:
		id = TypeID::F32;
		break;
	case TypeKind::TY_DOUBLE:
		id = TypeID::F64;
		break;
	default:
		break;
	}
	return static_cast<int>(id);
}

/**
 * @brief 型キャストする
 *
 * @param from 変換元の型
 * @param to 変換後の型
 */
void CodeGen::cast(Type *from, Type *to)
{
	/* void型に対する変換では何もしない */
	if (TypeKind::TY_VOID == to->_kind)
	{
		return;
	}

	if (TypeKind::TY_BOOL == to->_kind)
	{
		cmp_zero(from);
		/* al = (from != 0) ? 1 : 0 */
		*os << "  setne al\n";
		*os << "  movzb eax, al\n";
		return;
	}

	auto t1 = get_TypeId(from);
	auto t2 = get_TypeId(to);
	if (!cast_table[t1][t2].empty())
	{
		*os << cast_table[t1][t2];
	}
}

/**
 * @brief raxがさすアドレスの値をロードする。
 *
 * @param ty 読み込む値の型
 */
void CodeGen::load(const Type *ty)
{
	/* 変数の型が配列型の場合、レジスタへ値をロードをしようとしてはいけない。*/
	/* これは一般的に配列全体をレジスタに読み込むことはできないからである。*/
	/* そのため配列型の変数の評価は配列そのものではなく配列のアドレスとなる。*/
	/* これは配列型が暗黙に配列の最初の要素へのポインタへ返還されることを示している。 */
	switch (ty->_kind)
	{
	case TypeKind::TY_FLOAT:
		*os << "  movss xmm0, DWORD PTR [rax]\n";
		return;
	case TypeKind::TY_DOUBLE:
		*os << "  movsd xmm0, QWORD PTR [rax]\n";
		return;
	case TypeKind::TY_FUNC:
	case TypeKind::TY_ARRAY:
	case TypeKind::TY_STRUCT:
	case TypeKind::TY_UNION:
		return;
	default:
		break;
	}

	/* char型およびshort型の値をロードするとき、常にint型のサイズに拡張する。
	 * つまり、64ビットレジスタの下位32ビットは常に正しい値が入っているとみなせる。
	 * このとき上位32ビットレジスタにはごみが残っている可能性がある。
	 * int型よりサイズの大きな型の値をロードするときは単純にレジスタ全体が置き換えられる。
	 */
	string insn = ty->_is_unsigned ? "  movzx" : "  movsx";

	if (1 == ty->_size)
	{
		*os << insn << " eax, BYTE PTR [rax]\n";
	}
	else if (2 == ty->_size)
	{
		*os << insn << " eax, WORD PTR [rax]\n";
	}
	else if (4 == ty->_size)
	{
		*os << "  mov eax, [rax]\n";
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

	switch (ty->_kind)
	{
	case TypeKind::TY_FLOAT:
		*os << "  movss DWORD PTR [rdi], xmm0\n";
		return;

	case TypeKind::TY_DOUBLE:
		*os << "  movsd QWORD PTR [rdi], xmm0\n";
		return;

	/* 構造体、共用体の場合は1バイトずつr8b経由でストアする */
	case TypeKind::TY_STRUCT:
	case TypeKind::TY_UNION:
		for (int i = 0; i < ty->_size; ++i)
		{
			*os << "  mov r8b, [rax + " << i << " ]\n";
			*os << "  mov [rdi + " << i << " ], r8b\n";
		}
		return;

	default:
		break;
	}

	if (1 == ty->_size)
	{
		*os << "  mov [rdi], al\n";
	}
	else if (2 == ty->_size)
	{
		*os << "  mov [rdi], ax\n";
	}
	else if (4 == ty->_size)
	{
		*os << "  mov [rdi], eax\n";
	}
	else
	{
		*os << "  mov [rdi], rax\n";
	}
}

/**
 * @brief GPレジスタ上の引数の値をスタック領域にストアする
 *
 * @param r 何番目の引数か
 * @param offset ストアするスタック領域のオフセット
 * @param sz データサイズ
 */
void CodeGen::store_fp(const int &r, const int &offset, const int &sz)
{
	switch (sz)
	{
	case 4:
		*os << "  movss DWORD PTR [rbp - " << offset << "], xmm" << r << "\n";
		break;
	case 8:
		*os << "  movsd QWORD PTR [rbp - " << offset << "], xmm" << r << "\n";
		break;
	default:
		unreachable();
	}
}

/**
 * @brief GPレジスタ上の引数の値をスタック領域にストアする
 *
 * @param r 何番目の引数か
 * @param offset ストアするスタック領域のオフセット
 * @param sz データサイズ
 */
void CodeGen::store_gp(const int &r, const int &offset, const int &sz)
{
	switch (sz)
	{
	case 1:
		*os << "  mov [rbp - " << offset << "], " << arg_regs8[r] << "\n";
		break;
	case 2:
		*os << "  mov [rbp - " << offset << "], " << arg_regs16[r] << "\n";
		break;
	case 4:
		*os << "  mov [rbp - " << offset << "], " << arg_regs32[r] << "\n";
		break;
	case 8:
		*os << "  mov [rbp - " << offset << "], " << arg_regs64[r] << "\n";
		break;
	default:
		unreachable();
	}
}

/**
 * @brief raxの数値を0と比較するコードを出力
 *
 * @param ty 数値の型
 */
void CodeGen::cmp_zero(const Type *ty)
{
	switch (ty->_kind)
	{
	case TypeKind::TY_FLOAT:
		*os << "  xorps xmm1, xmm1\n";
		*os << "  ucomiss xmm0, xmm1\n";
		return;
	case TypeKind::TY_DOUBLE:
		*os << "  xorpd xmm1, xmm1\n";
		*os << "  ucomisd xmm0, xmm1\n";
		return;
	default:
		break;
	}

	if (ty->is_integer() && ty->_size <= 4)
	{
		*os << "  cmp eax, 0\n";
	}
	else
	{
		*os << "  cmp rax, 0\n";
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

/** @brief 'xmm0'の浮動小数点数をスタックにpushする */
void CodeGen::pushf()
{
	*os << "  sub rsp, 8\n";
	*os << "  movsd QWORD PTR [rsp], xmm0\n";
	++depth;
}

/**
 * @brief 引数を2回に分けてスタックにpushしていく。
 * 1回目ではスタック経由で渡す引数、２回目ではレジスタ経由で渡す引数をpushする
 *
 * @param args 引数ノード
 * @return スタック経由で渡す引数の数
 */
int CodeGen::push_args(Node *args)
{
	/* 引数の数(stack: スタック経由で渡す引数, gp:整数, fp:浮動小数点数) */
	int stack = 0, gp = 0, fp = 0;

	/* 引数の数を数える */
	for (auto arg = args; arg; arg = arg->_next.get())
	{
		if (arg->_ty->is_flonum())
		{
			if (fp++ >= FP_MAX)
			{
				arg->_pass_by_stack = true;
				++stack;
			}
		}
		else
		{
			if (gp++ >= GP_MAX)
			{
				arg->_pass_by_stack = true;
				++stack;
			}
		}
	}

	/* rspの値を16の倍数にする */
	if ((depth + stack) % 2 == 1)
	{
		*os << "  sub rsp, 8\n";
		depth++;
		stack++;
	}

	push_args2(args, true);
	push_args2(args, false);
	return stack;
}

/**
 * @brief 引数をスタックにpushする。２回に分けてpushするため、この関数は２回呼ばれる。
 *
 * @param args 引数ノード
 * @param first_pass １回目のpushであるか
 */
void CodeGen::push_args2(Node *args, const bool &first_pass)
{
	if (!args)
	{
		return;
	}

	push_args2(args->_next.get(), first_pass);

	/* 1回目ではスタック経由で渡す引数、２回目ではレジスタ経由で渡す引数をpushする */
	if ((first_pass && !args->_pass_by_stack) || (!first_pass && args->_pass_by_stack))
	{
		return;
	}

	generate_expression(args);

	if (args->_ty->is_flonum())
	{
		pushf();
	}
	else
	{
		push();
	}
}

/**
 * @brief スタックからpopした数値を指定したレジスタにセットする
 *
 * @param reg 数値をセットするレジスタ
 */
void CodeGen::pop(const string_view &reg)
{
	*os << "  pop " << reg << "\n";
	--depth;
}

/**
 * @brief スタックからpopした浮動小数点数値を指定したレジスタにセットする
 *
 * @param reg 数値をセットするレジスタの番号
 */
void CodeGen::popf(int reg)
{
	*os << "  movsd xmm" << reg << ", QWORD PTR [rsp]\n";
	*os << "  add rsp, 8\n";
	--depth;
}

/**
 * @brief 変数ノードの絶対アドレスを計算して'rax'にセットするコード
 *
 * @note ノードが変数ではないときエラーとする。
 * @param node 対象ノード
 */
void CodeGen::generate_address(Node *node)
{
	switch (node->_kind)
	{
	case NodeKind::ND_COMMA:
		generate_expression(node->_lhs.get());
		generate_address(node->_rhs.get());
		break;

	case NodeKind::ND_MEMBER:
		generate_address(node->_lhs.get());
		*os << "  add rax, " << node->_member->_offset << "\n";
		break;

	case NodeKind::ND_DEREF:
		generate_expression(node->_lhs.get());
		break;

	case NodeKind::ND_VAR:
		/* ローカル変数 */
		if (node->_var->_is_local)
		{
			*os << "  lea rax, [rbp - " << node->_var->_offset << "]\n";
			break;
		}
		/* 関数。RIP相対アドレッシングを使う */
		else if (TypeKind::TY_FUNC == node->_ty->_kind)
		{
			if (node->_var->_is_definition)
			{
				*os << "  lea rax, [rip + " << node->_var->_name << "]\n";
			}
			else
			{
				*os << "  mov rax, [rip + " << node->_var->_name << "@GOTPCREL]\n";
			}
		}
		/* グローバル変数。RIP相対アドレッシングを使う */
		else
		{
			*os << "  lea rax, [rip + " << node->_var->_name << "]\n";
		}
		break;

	default:
		error_token("左辺値ではありません", node->_token);
	}
}

/**
 * @brief 式をアセンブリに変換
 *
 * @param node 変換対象のAST
 */
void CodeGen::generate_expression(Node *node)
{
	if (print_dbg_info)
	{
		*os << "  .loc " << node->_token->_file->_file_no << " " << node->_token->_line_no << "\n";
	}

	switch (node->_kind)
	{
	/* NULL */
	case NodeKind::ND_NULL_EXPR:
		break;

	/* 単項演算子の'-' */
	case NodeKind::ND_NEG:
		/* '-'がかかる式を評価 */
		generate_expression(node->_lhs.get());

		switch (node->_ty->_kind)
		{
		case TypeKind::TY_FLOAT:
			/* 符号ビット(32ビット)を反転 */
			*os << "  mov rax, 1\n";
			*os << "  shl rax, 31\n";
			*os << "  movq xmm1, rax\n";
			*os << "  xorps xmm0, xmm1\n";
			break;
		case TypeKind::TY_DOUBLE:
			*os << "  mov rax, 1\n";
			*os << "  shl rax, 63\n";
			*os << "  movq xmm1, rax\n";
			*os << "  xorpd xmm0, xmm1\n";
			break;
		default:
			/* 符号を反転させる */
			*os << "  neg rax\n";
		}
		break;

	/* 代入 */
	case NodeKind::ND_ASSIGN:
		/* 左辺の代入先の変数のアドレスを計算 */
		generate_address(node->_lhs.get());
		/* 変数のアドレスをスタックにpush */
		push();
		/* 右辺を評価する。評価結果は'rax' */
		generate_expression(node->_rhs.get());
		/* raxの値をストアする */
		store(node->_ty.get());
		break;

	/* 3項演算子 */
	case NodeKind::ND_COND:
	{
		const int c = label_count();
		/* 条件 */
		generate_expression(node->_condition.get());
		cmp_zero(node->_condition->_ty.get());
		*os << "  je .L.else." << c << "\n";
		/* 条件が真のとき */
		generate_expression(node->_then.get());
		*os << "  jmp .L.end." << c << "\n";
		/* 条件が偽の時 */
		*os << ".L.else." << c << ":\n";
		generate_expression(node->_else.get());
		*os << ".L.end." << c << ":\n";
		break;
	}

	/* カンマ区切り */
	case NodeKind::ND_COMMA:
		generate_expression(node->_lhs.get());
		generate_expression(node->_rhs.get());
		break;

	/* 構造体のメンバ */
	case NodeKind::ND_MEMBER:
	{
		/* 変数のアドレスを計算 */
		generate_address(node);
		/* 変数のアドレスから値をロードする */
		load(node->_ty.get());
		break;
	}

	/* 参照 */
	case NodeKind::ND_ADDR:
		/* アドレスを計算 */
		generate_address(node->_lhs.get());
		break;

	/* 参照外し */
	case NodeKind::ND_DEREF:
		/* 参照先のアドレスを計算 */
		generate_expression(node->_lhs.get());
		/* 参照先のアドレスの値をロード */
		load(node->_ty.get());
		break;

	/* ! */
	case NodeKind::ND_NOT:
		generate_expression(node->_lhs.get());
		/* 0と比較 */
		cmp_zero(node->_lhs->_ty.get());
		/* 0と一致なら1, それ以外は0 */
		*os << "  sete al\n";
		*os << "  movzx rax, al\n";
		break;

	/* ~ */
	case NodeKind::ND_BITNOT:
		generate_expression(node->_lhs.get());
		*os << "  not rax\n";
		break;

	/* && */
	case NodeKind::ND_LOGAND:
	{
		const int c = label_count();
		generate_expression(node->_lhs.get());
		cmp_zero(node->_lhs->_ty.get());
		/* 短絡評価、前半がfalseなら後半は評価しない */
		*os << "  je .L.false." << c << "\n";
		generate_expression(node->_rhs.get());
		cmp_zero(node->_rhs->_ty.get());
		*os << "  je .L.false." << c << "\n";
		*os << "  mov rax, 1\n";
		*os << "  jmp .L.end." << c << "\n";
		*os << ".L.false." << c << ":\n";
		*os << "  mov rax, 0\n";
		*os << ".L.end." << c << ":\n";
		break;
	}

	/* || */
	case NodeKind::ND_LOGOR:
	{
		const int c = label_count();
		generate_expression(node->_lhs.get());
		cmp_zero(node->_lhs->_ty.get());
		/* 短絡評価、前半がtrueなら後半は評価しない */
		*os << "  jne .L.true." << c << "\n";
		generate_expression(node->_rhs.get());
		cmp_zero(node->_rhs->_ty.get());
		*os << "  jne .L.true." << c << "\n";
		*os << "  mov rax, 0\n";
		*os << "  jmp .L.end." << c << "\n";
		*os << ".L.true." << c << ":\n";
		*os << "  mov rax, 1\n";
		*os << ".L.end." << c << ":\n";
		break;
	}

	/* 関数呼び出し */
	case NodeKind::ND_FUNCALL:
	{
		/* スタックに入れる */
		int stack_args = push_args(node->_args.get());
		/* raxに関数のアドレスを入れる */
		generate_expression(node->_lhs.get());

		/* 引数の数(gp:整数, fp:浮動小数点数) */
		int gp = 0, fp = 0;

		/* 引数をレジスタにセットしていく */
		for (auto arg = node->_args.get(); arg; arg = arg->_next.get())
		{
			if (arg->_ty->is_flonum())
			{
				if (fp < FP_MAX)
				{
					popf(fp++);
				}
			}
			else
			{
				if (gp < GP_MAX)
				{
					pop(arg_regs64[gp++]);
				}
			}
		}

		/* 関数のアドレスをr10に格納 */
		*os << "  mov r10, rax\n";
		/* fpの数をraxに格納 */
		*os << "  mov rax, " << fp << "\n";
		/* 関数を呼び出す */
		*os << "  call r10\n";

		/* スタック経由で渡した引数を片付ける */
		*os << "  add rsp, " << stack_args * 8 << "\n";
		depth -= stack_args;

		/* x86-64において関数の戻り値がchar/bool, shortのときraxの上位48 または56ビット
		 * は不定となる。そのため関数の戻り値がchar/bool, shortのとき上位ビットをクリアする。
		 */
		switch (node->_ty->_kind)
		{
		case TypeKind::TY_BOOL:
			*os << "  movzx eax, al\n";
			break;
		case TypeKind::TY_CHAR:
			*os << (node->_ty->_is_unsigned ? "  movzx" : "  movsx") << " eax, al\n";
			break;
		case TypeKind::TY_SHORT:
			*os << (node->_ty->_is_unsigned ? "  movzx" : "  movsx") << " eax, ax\n";
			break;
		default:
			break;
		}
		break;
	}

	/* 数値 */
	case NodeKind::ND_NUM:
	{
		/* 浮動小数点を扱うための共用体 */
		union
		{
			float f32;
			double f64;
			uint32_t u32;
			uint64_t u64;
		} u;

		if (TypeKind::TY_FLOAT == node->_ty->_kind)
		{
			u.f32 = node->_fval;
			*os << "  mov eax, " << u.u32 << " #float " << node->_fval << "\n";
			*os << "  movq xmm0, rax\n";
			break;
		}
		else if (TypeKind::TY_DOUBLE == node->_ty->_kind)
		{
			u.f64 = node->_fval;
			*os << "  mov rax, " << u.u64 << " #double " << node->_fval << "\n";
			*os << "  movq xmm0, rax\n";
			break;
		}

		/* 数値を'rax'に格納 */
		*os << "  mov rax, " << node->_val << "\n";
		break;
	}

	/* 重複文 */
	case NodeKind::ND_STMT_EXPR:
	{
		for (auto cur = node->_body.get(); cur; cur = cur->_next.get())
		{
			generate_statement(cur);
		}
		break;
	}

	/* 構造体のメンバ */
	case NodeKind::ND_VAR:
	{
		/* メンバのアドレスを計算 */
		generate_address(node);
		/* メンバのアドレスから値をロードする */
		load(node->_ty.get());
		break;
	}

	case NodeKind::ND_CAST:
		generate_expression(node->_lhs.get());
		cast(node->_lhs->_ty.get(), node->_ty.get());
		break;

	case NodeKind::ND_MEMZERO:
		/* 変数が使うメモリサイズ */
		*os << "  mov rcx, " << node->_var->_ty->_size << "\n";
		/* 変数のアドレス */
		*os << "  lea rdi, [rbp - " << node->_var->_offset << "]\n";
		/* alに0をセット */
		*os << "  mov al, 0\n";
		/* 0クリア */
		*os << "  rep stosb\n";
		break;

	default:
		generate_expression2(node);
	}
}

/**
 * @brief 右辺と左辺を持つ式をアセンブリに変換
 *
 * @param node 変換対象のAST
 */
void CodeGen::generate_expression2(Node *node)
{
	/* 左辺が浮動小数点数の場合 */
	if (node->_lhs->_ty->is_flonum())
	{
		/* 右辺を計算 */
		generate_expression(node->_rhs.get());
		/* 計算結果をスタックにpush */
		pushf();
		/* 左辺を計算 */
		generate_expression(node->_lhs.get());
		/* 右辺の計算結果を'rdi'にpop */
		popf(1);

		string sz = (TypeKind::TY_FLOAT == node->_lhs->_ty->_kind) ? "ss" : "sd";

		switch (node->_kind)
		{
		case NodeKind::ND_ADD:
			/* 'xmm0' = 'xmm0' + 'xmm1' */
			*os << "  add" << sz << " xmm0, xmm1\n";
			break;

		case NodeKind::ND_SUB:
			/* 'xmm0' = 'xmm0' - 'xmm1' */
			*os << "  sub" << sz << " xmm0, xmm1\n";
			break;

		case NodeKind::ND_MUL:
			/* 'xmm0' = 'xmm0' * 'xmm1' */
			*os << "  mul" << sz << " xmm0, xmm1\n";
			break;

		case NodeKind::ND_DIV:
			/* 'xmm0' = 'xmm0' / 'xmm1' */
			*os << "  div" << sz << " xmm0, xmm1\n";
			break;

		case NodeKind::ND_EQ:
		case NodeKind::ND_NE:
		case NodeKind::ND_LT:
		case NodeKind::ND_LE:
			*os << "  ucomi" << sz << " xmm1, xmm0\n";
			if (NodeKind::ND_EQ == node->_kind)
			{
				*os << "  sete al\n";
				*os << "  setnp dl\n";
				*os << "  and al, dl\n";
			}
			else if (NodeKind::ND_NE == node->_kind)
			{
				*os << "  setne al\n";
				*os << "  setp dl\n";
				*os << "  or al, dl\n";
			}
			else if (NodeKind::ND_LT == node->_kind)
			{
				*os << "  seta al\n";
			}
			else if (NodeKind::ND_LE == node->_kind)
			{
				*os << "  setae al\n";
			}
			*os << "  and al, 1\n";
			*os << "  movzb rax, al\n";
			break;
		default:
			error_token("不正な式です", node->_token);
		}
		return;
	}

	/* 右辺を計算 */
	generate_expression(node->_rhs.get());
	/* 計算結果をスタックにpush */
	push();
	/* 左辺を計算 */
	generate_expression(node->_lhs.get());
	/* 右辺の計算結果を'rdi'にpop */
	pop("rdi");

	string ax, di, dx;

	/* long型およびポインタに対しては64ビットレジスタを使う */
	if (TypeKind::TY_LONG == node->_lhs->_ty->_kind || node->_lhs->_ty->_base)
	{
		ax = "rax";
		di = "rdi";
		dx = "rdx";
	}
	/* それ以外は32ビットレジスタを使う */
	else
	{
		ax = "eax";
		di = "edi";
		dx = "edx";
	}

	/* 演算子ノードなら演算命令を出力する */
	switch (node->_kind)
	{
	case NodeKind::ND_ADD:
		/* 'rax' = 'rax' + 'rdi' */
		*os << "  add " << ax << ", " << di << "\n";
		break;

	case NodeKind::ND_SUB:
		/* 'rax' = 'rax' - 'rdi' */
		*os << "  sub " << ax << ", " << di << "\n";
		break;

	case NodeKind::ND_MUL:
		/* 'rax' = 'rax' * 'rdi' */
		*os << "  imul " << ax << ", " << di << "\n";
		break;

	case NodeKind::ND_DIV:
	case NodeKind::ND_MOD:
		if (node->_ty->_is_unsigned)
		{
			*os << "  mov " << dx << ", 0\n";
			*os << "  div " << di << "\n";
		}
		else
		{

			if (8 == node->_lhs->_ty->_size)
			{
				/* 'rax'(64ビット)を128ビットに拡張して'rdx'と'rax'にセット */
				*os << "  cqo\n";
			}
			else
			{
				/* 'eax'(32ビット)を128ビットに拡張して'rdx'と'rax'にセット */
				*os << "  cdq\n";
			}
			/* 'rdx(edx)'と'rax(eax)'を合わせた128ビットの値を'rdi'で割って商を'rax(eax)', 余りを'rdx(edx)'にセット */
			*os << "  idiv " << di << "\n";
		}

		if (NodeKind::ND_MOD == node->_kind)
		{
			*os << "  mov rax, rdx\n";
		}
		break;

	case NodeKind::ND_BITAND:
		*os << "  and " << ax << ", " << di << "\n";
		break;

	case NodeKind::ND_BITOR:
		*os << "  or " << ax << ", " << di << "\n";
		break;

	case NodeKind::ND_BITXOR:
		*os << "  xor " << ax << ", " << di << "\n";
		break;

	case NodeKind::ND_SHL:
		/* 右辺の値をrdiからrcxに転送 */
		*os << "  mov rcx, rdi\n";
		*os << "  shl " << ax << ", cl\n";
		break;

	case NodeKind::ND_SHR:
		/* 右辺の値をrdiからrcxに転送 */
		*os << "  mov rcx, rdi\n";
		*os << (node->_lhs->_ty->_is_unsigned ? "  shr " : "  sar ") << ax << ", cl\n";
		break;

	case NodeKind::ND_EQ:
	case NodeKind::ND_NE:
	case NodeKind::ND_LT:
	case NodeKind::ND_LE:
		*os << "  cmp " << ax << ", " << di << "\n";

		if (NodeKind::ND_EQ == node->_kind)
		{
			*os << "  sete al\n";
		}
		else if (NodeKind::ND_NE == node->_kind)
		{
			*os << "  setne al\n";
		}
		else if (NodeKind::ND_LT == node->_kind)
		{
			*os << (node->_lhs->_ty->_is_unsigned ? "  setb al\n" : "  setl al\n");
		}
		else if (NodeKind::ND_LE == node->_kind)
		{
			*os << (node->_lhs->_ty->_is_unsigned ? "  setbe al\n" : "  setle al\n");
		}
		*os << "  movzb rax, al\n";
		break;

	default:
		/* エラー */
		error_token("不正な式です", node->_token);
	}
}

/**
 * @brief 文をアセンブリに変換
 *
 * @param node 変換対象のAST
 */
void CodeGen::generate_statement(Node *node)
{
	if (print_dbg_info)
	{
		*os << "  .loc " << node->_token->_file->_file_no << " " << node->_token->_line_no << "\n";
	}

	switch (node->_kind)
	{
	case NodeKind::ND_RETURN:
		/* return の後に式が存在する場合、戻り値を評価 */
		if (node->_lhs)
		{
			generate_expression(node->_lhs.get());
		}

		/* エピローグまでjmpする */
		*os << "  jmp .L.return." << current_func->_name << "\n";
		break;

	case NodeKind::ND_IF:
	{
		/* 通し番号を取得 */
		const int c = label_count();

		/* 条件を評価 */
		generate_expression(node->_condition.get());
		/* 条件を比較 */
		cmp_zero(node->_condition->_ty.get());
		/* 条件がfalseなら.L.else.cラベルに飛ぶ */
		*os << "  je .L.else." << c << "\n";
		/* trueのときに実行 */
		generate_statement(node->_then.get());
		/* elseは実行しない */
		*os << "  jmp .L.end." << c << "\n";

		/* falseのとき実行 */
		*os << ".L.else." << c << ":\n";
		if (node->_else)
		{
			generate_statement(node->_else.get());
		}
		*os << ".L.end." << c << ":\n";
		break;
	}

	/* for or while */
	case NodeKind::ND_FOR:
	{
		/* 通し番号を取得 */
		const int c = label_count();

		/* forの場合、初期化処理 */
		if (node->_init)
		{
			generate_statement(node->_init.get());
		}

		*os << ".L.begin." << c << ":\n";
		/* 終了条件判定 */
		if (node->_condition)
		{
			generate_expression(node->_condition.get());
			cmp_zero(node->_condition->_ty.get());
			*os << "  je " << node->_brk_label << "\n";
		}
		/* forの中身 */
		generate_statement(node->_then.get());

		/* continue */
		*os << node->_cont_label << ":\n";

		/* 加算処理 */
		if (node->_inc)
		{
			generate_expression(node->_inc.get());
		}
		*os << "  jmp .L.begin." << c << "\n";
		*os << node->_brk_label << ":\n";
		break;
	}

	case NodeKind::ND_DO:
	{
		int c = label_count();
		*os << ".L.begin." << c << ":\n";
		generate_statement(node->_then.get());
		*os << node->_cont_label << ":\n";
		generate_expression(node->_condition.get());
		cmp_zero(node->_condition->_ty.get());
		*os << "  jne .L.begin." << c << "\n";
		*os << node->_brk_label << ":\n";
		break;
	}

	case NodeKind::ND_SWITCH:
	{
		generate_expression(node->_condition.get());

		auto reg = (8 == node->_condition->_ty->_size) ? "rax" : "eax";

		/* 各ケースの条件の値と一致すればラベルにjump */
		for (auto n = node->case_next; n; n = n->case_next)
		{
			*os << "  cmp " << reg << ", " << n->_val << "\n";
			*os << "  je " << n->_label << "\n";
		}
		/* defaultがあればdefaultにjump */
		if (node->default_case)
		{
			*os << "  jmp " << node->default_case->_label << "\n";
		}
		/* 一致する数値がなければ抜ける */
		*os << "  jmp " << node->_brk_label << "\n";

		/* 各ケース */
		generate_statement(node->_then.get());
		*os << node->_brk_label << ":\n";
		break;
	}

	case NodeKind::ND_CASE:
		*os << node->_label << ":\n";
		generate_statement(node->_lhs.get());
		break;

	case NodeKind::ND_BLOCK:
	{
		/* Bodyに含まれるノードをすべて評価する */
		for (auto cur = node->_body.get(); cur; cur = cur->_next.get())
		{
			generate_statement(cur);
		}
		break;
	}

	case NodeKind::ND_GOTO:
		*os << "  jmp " << node->_unique_label << "\n";
		break;

	case NodeKind::ND_LABEL:
		*os << node->_unique_label << ":\n";
		generate_statement(node->_lhs.get());
		break;

	case NodeKind::ND_EXPR_STMT:
		generate_expression(node->_lhs.get());
		break;

	default:
		error_token("不正な構文です", node->_token);
	}
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
		/* 関数または宣言の場合は何もしない */
		if (var->_is_function || !var->_is_definition)
		{
			continue;
		}

		/* staticなグローバル変数であるか */
		if (var->_is_static)
		{
			*os << "  .local " << var->_name << "\n";
		}
		else
		{
			*os << "  .globl " << var->_name << "\n";
		}

		/* アライメントの指定 */
		*os << "  .align " << var->_align << "\n";

		if (var->_init_data)
		{
			/* 初期化式がある場合は.dataセクションに配置 */
			*os << "  .data\n";
			*os << var->_name << ":\n";

			auto rel = var->_rel.get();
			int pos = 0;

			while (pos < var->_ty->_size)
			{
				if (rel && rel->_offset == pos)
				{
					*os << "  .quad " << rel->_label << " + " << rel->_addend << "\n";
					rel = rel->_next.get();
					pos += 8;
				}
				else
				{
					*os << "  .byte " << static_cast<unsigned int>(var->_init_data[pos++]) << "\n";
				}
			}
			continue;
		}

		/* 初期化式がない場合は.bssセクションに配置 */
		*os << "  .bss\n";
		*os << var->_name << ":\n";
		*os << "  .zero " << var->_ty->_size << "\n";
	}
}

/**
 * @brief プログラムの.text部を出力する
 *
 * @param program 入力プログラム
 */
void CodeGen::emit_text(const unique_ptr<Object> &program)
{
	for (auto fn = program.get(); fn; fn = fn->_next.get())
	{
		/* 関数の宣言でなければ何もしない */
		if (!fn->_is_function || !fn->_is_definition)
		{
			continue;
		}

		/* 関数のラベル部分を出力 */
		if (fn->_is_static)
		{
			*os << "  .local " << fn->_name << "\n";
		}
		else
		{
			*os << "  .globl " << fn->_name << "\n";
		}

		*os << "  .text\n";
		*os << fn->_name << ":\n";

		/* 現在の関数をセット */
		current_func = fn;

		/* プロローグ */
		/* スタックサイズの領域を確保する */
		*os << "  push rbp\n";
		*os << "  mov rbp, rsp\n";
		*os << "  sub rsp, " << fn->_stack_size << "\n";

		/* 可変長引数関数 */
		if (fn->_va_area)
		{
			int gp = 0, fp = 0;
			/* 引数の数を数える */
			for (auto var = fn->_params.get(); var; var = var->_next.get())
			{
				if (var->_ty->is_flonum())
				{
					++fp;
				}
				else
				{
					++gp;
				}
			}
			int off = fn->_va_area->_offset;

			/* va_elem */
			/* 最初にva_argを読んだときに返されるのは"..."の直後の名前なし引数なので
			 * "..."の前にある引数のサイズを記録しておく。
			 */
			*os << "  mov DWORD PTR [rbp - " << off << "], " << gp * 8 << "\n";
			*os << "  mov DWORD PTR [rbp - " << off - 4 << "], " << fp * 8 + 48 << "\n";

			/* ６個より多い引数をストアしている領域のアドレス */
			*os << "  mov QWORD PTR [rbp - " << off - 8 << "], rbp\n";
			*os << "  add QWORD PTR [rbp - " << off - 8 << "], 16\n";

			/* レジスタの引数をストアしている領域のアドレス */
			*os << "  mov QWORD PTR [rbp - " << off - 16 << "], rbp\n";
			*os << "  sub QWORD PTR [rbp - " << off - 16 << "], " << off - 24 << "\n";

			/* レジスタの引数をストアする */
			*os << "  mov QWORD PTR [rbp - " << off - 24 << "], rdi\n";
			*os << "  mov QWORD PTR [rbp - " << off - 32 << "], rsi\n";
			*os << "  mov QWORD PTR [rbp - " << off - 40 << "], rdx\n";
			*os << "  mov QWORD PTR [rbp - " << off - 48 << "], rcx\n";
			*os << "  mov QWORD PTR [rbp - " << off - 56 << "], r8\n";
			*os << "  mov QWORD PTR [rbp - " << off - 64 << "], r9\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 72 << "], xmm0\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 80 << "], xmm1\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 88 << "], xmm2\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 96 << "], xmm3\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 104 << "], xmm4\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 112 << "], xmm5\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 120 << "], xmm6\n";
			*os << "  movsd QWORD PTR [rbp - " << off - 128 << "], xmm7\n";
		}

		/* レジスタから引数を受け取って確保してあるスタック領域にローカル変数と同様にストアする */
		int gp = 0, fp = 0;
		for (auto var = fn->_params.get(); var; var = var->_next.get())
		{
			/* スタック渡しの引数 */
			if(var->_offset < 0){
				continue;
			}
			/* 浮動小数点数 */
			if (var->_ty->is_flonum())
			{
				store_fp(fp++, var->_offset, var->_ty->_size);
			}
			/* 整数 */
			else
			{
				store_gp(gp++, var->_offset, var->_ty->_size);
			}
		}

		/* コードを出力 */
		generate_statement(fn->_body.get());

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
void CodeGen::generate_code(const unique_ptr<Object> &program, const string &input_path, const string &output_path, const bool &opt_g)
{

	os = open_file(output_path);
	print_dbg_info = opt_g;

	/* intel記法であることを宣言 */
	*os << ".intel_syntax noprefix\n";

	/* .fileディレクティブを出力 */
	if (print_dbg_info)
	{
		auto &input_files = Token::get_input_files();
		for (auto &file : input_files)
		{
			*os << ".file " << file->_file_no << " \"" << file->_name << "\"\n";
		}
	}

	/* スタックサイズを計算してセット */
	assign_lvar_offsets(program);

	/* .data部を出力 */
	emit_data(program);

	/* text部を出力 */
	emit_text(program);
}


/** @brief 関数に必要なスタックサイズを計算してstack_sizeにセットする。
 *
 * @param prog スタックサイズをセットする関数
 */
void CodeGen::assign_lvar_offsets(const unique_ptr<Object> &prog)
{
	for (auto fn = prog.get(); fn; fn = fn->_next.get())
	{
		/* 関数でなければスキップ */
		if (!fn->_is_function)
		{
			continue;
		}

		/* 関数が多数の引数を持つとき、一部の引数はレジスタではなくスタック経由で渡される。
		 * 最初にスタック経由で渡される引数はRBP + 16に配置される
		 */
		/* RBPより上側 */
		int top = 16;
		/* RBPより下側 */
		int bottom = 0;

		int gp = 0, fp = 0;

		/* スタック経由で渡される引数 */
		for (auto *var = fn->_params.get(); var; var = var->_next.get()){
			if(var->_ty->is_flonum()){
				if(fp++ < FP_MAX){
					continue;
				}
			}else{
				if(gp++ < GP_MAX){
					continue;
				}
			}

			top = Object::align_to(top, 8);
			var->_offset = -top;
			top += var->_ty->_size;
		}

		/* ローカル変数 */
		for (Object *var = fn->_locals.get(); var; var = var->_next.get())
		{
			bottom += var->_ty->_size;
			bottom = Object::align_to(bottom, var->_align);
			var->_offset = bottom;
		}

		/* 引数 */
		for (auto *var = fn->_params.get(); var; var = var->_next.get())
		{
			if(var->_offset < 0){
				continue;
			}
			bottom += var->_ty->_size;
			bottom = Object::align_to(bottom, var->_align);
			var->_offset = bottom;
		}

		/* スタックサイズが16の倍数になるようにアライメントする */
		fn->_stack_size = Object::align_to(move(bottom), 16);
	}
}

