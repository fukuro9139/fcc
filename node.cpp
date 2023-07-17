#include "node.hpp"

namespace Parser{

    using _ptr_node = Node::_unique_ptr_node;

    /** @brief コンストラクタ */
    Node::Node() = default;

    /**
     * @brief コンストラクタ
     * @param kind ノードの種類
     * @param lhs 左辺
     * @param rhs 右辺
     */
    Node::Node(const NodeKind &kind, _unique_ptr_node lhs, _unique_ptr_node rhs)
        : _kind(kind), _lhs(std::move(lhs)), _rhs(std::move(rhs))
    {}

    /**
     * @brief コンストラクタ
     * @param val 数値
     */
    Node::Node(const int &val)
        : _kind(NodeKind::ND_NUM), _val(val)
    {}

    /** @brief デストラクタ */
    Node::~Node() = default;

    /**
     * @brief 
     * exprはmulの和または差で表される。
     * expr = mul ("+" mul | "-" mul)* 
     * @return _ptr_node 
     */
    _ptr_node Node::expr()
    {
        _ptr_node node = std::move(mul());
        for(;;){
            if(Token::consume('+')){
                node = std::make_unique<Node>(NodeKind::ND_ADD, std::move(node), std::move(mul()));
            }else if(Token::consume('-')){
                node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(node), std::move(mul()));
            }else{
                return std::move(node);
            }
        }
    }

    /**
     * @brief 
     * mulはprimaryの積または除算で表される。 \n 
     * mul = primary ("*" primary | "/" primary)*
     * @return _ptr_node 
     */
    _ptr_node Node::mul()
    {
        _ptr_node node = std::move(primary());
        for(;;){
            if(Token::consume('*')){
                node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), std::move(primary()));
            }else if(Token::consume('/')){
                node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::move(primary()));
            }else{
                return std::move(node);
            }
        }
    }
    
    /**
     * @brief 
     * 
     * @return _ptr_node 
     */
    _ptr_node Node::primary()
    {
        /* 次のトークンが"("なら、"(" expr ")"のはず */
        if(Token::consume('(')){
            _ptr_node node = std::move(expr());
            Token::expect(')');
            return std::move(node);
        }

        /* そうでなければ数値のはず */
        int num = Token::expect_number();
        return std::move(std::make_unique<Node>(num));
    }
    
    /**
     * @brief 
     * Nodeを計算する
     * @param node 抽象構文木のNode
     */
    void Node::gen(_unique_ptr_node node)
    {
        /* 数値ならPUSH命令を出力する */
        if(NodeKind::ND_NUM == node->_kind){
            std::cout << " push " << node->_val << "\n";
            return;
        }

        /* 左辺を計算 */
        gen(std::move(node->_lhs));
        /* 右辺を計算 */
        gen(std::move(node->_rhs));

        /* 計算対象をPOP */
        std::cout << " pop rdi\n";
        std::cout << " pop rax\n";

        /* 演算子ノードなら演算命令を出力する */
        switch (node->_kind)
        {
        case NodeKind::ND_ADD :
            std::cout << " add rax, rdi\n";
            break;
        case NodeKind::ND_SUB :
            std::cout << " sub rax, rdi\n";
            break;
        case NodeKind::ND_MUL :
            std::cout << " imul rax, rdi\n";
            break;
        case NodeKind::ND_DIV :
            std::cout << " cqo\n";
            std::cout << " idiv rdi\n";
            break;
        default:
            break;
        }

        /* 計算結果のPUSH命令を出力する */
        std::cout << " push rax\n";
    }
}