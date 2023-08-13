/**
 * @file codegen.hpp
 * @author K.Fukunaga
 * @brief 抽象構文木(AST)を意味解析してアセンブリを出力するコードジェネレーターの定義。
 * @version 0.1
 * @date 2023-07-02
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#pragma once

#include "parse.hpp"
#include "common.hpp"

/** @brief　アセンブリを生成 */
class CodeGen
{
public:
	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	static void generate_code(const unique_ptr<Object> &program, const string &input_path, const string &output_path);

private:
	/* このクラスのインスタンス化は禁止 */
	CodeGen();

	/**************************/
	/* 静的メンバ関数 (private) */
	/**************************/

	static void push();
	static void pushf();
	static void push_args(Node *args);
	static void pop(const string_view &reg);
	static void popf(int reg);
	static void load(const Type *ty);
	static void store(const Type *ty);
	static void store_fp(const int &r, const int &offset, const int &sz);
	static void store_gp(const int &r, const int &offset, const int &sz);
	static void cmp_zero(const Type *ty);
	static void generate_address(Node *node);
	static void generate_expression(Node *node);
	static void generate_expression2(Node *node);
	static void generate_statement(Node *node);
	static void emit_data(const unique_ptr<Object> &program);
	static void emit_text(const unique_ptr<Object> &program);
	static int label_count();
	static void cast(Type *from, Type *to);
	static int get_TypeId(Type *ty);

	/** 64ビット整数レジスタ、前から順に関数の引数を格納される */
	static constexpr string_view arg_regs64[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

	/** 整数レジスタの下位32ビットのエイリアス、前から順に関数の引数を格納される */
	static constexpr string_view arg_regs32[6] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

	/** 整数レジスタの下位16ビットのエイリアス、前から順に関数の引数を格納される */
	static constexpr string_view arg_regs16[6] = {"di", "si", "dx", "cx", "r8w", "r9w"};

	/** 整数レジスタの下位8ビットのエイリアス、前から順に関数の引数を格納される */
	static constexpr string_view arg_regs8[6] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};

	/* 転送命令 */
	static constexpr string_view i32i8 = "  movsx eax, al\n";
	static constexpr string_view i32u8 = "  movzx eax, al\n";
	static constexpr string_view i32i16 = "  movsx eax, ax\n";
	static constexpr string_view i32u16 = "  movzx eax, ax\n";
	static constexpr string_view i32f32 = "  cvtsi2ss xmm0, eax\n";
	static constexpr string_view i32i64 = "  movsxd rax, eax\n";
	static constexpr string_view i32f64 = "  cvtsi2sd xmm0, eax\n";

	static constexpr string_view u32f32 = "  mov eax, eax; cvtsi2ss xmm0, rax\n";
	static constexpr string_view u32i64 = "  mov eax, eax\n";
	static constexpr string_view u32f64 = "  mov eax, eax; cvtsi2sd xmm0, rax\n";

	static constexpr string_view i64f32 = "  cvtsi2ss xmm0, rax\n";
	static constexpr string_view i64f64 = "  cvtsi2sd xmm0, rax\n";

	static constexpr string_view u64f32 = "  cvtsi2ss xmm0, rax\n";
	static constexpr string_view u64f64 =
		"  test rax, rax\n"		 /* rax同士でAND演算。最上位ビットが1ならSF=1 */
		"  js 1f\n"				 /* SF=1のとき参照後に定義された一番近い1ラベルにjump */
		"  pxor xmm0, xmm0\n"	 /* xmm0を0初期化 */
		"  cvtsi2sd xmm0, rax\n" /* raxをxmm0へ転送 */
		"  jmp 2f\n"
		"1:\n"
		"  mov rdi, rax\n"		 /* raxをrdiに転送 */
		"  and eax, 1\n"		 /* eaxと1でAND演算 */
		"  shr rdi\n"			 /* rdiを1ビット右シフト */
		"  or rdi, rax\n"		 /* rdiとrdxをOR演算 */
		"  pxor xmm0, xmm0\n"	 /* xmm0を0初期化 */
		"  cvtsi2sd xmm0, rdi\n" /* rdiの数値を浮動小数点数としてxmm0に転送 */
		"  addsd xmm0, xmm0\n"	 /* xmm0の値を２倍する */
		"2:\n";

