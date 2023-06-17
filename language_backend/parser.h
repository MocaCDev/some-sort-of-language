#ifndef parser
#define parser
#include "dot_mem_parser/dot_mem_run.h"

typedef struct variable_decl_info
{
    enum DT_tokens datatype;
    uT8 *variable_name;

    union {
        uT8 *string_value;
        uT32 integer_value;
        uT8 char_value;
        uT8 *hex_value;
    } variable_value;
} _var_decl_info;

_var_decl_info *vdinfo = NULL;

typedef struct parser
{
    _lexer      *lang_lexer;
    _lexer      *previous_lexer_state;
    _token      previous_token;
} _parser;

#include "ast.h"

_parser *init_parser(_lexer *lang_lexer)
{
    lang_assert(lang_lexer, 
        "No valid memory for lexer.\n\tTry rerunning the program.\n", 
        OOC_allocation_error)

    _parser *language_parser = calloc(1, sizeof(*language_parser));

    lang_assert(language_parser, 
        "Error allocating memory for the parser.\n\tTry rerunning the program.\n", 
        OOC_allocation_error)

    language_parser->lang_lexer = lang_lexer;
    language_parser->previous_lexer_state = NULL; // We don't have a previous state

    make_new_token(DEF, uT8_PC "\0", 0);

    vdinfo = calloc(1, sizeof(*vdinfo));
    return language_parser;
}

void parse_keyword(_parser *p);
void parse_datatype(_parser *p);
void parse_var_decl(_parser *p);
void parse_macro(_parser *p);

void get_state(_parser *p, bool expect_string, uT8 opening_quote)
{
    p->previous_lexer_state = p->lang_lexer;
    p->lang_lexer = get_next_state(p->lang_lexer, expect_string, opening_quote);
}

void run_parser(_parser *lang_parser)
{
    get_state(lang_parser, false, 0);

    /* Get new lexer state. */
    while(get_TOT() != END)
    {
        /* If the ast has been comitted then the program ended whilst parsing a section of the code. */
        if(ast_has_been_comitted())
        {
            printf("Program Ended.");
            exit(0);
        }

        switch(get_TOT())
        {
            case KW: parse_keyword(lang_parser);break;
            case DT: parse_datatype(lang_parser);break;
            case VD: parse_var_decl(lang_parser);break;
            case GR: {
                /* Check if it is a valid grammar value for the parser to parse. */
                lang_assert(token_data->token_info.GR_token_info.grammar_token == G_hashtag, "Invalid grammar on line %ld.\n", invalid_grammar_error, lang_parser->lang_lexer->line)
            
                parse_macro(lang_parser);
                break;
            }
            default: printf("Unknown TOT: %d", get_TOT());break;
        }

        if(!(token_data->type_of_token == END))
            get_state(lang_parser, false, 0);
    }
    printf("Done");
}

void parse_macro(_parser *p)
{
    /* The ast does not deal with macros. */

    get_state(p, false, 0);

    switch(get_KTT())
    {
        case KW_include: {
            printf("Including other source file.\n");
            break;
        }
        case KW_incmem: {
            lang_assert(p->lang_lexer->line == 1, 
                "Expected `#incmem` on line 1. Found on line %ld.\n", 
                incmem_not_on_line_1_error, p->lang_lexer->line)

            get_state(p, false, 0);
            lang_assert(get_GTT() == G_double_quote, 
                "Expected opening double quote for `incmem`, got %s (%s) instead.\n",
                expected_DQ_error, get_GTV(), get_GTTN())
            get_state(p, true, '"');
            
            /* We now have whatever value is inside the double quotes.
             * The syntax should be as follows in the .sum file:
             *  `#incmem "filename.mem"
             * */
            uT8 *dot_mem_filename = get_DTV();
            dot_mem_filename = uT8_PC initiate_path(dot_mem_file_location_folder, uT8_PC dot_mem_filename);
            uT8 *dot_mem_file_data = NULL;

            FILE *f = fopen(nT8_PC dot_mem_filename, "r");

            lang_assert(f, 
                "The file \"%s\" passed to `incmem` doesn't exist, or the path is wrong.\n",
                file_not_exist_error, dot_mem_filename)


            fseek(f, 0, SEEK_END);
            size_t dot_mem_file_size = ftell(f);
            dot_mem_file_data = calloc(dot_mem_file_size, sizeof(*dot_mem_file_data));
            lang_assert(dot_mem_file_data,
                "Error allocating memory for the `.mem` file.\n\tTry rerunning the program.\n",
                OOC_allocation_error)
            fseek(f, 0, SEEK_SET);

            fread(dot_mem_file_data, sizeof(uT8), dot_mem_file_size, f);

            fclose(f);

            /* Parse the .mem file. */
            run_dot_mem_parser(dot_mem_file_data, dot_mem_filename);

            free(dot_mem_filename);
            free(dot_mem_file_data);
            get_state(p, false, 0);

            
            exit(0);
            break;
        }
        default: break;
    }
    exit(0);
}

