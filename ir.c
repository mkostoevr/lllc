#include "main.h"

static Type type_eof() {
	return (Type) { .kind = TYPE_EOF };
}

static Symbol sym_eof() {
	return (Symbol){ .type = type_eof() };
}

Ir ir_new(Compiler *lllc, Astificator *astificator) {
	Ir ir = { 0 };
	ir.lllc = lllc,
	ir.astificator = astificator;
	cdict_CStr_Symbol_init(&ir.symbol_table);
	return ir;
}

Symbol ir_next_symbol(Ir *ir) {
	// TODO: Parse function declarations and save them in symbol_table
	return sym_eof();
}
