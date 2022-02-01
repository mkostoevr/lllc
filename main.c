#include "main.h"

Compiler compiler_new(int argc, char **argv) {
	return (Compiler) {
		.output_file_name = (argc == 1 ? "a.asm" : argv[1]),
	};
}

int main(int argc, char **argv) {
	Compiler lllc = compiler_new(argc, argv);
	Tokenizer tokenizer = tokenizer_new(&lllc, "test/all.lll");
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	Ir ir = ir_new(&lllc, &astificator);
	return gen_win32fasm(&lllc, &ir);
}