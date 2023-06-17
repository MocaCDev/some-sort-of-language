#ifndef lexer
#define lexer

typedef struct lexer
{
    uT8         *file_source_code;

    /* We shouldn't need more then 4 bytes for the index. */
    uT32        source_code_index;

    /* Current value. */
    uT8         val;

    /* The size of the source code. */
    sSIZE       source_code_size; 

    /* What line are we on? */
    nTL32       line;
} _lexer;

/* Used in `get_GTV`. */
uT8 *make_uT8_ptr(uT8 val);

#include "tokens.h"

_lexer *init_lexer(nT8 *filename)
{
    _lexer *language_lexer = calloc(1, sizeof(*language_lexer));

    language_lexer->file_source_code = NULL;
    language_lexer->source_code_index = 0;
    language_lexer->line = 1;

    /* Get file content. */
    FILE *source_code = fopen(filename, "r");

    /* Check the size of the file. */
    fseek(source_code, 0, SEEK_END);
    language_lexer->source_code_size = ftell(source_code);
    lang_assert(language_lexer->source_code_size > 1, "The file `%s` is empty.\n\tTry putting some code in the file.\n", file_has_no_data_error, filename)
    fseek(source_code, 0, SEEK_SET);

    /* Allocate the required memory and make sure it is valid. */
    language_lexer->file_source_code = calloc(language_lexer->source_code_size, sizeof(*language_lexer->file_source_code));
    lang_assert(language_lexer->file_source_code, 
        "Error allocating memory to lex the source code for `%s`.\n\tTry running the program again.\n", 
        OOC_allocation_error, filename)

    nTL32 read_size = fread(language_lexer->file_source_code, sizeof(nT8), language_lexer->source_code_size, source_code);
    lang_assert(read_size == language_lexer->source_code_size, 
        "Error reading in all of the source code for `%s`.\n", 
        OOC_source_code_read_error, filename)

    fclose(source_code);

    language_lexer->val = language_lexer->file_source_code[language_lexer->source_code_index];
    return language_lexer;
}

void move_forward(_lexer *l)
{
    if(l->source_code_index + 1 < l->source_code_size)
    {
        l->source_code_index++;
        l->val = l->file_source_code[l->source_code_index];
    }
}

void move_backward(_lexer *l)
{
    if(l->source_code_index - 1 > 0)
    {
        l->source_code_index--;
        l->val = l->file_source_code[l->source_code_index];
    }
}

bool lexer_peek(_lexer *l, uT8 against_char)
{
    if(l->file_source_code[l->source_code_index + 1] == against_char) { move_forward(l); return true; }
    return false;
}

uT8 lexer_peek_and_return(_lexer *l)
{
    if(l->source_code_index != l->source_code_size - 1) return l->file_source_code[l->source_code_index + 1];
    return l->val;
}

uT8 *reallocate_uT8_ptr(uT8 *src, uT8 index)
{
    src = realloc(
        src,
        (index + 1) * sizeof(*src)
    );

    return src;
}

uT8 *make_uT8_ptr(uT8 val)
{
    uT8 *word = calloc(2, sizeof(*word));

    word[0] = val;
    word[1] = '\0';
    return word;
}

uT8 *obtain_ascii(_lexer *l, bool getting_string, uT8 opening_quote)
{
    uT8 *word = calloc(1, sizeof(*word));
    uT8 index = 0;

    cont:
    while(is_ascii(l->val))
    {
        if(getting_string && l->val == '\n')
            lang_error("Unexpected newline on line %ld.\n", 
                unexpected_new_line_error, l->line)

        /* If we're getting a string make sure the beginning quotation grammar matches the end.
         * We can't have, for example, "a string'.
         * */
        if(getting_string && ((lexer_peek_and_return(l) == '\'' && opening_quote == '"') ||
           (lexer_peek_and_return(l) == '"' && opening_quote == '\'')))
           lang_error("Mismatch of grammar on line %ld.\n\tExpecting `%c` (G_double_quote), got `%c` (%s).\n",
            grammar_mismatch_error, l->line, opening_quote, lexer_peek_and_return(l), token_name(0, uT8_PC make_uT8_ptr(lexer_peek_and_return(l)), GR))

        word[index] = l->val;
        index++;
        word = reallocate_uT8_ptr(word, index);
        move_forward(l);

        /* If we are getting a string and run into a value that is not >= 'a' <= 'z' / >= 'A' <= 'Z', still append it to `word`. */
        if(getting_string && !(is_ascii(l->val)))
        {
            while(!(is_ascii(l->val)))
            {
                /* If we run into a quotation mark and are obtaining a string, break. */
                if(l->val == '\'' || l->val == '"') break;

                word[index] = l->val;
                index++;
                word = reallocate_uT8_ptr(word, index);
                move_forward(l);
            }
            
            if(l->val == '\'' || l->val == '"')
            {
                /* Make sure the end quotation matches the beginning quotation. */
                lang_assert(l->val == opening_quote, 
                    "Mismatch of quotes on line %ld.\n", 
                    grammar_mismatch_error, l->line)

                goto end;
            }

            goto cont;
        }
    }

    end:
    memset(&word[index], '\0', 1);
    return word;
}

