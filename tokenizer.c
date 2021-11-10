#include "main.h"

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

Token *tokenize(Compiler *lllc, char *file_name) {
	Reader reader = reader_new(lllc, file_name);
	Token *tokens = cvec_Token_new(1024);
	char *number_string_buffer = cvec_char_new(32);

	for (char c = reader_getc(&reader); c; c = reader_getc(&reader)) {
		if (is_space(c)) {
			continue;
		} else if (c == '(') {
			cvec_Token_push_back(&tokens, token_lparen(&reader));
		} else if (c == ')') {
			cvec_Token_push_back(&tokens, token_rparen(&reader));
		} else if (is_digit(c)) {
			reader_memorize_position(&reader);
			while (is_digit(c)) {
				cvec_char_push_back(&number_string_buffer, c);
				c = reader_getc(&reader);
			}
			reader_ungetc(&reader, c);
			long value = strtol(number_string_buffer, NULL, 0);
			cvec_char_resize(&number_string_buffer, 0);
			cvec_Token_push_back(&tokens, token_integer(&reader, value));
		} else if (is_id_begin(c)) {
			reader_memorize_position(&reader);
			char *identifier = cvec_char_new(32);
			while (is_id(c)) {
				cvec_char_push_back(&identifier, c);
				c = reader_getc(&reader);
			}
			reader_ungetc(&reader, c);
			cvec_Token_push_back(&tokens, token_identifier(&reader, identifier));
		} else if (c == '"') {
			reader_memorize_position(&reader);
			char *string = cvec_char_new(32);
			char prev = '\0';
			// Get first string character
			char c = reader_getc(&reader);
			while (c && (c != '"' || prev == '\\')) {
				cvec_char_push_back(&string, c);
				prev = c;
				c = reader_getc(&reader);
			}
			if (c == '\0') {
				printf("Error: Unclosed string");
			}
			cvec_Token_push_back(&tokens, token_string(&reader, string));
		} else {
			printf("Error at %s:%d:%d: '%c' character", reader.file_name, reader.line, reader.column, c);
			exit(-1);
		}
	}

	return tokens;
}
