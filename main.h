#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool is_alpha(char c);
bool is_digit(char c);
bool is_id_begin(char c);
bool is_id(char c);
bool is_space(char c);

typedef struct {
	// Generic stuff for current compiler instance
	size_t something;
} Compiler;

Compiler compiler_new();

typedef struct {
	// Public stuff
	const char *file_name;
	size_t line;
	size_t column;
	size_t saved_line;
	size_t saved_column;
	// Internal stuff
	Compiler *lllc;
	FILE *f;
	size_t previous_line_size;
	char unread_character;
} Reader;

Reader reader_new(Compiler *lllc, const char *file_name);
char reader_getc(Reader *reader);
void reader_ungetc(Reader *reader, char c);
void reader_memorize_position(Reader *reader);

typedef enum {
	TOK_MIN = -1,
	#define ENTRY(token_name) TOK_ ## token_name
	#include "tokentypes.h"
	#undef ENTRY
	TOK_MAX
} TokenKind;

static char *token_kind_str[TOK_MAX] = {
	#define ENTRY_STR(token_name) #token_name
	#define ENTRY(token_name) ENTRY_STR(TOK_ ## token_name)
	#include "tokentypes.h"
	#undef ENTRY
	#undef ENTRY_STR
};

typedef struct {
	TokenKind kind;
	size_t line;
	size_t column;
	char *identifier;
	char *string;
	long value;
} Token;

typedef struct {
	Reader reader;
} Tokenizer;

Tokenizer tokenizer_new(Compiler *lllc, char *filename);
Token tokenizer_next_token(Tokenizer *tokenizer);
Token *tokenize(Compiler *lllc, char *file_name);

#define CVEC_TYPE Token
#include "cvec/cvec.h"

#define CVEC_TYPE char
#include "cvec/cvec.h"

typedef enum {
	AST_MIN = -1,
	#define ENTRY(name) AST_ ## name
	#include "asttypes.h"
	#undef ENTRY
	AST_MAX
} AstNodeKind;

static char *ast_node_kind_str[AST_MAX] = {
	#define ENTRY_STR(name) #name
	#define ENTRY(name) ENTRY_STR(AST_ ## name)
	#include "asttypes.h"
	#undef ENTRY
	#undef ENTRY_STR
};

typedef struct AstNode {
	AstNodeKind kind;
	size_t line;
	size_t column;
	char *name;
	struct AstNode *nodes;
} AstNode;

typedef struct Astificator {
	Tokenizer *tokenizer;
} Astificator;

Astificator astificator_new(Compiler *lllc, Tokenizer *tokenizer);
AstNode astificator_next_node(Astificator *astificator);

#define CVEC_TYPE AstNode
#include "cvec/cvec.h"

#ifdef ONE_SOURCE
#	include "reader.c"
#	include "ctype.c"
#	include "tokenizer.c"
#	include "astificator.c"
#	include "cvec_inst.c"
#endif
