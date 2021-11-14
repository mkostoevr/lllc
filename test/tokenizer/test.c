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
	Token *tokens = tokenize(&lllc, "test.txt");
	for (size_t i = 0; i < cvec_Token_size(&tokens); i++) {
		Token token = tokens[i];
		assert(token.kind > TOK_MIN && token.kind < TOK_MAX);
		printf("%u:%u: %s\n", token.line, token.column, token2str[token.kind]);
	}
	return 0;
}
