#pragma once
#include "parse.hpp"
#include <cassert>
#include <memory>




/**
 * @brief　アセンブリを生成
 *
 */
class CodeGen
{
public:
	/**************************/
	/* 静的メンバ関数 (public) */
	/**************************/

	/**
	 * @brief Intel記法でアセンブリを出力する
	 *
	 * @param program アセンブリを出力sする対象関数
	 */
	static void generate_code(std::unique_ptr<Function> &&program);

private:
	/**************************/
	/* 静的メンバ関数 (private) */
	/**************************/

	/**
	 * @brief スタックにpushするアセンブリを生成
	 *
	 */
	static void push();

	/**
	 * @brief スタックからpopしてregに値を書き込むコードを生成
	 *
	 * @param reg 書き込み先のレジスタ
	 */
	static void pop(std::string &&reg);

	/**
	 * @brief 変数ノードの絶対アドレスを計算して'rax'にセットするコードを生成。
	 *
	 * @note ノードが変数ではないときエラーとする。
	 * @param node 対象ノード
	 */

	/**
	 * @brief
	 *
	 * @param node
	 */
	static void generate_address(std::unique_ptr<Node> &&node);

	/**
	 * @brief expressionをアセンブリに変換
	 *
	 * @param node
	 */
	static void generate_expression(std::unique_ptr<Node> &&node);

	/**
	 * @brief statementをアセンブリに変換
	 *
	 * @param node
	 */
	static void generate_statement(std::unique_ptr<Node> &&node);


	/**
	 * @brief 新しいラベルの通し番号を返す
	 * 
	 * @return int これまでに用意したラベルの数+1
	 */
	static int label_count();
};