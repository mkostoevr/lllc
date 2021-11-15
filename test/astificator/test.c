#define ONE_SOURCE
#include "../../main.h"

void dump_function_declaration(AstNode function_declaration) {
	AstNode name = function_declaration.nodes[0];
	AstNode arguments = function_declaration.nodes[1];
	AstNode type = function_declaration.nodes[2];

	printf("Function: %.*s (", cvec_char_size(&name.name), name.name);
	for (size_t i = 0; i < cvec_AstNode_size(&arguments.nodes); i++) {
		if (i > 0) {
			printf(", ");
		}
		AstNode name = arguments.nodes[i].nodes[0];
		AstNode type = arguments.nodes[i].nodes[1];
		printf("%.*s %.*s", cvec_char_size(&name.name), name.name, cvec_char_size(&type.name), type.name);
	}
	printf(") %.*s\n", cvec_char_size(&type.name), type.name);
}

void dump_ast_node(AstNode node) {
	if (node.kind == AST_FUNCTION_DECLARATION) {
		dump_function_declaration(node);
	} else if (node.kind == AST_EOF) {
		return;
	} else {
		printf("Error: Can't dump %s", ast_node_kind_str[node.kind]);
	}
}

int main(int argc, char **argv) {
	Compiler lllc = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, "test.lll");
	AstNode node = {};
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	do {
		node = astificator_next_node(&astificator);
		dump_ast_node(node);
	} while (node.kind != AST_EOF);
	return 0;
}