void parse_keyword(_parser *p)
{
    switch(get_KTT())
    {
        case KW_print: {
            get_state(p, false, 0);    // we have `print`, this will get `'`
            
            switch(get_TOT())
            {
                case GR: {
                    /* With `print`, getting a grammar token means we are printing a string. */
                    lang_assert(get_GTT() == G_single_quote, 
                        "Expected string or integer/decimal/hexadecimal value on line %ld.\n",
                        invalid_grammar_error, p->lang_lexer->line)

                    printf("Printing String");
                }
                case DT: {
                    /* If the DTT (Data Token Type) is `DT_word`, then the `print` statement is recieving a variable name
                     * to print.
                     * */
                    if(get_DTT() == DT_word) goto printing_variable;

                    printf("Printing Data Type: %s\nValue: %s\n", get_DTTN(), get_DTV());
                    break;
                }
                case VD: {
                    printing_variable:
                    printf("Printing Variable");
                    break;
                }
                default: break;
            }
            exit(0);
            
            //get_state(p, true);     // get the string to print

            //new_tree_entry(print_statement);
            
            /* Make sure there is an ending `'`. */
            //lang_assert(p->lang_lexer->val == '\'', "Expected `'` at end of `print` statement on line %ld.\n", missing_quote_error, p->lang_lexer->line)

            //get_state(p, false);    // get the ending `'`

            break;
        }
        case KW_exit: {
            printf("Exiting");
            break;
        }
        default: break;
    }
}

void parse_datatype(_parser *p)
{
}

void parse_var_decl(_parser *p)
{
    vdinfo->datatype = token_data->token_info.DT_token_info.data_type_token;

    get_state(p, false, 0);
    lang_assert(token_data->type_of_token != END, 
        "Unexpected EOF.\n", 
        unexpected_EOF)

    vdinfo->variable_name = token_data->token_info.DT_token_info.data_type_value;

    get_state(p, false, 0);
    
    if(token_data->type_of_token == GR)
    {
        if(token_data->token_info.GR_token_info.grammar_token == G_equals)
        {
            switch(vdinfo->datatype)
            {
                case DT_string: {
                    printf("here");
                    get_state(p, false, 0);
                    lang_assert(token_data->token_info.GR_token_info.grammar_value == '\'', "Expected string on line %ld.\n", missing_quote_error, p->lang_lexer->line)
                    
                    get_state(p, true, token_data->token_info.GR_token_info.grammar_token == G_single_quote ? '\'' : '"');
                    printf("%s", token_data->token_info.DT_token_info.data_type_value);
                    get_state(p, false, 0);
                    lang_assert(token_data->token_info.GR_token_info.grammar_value == '\'', "Unexpected end to string on line %ld.\n", missing_quote_error, p->lang_lexer->line)
                    exit(0);
                }
                case DT_integer: {
                    printf("Integer");
                    exit(0);
                }
                default: break;
            }
        } else {
            if(program_memory_info->require_initialized_variables) { lang_error("Unexpected value without `=` on line %ld.\n", missing_equals_error, p->lang_lexer->line) }
        }
    } else {
        if(lexer_peek_and_return(p->lang_lexer) == '\n') { move_forward(p->lang_lexer); return; }

        /* If we are at the EOF, `check_is_EOF` will automatically commit the AST. */
        if(check_is_EOF(p)) return;

        /* Check if the programs memory specification requires variables to be initialized. */
        if(program_memory_info->require_initialized_variables)
            lang_assert(token_data->type_of_token != DT, "Unexpected value without `=` on line %ld.\n", unexpect_value_error, p->lang_lexer->line)
    }
}

void destroy_parser(_parser *lang_parser)
{
    if(!(lang_parser)) return;

    if(lang_parser->lang_lexer) { free(lang_parser->lang_lexer); lang_parser->lang_lexer = NULL; }
    if(lang_parser->previous_lexer_state) { free(lang_parser->previous_lexer_state); lang_parser->previous_lexer_state = NULL; }

    free(lang_parser);
}

#endif