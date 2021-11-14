#include "../../main.h"

int main(int argc, char **argv) {
	char *token2str[TOK_MAX] = {
		#define ENTRY_STR(token_name) #token_name
		#define ENTRY(token_name) ENTRY_STR(TOK_ ## token_name)
		#include "../../tokentypes.h"
		#undef ENTRY
		#undef ENTRY_STR
	};

	Compiler lllc = {};
	Token token = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, "test.txt");
	do {
		token = tokenizer_next_token(&tokenizer);
		assert(token.kind > TOK_MIN && token.kind < TOK_MAX);
		printf("%u:%u: %s\n", token.line, token.column, token2str[token.kind]);
	} while (token.kind != TOK_EOF);
	return 0;
}
