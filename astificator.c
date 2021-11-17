#include "main.h"

bool node_is_eof(AstNode node) {
	return node.kind == AST_EOF;
}

bool node_is_list_close(AstNode node) {
	return node.kind == AST_LIST_CLOSE;
}

static void astificator_error(const Astificator *astificator, Token token, const char *fmt, ...) {
    va_list args;
    printf("%s:%d:%d: Error: ", astificator->tokenizer->reader.file_name, token.line, token.column);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    exit(-1);
}

static char *copy_identifier_string(Token token) {
	char *string = cvec_char_new(cvec_char_size(&token.identifier));
	cvec_char_assign_range(&string, token.identifier, token.identifier + cvec_char_size(&token.identifier));
	return string;
}

static char *copy_string_string(Token token) {
	char *string = cvec_char_new(cvec_char_size(&token.string));
	cvec_char_assign_range(&string, token.string, token.string + cvec_char_size(&token.string));
	return string;
}

Astificator astificator_new(Compiler *lllc, Tokenizer *tokenizer) {
	Astificator astificator = {};
	astificator.tokenizer = tokenizer;
	return astificator;
}

static AstNode ast_eof() {
	return (AstNode) {
		.kind = AST_EOF,
	};
}

static AstNode ast_integer(Token token) {
	return (AstNode) {
		.kind = AST_INTEGER,
		.line = token.line,
		.column = token.column,
		.integer = token.value,
	};
}

static AstNode ast_string(Token token) {
	char *string = copy_string_string(token);
	return (AstNode) {
		.kind = AST_NAME,
		.line = token.line,
		.column = token.column,
		.name = string,
	};
}

static AstNode ast_name(Token token) {
	char *name = copy_identifier_string(token);
	return (AstNode) {
		.kind = AST_NAME,
		.line = token.line,
		.column = token.column,
		.name = name,
	};
}

static AstNode ast_type(Token token) {
	char *name = copy_identifier_string(token);
	return (AstNode) {
		.kind = AST_TYPE,
		.line = token.line,
		.column = token.column,
		.name = name,
	};
}

static AstNode ast_close_list(Token token) {
	return (AstNode) {
		.kind = AST_LIST_CLOSE,
		.line = token.line,
		.column = token.column,
	};
}

static AstNode ast_declaration_list(AstNode *declarations) {
	return (AstNode) {
		.kind = AST_DECLARATION_LIST,
		.line = declarations[0].line,
		.column = declarations[0].column,
		.nodes = declarations,
	};
}

static AstNode ast_declaration(AstNode name, AstNode type) {
	AstNode *nodes = cvec_AstNode_new(2);
	cvec_AstNode_push_back(&nodes, name);
	cvec_AstNode_push_back(&nodes, type);
	return (AstNode) {
		.kind = AST_DECLARATION,
		.line = name.line,
		.column = name.column,
		.nodes = nodes,
	};
}

static AstNode ast_function_declaration(AstNode name, AstNode arguments, AstNode type) {
	AstNode *nodes = cvec_AstNode_new(3);
	cvec_AstNode_push_back(&nodes, name);
	cvec_AstNode_push_back(&nodes, arguments);
	cvec_AstNode_push_back(&nodes, type);
	return (AstNode) {
		.kind = AST_FUNCTION_DECLARATION,
		.line = name.line,
		.column = name.column,
		.nodes = nodes,
	};
}

static AstNode ast_function_definition(AstNode name, AstNode arguments, AstNode type, AstNode body) {
	AstNode definition = ast_function_declaration(name, arguments, type);
	cvec_AstNode_push_back(&definition.nodes, body);
	definition.kind = AST_FUNCTION_DEFINITION;
	return definition;
}

static AstNode ast_import(AstNode symbol_name, AstNode imported_name, AstNode dll_name) {
	AstNode *nodes = cvec_AstNode_new(3);
	cvec_AstNode_push_back(&nodes, symbol_name);
	cvec_AstNode_push_back(&nodes, imported_name);
	cvec_AstNode_push_back(&nodes, dll_name);
	return (AstNode) {
		.kind = AST_IMPORT,
		.line = symbol_name.line,
		.column = symbol_name.column,
		.nodes = nodes,
	};
}

static AstNode ast_function_call_list(AstNode *calls) {
	return (AstNode) {
		.kind = AST_FUNCTION_CALL_LIST,
		.line = 999,
		.column = 999,
		.nodes = calls,
	};
}

static AstNode ast_function_call(AstNode name, AstNode *arguments) {
	return (AstNode) {
		.kind = AST_FUNCTION_CALL,
		.line = name.line,
		.column = name.column,
		.name = name.name,
		.nodes = arguments,
	};
}

static void astificator_token_expect_kind(Astificator *astificator, Token token, TokenKind kind) {
	if (token.kind != kind) {
		astificator_error(astificator, token, "Expected %s, not %s", token_kind_str[kind], token_kind_str[token.kind]);
	}
}

static void astificator_token_expect_lparen(Astificator *astificator, Token token) {
	astificator_token_expect_kind(astificator, token, TOK_LPAREN);
}

static void astificator_token_expect_integer(Astificator *astificator, Token token) {
	astificator_token_expect_kind(astificator, token, TOK_INT);
}

static Token astificator_next_token(Astificator *astificator) {
	Token token = tokenizer_next_token(astificator->tokenizer);
	return token;
}

static Token astificator_next_token_expect_string(Astificator *astificator) {
	Token token = astificator_next_token(astificator);
	astificator_token_expect_kind(astificator, token, TOK_STRING);
	return token;
}

