#include "node.hpp"

namespace Parser{

    using _ptr_node = Node::_unique_ptr_node;

	std::unique_ptr<_ptr_node[]> code = std::make_unique<_ptr_node[]>(100);

    /** @brief コンストラクタ */
    constexpr Node::Node() = default;

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
     * @param val 数値 or 識別子
     */
    constexpr Node::Node(const NodeKind &kind, const int &val, const int &offset)
        : _kind(kind), _val(val), _offset(offset)
    {}

    /** @brief デストラクタ */
    Node::~Node() = default;

	/**
     * @brief
     * program = stmt*
     * @return _ptr_node
     */
	void Node::program()
	{
		int cnt = 0;
		while(!Token::at_eof()){
			code[cnt] = std::move(stmt());
			++cnt;
		}
		code[cnt] = nullptr;
	}

	/**
     * @brief
     * stmt = expr ";"
     * @return _ptr_node
     */
	_ptr_node Node::stmt()
	{
		_ptr_node node = expr();
		Token::expect(";");
		return std::move(node);
	}

	/**
     * @brief
     * expr = assign
     * @return _ptr_node
     */
    _ptr_node Node::expr()
    {
		return std::move(assign());
    }

	/**
     * @brief
     * assign = equality ("=" assign)?
     * @return _ptr_node
     */
	_ptr_node Node::assign()
	{
		_ptr_node node = std::move(equality());
		if(Token::consume("=")){
			node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), std::move(assign()));
		}
		return std::move(node);
	}

	/**
     * @brief
     * equality = relational ("==" relational | "!=" relational)*
     * @return _ptr_node 
     */
	_ptr_node Node::equality()
	{
		_ptr_node node = std::move(relational());
		for(;;){
			if(Token::consume("==")){
				node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), std::move(relational()));
			}else if(Token::consume("!=")){
				node = std::make_unique<Node>(NodeKind::ND_NE, std::move(node), std::move(relational()));
			}else{
				return std::move(node);
			}
		}
	}

    /**
     * @brief
     * relational = add ("<" add | "<=" add | ">" add | ">=" add)*
     * @return _ptr_node
     */
	_ptr_node Node::relational()
	{
		_ptr_node node = std::move(add());
		for(;;){
			if(Token::consume("<")){
				node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), std::move(add()));
			}else if(Token::consume("<=")){
				node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), std::move(add()));
			}else if(Token::consume(">")){
				/* lhs > rhs は rhs < lhs と読み替える */
				node = std::make_unique<Node>(NodeKind::ND_LT, std::move(add()), std::move(node));
			}else if(Token::consume(">=")){
				/* lhs >= rhs は rhs <= lhs と読み替える */
				node = std::make_unique<Node>(NodeKind::ND_LE, std::move(add()), std::move(node));
			}else{
				return std::move(node);
			}
		}
	}

    /**
     * @brief
     * add = mul ("+" mul | "-" mul)*
     * @return _ptr_node
     */
	_ptr_node Node::add()
	{
        _ptr_node node = std::move(mul());
        for(;;){
            if(Token::consume("+")){
                node = std::make_unique<Node>(NodeKind::ND_ADD, std::move(node), std::move(mul()));
            }else if(Token::consume("-")){
                node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(node), std::move(mul()));
            }else{
                return std::move(node);
            }
        }
	}

	/**
     * @brief
     * mulはunaryの積または除算で表される。 \n 
     * mul = unary ("*" unary | "/" unary)*
     * @return _ptr_node
     */
    _ptr_node Node::mul()
    {
        _ptr_node node = std::move(unary());
        for(;;){
            if(Token::consume("*")){
                node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), std::move(unary()));
            }else if(Token::consume("/")){
                node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::move(unary()));
            }else{
                return std::move(node);
            }
        }
    }

	/**
	 * @brief
	 * unaryは'+'または'-'がprimaryの前に0回か1回つけて表される。
	 * unary = ("+" | "-")? unary | primary
	 * @return _ptr_node 
	*/
	_ptr_node Node::unary()
	{
		if(Token::consume("+")){
			return std::move(unary());
		}
		if(Token::consume("-")){
			_ptr_node node = std::make_unique<Node>(NodeKind::ND_SUB, std::make_unique<Node>(NodeKind::ND_NUM, 0, 0), std::move(unary()));
			return std::move(node);
		}
		return std::move(primary());
	}

	/**
     * @brief
     * primaryは'数'または'(式)'で表される。 \n
     * primary = num | ident | "(" expr ")"
     * @return _ptr_node
     */
    _ptr_node Node::primary()
    {
        /* 次のトークンが"("なら、"(" expr ")"のはず */
        if(Token::consume("(")){
            _ptr_node node = std::move(expr());
            Token::expect(")");
            return std::move(node);
        }

		/* 次のトークンが識別子の場合 */
		Token::_unique_ptr_token tok = std::move(Token::consume_ident());
		if(tok){
			/* 1変数あたりオフセットとして8バイト設定 */
			int offset = (*(tok->get_str()) - 'a' + 1 ) * 8;
			return std::move(std::make_unique<Node>(NodeKind::ND_LVAR, 0, offset));
		}

        /* そうでなければ数値のはず */
        int num = Token::expect_number();
        return std::move(std::make_unique<Node>(NodeKind::ND_NUM, num, 0));
    }
    
    /**
     * @brief
     * アセンブリコードを生成
     * @param node 抽象構文木のNode
     */
    void Node::gen(_unique_ptr_node node)
    {
        /* 数値 or 左辺値 or 代入演算子 */
		switch(node->_kind)
		{
		case NodeKind::ND_NUM :
			std::cout << " push " << node->_val << "\n";
            return;
		case NodeKind::ND_LVAR :
			gen_lval(std::move(node));
			std::cout << " pop rax\n";
			std::cout << " mov rax, [rax]\n";
			std::cout << " push rax\n";
			return;
		case NodeKind::ND_ASSIGN :
			gen_lval(std::move(node->_lhs));
			gen(std::move(node->_rhs));

			std::cout << " pop rdi\n";
			std::cout << " pop rax\n";
			std::cout << " mov [rax], rdi\n";
			std::cout << " push rdi\n";
			return;
		default :
			break;
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
		case NodeKind::ND_EQ :
			std::cout << " cmp rax, rdi\n";
			std::cout << " sete al\n";
			std::cout << " movzb rax, al\n";
			break;
		case NodeKind::ND_NE :
			std::cout << " cmp rax, rdi\n";
			std::cout << " setne al\n";
			std::cout << " movzb rax, al\n";
			break;
		case NodeKind::ND_LT :
			std::cout << " cmp rax, rdi\n";
			std::cout << " setl al\n";
			std::cout << " movzb rax, al\n";
			break;
		case NodeKind::ND_LE :
			std::cout << " cmp rax, rdi\n";
			std::cout << " setle al\n";
			std::cout << " movzb rax, al\n";
			break;
        default:
            break;
        }

        /* 計算結果のPUSH命令を出力する */
        std::cout << " push rax\n";
    }


	void Node::gen_lval(_unique_ptr_node node)
	{
	}
}