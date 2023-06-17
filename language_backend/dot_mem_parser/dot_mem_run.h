#ifndef dot_mem_run
#define dot_mem_run
#include "dot_mem_lexer.h"
#include "dot_mem_parser.h"

void DM_parser_get_next_token(_DotMemParser *p)
{
    p->DM_lexer = get_next_token(p->DM_lexer);
}

void parse_preset_data_for_byte(_DotMemParser *p)
{
    DM_parser_get_next_token(p);
    lang_assert(p->DM_lexer->token.token_id == colon, 
        "Error on line %d in %s.\n\tExpected `:` after \"preset_data\".\n", 
        invalid_grammar_error, p->DM_lexer->line, p->DM_lexer->path)
    
    DM_parser_get_next_token(p);
    lang_assert(p->DM_lexer->token.token_id == byteArray_builtin ||
                p->DM_lexer->token.token_id == emptyArray_builtin ||
                p->DM_lexer->token.token_id == none_KW,
                "Error on line %d in %s.\n\tExpected `byteArray`, `emptyArray` or `none`.\n\tInstead got %s.\n",
                unexpect_value_error, p->DM_lexer->line, p->DM_lexer->path, p->DM_lexer->token.token_value)
    
    switch(p->DM_lexer->token.token_id)
    {
        case byteArray_builtin: {
            DM_parser_get_next_token(p);
            lang_assert(p->DM_lexer->token.token_id == left_par,
                "Error on line %d in %s.\n\tExpected `(` for built-in function \"byteArray\".\n",
                grammar_mismatch_error, p->DM_lexer->line, p->DM_lexer->path)
            
            DM_parser_get_next_token(p);
            assign_PD_var_size(strtol(nT8_PCC p->DM_lexer->token.token_value, NULL, 10) * byte_size);

            DM_parser_get_next_token(p);
            lang_assert(p->DM_lexer->token.token_id == comma,
                "Error on line %d in %s.\n\tThe built-in function `byteArray` expects (size, values).\n",
                missing_parts_error, p->DM_lexer->line, p->DM_lexer->path)
            
            DM_parser_get_next_token(p);
            if(p->DM_lexer->token.token_id == left_brack)
            {
                DM_parser_get_next_token(p);
                lang_assert(p->DM_lexer->token.token_id == char_value,
                    "Error on line %d in %s.\n\tExpected byte value.\n\tInstead got %s.\n",
                    unexpect_value_error, p->DM_lexer->line, p->DM_lexer->path, p->DM_lexer->token.token_value)
                
                static uT16 index = 0;
                assign_PD_var_value_byte(p->DM_lexer->token.token_value[0], index);
                DM_parser_get_next_token(p);

                while(p->DM_lexer->token.token_id == comma)
                {
                    index++;

                    DM_parser_get_next_token(p);
                    assign_PD_var_value_byte(p->DM_lexer->token.token_value[0], index);
                    DM_parser_get_next_token(p);
                }
                printf("Done, %s", program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_data.ptr_byte_data);
            }

            break;
        }
        case emptyArray_builtin: {

            break;
        }
        case none_KW: {

            break;
        }
        default: break;
    }
    exit(0);
}

