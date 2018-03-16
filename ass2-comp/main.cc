#include <iostream>
#include <fstream>
#include <memory>
#include "binary.tab.hh"
#include "labTrees.hh"
extern Statement* root;
extern FILE *yyin;

void yy::parser::error(std::string const&err)
{
  std::cerr << "Parsing error: " << err << std::endl;
}

int main(int argc, char **argv)
{
	yy::parser parser;
	if (argc > 1) {
		yyin = fopen(argv[1], "r");
	}
	bool result = parser.parse();
	if (argc > 1) {
		fclose(yyin);
	}
	if (!result) {
		//root->dump();
		//std::cout << std::endl;
		Environment env;
		Statement *current_test = root;
		BBlock *first_block = new BBlock();
		//current_test->dump();
		current_test->convert(env, first_block);
		//current_test->dump();
		//dump_asm(first_block);
		
		std::ofstream asm_file;
		asm_file.open("target-raw.s");
		dumpASM(env, first_block, asm_file);
		asm_file.close();

		std::ofstream parse_tree_file;
		parse_tree_file.open("cfg.dot");
		dumpCFG(env, first_block, parse_tree_file);
		parse_tree_file.close();
		
		//std::cout << e;
	}
	return 0;
}
