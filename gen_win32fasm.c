#include "main.h"

typedef struct {
	char *symbol_name;
	const char *imported_name;
	const char *dll_name;
} Import;

#define CVEC_TYPE Import
#define CVEC_INST
#include "cvec/cvec.h"

typedef struct {
	char *code;
	char *data;
	Import *imports;
} Output;

static unsigned long unique_id = 0;

static void outf(char **output, const char *fmt, ...) {
	assert(output);
	assert(*output);
	assert(fmt);

	va_list args;
	va_start(args, fmt);
	size_t bufsz = vsnprintf(NULL, 0, fmt, args);
	char *buf = not_null(calloc(1, 1 + bufsz));
	if (vsnprintf(buf, bufsz, fmt, args) > bufsz) {
		printf("Internal error: printf buffer overflow");
		exit(-1);
	}
	va_end(args);
	for (size_t i = 0; i < strlen(buf); i++) {
		cvec_char_push_back(output, buf[i]);
	}
	free(buf);
}

static void gen_symbol(Symbol sym, Output *output);

static int create_string_data(Output *output, char *string) {
	assert(output);
	assert(string);

	int id = unique_id++;
	outf(&output->data, "  __string__%d db '%.*s',0\n", id, cvec_char_size(&string), string);
	return id;
}

static void gen_value(Output *s, Value value, char **output) {
	assert(output);
	assert(*output);

	if (value.kind == VAL_UINT32) {
		outf(output, "  push %d\n", value.uvalue);
	} else if (value.kind == VAL_STRING) {
		outf(output, "  push __string__%d\n", create_string_data(s, value.name));
	} else if (value.kind == VAL_FUNCTION_CALL_RESULT) {
		for (size_t i = 0; i < cvec_Value_size(&value.values); i++) {
			gen_value(s, value.values[i], output);
		}
		outf(output, "  call [%.*s]\n\n", cvec_char_size(&value.name), value.name);
	} else if (value.kind == VAL_FUNCTION_CALL_LIST) {
		for (size_t i = 0; i < cvec_Value_size(&value.values); i++) {
			gen_value(s, value.values[i], output);
		}
	} else if (value.kind == VAL_IF) {
		Value condition = value.values[0];
		unsigned long if_id = unique_id++;
		gen_value(s, condition, output);
		outf(output, "  pop eax\n");
		outf(output, "  test eax, eax\n");
		outf(output, "  jz .if%d.else\n\n", if_id);
		Value then_code = value.values[1];
		gen_value(s, then_code, output);
		outf(output, "  jmp .if%d.end\n\n", if_id);
		outf(output, ".if%d.else:\n", if_id);
		if (cvec_Value_size(&value.values) == 3) {
			Value else_code = value.values[2];
			gen_value(s, else_code, output);
		}
		outf(output, ".if%d.end:\n", if_id);
	} else {
		printf("Unexpected value kind: %d\n", value.kind);
		exit(-1);
	}
}

static void gen_function(Symbol sym, Output *output) {
	assert(output);

	char **output_code = &output->code;
	const char *name = not_null(sym.name);
	outf(output_code, "%.*s:\n", (int)cvec_char_size(&name), name);
	gen_value(output, sym.value, output_code);
	outf(output_code, "\n");
}

static void gen_imported_symbol(Symbol sym, Output *output) {
	assert(output);
	assert(sym.name);
	assert(sym.imported_name);
	assert(sym.dll_name);

	cvec_Import_push_back(&output->imports, (Import) {
		.symbol_name = sym.name,
		.imported_name = sym.imported_name,
		.dll_name = sym.dll_name,
	});
}

static void gen_symbol(Symbol sym, Output *output) {
	assert(sym.name);
	assert(output);

	if (sym.imported_name != NULL) {
		gen_imported_symbol(sym, output);
	} else if (sym.type.kind == TYPE_FUNCTION) {
		gen_function(sym, output);
	} else {
		printf("Only function and import generation is implemented!\n");
		exit(1);
	}
}