void run_dot_mem_parser(uT8 *src_code, uT8 *DM_path)
{
    _MemLexer *mem_lexer = init_dot_mem_lexer(src_code, DM_path);
    _DotMemParser *mem_parser = init_dot_mem_parser(mem_lexer);

    /* Make sure the lexer and parser got allocated accordingly. */
    lang_assert(mem_lexer,
        "Error allocating memory for Dot Mem lexer.\n\tTry rerunning the program.\n",
        OOC_allocation_error)
    lang_assert(mem_parser,
        "Error allocating memory for Dot Mem parser.\n\tTry rerunning the program.\n",
        OOC_allocation_error)
    
    DM_parser_get_next_token(mem_parser);

    while(mem_parser->DM_lexer->token.token_id != DM_EOF)
    {
        switch(mem_parser->DM_lexer->token.token_id)
        {
            case program_size_KW: {
                DM_parser_get_next_token(mem_parser);
                lang_assert(mem_parser->DM_lexer->token.token_id == colon, 
                    "Error on line %d in %s.\n\tExpected `:` after \"program_size\".\n", 
                    invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)
                DM_parser_get_next_token(mem_parser);

                program_memory_info->total_memory = (size_t) strtol(nT8_PC mem_parser->DM_lexer->token.token_value, NULL, 16);
                DM_parser_get_next_token(mem_parser);

                /* If we get `t_unknown` that means there was a value, but it is not valud. */
                lang_assert(mem_parser->DM_lexer->token.token_id != t_unknown, 
                    "Error on line %d in %s.\n\tExpected `B` (bytes), `M` (MB) or `G` (GB) after specifying program size.\n\tInstead got: %s\n\n\tMax Byte Size (Per Program): 0x100000 (1,048,576)\n\tMax  MB  Size (Per Program): 0x400 (1024)\n\tMax  GB  Size (Per Program): 0x1 (1024MB, 1,048,576 Bytes)\n",
                    missing_mem_size_type_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path, mem_parser->DM_lexer->token.token_value)
                
                /* TODO: Witht he above assertion, do we need the following assertion?
                 * Make sure we get a valid type (`B`, `M` or `G`).
                 * */
                lang_assert(mem_parser->DM_lexer->token.token_id == t_bytes ||
                            mem_parser->DM_lexer->token.token_id == t_mb ||
                            mem_parser->DM_lexer->token.token_id == t_gb,
                            "Error on line %d in %s.\n\tExpected `B` (bytes), `M` (MB) or `G` (GB) after specifying program size.\n\n\tMax Byte Size (Per Program): 0x100000 (1,048,576)\n\tMax  MB  Size (Per Program): 0x400 (1024)\n\tMax  GB  Size (Per Program): 0x1 (1024MB, 1,048,576 Bytes)\n", missing_mem_size_type_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)

                
                switch(mem_parser->DM_lexer->token.token_id)
                {
                    /* Nothing to do. `total_memory` will just represent bytes. */
                    case t_bytes: {
                        //printf("Bytes!\n");
                        if((program_memory_info->total_memory / 1024 / 1024) > 1024)
                            program_memory_info->total_memory = 1024 * 1024;
                        
                        program_memory_info->mem_type = byte;
                        program_memory_info->mem_in_bytes = program_memory_info->total_memory;
                        program_memory_info->mem_in_MB = program_memory_info->mem_in_bytes / 1024 / 1024;
                        program_memory_info->mem_in_GB = program_memory_info->mem_in_MB / 1024;

                        break;
                    }
                    case t_mb: {
                        //printf("MB!\n");
                        /* Cannot be over 1024MB (each program can take up to 1GB). */
                        if(program_memory_info->total_memory > 0x400) program_memory_info->total_memory = 0x400;
                        
                        program_memory_info->mem_type = MB;
                        program_memory_info->mem_in_bytes = program_memory_info->total_memory * 1024;
                        program_memory_info->mem_in_MB = program_memory_info->total_memory;
                        program_memory_info->mem_in_GB = program_memory_info->mem_in_MB / 1024 / 1024;

                        break;
                    }
                    case t_gb: {
                        //printf("GB!\n");
                        /* Cannot be over 1GB. */
                        if(program_memory_info->total_memory > 0x1) program_memory_info->total_memory = 0x1;
                        
                        program_memory_info->mem_type = GB;
                        program_memory_info->mem_in_bytes = program_memory_info->total_memory * 1024 * 1024;
                        program_memory_info->mem_in_MB = program_memory_info->mem_in_bytes / 1024;
                        program_memory_info->mem_in_GB = program_memory_info->mem_in_MB / 1024;
                        break;
                    }
                    default: break;
                }
                
                /*lang_assert(mem_parser->DM_lexer->token.token_id == colon, "Expected `:` after \"program_size\" on line %d in %s.\n", invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)

                DM_parser_get_next_token(mem_parser);
                DM_parser_get_next_token(mem_parser);
                printf("%s", mem_parser->DM_lexer->token.token_value);*/
                break;
            }
            case stack_access_KW: {
                DM_parser_get_next_token(mem_parser);
                lang_assert(mem_parser->DM_lexer->token.token_id == colon, 
                    "Error on line %d in %s.\n\tExpected `:` after \"stack_access\".\n", 
                    invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)

                DM_parser_get_next_token(mem_parser);
                
                /* Make sure we got `true` or `false`. */
                lang_assert(mem_parser->DM_lexer->token.token_id == boolean_true ||
                            mem_parser->DM_lexer->token.token_id == boolean_false,
                            "Error on line %d in %s.\n\tExpected `true` or `false` for \"stack_access\".\n\tInstead got %s.\n", unexpect_value_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path, mem_parser->DM_lexer->token.token_value)
                
                if(mem_parser->DM_lexer->token.token_id == boolean_true) program_memory_info->stack_access = true;
                else program_memory_info->stack_access = false;

                break;
            }
            case sections_KW: {
                DM_parser_get_next_token(mem_parser);
                lang_assert(mem_parser->DM_lexer->token.token_id == colon, 
                    "Error on line %d in %s.\n\tExpected `:` after \"sections\".\n", 
                    invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)

                DM_parser_get_next_token(mem_parser);
                lang_assert(mem_parser->DM_lexer->token.token_id == left_brack, 
                    "Error on line %d in %s.\n\tExpected `{` following `:`.\n\tInstead got %s.\n",
                    unexpect_value_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path, mem_parser->DM_lexer->token.token_value)
                
                DM_parser_get_next_token(mem_parser);
                switch(mem_parser->DM_lexer->token.token_id)
                {
                    case variable_KW: {
                        DM_parser_get_next_token(mem_parser);

                        /* Make sure there is memory. */
                        try_init_PD_vars();

                        /* Assign the new PD variable name. */
                        assign_PD_var_name(mem_parser->DM_lexer->token.token_value);

                        DM_parser_get_next_token(mem_parser);
                        lang_assert(mem_parser->DM_lexer->token.token_id == colon, 
                            "Error on line %d in %s.\n\tExpected `:` after \"%s\".\n", 
                            invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path, get_curr_PD_var_name())
                        
                        DM_parser_get_next_token(mem_parser);
                        lang_assert(mem_parser->DM_lexer->token.token_id == left_brack,
                            "Error on line %d in %s.\n\tExpected `{` after `:`.\n\tInstead got %s.\n",
                            invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path, mem_parser->DM_lexer->token.token_value)

                        DM_parser_get_next_token(mem_parser);
                        
                        redo:
                        switch(mem_parser->DM_lexer->token.token_id)
                        {
                            case store_in_KW: {
                                DM_parser_get_next_token(mem_parser);
                                lang_assert(mem_parser->DM_lexer->token.token_id == colon, 
                                    "Error on line %d in %s.\n\tExpected `:` after \"store_in\".\n", 
                                    invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)
                                
                                DM_parser_get_next_token(mem_parser);
                                assign_PD_storage_place(mem_parser->DM_lexer->token.token_id);
                                DM_parser_get_next_token(mem_parser);
                                lang_assert(mem_parser->DM_lexer->token.token_id == comma,
                                    "Error on line %d in %s.\n\tMissing: type(byte, word or dword), preset_data (can be \"none\"), liked_size (amount of memory you'd like the PD Variable to take up).\n",
                                    missing_parts_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)
                                
                                DM_parser_get_next_token(mem_parser);
                                lang_assert(mem_parser->DM_lexer->token.token_id != right_brack,
                                    "Error on line %d in %s.\n\tMissing: type (byte, word or dword), preset_data (can be \"none\"), liked_size (amount of memory you'd like the PD Variable to take up).\n",
                                    missing_parts_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)
                                
                                goto redo;
                                break;
                            }
                            case type_KW: {
                                DM_parser_get_next_token(mem_parser);
                                lang_assert(mem_parser->DM_lexer->token.token_id == colon, 
                                    "Error on line %d in %s.\n\tExpected `:` after \"type\".\n", 
                                    invalid_grammar_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)
                                
                                DM_parser_get_next_token(mem_parser);
                                lang_assert(mem_parser->DM_lexer->token.token_id == byte_KW ||
                                            mem_parser->DM_lexer->token.token_id == word_KW ||
                                            mem_parser->DM_lexer->token.token_id == dword_KW,
                                            "Error on line %d in %s.\n\tExpected `byte`, `word` or `dword`.\n\tInstead found %s.\n",
                                            unexpect_value_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path, mem_parser->DM_lexer->token.token_value)
                                
                                
                                switch(mem_parser->DM_lexer->token.token_id)
                                {
                                    case byte_KW: {
                                        /* The PD variable will access memory a byte at a time. */
                                        DM_parser_get_next_token(mem_parser);
                                        lang_assert(mem_parser->DM_lexer->token.token_id == comma,
                                            "Error on line %d in %s.\n\tMissing: preset_data (can be \"none\"), liked_size (amount of memory you'd like the PD Variable to take up).\n",
                                            missing_parts_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path)
                                        
                                        DM_parser_get_next_token(mem_parser);

                                        if(mem_parser->DM_lexer->token.token_id == preset_data_KW)
                                            parse_preset_data_for_byte(mem_parser);
                                        break;
                                    }
                                    case word_KW: {

                                        break;
                                    }
                                    case dword_KW: {
                                        
                                        break;
                                    }
                                    default: break;
                                }

                                break;
                            }
                            default: break;
                        }

                        exit(0);
                        break;
                    }
                    default: break;
                }
                exit(0);
                
                DM_parser_get_next_token(mem_parser);
                lang_assert(mem_parser->DM_lexer->token.token_id == right_brack, 
                    "Error on line %d in %s.\n\tExpected `}` at end of \"sections\" block.\n\tInstead got %s.\n",
                    unexpect_value_error, mem_parser->DM_lexer->line, mem_parser->DM_lexer->path, mem_parser->DM_lexer->token.token_value)
            
                exit(0);
            }
            default: exit(0); break;
        }
        DM_parser_get_next_token(mem_parser);
    }

    free(mem_lexer);
    free(mem_parser);
}

#endif