#include "main.h"

bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

bool is_id_begin(char c) {
	return is_alpha(c) || c == '_';
}

bool is_id(char c) {
	return is_alpha(c) || is_digit(c) || c == '_';
}

bool is_space(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
