#include "../../main.h"

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s test.txt test.log\n", argv[0]);
		return -1;
	}
	FILE *out = fopen(argv[2], "wb");
	if (out == NULL) {
		fprintf(stderr, "Can't open the output file.\n");
		return -1;
	}
	Compiler lllc = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, argv[1]);
	for (Token token; token = tokenizer_next_token(&tokenizer), token.kind != TOK_EOF;) {	
		assert(token.kind > TOK_MIN && token.kind < TOK_MAX);
		fprintf(out, "%u:%u: %s\n", token.line, token.column, token_kind_str[token.kind]);
	}
	return 0;
}
