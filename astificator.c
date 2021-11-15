#include "main.h"

static void astificator_error(const Astificator *astificator, Token token, const char *fmt, ...) {
    va_list args;
    printf("%s:%d:%d: Error: ", astificator->tokenizer->reader.file_name, token.line, token.column);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

static char *copy_identifier_string(Token token) {
	char *string = cvec_char_new(cvec_char_size(&token.identifier));
	cvec_char_assign_range(&string, token.identifier, token.identifier + cvec_char_size(&token.identifier));
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

static Token astificator_next_token(Astificator *astificator) {
	Token token = tokenizer_next_token(astificator->tokenizer);
	return token;
}

static AstNode astificator_handle_name(Astificator *astificator) {
	Token token = astificator_next_token(astificator);
	if (token.kind != TOK_IDENTIFIER) {
		astificator_error(astificator, token, "Identifier expected");
		return ast_eof();
	}
	return ast_name(token);
}

static AstNode astificator_handle_type(Astificator *astificator) {
	Token token = astificator_next_token(astificator);
	if (token.kind != TOK_IDENTIFIER) {
		astificator_error(astificator, token, "Type name expected");
		return ast_eof();
	}
	return ast_type(token);
}

static AstNode astificator_handle_declaration(Astificator *astificator) {
	Token first_token = astificator_next_token(astificator);
	if (first_token.kind == TOK_RPAREN) {
		return ast_close_list(first_token);
	}
	if (first_token.kind != TOK_LPAREN) {
		astificator_error(astificator, first_token, "Declarations should be in parentheses, like so: (name Type)");
		return ast_eof();
	}
	AstNode name = astificator_handle_name(astificator);
	AstNode type = astificator_handle_type(astificator);
	Token last_token = astificator_next_token(astificator);
	if (last_token.kind != TOK_RPAREN) {
		astificator_error(astificator, last_token, "Expected ')' at the end of declaration");
	}
	return ast_declaration(name, type);
}

static AstNode astificator_handle_declaration_list(Astificator *astificator) {
	Token first_token = astificator_next_token(astificator);
	if (first_token.kind != TOK_LPAREN) {
		astificator_error(astificator, first_token, "Declaration list should start with '('");
		return ast_eof();
	}
	AstNode node = {};
	AstNode *declarations = cvec_AstNode_new(4);
	do {
		node = astificator_handle_declaration(astificator);
		if (node.kind == AST_LIST_CLOSE) {
			break;
		}
		cvec_AstNode_push_back(&declarations, node);
	} while (node.kind != AST_EOF);
	if (node.kind == AST_EOF) {
		astificator_error(astificator, first_token, "Unclosed declaration list");
		return ast_eof();
	}
	return ast_declaration_list(declarations);
}

static AstNode astificator_handle_function(Astificator *astificator) {
	Token name_token = astificator_next_token(astificator);
	if (name_token.kind != TOK_IDENTIFIER) {
		astificator_error(astificator, name_token, "Expected function name, not %s", token_kind_str[name_token.kind]);
		return ast_eof();
	}
	AstNode name = ast_name(name_token);
	AstNode arguments = astificator_handle_declaration_list(astificator);
	AstNode type = astificator_handle_type(astificator);
	Token next_token = astificator_next_token(astificator);
	if (next_token.kind == TOK_RPAREN) {
		return ast_function_declaration(name, arguments, type);
	}
	astificator_error(astificator, next_token, "Function definition isn't implemented");
	return ast_eof();
}

static AstNode astificator_handle_root_list(Astificator *astificator) {
	Token first_token = astificator_next_token(astificator);
	if (first_token.kind == TOK_IDENTIFIER) {
		if (!memcmp(first_token.identifier, "function", strlen("function"))) {
			return astificator_handle_function(astificator);
		}
	}
	astificator_error(astificator, first_token, "Only function declaration and definition is implemented");
	return ast_eof();
}

AstNode astificator_next_node(Astificator *astificator) {
	assert(astificator);

	Token first_token = astificator_next_token(astificator);
	if (first_token.kind == TOK_LPAREN) {
		return astificator_handle_root_list(astificator);
	} else if (first_token.kind == TOK_EOF) {
		return ast_eof();
	}
	astificator_error(astificator, first_token, "Root of file can only contain a list");
	return ast_eof();
}
