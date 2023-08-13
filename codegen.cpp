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

/** スタックの深さ */
static int depth = 0;

/** 64ビット整数レジスタ、前から順に関数の引数を格納される */
static const vector<string> arg_regs64 = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

/** 整数レジスタの下位32ビットのエイリアス、前から順に関数の引数を格納される */
static const vector<string> arg_regs32 = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

/** 整数レジスタの下位16ビットのエイリアス、前から順に関数の引数を格納される */
static const vector<string> arg_regs16 = {"di", "si", "dx", "cx", "r8w", "r9w"};

/** 整数レジスタの下位8ビットのエイリアス、前から順に関数の引数を格納される */
static const vector<string> arg_regs8 = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

/** 現在処理中の関数*/
static Object *current_func = nullptr;

/* 入出力 */
static std::ostream *os = &std::cout;

/* 符号拡張転送 */
constexpr std::string_view i32i8 = "  movsx eax, al\n";
constexpr std::string_view i32u8 = "  movzx eax, al\n";
constexpr std::string_view i32i16 = "  movsx eax, ax\n";
constexpr std::string_view i32u16 = "  movzx eax, ax\n";
constexpr std::string_view i32i64 = "  movsxd rax, eax\n";
constexpr std::string_view u32i64 = "  mov eax, eax\n";
constexpr std::string_view none = "";

static const std::string_view cast_table[8][8] = {
	// i8	 i16	i32  i64   u8	u16    u32   	u64
	{none, none, none, i32i64, i32u8, i32u16, none, i32i64},	// i8
	{i32i8, none, none, i32i64, i32u8, i32u16, none, i32i64},	// i16
	{i32i8, i32i16, none, i32i64, i32u8, i32u16, none, i32i64}, // i32
	{i32i8, i32i16, none, none, i32u8, i32u16, none, none},		// i64
	{i32i8, none, none, i32i64, none, none, none, i32i64},		// u8
	{i32i8, i32i16, none, i32i64, i32u8, none, none, i32i64},	// u16
	{i32i8, i32i16, none, u32i64, i32u8, i32u16, none, u32i64}, // u32
	{i32i8, i32i16, none, none, i32u8, i32u16, none, none},		// u64
};

/*****************/
/* CodeGen Class */
/*****************/

/**
 * @brief 型をサイズごとに分類したIDを返す
 *
 * @param ty 判定する型
 * @return 型のID
 */
