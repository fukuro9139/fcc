/**
 * @file parse.cpp
 * @author K.Fukunaga
 * @brief
 * C言語の再帰下降構文解析を行うパーサーの定義
 *
 * このファイルのほとんどの関数の名前は、入力トークン・リストから読み取る対象に対応している。
 * 例えば、statement() ははトークン・リストからステートメントを読み取ってステートメントに対応する
 * 抽象構文木（abstract syntax tree、AST）ノードを返す。
 * 各関数はASTノードと入力トークンの残りの部分を返すように設計してある。
 * C言語は複数の戻り値をサポートしていないので、残りのトークンはポインタ引数を通じて呼び出し元に返される。
 *
 * @version 0.1
 * @date 2023-07-22
 *
 * @copyright Copyright (c) 2023 MIT License
 *
 */

#include "parse.hpp"

using std::shared_ptr;
using std::unique_ptr;

/**************/
/* Node Class */
/**************/

/* コンストラクタ */

Node::Node() = default;

Node::Node(NodeKind &&kind, Token *token) : _kind(std::move(kind)), _token(token) {}

Node::Node(NodeKind &&kind, unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _rhs(std::move(rhs)), _token(token) {}

Node::Node(NodeKind &&kind, unique_ptr<Node> &&lhs, Token *token)
	: _kind(std::move(kind)), _lhs(std::move(lhs)), _token(token) {}

Node::Node(const int64_t &val, Token *token) : _kind(NodeKind::ND_NUM), _val(val), _token(token) {}

Node::Node(const Object *var, Token *token) : _kind(NodeKind::ND_VAR), _var(var), _token(token) {}

/**
 * @brief プログラム を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @see expression compound-statement expression-statement
 *
 * @details 下記のEBNF規則に従う。 @n
 * statement = "return" expression ";" @n
 * 			 | "if" "(" expression ")" statement ("else" statement)? @n
 * 			 | "for" "(" expression-statement expression? ";" expression? ")" statement @n
 * 			 | "while" "(" expression ")" statement @n
 * 			 | "{" compound-statement @n
 * 			 | expression-statement
 */
unique_ptr<Node> Node::statement(Token **next_token, Token *current_token)
{
	/* return */
	if (current_token->is_equal("return"))
	{
		/* "return"の次はexpresionがくる */
		auto node = std::make_unique<Node>(NodeKind::ND_RETURN, current_token);
		node->_lhs = expression(&current_token, current_token->_next.get());

		/* 最後は';'で終わるはず */
		*next_token = Token::skip(current_token, ";");
		return node;
	}

	/* if */
	if (current_token->is_equal("if"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_IF, current_token);

		/* ifの次は'('がくる */
		current_token = Token::skip(current_token->_next.get(), "(");

		/* 条件文 */
		node->_condition = expression(&current_token, current_token);

		/* 条件文のは')'がくる */
		current_token = Token::skip(current_token, ")");
		node->_then = statement(&current_token, current_token);

		/* else節が存在する */
		if (current_token->is_equal("else"))
		{
			node->_else = statement(&current_token, current_token->_next.get());
		}
		*next_token = current_token;
		return node;
	}

	/* for */
	if (current_token->is_equal("for"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_FOR, current_token);

		/* forの次は'('がくる */
		current_token = Token::skip(current_token->_next.get(), "(");

		/* 初期化処理 */
		node->_init = expression_statement(&current_token, current_token);

		/* 次のトークンが';'でなければ条件が存在する */
		if (!current_token->is_equal(";"))
		{
			node->_condition = expression(&current_token, current_token);
		}
		current_token = Token::skip(current_token, ";");

		/* 次のトークンが')'でなければ加算処理が存在する */
		if (!current_token->is_equal(")"))
		{
			node->_inc = expression(&current_token, current_token);
		}
		current_token = Token::skip(current_token, ")");
		/* forの中の処理 */
		node->_then = statement(next_token, current_token);
		return node;
	}

	/* while */
	if (current_token->is_equal("while"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_FOR, current_token);

		/* whileの次は'('がくる */
		current_token = Token::skip(current_token->_next.get(), "(");
		node->_condition = expression(&current_token, current_token);

		/* 条件文のは')'がくる */
		current_token = Token::skip(current_token, ")");
		node->_then = statement(next_token, current_token);
		return node;
	}

	/* ブロック */
	if (current_token->is_equal("{"))
	{
		return compound_statement(next_token, current_token->_next.get());
	}

	/* 式 */
	return expression_statement(next_token, current_token);
}

/**
 * @brief ブロックを読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n compound-statement = (typedef | declaration | statement)* "}"
 */
unique_ptr<Node> Node::compound_statement(Token **next_token, Token *current_token)
{

	auto node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token);

	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* ブロックスコープに入る */
	Object::enter_scope();

	/* '}'が出てくるまでstatementをパースする */
	while (!current_token->is_equal("}"))
	{
		/* 変数宣言、定義 */
		if (current_token->is_typename())
		{
			/* 型指定子を読み取る */
			VarAttr attr = {};
			auto base = declspec(&current_token, current_token, &attr);

			/* typedefの場合 */
			if (attr.is_typedef)
			{
				current_token = parse_typedef(current_token, base);
				continue;
			}
			/* それ以外の場合は変数の定義または宣言 */
			cur->_next = declaration(&current_token, current_token, base);
		}
		/* 宣言、定義以外の文 */
		else
		{
			cur->_next = statement(&current_token, current_token);
		}

		cur = cur->_next.get();
		Type::add_type(cur);
	}

	/* ブロックスコープから抜け出す */
	Object::leave_scope();

	/* ダミーのheadからの次のトークン以降を切り離し、新しいノードのbodyに繋ぐ*/
	node->_body = std::move(head->_next);
	/* '}' の次のトークンからパースを続ける*/
	*next_token = current_token->_next.get();
	return node;
}

