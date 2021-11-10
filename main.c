#include "main.h"

Compiler compiler_new(int argc, char **argv) {
	return (Compiler) {};
}

char *token2str[TOK_MAX] = {
	"TOK_LPAREN",
	"TOK_RPAREN",
	"TOK_IDENTIFIER",
	"TOK_INT",
	"TOK_CHAR",
	"TOK_STRING",
};

int main(int argc, char **argv) {
	Compiler lllc = compiler_new(argc, argv);
	Token *tokens = tokenize(&lllc, "test/all.lll");
	for (size_t i = 0; i < cvec_Token_size(&tokens); i++) {
		Token token = tokens[i];
		printf("[%u:%u]%s ", token.line, token.column, token2str[token.kind]);
	}
	return 0;
}