#include "main.h"

Reader reader_new(Compiler *lllc, char *file_name) {
	assert(lllc);
	assert(file_name);

	FILE *f = fopen(file_name, "r");
	if (f == NULL) {
		printf("Error: Can't open file: %s", file_name);
		exit(-1);
	}
	return (Reader) {
		.lllc = lllc,
		.file_name = file_name,
		.f = f,
		.line = 1,
		.column = 0,
	};
}

static char reader_getc_basic(Reader *reader) {
	assert(reader);

	int c = fgetc(reader->f);
	if (c == EOF) {
		return '\0';
	} else {
		return c;
	}
}

static char reader_getc_read_ungetted_if_is_there(Reader *reader) {
	assert(reader);

	if (reader->unread_character) {
		char c = reader->unread_character;
		reader->unread_character = '\0';
		return c;
	} else {
		return reader_getc_basic(reader);
	}
}

static char reader_getc_save_previous_line_size(Reader *reader) {
	assert(reader);

	char c = reader_getc_read_ungetted_if_is_there(reader);
	if (c == '\n') {
		reader->previous_line_size = reader->column;
	}
	return c;
}

static char reader_getc_update_line_column(Reader *reader) {
	assert(reader);

	char c = reader_getc_save_previous_line_size(reader);
	if (c == '\n') {
		reader->line++;
		reader->column = 0;
	} else if (c != '\0') {
		reader->column++;
	}
	return c;
}

char reader_getc(Reader *reader) {
	assert(reader);

	return reader_getc_update_line_column(reader);
}

static void reader_ungetc_basic(Reader *reader, char c) {
	assert(reader);
	assert(reader->unread_character == '\0');

	reader->unread_character = c;
}

static void reader_ungetc_revert_column_on_lew_line(Reader *reader, char c) {
	assert(reader);

	reader_ungetc_basic(reader, c);
	// If we are just after newline - revert column at previous line
	if (reader->column == 0) {
		assert(reader->line > 1);
		reader->line--;
		reader->column = reader->previous_line_size;
	} else {
		reader->column--;
	}
}

void reader_ungetc(Reader *reader, char c) {
	assert(reader);

	return reader_ungetc_revert_column_on_lew_line(reader, c);
}

void reader_memorize_position(Reader *reader) {
	assert(reader);

	reader->saved_line = reader->line;
	reader->saved_column = reader->column;
}