/**
 * @brief 関数宣言、定義を読み取る。
 *
 * @param token 現在処理しているトークン
 * @param base 関数の戻り値の型のベース
 * @return 次のトークン
 * @details 下記のEBNF規則に従う。 @n function-definition = declarator ( "{" compound-statement | ";" )
 */
Token *Node::function_definition(Token *token, shared_ptr<Type> &&base)
{
	/* 型を判定 */
	auto ty = declarator(&token, token, base);

	auto parameters = ty->_params;
	/* 新しい関数を生成してObject::globalsの先頭に追加する。 */
	auto fn = std::make_unique<Object>(std::move(ty->_token->_str), std::move(Object::globals), std::move(ty));
	/* グローバル変数としてscopeに追加 */
	Object::push_scope(fn->_name);

	/* 関数であるフラグをセット */
	fn->is_function = true;

	/* 定義か宣言か、後ろに";"がくるなら宣言 */
	fn->is_definition = !Token::consume(&token, token, ";");

	/* 宣言であるなら現在のトークンを返して抜ける */
	if (!fn->is_definition)
	{
		return token;
	}

	/* 関数のブロックスコープに入る */
	Object::enter_scope();

	/* 引数をローカル変数として作成 */
	Object::create_params_lvars(std::move(parameters));
	fn->_params = std::move(Object::locals);

	/* 引数の次は"{"がくる */
	token = Token::skip(token, "{");

	/* 後で中身を入れられるようにポインタをメモ */
	auto current_function = fn.get();

	/* 作成した関数オブジェクトをObject::globalにセット */
	Object::globals = std::move(fn);

	/* 関数の中身を読み取る */
	current_function->_body = compound_statement(&token, token);
	/* ローカル変数をセット */
	current_function->_locals = std::move(Object::locals);

	/* 関数のブロックスコープを抜ける */
	Object::leave_scope();

	return token;
}

/**
 * @brief 変数宣言を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
 */
unique_ptr<Node> Node::declaration(Token **next_token, Token *current_token, shared_ptr<Type> base)
{
	/* ノードリストの先頭としてダミーのノードを作成 */
	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* 初回かどうか */
	bool first = true;

	/* ";"が出てくるまで読み取りを続ける */
	while (!current_token->is_equal(";"))
	{
		/* 2個目以降の宣言には",""区切りが必要 */
		if (!first)
		{
			current_token = Token::skip(current_token, ",");
		}
		/* 初回フラグを下げる */
		first = false;

		/* 変数の最終的な型を決定 */
		auto ty = declarator(&current_token, current_token, base);

		/* 変数がvoid型で宣言されていたらエラー */
		if (TypeKind::TY_VOID == ty->_kind)
		{
			error_token("変数がvoid型で宣言されています", current_token);
		}

		const auto var = Object::new_lvar(ty->_token->_str, std::move(ty));

		/* 宣言の後に初期化式がない場合は次のループへ */
		if (!current_token->is_equal("="))
		{
			continue;
		};

		/* 変数を表すノードを生成 */
		auto lhs = std::make_unique<Node>(var, var->_ty->_token);
		/* 変数の初期化値を表すノードを生成 */
		auto rhs = assign(&current_token, current_token->_next.get());
		/* 初期化を代入式として表すノードを生成 */
		auto node = std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(lhs), std::move(rhs), current_token);
		/* ノードリストの末尾に単文ノードとして追加 */
		cur->_next = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, std::move(node), current_token);
		/* ノードリストの末尾を更新 */
		cur = cur->_next.get();
	}
	auto node = std::make_unique<Node>(NodeKind::ND_BLOCK, current_token);
	/* ヘッダの次のノード以降を切り離してnodeのbodyに繋ぐ */
	node->_body = std::move(head->_next);
	*next_token = current_token->_next.get();
	return node;
}

/**
 * @brief 関数の引数を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 宣言されている型
 * @return 引数の情報を含む関数の型
 * @details 以下のEBNF規則に従う。 @n
 * function-parameters = (parameters ("," parameters)*)? ")" @n
 * parameters = declspec declarator
 */
shared_ptr<Type> Node::function_parameters(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	auto head = std::make_unique_for_overwrite<Type>();
	auto cur = head.get();

	/* ")"が出てくるまで読み取りを続ける */
	while (!current_token->is_equal(")"))
	{
		if (head.get() != cur)
		{
			/* 2個目以降の引数では","区切りが必要 */
			current_token = Token::skip(current_token, ",");
		}
		auto base = declspec(&current_token, current_token, nullptr);
		cur->_next = std::make_shared<Type>(*(declarator(&current_token, current_token, base)));
		cur = cur->_next.get();
	}
	ty = Type::func_type(ty);
	ty->_params = std::move(head->_next);
	*next_token = current_token->_next.get();
	return ty;
}

/**
 * @brief 宣言を変数 or 関数か判断し結果の型を返す
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 宣言されている型
 * @return 変数 or 関数の型
 * @details
 * 次のEBNF規則に従う。 @n
 * type-suffix = "(" function-parameters | "[" number"]" type-suffix | ε @n
 * function-parameters = parameter ("," parameter)* @n
 * parameter = declspec declarator
 */
