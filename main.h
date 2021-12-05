#pragma once

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
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

bool token_is_eof(Token token);
bool token_is_lparen(Token token);
bool token_is_rparen(Token token);
bool token_is_identifier(Token token);
bool token_is_integer(Token token);
bool token_is_string(Token token);
bool token_identifier_is(Token token, const char *str);

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
	// NAME: name
	// TODO: What about TYPE?
	// FUNCTION_CALL: callee name
	char *name;
	long integer;
	// FUNCTION_DECLARATION:
	//   name: NAME
	//   arguments: DECLARATION_LIST
	//   type: TYPE
	// FUNCTION_DEFINITION:
	//   name: NAME
	//   arguments: DECLARATION_LIST
	//   type: TYPE
	//   body: FUNCTION_CALL_LIST
	// DECLARATION_LIST:
	//   array of { declaration: DECLARATION }
	// DECLARATION:
	//   name: NAME
	//   type: TYPE
	// FUNCTION_CALL_LIST:
	//   array of { function_call: FUNCTION_CALL }
	// FUNCTION_CALL:
	//   array of { argument: whatever expression }
	// IMPORT:
	//   symbol_name: NAME
	//   imported_name: NAME
	//   dll_name: NAME
	struct AstNode *nodes;
} AstNode;

typedef struct Astificator {
	Tokenizer *tokenizer;
} Astificator;

Astificator astificator_new(Compiler *lllc, Tokenizer *tokenizer);
AstNode astificator_next_list(Astificator *astificator);

#define CVEC_TYPE AstNode
#include "cvec/cvec.h"

typedef enum {
	TYPE_FUNCTION,
	TYPE_UINT32,
	TYPE_VOID,
	TYPE_EOF,
} TypeKind;

typedef struct Type {
	TypeKind kind;
	const char *name;
	// TYPE_FUNCTION:
	//   returnType[, argumentTypes...]
	struct Type *types;
} Type;

typedef enum {
	VAL_UNDEFINED,
	VAL_FUNCTION_CALL_LIST,
	VAL_FUNCTION_CALL_RESULT,
	VAL_UINT32,
} ValueKind;

// VAL_FUNCTION_CALL_RESULT
//   name: name of the function
//   values: function call arguments
// VAL_FUNCTION_CALL_LIST
//   values: function call results
// VAL_UINT32:
//   uvalue: value of the integer
typedef struct Value {
	ValueKind kind;
	char *name;
	struct Value *values;
	unsigned long uvalue;
} Value;

typedef struct Symbol {
	char *name;
	const char *imported_name;
	const char *dll_name;
	Type type;
	Value value;
} Symbol;

#define CDICT_VAL_T Symbol
#define CDICT_INST
#include "cdict/cdict.h"

#define CVEC_TYPE Type
#include "cvec/cvec.h"

#define CVEC_TYPE Value
#include "cvec/cvec.h"

typedef struct Ir {
	Compiler *lllc;
	Astificator *astificator;
	CDict_CStr_Symbol symbol_table;
} Ir;

Ir ir_new(Compiler *lllc, Astificator *astificator);
Symbol ir_next_symbol(Ir *ir);

#ifdef ONE_SOURCE
#	include "reader.c"
#	include "ctype.c"
#	include "tokenizer.c"
#	include "astificator.c"
#	include "cvec_inst.c"
#	include "ir.c"
#endif
