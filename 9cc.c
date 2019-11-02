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
		error("''%c'ではありません", op);
	}
	currentToken = currentToken->next;
}

// 次のトークンが数値である場合、トークンを１つ進めてその値を返す。
// それ以外の場合、エラーを報告する
int expect_number() {
	if (currentToken->kind != TK_NUM) {
		error("数ではありません");
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
			if(*p == '+' || *p == '-') {
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

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	currentToken = tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

	// 最初は数値のはずなのでそれをmov命令へ出力
  printf("  mov rax, %d\n", expect_number());

	while(!at_eof()) {
		if (consume('+')) {
			printf("  add rax, %d\n", expect_number());
			continue;
		}
		// 今の仕様では+か-しか受け付けないので、+でなければ-であるはず
		expect('-');
		printf("  sub rax, %d\n", expect_number());
	}

  printf("  ret\n");
	return 0;
}