shared_ptr<Type> Node::type_suffix(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	/* 識別子名の後に"("があれば関数 */
	if (current_token->is_equal("("))
	{
		return function_parameters(next_token, current_token->_next.get(), std::move(ty));
	}

	/* 識別子名の後に"["があれば配列 */
	if (current_token->is_equal("["))
	{
		int sz = current_token->_next->get_number();
		current_token = Token::skip(current_token->_next->_next.get(), "]");
		ty = type_suffix(next_token, current_token, std::move(ty));
		return Type::array_of(ty, sz);
	}

	/* そうでなければ普通の変数 */
	*next_token = current_token;
	return ty;
}

/**
 * @brief 変数宣言を変数名部分を読み取る
 *
 * @details
 * 変数の型は変数名に辿りつくまで確定できない。 @n
 * 例：int a, *b, **c; aはint型、bはint型へのポインタ、cはint型へのポインタへのポインタ @n
 * 下記のEBNF規則に従う。 @n declarator = "*"* ("(" identifier ")" | "(" declarator ")" | identifier) type-suffix
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 変数の型の基準
 * @return 変数の型
 */
shared_ptr<Type> Node::declarator(Token **next_token, Token *current_token, shared_ptr<Type> ty)
{
	/* "*"の数だけ直前の型へのポインタになる */
	while (Token::consume(&current_token, current_token, "*"))
	{
		ty = Type::pointer_to(ty);
	}

	/* ネストした型の場合、外側を先に評価する */
	if (current_token->is_equal("("))
	{
		auto start = current_token;
		auto dummy = std::make_shared<Type>();
		/* ネスト部分を飛ばす */
		declarator(&current_token, start->_next.get(), dummy);
		current_token = Token::skip(current_token, ")");
		/* ネストの外側の型を決める */
		ty = type_suffix(next_token, current_token, std::move(ty));
		/* 外側の型をベースの型としてネスト部分の型を決定する */
		return declarator(&current_token, start->_next.get(), ty);
	}

	/* トークンの種類が識別子でないときエラー */
	if (TokenKind::TK_IDENT != current_token->_kind)
	{
		error_token("識別子ではありません", current_token);
	}

	/* 関数か変数か */
	ty = type_suffix(next_token, current_token->_next.get(), std::move(ty));

	/* 参照トークンを設定 */
	ty->_token = current_token;

	return ty;
}

/**
 * @brief 構造体,共用体の宣言、定義を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 構造体、共用体の型
 * @details 下記のEBNF規則に従う。 @n struct-union-decl = identifier? ( "{" struct-members )?
 */
shared_ptr<Type> Node::struct_union_decl(Token **next_token, Token *current_token)
{
	/* 存在するならば構造体,共用体のタグを読む */
	Token *tag = nullptr;
	if (current_token->_kind == TokenKind::TK_IDENT)
	{
		tag = current_token;
		current_token = current_token->_next.get();
	}

	/* 構造体,共用体のタグが存在し、かつ定義ではない場合 */
	/* 例 struct foo hoge; */
	if (tag && !current_token->is_equal("{"))
	{
		auto ty = Object::find_tag(tag);
		/* タグが見つからなかったらエラー */
		if (!ty)
		{
			error_token("未定義の構造体です", tag);
		}
		*next_token = current_token;
		return ty;
	}

	/* 構造体の情報を読み込む */
	auto ty = std::make_shared<Type>(TypeKind::TY_STRUCT);
	struct_members(next_token, current_token->_next.get(), ty.get());
	ty->_align = 1;

	/* タグが存在するならば現在のスコープに登録する */
	if (tag)
	{
		Object::push_tag_scope(tag, ty);
	}

	return ty;
}

/**
 * @brief 構造体の宣言、定義を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 構造体の型
 * @details 下記のEBNF規則に従う。 @n struct-decl = struct-union-decl
 */
shared_ptr<Type> Node::struct_decl(Token **next_token, Token *current_token)
{
	/* 構造体の情報を読み込む */
	auto ty = struct_union_decl(next_token, current_token);
	ty->_kind = TypeKind::TY_STRUCT;

	/* 構造体のメンバのオフセットを計算する */
	int offset = 0;
	for (auto mem = ty->_members.get(); mem; mem = mem->_next.get())
	{
		mem->_offset = offset;
		offset += mem->_ty->_size;
		/* アライメントの基数はメンバの基数のうち最大値に合わせる */
		if (ty->_align < mem->_ty->_align)
		{
			ty->_align = mem->_ty->_align;
		}
	}
	ty->_size = Object::align_to(offset, ty->_align);
	return ty;
}

/**
 * @brief 共用体の宣言、定義を読み込む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 共用体の型
 * @details 下記のEBNF規則に従う。 @n union-decl = struct-union-decl
 */
shared_ptr<Type> Node::union_decl(Token **next_token, Token *current_token)
{
	auto ty = struct_union_decl(next_token, current_token);
	ty->_kind = TypeKind::TY_UNION;

	/* 共用体の場合、全てのoffsetは0で共通である。 */
	/* アライメントと全体のサイズの計算だけ行う */
	for (auto mem = ty->_members.get(); mem; mem = mem->_next.get())
	{
		if (ty->_align < mem->_ty->_align)
		{
			ty->_align = mem->_ty->_align;
		}
		if (ty->_size < mem->_ty->_size)
		{
			ty->_size = mem->_ty->_size;
		}
	}
	ty->_size = Object::align_to(ty->_size, ty->_align);
	return ty;
}

/**
 * @brief 構造体定義のメンバの定義を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 構造体の型
 * @details 下記のEBNF規則に従う。 @n struct-members = (declspec declarator ("," declarator)* ";")* "}"
 */
