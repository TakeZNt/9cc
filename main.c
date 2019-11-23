#include "9cc.h"

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
