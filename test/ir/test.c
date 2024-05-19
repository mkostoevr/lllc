#define ONE_SOURCE
#include "../../main.h"

static void dump_type_function(FILE *out, Type type);
static void dump_value(FILE *out, Value value, int i);
static void dump_type(FILE *out, Type type);
static void dump_symbol(FILE *out, Symbol sym);

static void dump_type_function(FILE *out, Type type) {
	Type *arguments = type.types;
	Type return_type = type.types[0];

	fprintf(out, "function (");
	for (size_t i = 1; i < cvec_Type_size(&arguments); i++) {
		if (i != 1) {
			fprintf(out, ", ");
		}
		fprintf(out, "%.*s: ", cvec_char_size(&arguments[i].name), arguments[i].name);
		dump_type(out, arguments[i]);
	}
	fprintf(out, "): ");
	dump_type(out, return_type);
}

static void indent(FILE *out, int i) {
	while (i--) {
		fprintf(out, "    ");
	}
}

static void dump_value(FILE *out, Value value, int _i) {
	if (value.kind == VAL_UINT32) {
		fprintf(out, "%d", value.uvalue);
	} else if (value.kind == VAL_FUNCTION_CALL_RESULT) {
		fprintf(out, "%.*s(", cvec_char_size(&value.name), value.name);
		for (size_t i = 0; i < cvec_Value_size(&value.values); i++) {
			if (i != 0) {
				fprintf(out, ", ");
			}
			dump_value(out, value.values[i], _i + 1);
		}
		fprintf(out, ");\n");
	} else if (value.kind == VAL_FUNCTION_CALL_LIST) {
		fprintf(out, "{\n");
		for (size_t i = 0; i < cvec_Value_size(&value.values); i++) {
			indent(out, _i + 1);
			dump_value(out, value.values[i], _i + 1);
		}
		indent(out, _i);
		fprintf(out, "}");
	} else if (value.kind == VAL_IF) {
		Value condition = value.values[0];
		Value then_code = value.values[1];

		fprintf(out, "if (");
		dump_value(out, condition, _i);
		fprintf(out, ") ");
		dump_value(out, then_code, _i);
		if (cvec_Value_size(&value.values) == 3) {
			Value else_code = value.values[2];
			fprintf(out, " else ");
			dump_value(out, else_code, _i);
		}
		fprintf(out, "\n");
	} else {
		assert(("You should never get here", 0));
	}
}

static void dump_type(FILE *out, Type type) {
	if (type.kind == TYPE_FUNCTION) {
		dump_type_function(out, type);
	} else if (type.kind == TYPE_UINT32) {
		fprintf(out, "UInt32");
	} else if (type.kind == TYPE_VOID) {
		fprintf(out, "Void");
	} else {
		fprintf(stderr, "UNEXPECTED TYPE: %d\n", type.kind);
		exit(1);
	}
}

static void dump_symbol(FILE *out, Symbol sym) {
	const char *name = sym.name;

	if (sym.imported_name != NULL) {
		assert(sym.imported_name);
		assert(sym.dll_name);
		fprintf(out, "[\"%.*s\",", cvec_char_size(&sym.dll_name), sym.dll_name);
		fprintf(out, " \"%.*s\"] ", cvec_char_size(&sym.imported_name), sym.imported_name);
	}
	fprintf(out, "%.*s: ", cvec_char_size(&name), name);
	dump_type(out, sym.type);
	if (sym.value.kind != VAL_UNDEFINED) {
		fprintf(out, " = ");
		dump_value(out, sym.value, 0);
	}
	fprintf(out, "\n\n");
}

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s test.lll test.log\n", argv[0]);
		return -1;
	}
	FILE *out = fopen(argv[2], "wb");
	if (out == NULL) {
		fprintf(stderr, "Can't open the output file.\n");
		return -1;
	}
	Compiler lllc = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, argv[1]);
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	Ir ir = ir_new(&lllc, &astificator);
	for (Symbol sym; sym = ir_next_symbol(&ir), sym.type.kind != TYPE_EOF;) {
		dump_symbol(out, sym);
	}
	return 0;
}