void Node::struct_members(Token **next_token, Token *current_token, Type *ty)
{
	/* ダミーの先頭 */
	auto head = std::make_shared<Member>();
	auto cur = head.get();

	/* '}'が出てくるまで読み込み続ける */
	while (!current_token->is_equal("}"))
	{
		auto base = declspec(&current_token, current_token, nullptr);
		bool first = true;

		/* ';'が出てくるまで読み込み続ける */
		while (!Token::consume(&current_token, current_token, ";"))
		{
			if (!first)
			{
				/* 2個目以降はカンマ区切りが必要 */
				current_token = Token::skip(current_token, ",");
			}
			first = false;
			auto mem = std::make_shared<Member>();
			/* メンバ名部分を読み込む */
			mem->_ty = declarator(&current_token, current_token, base);
			mem->_token = mem->_ty->_token;
			/* リストの先頭に繋ぐ */
			cur->_next = std::move(mem);
			cur = cur->_next.get();
		}
	}
	*next_token = current_token->_next.get();
	ty->_members = std::move(head->_next);
}

/**
 * @brief 入力トークンと対応する構造体のメンバーを取得する。
 * 見つからなければエラーとする。
 *
 * @param ty 構造体の型情報
 * @param token 取得するメンバのトークン
 * @return 構造体のメンバのポインタ
 */
shared_ptr<Member> Node::get_struct_member(Type *ty, Token *token)
{
	for (auto mem = ty->_members; mem; mem = mem->_next)
	{
		if (mem->_token->_str.size() == token->_str.size() &&
			std::equal(token->_str.begin(), token->_str.end(), mem->_token->_str.begin()))
		{
			return mem;
		}
	}
	/* 見つからなければエラー */
	error_token("存在しないメンバです", token);

	/* コンパイルエラー対策 */
	return nullptr;
}

/**
 * @brief 構造体,共用体のメンバにアクセスするノードを生成する
 *
 * @param lhs 参照される構造体、共用体
 * @param token メンバの呼び出しに対応するトークン
 * @return メンバに対応するノード
 */
unique_ptr<Node> Node::struct_ref(unique_ptr<Node> &&lhs, Token *token)
{
	/* 型を確定させる */
	Type::add_type(lhs.get());

	/* 構造体,共用体でなければエラー */
	if (lhs->_ty->_kind != TypeKind::TY_STRUCT && lhs->_ty->_kind != TypeKind::TY_UNION)
	{
		error_token("構造体または共用体ではありません", token);
	}

	auto mem = get_struct_member(lhs->_ty.get(), token);
	auto node = std::make_unique<Node>(NodeKind::ND_MEMBER, std::move(lhs), token);
	node->_member = std::move(mem);
	return node;
}

/**
 * @brief 変数宣言の型指定子部分を読み取る
 *
 * @details
 * 下記のEBNF規則に従う。 @n
 * declspec =  ("void" | "int" | "short" | "long" | "char"
 * 				| "typedef"
 * 				| struct-decl | union-decl)+ @n
 * 型指定子における型名の順番は重要ではない。例えば、`int long static` は `static long int` と同じ意味である。
 * 'long` や `short` が指定されていれば `int` を省略できるので、`static long` と書くこともできる。
 * しかし、`char int` のようなものは有効な型指定子ではなく、型名の組み合わせは限られている。
 * この関数では、それまでの型名が表す「現在の」型オブジェクトを保持したまま、各型名の出現回数を数える。
 * 型名でないトークンに達すると、現在の型オブジェクトを返す。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param attr 読み取った型の属性を返すための参照
 * @return 変数の型
 */
shared_ptr<Type> Node::declspec(Token **next_token, Token *current_token, VarAttr *attr)
{
	constexpr int VOID = 1 << 0;
	constexpr int CHAR = 1 << 2;
	constexpr int SHORT = 1 << 4;
	constexpr int INT = 1 << 6;
	constexpr int LONG = 1 << 8;
	constexpr int OTHER = 1 << 10;

	/* それぞれの型名の出現回数を表すカウンタ。
	 * 例えばビット0, 1は「void」という型名の出現回数を表す。
	 */
	int counter = 0;
	auto ty = Type::INT_BASE;

	while (current_token->is_typename())
	{
		/* typedef */
		if (current_token->is_equal("typedef"))
		{
			/* typedefが使用できない箇所である場合エラー 例 function(int i, typedef int INT) */
			if (!attr)
			{
				error_token("ここでストレージクラス指定子は使用できません", current_token);
			}
			attr->is_typedef = true;
			current_token = current_token->_next.get();
			continue;
		}

		/* ユーザー定義の型 */
		auto ty2 = Object::find_typedef(current_token);
		if (current_token->is_equal("struct") || current_token->is_equal("union") || ty2)
		{
			/* ユーザー定義型が他の型と組み合わさることはない 例 int hoge x; */
			if (counter)
			{
				break;
			}
			if (current_token->is_equal("struct"))
			{
				ty = struct_decl(&current_token, current_token->_next.get());
			}
			else if (current_token->is_equal("union"))
			{
				ty = union_decl(&current_token, current_token->_next.get());
			}
			else
			{
				ty = ty2;
				current_token = current_token->_next.get();
			}
			counter += OTHER;
			continue;
		}

		/* void型 */
		if (current_token->is_equal("void"))
		{
			counter += VOID;
		}
		/* char型 */
		else if (current_token->is_equal("char"))
		{
			counter += CHAR;
		}
		/* short型 */
		else if (current_token->is_equal("short"))
		{
			counter += SHORT;
		}
		/* int型 */
		else if (current_token->is_equal("int"))
		{
			counter += INT;
		}
		/* long型 */
		else if (current_token->is_equal("long"))
		{
			counter += LONG;
		}
		/* どれでもなければエラー */
		else
		{
			error_token("型名ではありません", current_token);
		}

		switch (counter)
		{
		case VOID:
			ty = Type::VOID_BASE;
			break;
		case CHAR:
			ty = Type::CHAR_BASE;
			break;
		case SHORT:
		case SHORT + INT:
			ty = Type::SHORT_BASE;
			break;
		case INT:
			ty = Type::INT_BASE;
			break;
		case LONG:
		case LONG + INT:
		case LONG + LONG:
		case LONG + LONG + INT:
			ty = Type::LONG_BASE;
			break;
		default:
			error_token("無効な型指定です", current_token);
		}

		current_token = current_token->_next.get();
	}

	*next_token = current_token;
	return ty;
}

