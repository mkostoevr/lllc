#define ONE_SOURCE
#include "../../main.h"

static void dump_symbol(Symbol sym);
static void dump_type(Type type);
static void dump_type_function(Type type);

static void dump_type_function(Type type) {
	Type *arguments = type.types;
	Type return_type = type.types[0];

	printf("function (");
	for (size_t i = 1; i < cvec_Type_size(&arguments); i++) {
		if (i != 1) {
			printf(", ");
		}
		printf("%.*s: ", cvec_char_size(&arguments[i].name), arguments[i].name);
		dump_type(arguments[i]);
	}
	printf("): ");
	dump_type(return_type);
}

static void dump_type(Type type) {
	if (type.kind == TYPE_FUNCTION) {
		dump_type_function(type);
	} else if (type.kind == TYPE_UINT32) {
		printf("UInt32");
	} else if (type.kind == TYPE_VOID) {
		printf("Void");
	} else {
		printf("UNEXPECTED TYPE: %d\n", type.kind);
		exit(1);
	}
}

static void dump_symbol(Symbol sym) {
	const char *name = sym.name;

	if (sym.imported_name != NULL) {
		assert(sym.imported_name);
		assert(sym.dll_name);
		printf("[\"%.*s\",", cvec_char_size(&sym.dll_name), sym.dll_name);
		printf(" \"%.*s\"] ", cvec_char_size(&sym.imported_name), sym.imported_name);
	}
	printf("%.*s: ", cvec_char_size(&name), name);
	dump_type(sym.type);
	printf("\n");
}

int main(int argc, char **argv) {
	Compiler lllc = {};
	Tokenizer tokenizer = tokenizer_new(&lllc, argv[1]);
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	Ir ir = ir_new(&lllc, &astificator);
	for (Symbol sym; sym = ir_next_symbol(&ir), sym.type.kind != TYPE_EOF;) {
		dump_symbol(sym);
	}
	return 0;
}