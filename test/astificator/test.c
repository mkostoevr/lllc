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

void dump_expression(AstNode expr) {
	if (expr.kind == AST_INTEGER) {
		printf("%d", expr.integer);
	} else {
		printf("%s dump isn't implemented\n", ast_node_kind_str[expr.kind]);
		exit(-1);
	}
}

void dump_function_call_list(AstNode *list, int ident);

void print_ident(int ident) {
	while (ident--) {
		printf("    ");
	}
}

void dump_function_call_generic(AstNode function_call, int ident) {
	if (function_call.kind == AST_IF) {
		print_ident(ident);
		printf("if (");
		dump_expression(function_call.nodes[0]);
		printf(") {\n");
		dump_function_call_list(function_call.nodes[1].nodes, ident + 1);
		if (cvec_AstNode_size(&function_call.nodes) == 3) {
			print_ident(ident);
			printf("} else {\n");
			dump_function_call_list(function_call.nodes[2].nodes, ident + 1);
		}
		print_ident(ident);
		printf("}\n");
	} else {
		print_ident(ident);
		printf("%.*s(", cvec_char_size(&function_call.name), function_call.name);
		for (size_t i = 0; i < cvec_AstNode_size(&function_call.nodes); i++) {
			if (i != 0) {
				printf(", ");
			}
			dump_expression(function_call.nodes[i]);
		}
		printf(")\n");
	}
}

void dump_function_call_list(AstNode *list, int ident) {
	for (size_t i = 0; i < cvec_AstNode_size(&list); i++) {
		AstNode function_call = list[i];
		dump_function_call_generic(function_call, ident);
	}
}

void dump_function_definition(AstNode definition) {
	dump_function_declaration(definition);
	printf("Body:\n");
	dump_function_call_list(definition.nodes[3].nodes, 1);
}

void dump_ast_node(AstNode node) {
	if (node.kind == AST_FUNCTION_DECLARATION) {
		dump_function_declaration(node);
	} else if (node.kind == AST_FUNCTION_DEFINITION) {
		dump_function_definition(node);
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
	Tokenizer tokenizer = tokenizer_new(&lllc, argv[1]);
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	for (AstNode node; node = astificator_next_list(&astificator), node.kind != AST_EOF;) {
		dump_ast_node(node);
	}
	return 0;
}
