#include "codegen.hpp"

using std::cout;
using std::endl;
using std::string;

/* スタックの深さ */
static int depth = 0;

/**
 * @brief
 * スタックにpushするコードを生成
 */
void CodeGen::push()
{
	cout << " push rax\n";
	++depth;
}

/**
 * @brief
 * スタックからpopしてregに値を書き込むコードを生成
 */
void CodeGen::pop(const string &reg)
{
	cout << " pop " << reg << "\n";
	--depth;
}

/**
 * @brief
 * 変数ノードの絶対アドレスを計算する。
 * 計算結果は'rax'にストアする
 * ノードが変数ではないときエラーとする。
 */
void CodeGen::generate_address(const Node *node)
{
	/* 1変数当たり8バイト確保 */
	if (NodeKind::ND_VAR == node->_kind)
	{
		int offset = (node->_name[0] - 'a' + 1) * 8;
		cout << " mov rax, rbp\n";
		cout << " sub rax, " << offset << "\n";
		return;
	}
	error("左辺値ではありません");
}

/**
 * @brief
 * 式ノードのコードを出力する。
 * 計算結果はraxにストア
 */
void CodeGen::generate_expression(const Node *node)
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
		generate_expression(node->_lhs.get());
		/* 符号を反転させる */
		cout << " neg rax\n";
		return;
	/* 変数 */
	case NodeKind::ND_VAR:
		/* 変数のアドレスを計算 */
		generate_address(node);
		/* 変数のアドレスから値を'rax'に読み込む */
		cout << " mov rax, [rax]\n";
		return;
	/* 代入 */
	case NodeKind::ND_ASSIGN:
		/* 左辺の代入先の変数のアドレスを計算 */
		generate_address(node->_lhs.get());
		/* 変数のアドレスをスタックにpush */
		push();
		/* 右辺を評価する。評価結果は'rax' */
		generate_expression(node->_rhs.get());
		/* 変数のアドレスを'rdi にpop*/
		pop("rdi");
		/* 'rax'の値を'rdi'のアドレスのメモリに格納 */
		cout << " mov [rdi], rax\n";
		return;
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
	error("不正な式です");
}

/**
 * @brief
 * 文ノードのコードを出力する。
 * ノードが文ノードでないときエラーとする
 */
void CodeGen::generate_statement(const Node * node)
{
	if(NodeKind::ND_EXPR_STMT == node->_kind){
		generate_expression(node->_lhs.get());
		return;
	}
	error("不正な文です");
}
/**
 * @brief
 * プログラム全体のコードを出力する。
 */
void CodeGen::generate_code(std::unique_ptr<const Node> node)
{
		/* アセンブリの前半部分を出力 */
	cout << ".intel_syntax noprefix\n";
	cout << ".globl main\n";
	cout << "main:\n";

	/* プロローグ */
	/* 変数２６個分の領域を確保する */
	/* 1変数につき8バイト */
	cout << " push rbp\n";
	cout << " mov rbp, rsp\n";
	cout << " sub rsp, 208\n";

	for(const Node* p = node.get(); p; p = p->_next.get()){
		generate_statement(p);
		assert(0 == depth);
	}
	/* エピローグ */
	/* 最後の結果がRAXに残っているのでそれが返り値になる */
	cout << " mov rsp, rbp\n";
	cout << " pop rbp\n";
	cout << " ret\n";
}