TypeID CodeGen::get_TypeId(Type *ty)
{
	switch (ty->_kind)
	{
	case TypeKind::TY_CHAR:
		return ty->_is_unsigned ? TypeID::U8 : TypeID::I8;
	case TypeKind::TY_SHORT:
		return ty->_is_unsigned ? TypeID::U16 : TypeID::I16;
	case TypeKind::TY_INT:
		return ty->_is_unsigned ? TypeID::U32 : TypeID::I32;
	case TypeKind::TY_LONG:
		return ty->_is_unsigned ? TypeID::U64 : TypeID::I64;
	default:
		break;
	}
	return TypeID::U64;
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

	auto t1 = static_cast<int>(get_TypeId(from));
	auto t2 = static_cast<int>(get_TypeId(to));
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
	if (TypeKind::TY_ARRAY == ty->_kind ||
		TypeKind::TY_STRUCT == ty->_kind ||
		TypeKind::TY_UNION == ty->_kind)
	{
		return;
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

	/* 構造体、共用体の場合は1バイトずつr8b経由でストアする */
	if (ty->_kind == TypeKind::TY_STRUCT || ty->_kind == TypeKind::TY_UNION)
	{
		for (int i = 0; i < ty->_size; ++i)
		{
			*os << "  mov r8b, [rax + " << i << " ]\n";
			*os << "  mov [rdi + " << i << " ], r8b\n";
		}
		return;
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
void CodeGen::store_gp(const int &r, const int &offset, const int &sz)
{
	switch (sz)
	{
	case 1:
		*os << "  mov [rbp - " << offset << "], " << arg_regs8[r] << "\n";
		return;
	case 2:
		*os << "  mov [rbp - " << offset << "], " << arg_regs16[r] << "\n";
		return;
	case 4:
		*os << "  mov [rbp - " << offset << "], " << arg_regs32[r] << "\n";
		return;
	case 8:
		*os << "  mov [rbp - " << offset << "], " << arg_regs64[r] << "\n";
		return;
	default:
		break;
	}
	unreachable();
}

/**
 * @brief raxの数値を0と比較するコードを出力
 *
 * @param ty 数値の型
 */
void CodeGen::cmp_zero(const Type *ty)
{
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
void CodeGen::pop(const string &reg)
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
void CodeGen::generate_address(Node *node)
{
	switch (node->_kind)
	{
	case NodeKind::ND_VAR:
		/* ローカル変数 */
		if (node->_var->_is_local)
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
		generate_expression(node->_lhs.get());
		return;
	case NodeKind::ND_COMMA:
		generate_expression(node->_lhs.get());
		generate_address(node->_rhs.get());
		return;
	case NodeKind::ND_MEMBER:
		generate_address(node->_lhs.get());
		*os << "  add rax, " << node->_member->_offset << "\n";
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
void CodeGen::generate_expression(Node *node)
{
	*os << "  .loc 1 " << node->_token->_line_no << "\n";
	switch (node->_kind)
	{
	/* NULL */
	case NodeKind::ND_NULL_EXPR:
		return;

	/* 数値 */
	case NodeKind::ND_NUM:
	{	
		/* 浮動小数点を扱うための共用体 */
		union { float f32; double f64; uint32_t u32; uint64_t u64; } u;

		if(TypeKind::TY_FLOAT == node->_ty->_kind){
			u.f32 = node->_fval;
			*os << "  mov eax, " << u.u32 << " #float " << node->_fval << "\n";
			*os << "  movss xmm0, eax\n";
			return;
		}else if(TypeKind::TY_DOUBLE == node->_ty->_kind){
			u.f64 = node->_fval;
			*os << "  mov rax, " << u.u64 << " #double " << node->_fval << "\n";
			*os << "  movsd xmm0, rax\n";
			return;
		}

		/* 数値を'rax'に格納 */
		*os << "  mov rax, " << node->_val << "\n";
		return;
	}

	/* 単項演算子の'-' */
	case NodeKind::ND_NEG:
		/* '-'がかかる式を評価 */
		generate_expression(node->_lhs.get());
		/* 符号を反転させる */
		*os << "  neg rax\n";
		return;

	/* 変数, 構造体のメンバ */
	case NodeKind::ND_VAR:
	case NodeKind::ND_MEMBER:
	{
		/* 変数のアドレスを計算 */
		generate_address(node);
		/* 変数のアドレスから値をロードする */
		load(node->_ty.get());
		return;
	}

	/* 参照外し */
	case NodeKind::ND_DEREF:
		/* 参照先のアドレスを計算 */
		generate_expression(node->_lhs.get());
		/* 参照先のアドレスの値をロード */
		load(node->_ty.get());
		return;

	/* 参照 */
	case NodeKind::ND_ADDR:
		/* アドレスを計算 */
		generate_address(node->_lhs.get());
		return;

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
		return;

	/* 重複文 */
	case NodeKind::ND_STMT_EXPR:
	{
		for (auto cur = node->_body.get(); cur; cur = cur->_next.get())
		{
			generate_statement(cur);
		}
		return;
	}

	/* カンマ区切り */
	case NodeKind::ND_COMMA:
		generate_expression(node->_lhs.get());
		generate_expression(node->_rhs.get());
		return;

	case NodeKind::ND_MEMZERO:
		/* 変数が使うメモリサイズ */
		*os << "  mov rcx, " << node->_var->_ty->_size << "\n";
		/* 変数のアドレス */
		*os << "  lea rdi, [rbp - " << node->_var->_offset << "]\n";
		/* alに0をセット */
		*os << "  mov al, 0\n";
		/* 0クリア */
		*os << "  rep stosb\n";
		return;

	/* 3項演算子 */
	case NodeKind::ND_COND:
	{
		const int c = label_count();
		/* 条件 */
		generate_expression(node->_condition.get());
		*os << "  cmp rax, 0\n";
		*os << "  je .L.else." << c << "\n";
		/* 条件が真のとき */
		generate_expression(node->_then.get());
		*os << "  jmp .L.end." << c << "\n";
		/* 条件が偽の時 */
		*os << ".L.else." << c << ":\n";
		generate_expression(node->_else.get());
		*os << ".L.end." << c << ":\n";
		return;
	}

	/* ! */
	case NodeKind::ND_NOT:
		generate_expression(node->_lhs.get());
		/* 0と比較 */
		*os << "  cmp rax, 0\n";
		/* 0と一致なら1, それ以外は0 */
		*os << "  sete al\n";
		*os << "  movzx rax, al\n";
		return;

	/* ~ */
	case NodeKind::ND_BITNOT:
		generate_expression(node->_lhs.get());
		*os << "  not rax\n";
		return;

	/* && */
	case NodeKind::ND_LOGAND:
	{
		const int c = label_count();
		generate_expression(node->_lhs.get());
		*os << "  cmp rax, 0\n";
		/* 短絡評価、前半がfalseなら後半は評価しない */
		*os << "  je .L.false." << c << "\n";
		generate_expression(node->_rhs.get());
		*os << "  cmp rax, 0\n";
		*os << "  je .L.false." << c << "\n";
		*os << "  mov rax, 1\n";
		*os << "  jmp .L.end." << c << "\n";
		*os << ".L.false." << c << ":\n";
		*os << "  mov rax, 0\n";
		*os << ".L.end." << c << ":\n";
		return;
	}

	case NodeKind::ND_LOGOR:
	{
		const int c = label_count();
		generate_expression(node->_lhs.get());
		*os << "  cmp rax, 0\n";
		/* 短絡評価、前半がtrueなら後半は評価しない */
		*os << "  jne .L.true." << c << "\n";
		generate_expression(node->_rhs.get());
		*os << "  cmp rax, 0\n";
		*os << "  jne .L.true." << c << "\n";
		*os << "  mov rax, 0\n";
		*os << "  jmp .L.end." << c << "\n";
		*os << ".L.true." << c << ":\n";
		*os << "  mov rax, 1\n";
		*os << ".L.end." << c << ":\n";
		return;
	}

	case NodeKind::ND_CAST:
		generate_expression(node->_lhs.get());
		cast(node->_lhs->_ty.get(), node->_ty.get());
		return;

	/* 関数呼び出し */
	case NodeKind::ND_FUNCALL:
	{
		/* 引数の数 */
		int nargs = 0;

		for (auto arg = node->_args.get(); arg; arg = arg->_next.get())
		{
			/* 引数の値を評価 */
			generate_expression(arg);
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

		/* 関数を呼び出す時点でのスタックフレームが16の倍数になるように調整 */
		if (depth % 2 == 0)
		{
			*os << "  call " << node->_func_name << "\n";
		}
		else
		{
			*os << "  sub rsp, 8\n";
			*os << "  call " << node->_func_name << "\n";
			*os << "  add rsp, 8\n";
		}

		/* x86-64において関数の戻り値がchar/bool, shortのときraxの上位48 または56ビット
		 * は不定となる。そのため関数の戻り値がchar/bool, shortのとき上位ビットをクリアする。
		 */
		switch (node->_ty->_kind)
		{
		case TypeKind::TY_BOOL:
			*os << "  movzx eax, al\n";
			return;
		case TypeKind::TY_CHAR:
			*os << (node->_ty->_is_unsigned ? "  movzx" : "  movsx") << " eax, al\n";
			return;
		case TypeKind::TY_SHORT:
			*os << (node->_ty->_is_unsigned ? "  movzx" : "  movsx") << " eax, ax\n";
			return;
		default:
			break;
		}

		return;
	}

	default:
		break;
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
		return;

	case NodeKind::ND_SUB:
		/* 'rax' = 'rax' - 'rdi' */
		*os << "  sub " << ax << ", " << di << "\n";
		return;

	case NodeKind::ND_MUL:
		/* 'rax' = 'rax' * 'rdi' */
		*os << "  imul " << ax << ", " << di << "\n";
		return;

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
		return;

	case NodeKind::ND_BITAND:
		*os << "  and rax, rdi\n";
		return;

	case NodeKind::ND_BITOR:
		*os << "  or rax, rdi\n";
		return;

	case NodeKind::ND_BITXOR:
		*os << "  xor rax, rdi\n";
		return;

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
		return;

	case NodeKind::ND_SHL:
		/* 右辺の値をrdiからrcxに転送 */
		*os << "  mov rcx, rdi\n";
		*os << "  shl " << ax << ", cl\n";
		return;

	case NodeKind::ND_SHR:
		/* 右辺の値をrdiからrcxに転送 */
		*os << "  mov rcx, rdi\n";
		*os << (node->_lhs->_ty->_is_unsigned ? "  shr " : "  sar ") << ax << ", cl\n";
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
void CodeGen::generate_statement(Node *node)
{
	*os << "  .loc 1 " << node->_token->_line_no << "\n";

	switch (node->_kind)
	{
	case NodeKind::ND_GOTO:
		*os << "  jmp " << node->_unique_label << "\n";
		return;
	case NodeKind::ND_LABEL:
		*os << node->_unique_label << ":\n";
		generate_statement(node->_lhs.get());
		return;
	case NodeKind::ND_RETURN:
		/* return の後に式が存在する場合、戻り値を評価 */
		if (node->_lhs)
		{
			generate_expression(node->_lhs.get());
		}

		/* エピローグまでjmpする */
		*os << "  jmp .L.return." << current_func->_name << "\n";
		return;
	case NodeKind::ND_EXPR_STMT:
		generate_expression(node->_lhs.get());
		return;
	case NodeKind::ND_IF:
	{
		/* 通し番号を取得 */
		const int c = label_count();

		/* 条件を評価 */
		generate_expression(node->_condition.get());
		/* 条件を比較 */
		*os << "  cmp rax, 0\n";
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
			generate_statement(node->_init.get());
		}

		*os << ".L.begin." << c << ":\n";
		/* 終了条件判定 */
		if (node->_condition)
		{
			generate_expression(node->_condition.get());
			*os << "  cmp rax, 0\n";
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
		return;
	}

	case NodeKind::ND_DO:
	{
		int c = label_count();
		*os << ".L.begin." << c << ":\n";
		generate_statement(node->_then.get());
		*os << node->_cont_label << ":\n";
		generate_expression(node->_condition.get());
		*os << "  cmp rax, 0\n";
		*os << "  jne .L.begin." << c << "\n";
		*os << node->_brk_label << ":\n";
		return;
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
		return;
	}

	case NodeKind::ND_CASE:
		*os << node->_label << ":\n";
		generate_statement(node->_lhs.get());
		return;

	case NodeKind::ND_BLOCK:
	{
		/* Bodyに含まれるノードをすべて評価する */
		for (auto cur = node->_body.get(); cur; cur = cur->_next.get())
		{
			generate_statement(cur);
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
			int gp = 0;
			/* 引数の数を数える */
			for (auto var = fn->_params.get(); var; var = var->_next.get())
			{
				++gp;
			}
			int off = fn->_va_area->_offset;

			/* va_elem */
			/* 最初にva_argを読んだときに返されるのは"..."の直後の名前なし引数なので
			 * "..."の前にある引数のサイズを記録しておく。
			 */
			*os << "  mov DWORD PTR [rbp - " << off << "], " << gp * 8 << "\n";
			*os << "  mov DWORD PTR [rbp - " << off - 4 << "], 0\n";

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

		/* 関数の場合、レジスタから引数を受け取って確保してあるスタック領域にローカル変数と同様にストアする */
		int cnt = 0;
		for (auto var = fn->_params.get(); var; var = var->_next.get())
		{
			store_gp(cnt++, var->_offset, var->_ty->_size);
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
void CodeGen::generate_code(const unique_ptr<Object> &program, const string &input_path, const string &output_path)
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
