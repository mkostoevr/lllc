#define ONE_SOURCE
#include "../../main.h"

void dump_import(AstNode import) {
	AstNode symbol_name = import.nodes[0];
	AstNode imported_name = import.nodes[1];
	AstNode dll_name = import.nodes[2];

	printf("Import '%.*s' from '%.*s' as '%.*s'\n",
		   cvec_char_size(&imported_name.name), imported_name.name,
		   cvec_char_size(&dll_name.name), dll_name.name,
		   cvec_char_size(&symbol_name.name), symbol_name.name);
}

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
	} else if (node.kind == AST_IMPORT) {
		dump_import(node);
	} else if (node.kind == AST_EOF) {
		return;
	} else {
		printf("Error: Can't dump %s", ast_node_kind_str[node.kind]);
	}
}

int main(int argc, char **argv) {
	Compiler lllc = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, "test.lll");
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	for (AstNode node; node = astificator_next_list(&astificator), node.kind != AST_EOF;) {
		dump_ast_node(node);
	}
	return 0;
}
