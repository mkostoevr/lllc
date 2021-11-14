#include "main.h"

Token token_eof(Reader *reader) {
	assert(reader);

	return (Token) {
		.kind = TOK_EOF,
		.line = reader->line,
		.column = reader->column,
	};
}

Token token_lparen(Reader *reader) {
	return (Token) {
		.kind = TOK_LPAREN,
		.line = reader->line,
		.column = reader->column,
	};
}

Token token_rparen(Reader *reader) {
	return (Token) {
		.kind = TOK_RPAREN,
		.line = reader->line,
		.column = reader->column,
	};
}

Token token_identifier(Reader *reader, char *identifier) {
	return (Token) {
		.kind = TOK_IDENTIFIER,
		.line = reader->saved_line,
		.column = reader->saved_column,
		.identifier = identifier,
	};
}

Token token_string(Reader *reader, char *string) {
	return (Token) {
		.kind = TOK_STRING,
		.line = reader->saved_line,
		.column = reader->saved_column,
		.string = string,
	};
}

Token token_integer(Reader *reader, long value) {
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
	return tokenizer;
}

Token tokenizer_next_token(Tokenizer *tokenizer) {
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

Token *tokenize(Compiler *lllc, char *file_name) {
	assert(lllc);
	assert(file_name);

	Tokenizer tokenizer = tokenizer_new(lllc, file_name);
	Token *tokens = cvec_Token_new(1024);
	Token next_token = {};

	do {
		next_token = tokenizer_next_token(&tokenizer);
		cvec_Token_push_back(&tokens, next_token);
	} while (next_token.kind != TOK_EOF);

	return tokens;
}
