#define ONE_SOURCE
#include "../../main.h"

int main(int argc, char **argv) {
	assert(argc == 3);
	Compiler lllc = (Compiler) {
		.output_file_name = argv[2],
	};
	Tokenizer tokenizer = tokenizer_new(&lllc, argv[1]);
	Astificator astificator = astificator_new(&lllc, &tokenizer);
	Ir ir = ir_new(&lllc, &astificator);
	return gen_win32fasm(&lllc, &ir);
}
