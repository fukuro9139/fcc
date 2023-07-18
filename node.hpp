#pragma once

#include "token.hpp"
#include <vector>

namespace Parser{
    /** @brief ノードの種類 */
    enum class NodeKind{
        ND_ADD, 	/** @brief + */      
        ND_SUB, 	/** @brief - */
        ND_MUL, 	/** @brief * */
        ND_DIV, 	/** @brief / */
		ND_ASSIGN, 	/** @brief = */
		ND_EQ,		/** @brief == */
		ND_NE,		/** @brief != */
		ND_LT,		/** @brief < */
		ND_LE,		/** @brief <= */
        ND_NUM, 	/** @brief 整数 */

		ND_LVAR, 	/** @brief 左辺値*/
		ND_NONE,
    };

    /**
     * @brief
     * 以下のEBNFに従って抽象構文木を構成する。 \n
	 *	program    = stmt*
	 *	stmt       = expr ";"
	 *	expr       = assign
	 *	assign     = equality ("=" assign)?
	 *	equality   = relational ("==" relational | "!=" relational)*
	 *	relational = add ("<" add | "<=" add | ">" add | ">=" add)*
	 *	add        = mul ("+" mul | "-" mul)*
	 *	mul        = unary ("*" unary | "/" unary)*
	 *	unary      = ("+" | "-")? primary
	 *	primary    = num | ident | "(" expr ")"
     */
    struct Node
    {
        using _unique_ptr_node = std::unique_ptr<Node>;
    public:
        constexpr Node();
        Node(const NodeKind &kind, _unique_ptr_node lhs, _unique_ptr_node rhs);
        constexpr Node(const NodeKind &kind, const int &val, const int &offset);
        ~Node();

		static void program();
		static _unique_ptr_node stmt();
        static _unique_ptr_node expr();
		static _unique_ptr_node assign();
		static _unique_ptr_node equality();
        static _unique_ptr_node relational();
		static _unique_ptr_node add();
		static _unique_ptr_node mul();
		static _unique_ptr_node unary();
        static _unique_ptr_node primary();
        static void gen(_unique_ptr_node node);
		static void gen_lval(_unique_ptr_node node);

		/** @brief パース結果 */
		static std::unique_ptr<_unique_ptr_node[]> code;

    private:
        /** @brief ノードの種類*/
        NodeKind _kind = NodeKind::ND_NONE;
        /** @brief 左辺*/
        _unique_ptr_node _lhs = nullptr;
        /** @brief 右辺 */
        _unique_ptr_node _rhs = nullptr;
        /** @brief kindがND_NUMの場合のみ使う、数値の値 */
        int _val = 0;
		/** @brief kindがND_LVARの場合のみ使う、ベースポインタからのオフセット*/
		int _offset = 0;

    };

}