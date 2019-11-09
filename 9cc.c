#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
	TK_NUM, // 数値
	TK_EOF, // 入力の終わり
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
	TokenKind kind; // トークンの型
	Token *next; // 次のトークン
	int val; // 数値の場合の値
	char *str; // トークン文字列
};

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノード
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

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

// 次のトークンがopである場合、トークンを１つ進めてtrueを返す。
// それ以外の場合、falseを返す
bool consume(char op) {
	if (currentToken->kind != TK_RESERVED || currentToken->str[0] != op) {
		return false;
	}
	currentToken = currentToken->next;
	return true;
}

// 次のトークンがopである場合、トークンを１つ進める。
// それ以外の場合、エラーを報告する
void expect(char op) {
	if (currentToken->kind != TK_RESERVED || currentToken->str[0] != op) {
		error_at(currentToken->str, "''%c'ではありません", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
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
			if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
				cur = new_token(TK_RESERVED, cur, p++);
				continue;
			}
			if (isdigit(*p)) {
				cur = new_token(TK_NUM, cur, p);
				cur->val = strtol(p, &p, 10);
				continue;
			}
			error("トークナイズできません");
		}

		new_token(TK_EOF, cur, p);
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

Node *expr();
Node *mul();
Node *primary();

// 加減算
Node *expr(){
	Node *node = mul();

	for(;;){
		if (consume('+')) {
			node = new_binary(ND_ADD, node, mul());
		}
		else if (consume('-')) {
			node = new_binary(ND_SUB, node, mul());
		}
		else{
			return node;
		}
	}
}

// 乗除算
Node *mul() {
	Node *node = primary();

	for(;;) {
		if (consume('*')) {
			node = new_binary(ND_MUL, node, primary());
		}
		else if (consume('/')) {
			node = new_binary(ND_DIV, node, primary());
		}
		else {
			return node;
		}
	}
}

// 単項式
Node *primary() {
	if (consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	}
	else {
		// 今の文法では整数のはず
		return new_node_num(expect_number());
	}
}

// アセンブラコードを出力する
void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
  printf("  pop rax\n");

	switch (node->kind) {
		case ND_ADD:
			printf("  add rax, rdi\n");
			break;
		case ND_SUB:
			printf("  sub rax, rdi\n");
			break;
		case ND_MUL:
			printf("  imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("  cqo\n");
			printf("  idiv rdi\n");
			break;
	}

	printf("  push rax\n");
}


int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	user_input = argv[1];
	currentToken = tokenize(user_input);
	Node *node = expr();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

	// アセンブリの本体を出力
	gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをRAXにロードして関数からの返り値とする
	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}