	static constexpr string_view f32i8 = "  cvttss2si eax, xmm0; movsx eax, al\n";
	static constexpr string_view f32u8 = "  cvttss2si eax, xmm0; movzx eax, al\n";
	static constexpr string_view f32i16 = "  cvttss2si eax, xmm0; movsx eax, ax\n";
	static constexpr string_view f32u16 = "  cvttss2si eax, xmm0; movzx eax, ax\n";
	static constexpr string_view f32i32 = "  cvttss2si eax, xmm0\n";
	static constexpr string_view f32u32 = "  cvttss2si rax, xmm0\n";
	static constexpr string_view f32i64 = "  cvttss2si rax, xmm0\n";
	static constexpr string_view f32u64 = "  cvttss2si rax, xmm0\n";
	static constexpr string_view f32f64 = "  cvtss2sd xmm0, xmm0\n";

	static constexpr string_view f64i8 = "  cvttsd2si eax, xmm0; movsx eax, al\n";
	static constexpr string_view f64u8 = "  cvttsd2si eax, xmm0; movzx eax, al\n";
	static constexpr string_view f64i16 = "  cvttsd2si eax, xmm0; movsx eax, ax\n";
	static constexpr string_view f64u16 = "  cvttsd2si eax, xmm0; movzx eax, ax\n";
	static constexpr string_view f64i32 = "  cvttsd2si eax, xmm0\n";
	static constexpr string_view f64u32 = "  cvttsd2si rax, xmm0\n";
	static constexpr string_view f64f32 = "  cvtsd2ss xmm0, xmm0\n";
	static constexpr string_view f64i64 = "  cvttsd2si rax, xmm0\n";
	static constexpr string_view f64u64 = "  cvttsd2si rax, xmm0\n";

	static constexpr string_view none = "";

	static constexpr string_view cast_table[10][10] = {
		/* i8   i16   i32   i64     u8     u16     u32   u64   f32     f64 */
		{none, none, none, i32i64, i32u8, i32u16, none, i32i64, i32f32, i32f64},	  /* i8 */
		{i32i8, none, none, i32i64, i32u8, i32u16, none, i32i64, i32f32, i32f64},	  /* i16 */
		{i32i8, i32i16, none, i32i64, i32u8, i32u16, none, i32i64, i32f32, i32f64},	  /* i32 */
		{i32i8, i32i16, none, none, i32u8, i32u16, none, none, i64f32, i64f64},		  /* i64 */
		{i32i8, none, none, i32i64, none, none, none, i32i64, i32f32, i32f64},		  /* u8 */
		{i32i8, i32i16, none, i32i64, i32u8, none, none, i32i64, i32f32, i32f64},	  /* u16 */
		{i32i8, i32i16, none, u32i64, i32u8, i32u16, none, u32i64, u32f32, u32f64},	  /* u32 */
		{i32i8, i32i16, none, none, i32u8, i32u16, none, none, u64f32, u64f64},		  /* u64 */
		{f32i8, f32i16, f32i32, f32i64, f32u8, f32u16, f32u32, f32u64, none, f32f64}, /* f32 */
		{f64i8, f64i16, f64i32, f64i64, f64u8, f64u16, f64u32, f64u64, f64f32, none}, /* f64 */
	};

	/* サイズにより型を分類したID */
	enum class TypeID
	{
		I8,
		I16,
		I32,
		I64,
		U8,
		U16,
		U32,
		U64,
		F32,
		F64,
	};
};
