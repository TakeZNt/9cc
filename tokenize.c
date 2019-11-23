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

// 次のトークンが識別子である場合、トークンを１つ進めてその値を返す。
// それ以外の場合、NULLを返す。
Token *consume_ident(void) {
	if (currentToken->kind != TK_IDENT) {
		return NULL;
	}
	Token *t = currentToken;
	currentToken = currentToken->next;
	return t;
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
			// 2文字の記号の処理
			if (startsWith(p, "==") || startsWith(p, "!=") || startsWith(p, "<=") || startsWith(p, ">=")) {
				cur = new_token(TK_RESERVED, cur, p, 2);
				p += 2;
				continue;
			}
			// 1文字の記号の処理
			if(strchr("+-*/()<>=;", *p)) {
				cur = new_token(TK_RESERVED, cur, p++, 1);
				continue;
			}
			// 数値の処理
			if (isdigit(*p)) {
				cur = new_token(TK_NUM, cur, p, 0); // この時点ではトークンの長さは不明なので0を入れておく
				char *q = p; // 数値のパース前の位置を覚えておく
				cur->val = strtol(p, &p, 10); // ここでpが進む
				cur->len = p - q; // 進んだ分が文字列の長さ
				continue;
			}
			// 変数の処理
			if('a' <= *p && *p <= 'z'){
				cur = new_token(TK_IDENT, cur, p++, 1);
				continue;
			}
			error("トークナイズできません");
		}

		new_token(TK_EOF, cur, p, 0);
		return head.next;
}
