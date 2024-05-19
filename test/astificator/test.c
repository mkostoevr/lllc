#define ONE_SOURCE
#include "../../main.h"

void dump_import(FILE *out, AstNode import) {
	AstNode symbol_name = import.nodes[0];
	AstNode imported_name = import.nodes[1];
	AstNode dll_name = import.nodes[2];

	fprintf(out, "Import '%.*s' from '%.*s' as '%.*s'\n",
		   cvec_char_size(&imported_name.name), imported_name.name,
		   cvec_char_size(&dll_name.name), dll_name.name,
		   cvec_char_size(&symbol_name.name), symbol_name.name);
}

void dump_function_declaration(FILE *out, AstNode function_declaration) {
	AstNode name = function_declaration.nodes[0];
	AstNode arguments = function_declaration.nodes[1];
	AstNode type = function_declaration.nodes[2];

	fprintf(out, "Function: %.*s (", cvec_char_size(&name.name), name.name);
	for (size_t i = 0; i < cvec_AstNode_size(&arguments.nodes); i++) {
		if (i > 0) {
			fprintf(out, ", ");
		}
		AstNode name = arguments.nodes[i].nodes[0];
		AstNode type = arguments.nodes[i].nodes[1];
		fprintf(out, "%.*s %.*s", cvec_char_size(&name.name), name.name, cvec_char_size(&type.name), type.name);
	}
	fprintf(out, ") %.*s\n", cvec_char_size(&type.name), type.name);
}

void dump_expression(FILE *out, AstNode expr) {
	if (expr.kind == AST_INTEGER) {
		fprintf(out, "%d", expr.integer);
	} else {
		fprintf(out, "%s dump isn't implemented\n", ast_node_kind_str[expr.kind]);
		exit(-1);
	}
}

void dump_function_call_list(FILE *out, AstNode *list, int ident);

void print_ident(FILE *out, int ident) {
	while (ident--) {
		fprintf(out, "    ");
	}
}

void dump_function_call_generic(FILE *out, AstNode function_call, int ident) {
	if (function_call.kind == AST_IF) {
		print_ident(out, ident);
		fprintf(out, "if (");
		dump_expression(out, function_call.nodes[0]);
		fprintf(out, ") {\n");
		dump_function_call_list(out, function_call.nodes[1].nodes, ident + 1);
		if (cvec_AstNode_size(&function_call.nodes) == 3) {
			print_ident(out, ident);
			fprintf(out, "} else {\n");
			dump_function_call_list(out, function_call.nodes[2].nodes, ident + 1);
		}
		print_ident(out, ident);
		fprintf(out, "}\n");
	} else {
		print_ident(out, ident);
		fprintf(out, "%.*s(", cvec_char_size(&function_call.name), function_call.name);
		for (size_t i = 0; i < cvec_AstNode_size(&function_call.nodes); i++) {
			if (i != 0) {
				fprintf(out, ", ");
			}
			dump_expression(out, function_call.nodes[i]);
		}
		fprintf(out, ")\n");
	}
}

void dump_function_call_list(FILE *out, AstNode *list, int ident) {
	for (size_t i = 0; i < cvec_AstNode_size(&list); i++) {
		AstNode function_call = list[i];
		dump_function_call_generic(out, function_call, ident);
	}
}

void dump_function_definition(FILE *out, AstNode definition) {
	dump_function_declaration(out, definition);
	fprintf(out, "Body:\n");
	dump_function_call_list(out, definition.nodes[3].nodes, 1);
}

void dump_ast_node(FILE *out, AstNode node) {
	if (node.kind == AST_FUNCTION_DECLARATION) {
		dump_function_declaration(out, node);
	} else if (node.kind == AST_FUNCTION_DEFINITION) {
		dump_function_definition(out, node);
	} else if (node.kind == AST_IMPORT) {
		dump_import(out, node);
	} else if (node.kind == AST_EOF) {
		return;
	} else {
		fprintf(out, "Error: Can't dump %s", ast_node_kind_str[node.kind]);
	}
}

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s test.lll test.log\n", argv[0]);
		return -1;
	}
	FILE *out = fopen(argv[2], "wb");
	if (out == NULL) {
		fprintf(stderr, "Can't open the output file.\n");
		return -1;
	}
	Compiler lllc = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, argv[1]);
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	for (AstNode node; node = astificator_next_list(&astificator), node.kind != AST_EOF;) {
		dump_ast_node(out, node);
	}
	return 0;
}