uT8 *obtain_number(_lexer *l)
{
    uT8 *number = calloc(1, sizeof(*number));
    uT8 index = 0;

    cont:
    while(is_number(l->val) || is_hex_valid_ascii(l->val))
    {
        number[index] = l->val;
        index++;

        number = realloc(
            number,
            (index + 1) * sizeof(*number)
        );
        move_forward(l);

        switch(l->val)
        {
            case '.': {
                number[index] = l->val;
                index++;

                number = realloc(
                    number,
                    (index + 1) * sizeof(*number)
                );

                lang_assert(is_number(lexer_peek_and_return(l)), 
                    "Unexpected end of hexadecimal number on line %ld.\n",
                    unexpected_end_of_decimal_error, l->line)

                move_forward(l);
                goto cont;
                break;
            }
            case 'x': {
                number[index] = l->val;
                index++;

                number = realloc(
                    number,
                    (index + 1) * sizeof(*number)
                );

                lang_assert(is_number(lexer_peek_and_return(l)) || is_hex_valid_ascii(lexer_peek_and_return(l)),
                    "Unexpected end of hexadecimal number on line %ld.\n",
                    unexpected_end_of_hex_error, l->line)

                move_forward(l);
                goto cont;
                break;
            }
            case 'h': {
                number[index] = l->val;
                index++;

                number = realloc(
                    number,
                    (index + 1) * sizeof(*number)
                );

                lang_assert(!(is_number(lexer_peek_and_return(l)) || is_hex_valid_ascii(l->val)),
                    "Trailing numbers after hexadecimal number ended on line %ld.\n",
                    trailing_numbers_after_eoh_error, l->line)

                if(!(lexer_peek(l, '\0'))) move_forward(l);
                goto end;
            }
            default: break;
        }
    }

    end:
    memset(&number[index], '\0', 1);
    return number;
}

bool is_decimal(uT8 *number)
{
    uT8 index = 0;
    while(number[index] != '\0')
        if(number[index++] == '.') return true;
    
    return false;
}

bool is_hex(uT8 *number)
{
    uT8 index = 0;
    while(number[index] != '\0')
    {
        if(number[index] == 'h' || number[index] == 'x')
            return true;
        
        index++;
    }

    return false;
}

uT8 *validate_hex(uT8 *hex)
{
    uT8 index = 0;

    while(hex[index] != 'x' && hex[index] != 'h')
        index++;

    return hex;
}

_lexer *get_next_state(_lexer *lang_lexer, bool expect_string, uT8 opening_quote)
{
    if(lang_lexer->source_code_index == lang_lexer->source_code_size - 1)
        { make_new_token(END, uT8_PC "\0", G_end_of_file); goto end; }

    if(lang_lexer->val == ' ')
    {
        while(lang_lexer->val == ' ' && 
             !(lang_lexer->file_source_code[lang_lexer->source_code_index] == '\0') &&
             !(lang_lexer->source_code_index == lang_lexer->source_code_size - 1))
            move_forward(lang_lexer);
    }
    if(lang_lexer->val == '\n')
    {
        lang_lexer->line++;
        while(lang_lexer->val == '\n' && 
             !(lang_lexer->file_source_code[lang_lexer->source_code_index] == '\0') &&
             !(lang_lexer->source_code_index == lang_lexer->source_code_size - 1))
            move_forward(lang_lexer);
    }

    if(is_ascii(lang_lexer->val))
    {
        if(!(is_ascii(lexer_peek_and_return(lang_lexer))))
        {
            uT8 *val = make_uT8_ptr(lang_lexer->val);
            make_new_token(DT, uT8_PC val, DT_char);

            free(val);
            val = NULL;

            goto ret1; 
        }
        
        uT8 *word = obtain_ascii(lang_lexer, expect_string, opening_quote);
        make_new_token_alone(word, lang_lexer);

        free(word);
        word = NULL;

        ret1:
        return lang_lexer;
    }

    if(is_number(lang_lexer->val))
    {
        uT8 *number = obtain_number(lang_lexer);

        if(is_decimal(number)) { make_new_token(DT, uT8_PC number, DT_float); goto ret2; }
        if(is_hex(number)) { validate_hex(number); make_new_token(DT, uT8_PC number, DT_hex); goto ret2; }
        
        make_new_token(DT, uT8_PC number, DT_integer);

        ret2:
        free(number);
        number = NULL;

        return lang_lexer;
    }

    switch(lang_lexer->val)
    {
        case '\'': { make_new_token(GR, uT8_PC "'", G_single_quote); move_forward(lang_lexer); return lang_lexer; break; }
        case '(': { make_new_token(GR, uT8_PC "(", G_left_par); move_forward(lang_lexer); return lang_lexer; break; }
        case ')': { make_new_token(GR, uT8_PC ")", G_right_par); move_forward(lang_lexer); return lang_lexer; break; }
        case '=': { make_new_token(GR, uT8_PC "=", G_equals); move_forward(lang_lexer); return lang_lexer; break; }
        case '#': { make_new_token(GR, uT8_PC "#", G_hashtag); move_forward(lang_lexer); return lang_lexer; break; }
        case ',': { make_new_token(GR, uT8_PC ",", G_comma); move_forward(lang_lexer); return lang_lexer; break; }
        case '"': { make_new_token(GR, uT8_PC "\"", G_double_quote); move_forward(lang_lexer); return lang_lexer; break; }
        default: break;
    }
    
    if(lang_lexer->file_source_code[lang_lexer->source_code_index] == '\0')
        make_new_token(END, uT8_PC "\0", G_end_of_file);
    
    end:
    return lang_lexer;
}

void destroy_lexer(_lexer *lex)
{
    if(!(lex)) return;

    /* Free the source code. */
    free(lex->file_source_code);
    lex->file_source_code = NULL;

    /* Free the overall structure pointer. */
    free(lex);
    lex = NULL;
}

#endif