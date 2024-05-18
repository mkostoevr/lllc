#include "main.h"

bool token_is_eof(Token token) {
	return token.kind == TOK_EOF;
}

bool token_is_lparen(Token token) {
	return token.kind == TOK_LPAREN;
}

bool token_is_rparen(Token token) {
	return token.kind == TOK_RPAREN;
}

bool token_is_identifier(Token token) {
	return token.kind == TOK_IDENTIFIER;
}

bool token_is_integer(Token token) {
	return token.kind == TOK_INT;
}

bool token_is_string(Token token) {
	return token.kind == TOK_STRING;
}

bool token_identifier_is(Token token, const char *str) {
	assert(str);
	assert(token.identifier);

	return cvec_char_size(&token.identifier) == strlen(str)
		   && !memcmp(token.identifier, str, strlen(str));
}

static Token token_eof(Reader *reader) {
	assert(reader);

	return (Token) {
		.kind = TOK_EOF,
		.line = reader->line,
		.column = reader->column,
	};
}

static Token token_lparen(Reader *reader) {
	assert(reader);

	return (Token) {
		.kind = TOK_LPAREN,
		.line = reader->line,
		.column = reader->column,
	};
}

static Token token_rparen(Reader *reader) {
	assert(reader);

	return (Token) {
		.kind = TOK_RPAREN,
		.line = reader->line,
		.column = reader->column,
	};
}

static Token token_identifier(Reader *reader, char *identifier) {
	assert(reader);

	return (Token) {
		.kind = TOK_IDENTIFIER,
		.line = reader->saved_line,
		.column = reader->saved_column,
		.identifier = identifier,
	};
}

static Token token_string(Reader *reader, char *string) {
	assert(reader);

	return (Token) {
		.kind = TOK_STRING,
		.line = reader->saved_line,
		.column = reader->saved_column,
		.string = string,
	};
}

static Token token_integer(Reader *reader, long value) {
	assert(reader);

	return (Token) {
		.kind = TOK_INT,
		.line = reader->saved_line,
		.column = reader->saved_column,
		.value = value,
	};
}

Tokenizer tokenizer_new(Compiler *lllc, char *file_name) {
	assert(lllc);
	assert(file_name);

	Tokenizer tokenizer;
	tokenizer.reader = reader_new(lllc, file_name);
	tokenizer.lookahead = cvec_Token_new(1);
	return tokenizer;
}

static Token tokenizer_generate_next_token(Tokenizer *tokenizer) {
	assert(tokenizer);

	Reader *reader = &tokenizer->reader;
	for (char c = reader_getc(&tokenizer->reader); c; c = reader_getc(reader)) {
		if (is_space(c)) {
			continue;
		} else if (c == '(') {
			return token_lparen(reader);
		} else if (c == ')') {
			return token_rparen(reader);
		} else if (is_digit(c)) {
			reader_memorize_position(reader);
			char *number_string_buffer = cvec_char_new(32);
			while (is_digit(c)) {
				cvec_char_push_back(&number_string_buffer, c);
				c = reader_getc(reader);
			}
			cvec_char_push_back(&number_string_buffer, '\0');
			reader_ungetc(reader, c);
			long value = strtol(number_string_buffer, NULL, 0);
			cvec_char_free(&number_string_buffer);
			return token_integer(reader, value);
		} else if (is_id_begin(c)) {
			reader_memorize_position(reader);
			char *identifier = cvec_char_new(32);
			while (is_id(c)) {
				cvec_char_push_back(&identifier, c);
				c = reader_getc(reader);
			}
			reader_ungetc(reader, c);
			return token_identifier(reader, identifier);
		} else if (c == '"') {
			reader_memorize_position(reader);
			char *string = cvec_char_new(32);
			char prev = '\0';
			// Get first string character
			char c = reader_getc(reader);
			while (c && (c != '"' || prev == '\\')) {
				cvec_char_push_back(&string, c);
				prev = c;
				c = reader_getc(reader);
			}
			if (c == '\0') {
				printf("Error: Unclosed string");
			}
			return token_string(reader, string);
		} else {
			printf("Error at %s:%d:%d: '%c' character", reader->file_name, reader->line, reader->column, c);
			exit(-1);
		}
	}
	return token_eof(reader);
}

Token tokenizer_next_token(Tokenizer *tokenizer) {
	if (cvec_Token_size(&tokenizer->lookahead) > 0) {
		return cvec_Token_pop_front(&tokenizer->lookahead);
	}
	return tokenizer_generate_next_token(tokenizer);
}

Token tokenizer_peek_token(Tokenizer *tokenizer, int i) {
	while (i >= cvec_Token_size(&tokenizer->lookahead)) {
		cvec_Token_push_back(&tokenizer->lookahead, tokenizer_generate_next_token(tokenizer));
	}
	return tokenizer->lookahead[i];
}