/**
 * @brief 単文を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n expression-statement = expression? ';'
 */
unique_ptr<Node> Node::expression_statement(Token **next_token, Token *current_token)
{
	/* 空のstatementに対応 */
	if (current_token->is_equal(";"))
	{
		*next_token = current_token->_next.get();
		return std::make_unique<Node>(NodeKind::ND_BLOCK, current_token);
	}
	auto node = std::make_unique<Node>(NodeKind::ND_EXPR_STMT, current_token);
	node->_lhs = expression(&current_token, current_token);

	/* expression-statementは';'で終わるはず */
	*next_token = Token::skip(current_token, ";");
	return node;
}

/**
 * @brief 式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n expression = assign (',' expression)?
 */
unique_ptr<Node> Node::expression(Token **next_token, Token *current_token)
{
	auto node = assign(&current_token, current_token);

	/* 後ろにカンマがあるときは式が続く */
	if (current_token->is_equal(","))
	{
		return std::make_unique<Node>(NodeKind::ND_COMMA, std::move(node), expression(next_token, current_token->_next.get()), current_token);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief 代入式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n assign = equality ("=" assign)?
 */
unique_ptr<Node> Node::assign(Token **next_token, Token *current_token)
{
	auto node = equality(&current_token, current_token);
	if (current_token->is_equal("="))
	{
		return std::make_unique<Node>(NodeKind::ND_ASSIGN, std::move(node), assign(next_token, current_token->_next.get()), current_token);
	}
	*next_token = current_token;
	return node;
}

/**
 * @brief 等価比較式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n equality = relational ("==" relational | "!=" relational)*
 */
unique_ptr<Node> Node::equality(Token **next_token, Token *current_token)
{
	auto node = relational(&current_token, current_token);

	for (;;)
	{
		auto start = current_token;

		if (current_token->is_equal("=="))
		{
			node = std::make_unique<Node>(NodeKind::ND_EQ, std::move(node), relational(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("!="))
		{
			node = std::make_unique<Node>(NodeKind::ND_NE, std::move(node), relational(&current_token, current_token->_next.get()), start);
			continue;
		}

		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 比較式を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n relational = add ("<" add | "<=" add | ">" add | ">=" add)*
 */
unique_ptr<Node> Node::relational(Token **next_token, Token *current_token)
{
	auto node = add(&current_token, current_token);

	while (true)
	{
		auto start = current_token;

		if (current_token->is_equal("<"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, std::move(node), add(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("<="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, std::move(node), add(&current_token, current_token->_next.get()), start);
			continue;
		}

		/* lhs > rhs は rhs < lhs と読み替える */
		if (current_token->is_equal(">"))
		{
			node = std::make_unique<Node>(NodeKind::ND_LT, add(&current_token, current_token->_next.get()), std::move(node), start);
			continue;
		}

		/* lhs >= rhs は rhs <= lhs と読み替える */
		if (current_token->is_equal(">="))
		{
			node = std::make_unique<Node>(NodeKind::ND_LE, add(&current_token, current_token->_next.get()), std::move(node), start);
			continue;
		}

		/* 比較演算子が出てこなくなったらループを抜ける */
		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 和を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n add = mul ("+" mul | "-" mul)*
 */
unique_ptr<Node> Node::add(Token **next_token, Token *current_token)
{
	auto node = mul(&current_token, current_token);

	while (true)
	{
		auto start = current_token;
		if (current_token->is_equal("+"))
		{
			node = new_add(std::move(node), mul(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("-"))
		{
			node = new_sub(std::move(node), mul(&current_token, current_token->_next.get()), start);
			continue;
		}

		/* "+", "-"どちらも出てこなくなったらループを抜ける */
		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 型キャストを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n "(" type-name ")" cast | unary
 */
unique_ptr<Node> Node::cast(Token **next_token, Token *current_token)
{
	/* キャストを含む場合 */
	if (current_token->is_equal("(") && current_token->_next->is_typename())
	{
		auto start = current_token;

		/* キャスト先の型を読み取る */
		auto ty = type_name(&current_token, current_token->_next.get());
		current_token = Token::skip(current_token, ")");

		/* キャスト対象の式を評価 */
		auto lhs = cast(next_token, current_token);
		Type::add_type(lhs.get());

		auto node = std::make_unique<Node>(NodeKind::ND_CAST, std::move(lhs), start);
		node->_ty = ty;
		return node;
	}

	return unary(next_token, current_token);
}

/**
 * @brief 積を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n mul = cast ("*" cast | "/" cast)*
 */
unique_ptr<Node> Node::mul(Token **next_token, Token *current_token)
{
	auto node = cast(&current_token, current_token);

	while (true)
	{
		auto start = current_token;
		if (current_token->is_equal("*"))
		{
			node = std::make_unique<Node>(NodeKind::ND_MUL, std::move(node), cast(&current_token, current_token->_next.get()), start);
			continue;
		}

		if (current_token->is_equal("/"))
		{
			node = std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), cast(&current_token, current_token->_next.get()), start);
			continue;
		}

		/* "*", "/"どちらも出てこなくなったらループを抜ける */
		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 符号付の単項を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n unary = ("+" | "-" | "*" | "&") cast | postfix
 */
unique_ptr<Node> Node::unary(Token **next_token, Token *current_token)
{
	if (current_token->is_equal("+"))
	{
		return cast(next_token, current_token->_next.get());
	}

	if (current_token->is_equal("-"))
	{
		return std::make_unique<Node>(NodeKind::ND_NEG, cast(next_token, current_token->_next.get()), current_token);
	}

	if (current_token->is_equal("&"))
	{
		return std::make_unique<Node>(NodeKind::ND_ADDR, cast(next_token, current_token->_next.get()), current_token);
	}

	if (current_token->is_equal("*"))
	{
		return std::make_unique<Node>(NodeKind::ND_DEREF, cast(next_token, current_token->_next.get()), current_token);
	}

	return postfix(next_token, current_token);
}

/**
 * @brief 配列の添え字[]または構造体のメンバを読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n postfix = primary ("[" expression "]" | "." identifier | "->" identifier)*
 */
unique_ptr<Node> Node::postfix(Token **next_token, Token *current_token)
{
	/* 単項を読む */
	auto node = primary(&current_token, current_token);

	for (;;)
	{
		/* 配列 */
		if (current_token->is_equal("["))
		{
			/* x[y] を *(x+y) に置き換える */
			auto start = current_token;
			auto idx = expression(&current_token, current_token->_next.get());
			current_token = Token::skip(current_token, "]");
			node = std::make_unique<Node>(NodeKind::ND_DEREF, new_add(std::move(node), std::move(idx), start), start);
			continue;
		}
		/* 構造体 */
		if (current_token->is_equal("."))
		{
			node = struct_ref(std::move(node), current_token->_next.get());
			current_token = current_token->_next->_next.get();
			continue;
		}
		/* 構造体のポインタ */
		if (current_token->is_equal("->"))
		{
			/* x->y を(*x).y を読み替える */
			node = std::make_unique<Node>(NodeKind::ND_DEREF, std::move(node), current_token);
			node = struct_ref(std::move(node), current_token->_next.get());
			current_token = current_token->_next->_next.get();
			continue;
		}

		*next_token = current_token;
		return node;
	}
}

/**
 * @brief 単項を読み取る。
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * primary = "(" "{" statement+ "}" ")" | "(" expression ")" | "sizeof" unary | "sizeof" "(" type-name ")" | identifier args? | str | num @n
 * args = "(" ")"
 */
unique_ptr<Node> Node::primary(Token **next_token, Token *current_token)
{
	/* トークンが"(" "{"ならステートメント式 */
	if (current_token->is_equal("(") && current_token->_next->is_equal("{"))
	{
		auto node = std::make_unique<Node>(NodeKind::ND_STMT_EXPR, current_token);
		/* ブロックを読み取って、ブロック内の処理を新しいnodeのbodyに付け替える */
		auto stmt = compound_statement(&current_token, current_token->_next->_next.get());
		node->_body = std::move(stmt->_body);
		/* ブロックの後は') */
		*next_token = Token::skip(current_token, ")");
		return node;
	}

	/* トークンが"("なら、"(" expression ")"のはず */
	if (current_token->is_equal("("))
	{
		auto node = expression(&current_token, current_token->_next.get());
		*next_token = Token::skip(current_token, ")");
		return node;
	}

	/* 文字列リテラル */
	if (TokenKind::TK_STR == current_token->_kind)
	{
		auto var = new_string_literal(current_token->_str);
		*next_token = current_token->_next.get();
		return std::make_unique<Node>(var, current_token);
	}

	/* sizeof演算子（対象が型そのもの） */
	if (current_token->is_equal("sizeof") &&
		current_token->_next->is_equal("(") &&
		current_token->_next->_next->is_typename())
	{
		auto start = current_token;
		/* sizeof演算子の対象の型情報を読む */
		auto ty = type_name(&current_token, current_token->_next->_next.get());
		*next_token = Token::skip(current_token, ")");
		return std::make_unique<Node>(ty->_size, start);
	}

	/* sizeof演算子（対象が式） */
	if (current_token->is_equal("sizeof"))
	{
		/* sizeofの対象を評価 */
		auto node = unary(next_token, current_token->_next.get());
		/* sizeofの対象の型を決定 */
		Type::add_type(node.get());
		/* 型のサイズの数値ノードを返す */
		return std::make_unique<Node>(node->_ty->_size, current_token);
	}

	/* トークンが識別子の場合 */
	if (TokenKind::TK_IDENT == current_token->_kind)
	{
		/* 識別子の後に()がついていたら関数呼び出し */
		if (current_token->_next->is_equal("("))
		{
			return function_call(next_token, current_token);
		}

		/* それ以外なら普通の変数 */
		const auto sc = Object::find_var(current_token);

		/* 変数が宣言されていない場合はエラー */
		if (!sc || !sc->_obj)
		{
			error_token("未宣言の変数です", current_token);
		}
		auto node = std::make_unique<Node>(sc->_obj, current_token);
		*next_token = current_token->_next.get();
		return node;
	}

	/* トークンが数値の場合 */
	if (TokenKind::TK_NUM == current_token->_kind)
	{
		auto node = std::make_unique<Node>(current_token->_value, current_token);
		*next_token = current_token->_next.get();
		return node;
	}

	/* どれでもなければエラー */
	error_token("式ではありません", current_token);

	/* コンパイルエラー対策、error_token()内でプログラムは終了するためnullptrが返ることはない */
	return nullptr;
}

/**
 * @brief typedefを読み取る
 *
 * @param token 現在のトークン
 * @param base ベースの型
 * @return 次のトークン
 */
Token *Node::parse_typedef(Token *token, shared_ptr<Type> base)
{
	bool first = true;

	/* ";"が出てくるまで読み込み続ける */
	while (!Token::consume(&token, token, ";"))
	{
		/* 2個目以降では","区切りが必要 */
		if (!first)
		{
			token = Token::skip(token, ",");
		}
		first = false;

		auto ty = declarator(&token, token, base);
		Object::push_scope(ty->_token->_str)->type_def = ty;
	}
	return token;
}

/**
 * @brief 識別子をもたない仮想的な変数宣言として型情報を読む
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @param ty 型指定子の型
 * @return 読み取った型
 * @details 下記のEBNF規則に従う。 @n abstract-declarator = "*"* ("(" abstract-declarator ")")? type-suffix
 */
shared_ptr<Type> Node::abstract_declarator(Token **next_token, Token *current_token, shared_ptr<Type> &&ty)
{
	/* "*"の数だけその前までの型に対するポインター */
	while (current_token->is_equal("*"))
	{
		ty = Type::pointer_to(ty);
		current_token = current_token->_next.get();
	}

	/* ネストしている型 */
	if (current_token->is_equal("("))
	{
		auto start = current_token;
		auto dummy = std::make_shared<Type>();
		/* ネストの中を飛ばす */
		abstract_declarator(&current_token, start->_next.get(), std::move(dummy));
		current_token = Token::skip(current_token, ")");
		/* ネストの外側の型を読み込む */
		ty = type_suffix(next_token, current_token, std::move(ty));
		/* ネストの外側の型をベースとして内側の型を読む */
		return abstract_declarator(&current_token, start->_next.get(), std::move(ty));
	}
	return type_suffix(next_token, current_token, std::move(ty));
}

/**
 * @brief 型情報を読み取る
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 読み取った型
 * @details 下記のEBNF規則に従う。 @n type-name = declspec abstract-declarator
 */
shared_ptr<Type> Node::type_name(Token **next_token, Token *current_token)
{
	auto ty = declspec(&current_token, current_token, nullptr);
	return abstract_declarator(next_token, current_token, std::move(ty));
}

/**
 * @brief 関数呼び出し
 *
 * @param next_token 残りのトークンを返すための参照
 * @param current_token 現在処理しているトークン
 * @return 対応するASTノード
 * @details 下記のEBNF規則に従う。 @n
 * function_call = identifier "(" (assign ("," assign)*)? ")"
 */
unique_ptr<Node> Node::function_call(Token **next_token, Token *current_token)
{
	/* 関数呼び出しノードを作成 */
	auto node = std::make_unique<Node>(NodeKind::ND_FUNCALL, current_token);
	/* 関数の名前をセット */
	node->_func_name = std::move(current_token->_str);

	current_token = current_token->_next->_next.get();

	/* ノードリストの先頭としてダミーのノードを生成 */
	auto head = std::make_unique_for_overwrite<Node>();
	auto cur = head.get();

	/* ')'が出てくるまで読み込み続ける */
	while (!current_token->is_equal(")"))
	{
		if (head.get() != cur)
		{
			/* 2個目以降の引数には区切りとして","が必要 */
			current_token = Token::skip(current_token, ",");
		}
		cur->_next = assign(&current_token, current_token);
		cur = cur->_next.get();
	}

	/* 最後は")"" */
	*next_token = Token::skip(current_token, ")");
	/* headの次のノード以降を切り離し返り値用のnodeのargsに繋ぐ */
	node->_args = std::move(head->_next);
	return node;
}

/**
 * @brief 左辺 + 右辺の計算結果を表すノードを生成する。
 *
 * @details
 * C言語では、+演算子はポインタ演算を行うためにオーバーロードされている。
 * もしpがポインタである場合、p+nはnを加えるのではなく、sizeof(*p)*nをpの値に加える。
 * そのため、p+nはpからn個先の要素（バイトではなく）を指すようになる。
 * 言い換えれば、ポインタ値に加える前に整数値をスケールする必要があり、この関数はそのスケーリングを処理する。
 * @param lhs 左辺
 * @param rhs 右辺
 * @param location ノードと対応する入力文字列の位置
 * @return 対応するASTノード
 */
unique_ptr<Node> Node::new_add(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 + 数 */
	if (lhs->_ty->is_integer() && rhs->_ty->is_integer())
	{
		return std::make_unique<Node>(NodeKind::ND_ADD, std::move(lhs), std::move(rhs), token);
	}

	/* ptr + ptr は無効な演算 */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		error_token("無効な演算です", token);
	}

	/* "数 + ptr" を "ptr + 数" に変換する*/
	if (!lhs->_ty->_base && rhs->_ty->_base)
	{
		std::swap(lhs, rhs);
	}

	/* ptr + 数 */
	rhs = std::make_unique<Node>(NodeKind::ND_MUL, std::move(rhs), std::make_unique<Node>(lhs->_ty->_base->_size, token), token);
	return std::make_unique<Node>(NodeKind::ND_ADD, std::move(lhs), std::move(rhs), token);
}

/**
 * @brief 左辺 - 右辺の計算結果を表すノードを生成する。
 *
 * @details
 * C言語では、-演算子も'+'演算子と同様にポインタ演算を行うためにオーバーロードされている。
 * ポインタ - ポインタは2つのポインタ間にある要素の数を返す。
 * @param lhs 左辺
 * @param rhs 右辺
 * @param location ノードと対応する入力文字列の位置
 * @return 対応するASTノード
 */
unique_ptr<Node> Node::new_sub(unique_ptr<Node> &&lhs, unique_ptr<Node> &&rhs, Token *token)
{
	/* 右辺と左辺の型を確定する */
	Type::add_type(lhs.get());
	Type::add_type(rhs.get());

	/* 数 - 数 */
	if (lhs->_ty->is_integer() && rhs->_ty->is_integer())
	{
		return std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), token);
	}

	/* ptr - 数 */
	if (lhs->_ty->_base && !rhs->_ty->_base)
	{
		rhs = std::make_unique<Node>(NodeKind::ND_MUL, std::move(rhs), std::make_unique<Node>(lhs->_ty->_base->_size, token), token);
		Type::add_type(rhs.get());
		auto node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), token);
		return node;
	}

	/* ptr - ptr */
	if (lhs->_ty->_base && rhs->_ty->_base)
	{
		int sz = lhs->_ty->_base->_size;
		auto node = std::make_unique<Node>(NodeKind::ND_SUB, std::move(lhs), std::move(rhs), token);
		node->_ty = Type::INT_BASE;
		return std::make_unique<Node>(NodeKind::ND_DIV, std::move(node), std::make_unique<Node>(sz, token), token);
	}

	/* 数 - ptr はエラー */
	error_token("無効な演算です", token);

	/* コンパイルエラー対策。直前のerror_token()で終了されるのでnullptrが返ることはない */
	return nullptr;
}

/**
 * @brief トークン・リストを構文解析して関数ごとにASTを構築する
 *
 * @param token トークン・リストの先頭
 * @return 構文解析結果
 * @details program = (typedef | function-definition | global-variable)*
 */
unique_ptr<Object> Node::parse(Token *token)
{
	/* トークンリストを最後まで辿る*/
	while (TokenKind::TK_EOF != token->_kind)
	{
		VarAttr attr = {};
		auto base = declspec(&token, token, &attr);

		/* typedef */
		if (attr.is_typedef)
		{
			token = parse_typedef(token, base);
			continue;
		}

		/* 関数 */
		if (is_function(token))
		{
			token = function_definition(token, std::move(base));
			continue;
		}

		/* グローバル変数 */
		token = global_variable(token, std::move(base));
	}

	return std::move(Object::globals);
}

/**
 * @brief トップレベルに出てくる定義が関数かどうか判定する
 *
 * @param token declarator部分のトークン
 * @return true 関数である
 * @return false 関数ではない
 */
bool Node::is_function(Token *token)
{
	/* 識別子にたどり着くまで辿り続ける */
	while (token->is_equal("*"))
	{
		token = token->_next.get();
	}

	/* 識別子が来なければエラー */
	if (TokenKind::TK_IDENT != token->_kind)
	{
		error_token("識別子ではありません", token);
	}

	/* 識別子の次に来るのが"("なら関数 */
	if (token->_next->is_equal("("))
	{
		return true;
	}

	/* そうでないなら変数 */
	return false;
}

/**
 * @brief グローバル変数を読み取る
 *
 * @param token 読み込むトークン
 * @param base 型宣言の前半部分から読み取れた型
 * @return 次のトークン
 */
Token *Node::global_variable(Token *token, shared_ptr<Type> &&base)
{
	/* 1個めの変数であるか */
	bool first = true;

	/* ;が現れるまで読み込みを続ける */
	while (!Token::consume(&token, token, ";"))
	{
		if (!first)
		{
			/* 2個目以降の変数定義には","区切りが必要 */
			token = Token::skip(token, ",");
		}
		/* 初回フラグを下す */
		first = false;

		/* 最終的な型を決定する */
		auto ty = declarator(&token, token, base);
		Object::new_gvar(ty->_token->_str, std::move(ty));
	}

	return token;
}

/**
 * @brief グローバル変数の仮名としてユニークな名前を生成する
 *
 * @return 生成した名前。".L..id"となる。(idは生成順)
 */
std::string Node::new_unique_name()
{
	static int id = 0;
	return ".L.." + std::to_string(id++);
}

/**
 * @brief 仮名を付けてグローバル変数を生成する
 *
 * @param ty 生成するグローバル変数の型
 * @return unique_ptr<Object>
 */
Object *Node::new_anonymous_gvar(std::shared_ptr<Type> &&ty)
{
	return Object::new_gvar(new_unique_name(), std::move(ty));
}

/**
 * @brief 文字列リテラルとしてstrをもつグローバル変数を生成する。
 *
 * @param str 文字列リテラル
 * @return 生成したグローバル変数オブジェクトへのポインタ
 */
Object *Node::new_string_literal(const std::string &str)
{
	/* 文字列リテラルの型はchar型配列で長さは文字数+'\0'終端 */
	auto ty = Type::array_of(Type::CHAR_BASE, str.size() + 1);

	/* 仮名を使ってオブジェクトを生成 */
	auto obj = new_anonymous_gvar(std::move(ty));

	/* init_dataに文字列を入れて'\0'終端を追加 */
	obj->_init_data = str;
	obj->_init_data.push_back('\0');
	obj->is_str_literal = true;
	return obj;
}