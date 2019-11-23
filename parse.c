#include "9cc.h"

// 入力文字列
char *user_input;

// 現在のトークン
Token *currentToken;


// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
		va_list ap;
		va_start(ap, fmt);

		int pos = loc - user_input;
		fprintf(stderr, "%s\n", user_input);
		fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
		fprintf(stderr, "^ ");
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		exit(1);
}

bool tokenEqual(char *op) {
	return strlen(op) != currentToken->len
	|| memcmp(currentToken->str, op, currentToken->len);
}

// 次のトークンがopである場合、トークンを１つ進めてtrueを返す。
// それ以外の場合、falseを返す
bool consume(char *op) {
	if (currentToken->kind != TK_RESERVED
		|| tokenEqual(op)
	) {
		return false;
	}
	currentToken = currentToken->next;
	return true;
}

// 次のトークンがopである場合、トークンを１つ進める。
// それ以外の場合、エラーを報告する
void expect(char *op) {
	if (currentToken->kind != TK_RESERVED
		|| tokenEqual(op)
	) {
		error_at(currentToken->str, "''%s'ではありません", op);
	}
	currentToken = currentToken->next;
}

// 次のトークンが数値である場合、トークンを１つ進めてその値を返す。
// それ以外の場合、エラーを報告する
int expect_number() {
	if (currentToken->kind != TK_NUM) {
		error_at(currentToken->str, "数ではありません");
	}
	int val = currentToken->val;
	currentToken = currentToken->next;
	return val;
}

// トークンがもうないか
bool at_eof() {
	return currentToken->kind == TK_EOF;
}

// curに新しいトークンを作成する
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startsWith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

// 入力文字列pをトークンに分割して返す。トークンのリストは連結リスト形式で返される。
Token *tokenize(char *p) {
		Token head; // ダミーのヘッダ
		head.next = NULL;
		Token *cur = &head;

		while(*p) {
			// 空白をスキップ
			if (isspace(*p)) {
				p++;
				continue;
			}
			if (startsWith(p, "==") || startsWith(p, "!=") || startsWith(p, "<=") || startsWith(p, ">=")) {
				cur = new_token(TK_RESERVED, cur, p, 2);
				p += 2;
				continue;
			}
			if(strchr("+-*/()<>", *p)) {
				cur = new_token(TK_RESERVED, cur, p++, 1);
				continue;
			}
			if (isdigit(*p)) {
				cur = new_token(TK_NUM, cur, p, 0); // この時点ではトークンの長さは不明なので0を入れておく
				char *q = p; // 数値のパース前の位置を覚えておく
				cur->val = strtol(p, &p, 10); // ここでpが進む
				cur->len = p - q; // 進んだ分が文字列の長さ
				continue;
			}
			error("トークナイズできません");
		}

		new_token(TK_EOF, cur, p, 0);
		return head.next;
}

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


// 式
Node *expr(){
	return equality();
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
	else {
		// 今の文法では整数のはず
		return new_node_num(expect_number());
	}
}
