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
	for (size_t i = 0; i < cvec_Type_size(&arguments); i++) {
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

static Value val_uint32(unsigned long value) {
	return (Value) {
		.kind = VAL_UINT32,
		.uvalue = value,
	};
}

static Value val_string(AstNode node) {
	return (Value) {
		.kind = VAL_STRING,
		.name = node.name,
	};
}

static Value val_function_call_list(Value *function_calls) {
	return (Value) {
		.kind = VAL_FUNCTION_CALL_LIST,
		.values = function_calls,
	};
}

static Value val_function_call(char *function_name, Value *arguments) {
	return (Value) {
		.kind = VAL_FUNCTION_CALL_RESULT,
		.name = function_name,
		.values = arguments,
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

	if (!strcmp(node.name, "UInt32")) {
		return type_uint32();
	} else if (!strcmp(node.name, "Void")) {
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

static Value eval_value(Ir *ir, AstNode node) {
	assert(node.kind == AST_INTEGER
		|| node.kind == AST_NAME);

	if (node.kind == AST_INTEGER) {
		return val_uint32(node.integer);
	} else if (node.kind == AST_NAME) {
		return val_string(node);
	} else {
		assert(("You should never get here", 0));
	}
}

static Value eval_function_call(Ir *ir, AstNode node) {
	assert(node.kind == AST_FUNCTION_CALL);

	char *name = node.name;
	Value *arguments = cvec_Value_new(4);
	for (size_t i = 0; i < cvec_AstNode_size(&node.nodes); i++) {
		Value argument = eval_value(ir, node.nodes[i]);
		cvec_Value_push_back(&arguments, argument);
	}
	return val_function_call(name, arguments);
}

static Value eval_function_call_list(Ir *ir, AstNode node) {
	assert(node.kind == AST_FUNCTION_CALL_LIST);

	Value *function_calls = cvec_Value_new(8);
	for (size_t i = 0; i < cvec_AstNode_size(&node.nodes); i++) {
		Value function_call = eval_function_call(ir, node.nodes[i]);
		cvec_Value_push_back(&function_calls, function_call);
	}
	return val_function_call_list(function_calls);
}

static Symbol parse_function_declaration(Ir *ir, AstNode node) {
	assert(node.kind == AST_FUNCTION_DECLARATION
		|| node.kind == AST_FUNCTION_DEFINITION);

	char *name = node.nodes[0].name;
	Type *arguments = parse_declaration_list(ir, node.nodes[1]);
	Type return_type = parse_type(ir, node.nodes[2]);
	return sym_function(name, arguments, return_type);
}

static void handle_function_declaration(Ir *ir, AstNode node) {
	assert(node.kind == AST_FUNCTION_DECLARATION);

	Symbol func = parse_function_declaration(ir, node);
	cdict_CStr_Symbol_add_vv(&ir->symbol_table, func.name, func, CDICT_NO_CHECK);
}

static Symbol parse_function_definition(Ir *ir, AstNode node) {
	assert(node.kind == AST_FUNCTION_DEFINITION);

	Symbol func = parse_function_declaration(ir, node);
	func.value = eval_function_call_list(ir, node.nodes[3]);
	return func;
}

Symbol parse_import(Ir *ir, AstNode node) {
	assert(node.kind == AST_IMPORT);
	assert(cvec_AstNode_size(&node.nodes) == 3);
	assert(node.nodes[0].kind == AST_NAME);
	assert(node.nodes[1].kind == AST_NAME);
	assert(node.nodes[2].kind == AST_NAME);
	assert(node.nodes[0].name);
	assert(node.nodes[1].name);
	assert(node.nodes[2].name);

	char *symbol_name = node.nodes[0].name;
	char *imported_name = node.nodes[1].name;
	char *dll_name = node.nodes[2].name;
	Symbol symbol = cdict_CStr_Symbol_get_v(&ir->symbol_table, symbol_name);
	symbol.imported_name = imported_name;
	symbol.dll_name = dll_name;
	return symbol;
}

Symbol ir_next_symbol(Ir *ir) {
	for (AstNode node; node = astificator_next_list(ir->astificator), node.kind != AST_EOF;) {
		if (node.kind == AST_FUNCTION_DECLARATION) {
			handle_function_declaration(ir, node);
			continue;
		} else if (node.kind == AST_FUNCTION_DEFINITION) {
			return parse_function_definition(ir, node);
		} else if (node.kind == AST_IMPORT) {
			return parse_import(ir, node);
		} else {
			return ir_error(ir, node, "Can't handle %s", ast_node_kind_str[node.kind]);
		}
	}
	return sym_eof();
}