static int lib_exists(const char **libs, const char *dll_name) {
	assert(libs);
	assert(dll_name);

	for (size_t i = 0; i < cvec_pcchar_size(&libs); i++) {
		const char *next_dll_name = not_null(libs[i]);
		if (!strcmp(next_dll_name, dll_name)) {
			return 1;
		}
	}
	return 0;
}

static char *gen_idata(Output *output) {
	assert(output);

	char *idata = not_null(cvec_char_new(1024));
	const char **libs = not_null(cvec_pcchar_new(4));

	for (size_t i = 0; i < cvec_Import_size(&output->imports); i++) {
		const char *dll_name = not_null(output->imports[i].dll_name);
		if (!lib_exists(libs, dll_name)) {
			cvec_pcchar_push_back(&libs, dll_name);
		}
	}
	// Libraries
	for (size_t i = 0; i < cvec_pcchar_size(&libs); i++) {
		const char *dll_name = not_null(libs[i]);
		outf(&idata, "dd 0,0,0,RVA %.*s_name,RVA %.*s_table\n",
			cvec_char_size(&dll_name), dll_name,
			cvec_char_size(&dll_name), dll_name);
	}
	outf(&idata, "dd 0,0,0,0,0\n\n");
	// Per-library tables
	for (size_t i = 0; i < cvec_pcchar_size(&libs); i++) {
		const char *dll_name = not_null(libs[i]);
		outf(&idata, "%.*s_table:\n", cvec_char_size(&dll_name), dll_name);
		for (size_t i = 0; i < cvec_Import_size(&output->imports); i++) {
			Import import = output->imports[i];
			const char *import_dll_name = not_null(import.dll_name);
			const char *import_symbol_name = not_null(import.symbol_name);
			if (!strcmp(import_dll_name, dll_name)) {
				outf(&idata, "  %.*s dd RVA _%.*s\n",
					cvec_char_size(&import_symbol_name), import_symbol_name,
					cvec_char_size(&import_symbol_name), import_symbol_name);
			}
		}
		outf(&idata, "  dd 0\n\n");
	}
	// DLL names
	for (size_t i = 0; i < cvec_pcchar_size(&libs); i++) {
		const char *dll_name = not_null(libs[i]);
		outf(&idata, "%.*s_name db '%.*s',0\n",
			cvec_char_size(&dll_name), dll_name,
			cvec_char_size(&dll_name), dll_name);
	}
	outf(&idata, "\n");
	// Symbol names
	for (size_t i = 0; i < cvec_Import_size(&output->imports); i++) {
		Import import = output->imports[i];
		const char *import_symbol_name = not_null(import.symbol_name);
		const char *import_imported_name = not_null(import.imported_name);
		outf(&idata, "_%.*s dw 0\n  db '%.*s',0\n",
			cvec_char_size(&import_symbol_name), import_symbol_name,
			cvec_char_size(&import_imported_name), import_imported_name);
	}
	return idata;
}

int gen_win32fasm(Compiler *lllc, Ir *ir) {
	assert(lllc);
	assert(ir);

	Output output = {
		.code = not_null(cvec_char_new(32)),
		.data = not_null(cvec_char_new(1024)),
		.imports = not_null(cvec_Import_new(8)),
	};

	for (Symbol sym; sym = ir_next_symbol(ir), sym.type.kind != TYPE_EOF;) {
		gen_symbol(sym, &output);
	}

	char *idata = not_null(gen_idata(&output));

	char *result = not_null(cvec_char_new(4096));
	outf(&result, "format PE GUI\n");
	outf(&result, "entry main\n\n");
	outf(&result, "section '.text' code readable executable\n\n");
	outf(&result, "%.*s\n", cvec_char_size(&output.code), output.code);
	outf(&result, "section '.data' data readable writeable\n\n");
	outf(&result, "%.*s\n", cvec_char_size(&output.data), output.data);
	outf(&result, "section '.idata' import data readable writeable\n\n");
	outf(&result, "%.*s\n", cvec_char_size(&idata), idata);

	{
		FILE *fout = not_null(fopen(lllc->output_file_name, "w"));
		fwrite(result, cvec_char_size(&result), 1, fout);
		fclose(fout);
	}
	return 0;
}
