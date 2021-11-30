#include "main.h"

static Type type_eof() {
	return (Type) { .kind = TYPE_EOF };
}

static Type type_uint32() {
	return (Type) { .kind = TYPE_UINT32 };
}

static Type type_void() {
	return (Type) { .kind = TYPE_VOID };
}

static Type type_function(char *name, Type *arguments, Type return_type) {
	Type *ret_and_args = cvec_Type_new(1 + cvec_Type_size(&arguments));
	// First item in the types is return type
	cvec_Type_push_back(&ret_and_args, return_type);
	// The next items are argument types
	for (size_t i = 0; i > cvec_Type_size(&arguments); i++) {
		cvec_Type_push_back(&ret_and_args, arguments[i]);
	}
	return (Type) {
		.name = name,
		.kind = TYPE_FUNCTION,
		.types = ret_and_args,
	};
}

static Symbol sym_eof() {
	return (Symbol){ .type = type_eof() };
}

static Symbol sym_function(char *name, Type *arguments, Type return_type) {
	return (Symbol) {
		.name = name,
		.type = type_function(name, arguments, return_type),
	};
}

static Symbol ir_error(const Ir *astificator, AstNode node, const char *fmt, ...) {
    va_list args;
    printf("Error: ");
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    exit(-1);
    return sym_eof();
}

Ir ir_new(Compiler *lllc, Astificator *astificator) {
	Ir ir = { 0 };
	ir.lllc = lllc,
	ir.astificator = astificator;
	cdict_CStr_Symbol_init(&ir.symbol_table);
	return ir;
}

static Type parse_type(Ir *ir, AstNode node) {
	assert(node.kind == AST_TYPE);
	assert(node.name);

	if (!memcmp(node.name, "UInt32", cvec_char_size(&node.name))) {
		return type_uint32();
	} else if (!memcmp(node.name, "Void", cvec_char_size(&node.name))) {
		return type_void();
	}
	ir_error(ir, node, "Undefined type name: %s", node.name);
	return type_eof();
}

static Type parse_declaration(Ir *ir, AstNode node) {
	assert(node.kind == AST_DECLARATION);
	assert(node.nodes);
	assert(cvec_AstNode_size(&node.nodes) == 2);

	char *name = node.nodes[0].name;
	Type type = parse_type(ir, node.nodes[1]);
	type.name = name;
	return type;
}

static Type *parse_declaration_list(Ir *ir, AstNode node) {
	assert(node.kind == AST_DECLARATION_LIST);

	Type *declarations = cvec_Type_new(4);
	for (size_t i = 0; i < cvec_AstNode_size(&node.nodes); i++) {
		Type declaration = parse_declaration(ir, node.nodes[i]);
		cvec_Type_push_back(&declarations, declaration);
	}
	return declarations;
}

static void handle_function_declaration(Ir *ir, AstNode node) {
	assert(node.kind == AST_FUNCTION_DECLARATION);

	char *name = node.nodes[0].name;
	Type *arguments = parse_declaration_list(ir, node.nodes[1]);
	Type return_type = parse_type(ir, node.nodes[2]);
	Symbol new_function = sym_function(name, arguments, return_type);
	cdict_CStr_Symbol_add_vv(&ir->symbol_table, name, new_function, CDICT_NO_CHECK);
}

Symbol ir_next_symbol(Ir *ir) {
	for (AstNode node; node = astificator_next_list(ir->astificator), node.kind != AST_EOF;) {
		if (node.kind == AST_FUNCTION_DECLARATION) {
			handle_function_declaration(ir, node);
			continue;
		} else {
			return ir_error(ir, node, "Can't handle %s", ast_node_kind_str[node.kind]);
		}
	}
	return sym_eof();
}
