#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
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

typedef enum {
	TOK_MIN = -1,
	#define ENTRY(token_name) TOK_ ## token_name
	#include "tokentypes.h"
	#undef ENTRY
	TOK_MAX
} TokenKind;

typedef struct {
	TokenKind kind;
	size_t line;
	size_t column;
	char *identifier;
	char *string;
	long value;
} Token;

Token *tokenize(Compiler *lllc, char *file_name);

#define CVEC_TYPE Token
#include "cvec/cvec.h"

#define CVEC_TYPE char
#include "cvec/cvec.h"

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

#ifdef ONE_SOURCE
#	include "reader.c"
#	include "ctype.c"
#	include "tokenizer.c"
#	include "cvec_inst.c"
#endif
