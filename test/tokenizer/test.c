#include "../../main.h"

int main(int argc, char **argv) {
	Compiler lllc = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, argv[1]);
	for (Token token; token = tokenizer_next_token(&tokenizer), token.kind != TOK_EOF;) {	
		assert(token.kind > TOK_MIN && token.kind < TOK_MAX);
		printf("%u:%u: %s\n", token.line, token.column, token_kind_str[token.kind]);
	}
	return 0;
}
