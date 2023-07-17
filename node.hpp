#pragma once

#include "token.hpp"

namespace Parser{
    /** @brief ノードの種類 */
    enum class NodeKind{
        ND_ADD, /** @brief + */      
        ND_SUB, /** @brief - */
        ND_MUL, /** @brief * */
        ND_DIV, /** @brief / */
        ND_NUM, /** @brief 整数 */
    };

    /**
     * @brief 
     * 以下のEBNFに従って抽象構文木を構成する。\n 
     *   expr    = mul ("+" mul | "-" mul)* \n 
     *   mul     = primary ("*" primary | "/" primary)* \n 
     *   primary = num | "(" expr ")" \n 
     */
    struct Node
    {
        using _unique_ptr_node = std::unique_ptr<Node>;
    public:
        Node();
        Node(const NodeKind &kind, _unique_ptr_node lhs, _unique_ptr_node rhs);
        Node(const int &val);
        ~Node();

        static _unique_ptr_node expr();
        static _unique_ptr_node mul();
        static _unique_ptr_node primary();
        static void gen(_unique_ptr_node node);

    private:
        /** @brief ノードの種類*/
        NodeKind _kind;
        /** @brief 左辺*/
        _unique_ptr_node _lhs = nullptr;
        /** @brief 右辺 */
        _unique_ptr_node _rhs = nullptr;
        /** @brief kindがND_NUMの場合のみ使う */
        int _val = 0;

    };

}