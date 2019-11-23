#include "9cc.h"

Node *code[100];

// 2項演算子のノードを作成する
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

// 整数のノードを作成する
Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

// プログラム全体
void program() {
	int i = 0;
	while(!at_eof()) {
		code[i++] = stmt();
	}
	code[i] = NULL;
}

// 文
Node *stmt() {
	Node *node = expr();
	expect(";");
	return node;

}
// 代入式
Node *assign() {
	Node *node = equality();
	if (consume("=")) {
		node = new_binary(ND_ASSIGN, node, assign());
	}
	return node;
}

// 式
Node *expr(){
	return assign();
}

// 等価式
Node *equality() {
	Node *node = relational();
	for(;;){
		if (consume("==")) {
			node = new_binary(ND_EQ, node, relational());
		}
		else if (consume("!=")) {
			node = new_binary(ND_NE, node, relational());
		}
		else {
			return node;
		}
	}
}

// 関係式
Node *relational() {
	Node *node = add();
	for (;;) {
		if (consume("<")) {
			node = new_binary(ND_LT, node, add());
		}
		else if (consume(">")) {
			node = new_binary(ND_LT, add(), node);
		}
		else if (consume("<=")) {
			node = new_binary(ND_LE, node, add());
		}
		else if (consume(">=")) {
			node = new_binary(ND_LE, add(), node);
		}
		else {
			return node;
		}
	}
}

// 加減算
Node *add() {
	Node *node = mul();

	for(;;){
		if (consume("+")) {
			node = new_binary(ND_ADD, node, mul());
		}
		else if (consume("-")) {
			node = new_binary(ND_SUB, node, mul());
		}
		else{
			return node;
		}
	}
}

// 乗除算
Node *mul() {
	Node *node = unary();

	for(;;) {
		if (consume("*")) {
			node = new_binary(ND_MUL, node, unary());
		}
		else if (consume("/")) {
			node = new_binary(ND_DIV, node, unary());
		}
		else {
			return node;
		}
	}
}

// 単項演算子
Node *unary() {
		if (consume("+")) {
			return unary();
		}
		else if (consume("-")) {
			return new_binary(ND_SUB, new_node_num(0), unary());
		}
		else {
			return primary();
		}
}

// 単項式
Node *primary() {
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

	Token *t = consume_ident();
	if (t) {
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;
		node->offset = (t->str[0] - 'a' + 1) * 8; // 変数の長さは1
		return node;
	}

	// 今の文法では整数のはず
	return new_node_num(expect_number());
}
