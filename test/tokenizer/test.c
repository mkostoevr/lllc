#include "../../main.h"

int main(int argc, char **argv) {
	char *token2str[TOK_MAX] = {
		"TOK_LPAREN",
		"TOK_RPAREN",
		"TOK_IDENTIFIER",
		"TOK_INT",
		"TOK_CHAR",
		"TOK_STRING",
	};

	Compiler lllc = {};
	Token *tokens = tokenize(&lllc, "test.txt");
	for (size_t i = 0; i < cvec_Token_size(&tokens); i++) {
		Token token = tokens[i];
		printf("%u:%u: %s\n", token.line, token.column, token2str[token.kind]);
	}
	return 0;
}
