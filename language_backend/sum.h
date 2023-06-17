#ifndef sum_language
#define sum_language

#define dot_mem_file_location_folder    uT8_PC "dot_mem/"

#include "lexer.h"
#include "../mem_outline_lang/mem_outline.h"
#include "parser.h"

void run(nT8 *filename)
{
    _lexer *lex = init_lexer(filename);
    _parser *pars = init_parser(lex);

    init_program_memory_info();
    new_tree_entry(ast_tree_init);

    run_parser(pars);

    destroy_lexer(lex);
    destroy_parser(pars);
    destroy_token_reference(token_data->type_of_token);
    destroy_tree();
}

#endif