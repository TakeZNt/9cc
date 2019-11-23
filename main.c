#include "9cc.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	user_input = argv[1];
	currentToken = tokenize(user_input);
	program();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

	// アセンブリの本体を出力
	// 変数26個分の領域を確保する
	printf("  push rbp\n");
	printf("  mov rbp, rsp\n");
	printf("  sub rsp, 208\n"); // 26 * 8 -> 208

	// program()の実行結果を順番にコード生成する。
	for (int i = 0; code[i]; i++) {
		gen(code[i]);

		// 式の評価結果としてスタックに１つ値が残っているはずなので、それを取り除いておく
		printf("  pop rax\n");
	}

	printf("  mov rsp, rbp\n");
	printf("  pop rbp\n");
	printf("  ret\n");
	return 0;
}