static Token astificator_next_token_expect_integer(Astificator *astificator) {
	Token token = astificator_next_token(astificator);
	astificator_token_expect_kind(astificator, token, TOK_INT);
	return token;
}

static Token astificator_next_token_expect_identifier(Astificator *astificator) {
	Token token = astificator_next_token(astificator);
	astificator_token_expect_kind(astificator, token, TOK_IDENTIFIER);
	return token;
}

static Token astificator_next_token_expect_lparen(Astificator *astificator) {
	Token token = astificator_next_token(astificator);
	astificator_token_expect_kind(astificator, token, TOK_LPAREN);
	return token;
}

static Token astificator_next_token_expect_rparen(Astificator *astificator) {
	Token token = astificator_next_token(astificator);
	astificator_token_expect_kind(astificator, token, TOK_RPAREN);
	return token;
}

static AstNode astificator_handle_number(Astificator *astificator) {
	assert(astificator);

	Token first_token = astificator_next_token(astificator);
	if (token_is_rparen(first_token)) {
		return ast_close_list(first_token);
	}
	astificator_token_expect_integer(astificator, first_token);
	return ast_integer(first_token);
}

static AstNode astificator_handle_string(Astificator *astificator) {
	assert(astificator);

	Token token = astificator_next_token_expect_string(astificator);
	return ast_string(token);
}

static AstNode astificator_handle_name(Astificator *astificator) {
	Token token = astificator_next_token_expect_identifier(astificator);
	return ast_name(token);
}

static AstNode astificator_handle_type(Astificator *astificator) {
	Token token = astificator_next_token_expect_identifier(astificator);
	return ast_type(token);
}

static AstNode astificator_handle_declaration(Astificator *astificator) {
	Token first_token = astificator_next_token(astificator);
	if (token_is_rparen(first_token)) {
		return ast_close_list(first_token);
	}
	astificator_token_expect_lparen(astificator, first_token);
	AstNode name = astificator_handle_name(astificator);
	AstNode type = astificator_handle_type(astificator);
	Token last_token = astificator_next_token_expect_rparen(astificator);
	return ast_declaration(name, type);
}

static AstNode astificator_handle_declaration_list(Astificator *astificator) {
	Token first_token = astificator_next_token_expect_lparen(astificator);
	AstNode *declarations = cvec_AstNode_new(4);
	for (AstNode node; node = astificator_handle_declaration(astificator), true;) {
		if (node_is_eof(node)) {
			astificator_error(astificator, first_token, "Unclosed declaration list");
			return ast_eof();
		}
		if (node_is_list_close(node)) {
			break;
		}
		cvec_AstNode_push_back(&declarations, node);
	}
	return ast_declaration_list(declarations);
}

static AstNode astificator_handle_function_call(Astificator *astificator) {
	Token first_token = astificator_next_token(astificator);
	if (token_is_rparen(first_token)) {
		return ast_close_list(first_token);
	}
	astificator_token_expect_lparen(astificator, first_token);
	AstNode name = astificator_handle_name(astificator);
	AstNode *arguments = cvec_AstNode_new(2);
	for (AstNode node; node = astificator_handle_number(astificator), true;) {
		if (node_is_eof(node)) {
			astificator_error(astificator, first_token, "Unclosed function body");
			return ast_eof();
		}
		if (node_is_list_close(node)) {
			break;
		}
		cvec_AstNode_push_back(&arguments, node);
	}
	return ast_function_call(name, arguments);
}

static AstNode astificator_handle_function_body(Astificator *astificator, Token first_token) {
	AstNode *list = cvec_AstNode_new(8);
	for (AstNode node; node = astificator_handle_function_call(astificator), true;) {
		if (node_is_eof(node)) {
			astificator_error(astificator, first_token, "Unclosed function body");
			return ast_eof();
		}
		if (node_is_list_close(node)) {
			break;
		}
		cvec_AstNode_push_back(&list, node);
	}
	return ast_function_call_list(list);
}

static AstNode astificator_handle_function(Astificator *astificator) {
	Token name_token = astificator_next_token_expect_identifier(astificator);
	AstNode name = ast_name(name_token);
	AstNode arguments = astificator_handle_declaration_list(astificator);
	AstNode type = astificator_handle_type(astificator);
	Token next_token = astificator_next_token(astificator);
	if (token_is_rparen(next_token)) {
		return ast_function_declaration(name, arguments, type);
	}
	AstNode body = astificator_handle_function_body(astificator, next_token);
	astificator_next_token_expect_rparen(astificator);
	return ast_function_definition(name, arguments, type, body);
}

static AstNode astificator_handle_import(Astificator *astificator) {
	assert(astificator);

	AstNode symbol_name = astificator_handle_name(astificator);
	AstNode imported_name = astificator_handle_string(astificator);
	AstNode dll_name = astificator_handle_string(astificator);
	Token closing_parentheses = astificator_next_token_expect_rparen(astificator);
	return ast_import(symbol_name, imported_name, dll_name);
}

AstNode astificator_next_list(Astificator *astificator) {
	Token first_token = astificator_next_token(astificator);
	if (token_is_eof(first_token)) {
		return ast_eof();
	}
	astificator_token_expect_lparen(astificator, first_token);
	Token second_token = astificator_next_token_expect_identifier(astificator);
	if (token_identifier_is(second_token, "function")) {
		return astificator_handle_function(astificator);
	} else if (token_identifier_is(second_token, "import")) {
		return astificator_handle_import(astificator);
	}
	astificator_error(astificator, second_token, "Only function declaration and definition is implemented");
	return ast_eof();
}
