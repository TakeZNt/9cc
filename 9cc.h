#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Token Token;
typedef struct Node Node;

// トークンの種類
typedef enum {
	TK_RESERVED, // 記号
	TK_IDENT, // 識別子
	TK_NUM, // 数値
	TK_EOF, // 入力の終わり
} TokenKind;

// トークン型
struct Token {
	TokenKind kind; // トークンの型
	Token *next; // 次のトークン
	int val; // 数値の場合の値
	char *str; // トークン文字列
	int len;  // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
	ND_ASSIGN, // =
	ND_LVAR, // ローカル変数
	ND_NUM, // 整数
} NodeKind;

// 抽象構文木のノード
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val; // ND_NUMの場合に使う
	int offset;  // ND_LVARの場合に使う
};

// parse.c用

// 入力文字列
extern char *user_input;

// 現在のトークン
extern Token *currentToken;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool tokenEqual(char *op);
bool consume(char *op);
void expect(char *op);
int expect_number();
Token *consume_ident(void);
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startsWith(char *p, char *q);
Token *tokenize(char *p);

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

extern Node *code[100]; // stmtのリスト
void program();
Node *stmt();
Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// codegen.c用

void gen_lval(Node *node);
void gen(Node *node);
