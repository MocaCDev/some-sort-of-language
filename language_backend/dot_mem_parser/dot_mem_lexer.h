#ifndef dot_mem_lexer
#define dot_mem_lexer

enum dot_mem_tokens
{
    DM_DEF,
    program_size_KW,
    require_var_inits_KW,
    stack_access_KW,
    sections_KW,
    variable_KW,
    store_in_KW,
    colon,
    comma,
    left_par,
    right_par,
    left_brack,
    right_brack,
    decimal,
    hex,
    boolean_true,
    boolean_false,
    DM_word,
    DM_EOF,
    t_bytes,
    t_mb,
    t_gb,
    t_unknown,
    data_KW = 0x20,
    rodata_KW = 0x21,
    stack_KW = 0x22,
    type_KW,
    preset_data_KW,
    liked_size_KW,
    byte_KW,
    word_KW,
    dword_KW,
    none_KW,
    emptyArray_builtin,
    byteArray_builtin,
    char_value,
};

typedef struct DotMemToken
{
    enum dot_mem_tokens     token_id;
    uT8                     token_value[50];
} _DotMemToken;

typedef struct MemLexer
{
    uT8             *src;
    uT8             *path;
    uT32            index;
    uT8             val;
    sSIZE           src_size;
    nT8             line;
    _DotMemToken    token;
} _MemLexer;

_MemLexer *init_dot_mem_lexer(uT8 *source_code, uT8 *path)
{
    _MemLexer *dot_mem_lex = calloc(1, sizeof(*dot_mem_lex));

    dot_mem_lex->src = calloc(strlen(nT8_PCC source_code), sizeof(*dot_mem_lex->src));
    memcpy(dot_mem_lex->src, source_code, strlen(nT8_PCC source_code));

    dot_mem_lex->path = path;
    dot_mem_lex->index = 0;
    dot_mem_lex->line = 1;
    dot_mem_lex->src_size = strlen(nT8_PCC source_code);
    dot_mem_lex->val = dot_mem_lex->src[dot_mem_lex->index];
    dot_mem_lex->token = (_DotMemToken) {
        .token_id = DM_DEF,
        .token_value = {0}
    };

    free(source_code);

    return dot_mem_lex;
}

void init_token(_MemLexer *l, uT8 *token_value, enum dot_mem_tokens tid)
{
    l->token = (_DotMemToken) {
        .token_id = tid
    };
    
    memcpy(l->token.token_value, token_value, strlen(nT8_PCC token_value));

    /* Make sure the rest of `l->token.token_value` is null terminated. */
    memset(&l->token.token_value[strlen(nT8_PCC token_value)], '\0', 50 - strlen(nT8_PCC token_value));
}

void advance(_MemLexer *l)
{
    if((l->index + 1) <= l->src_size)
    {
        l->index++;
        l->val = l->src[l->index];

        if(l->val == '\n') l->line++;
    }
}

bool peek(_MemLexer *l, uT8 against_char)
{
    if(l->src[l->index + 1] == against_char) return true;
    return false;
}
uT8 peek_and_return(_MemLexer *l)
{
    if((l->index + 1 < l->src_size))
        return l->src[l->index + 1];
    
    return '\0';
}

_MemLexer *get_next_token(_MemLexer *lex)
{
    top:
    if(lex->val == ' ' || lex->val == '\n')
        while(lex->val == ' ' || lex->val == '\n') advance(lex);
    
    if(lex->val == '\t')
        while(lex->val == '\t') advance(lex);
    
    if(is_ascii(lex->val))
    {
        if(peek(lex, '\n') || peek(lex, '\0') || peek(lex, ' ')) goto check_char;

        uT8 *word = calloc(1, sizeof(*word));
        lang_assert(word,
            "Error allocating memory to obtain word from %s.\n\tTry rerunning the program.\n",
            OOC_allocation_error, lex->path)
        uT8 index = 0;

        while(is_ascii_with_exception(lex->val, '_'))
        {
            word[index] = lex->val;
            index++;

            word = realloc(
                word,
                (index + 1) * sizeof(*word)
            );

            advance(lex);

            if(lex->val == ' ') break;
        }

        memset(&word[index], '\0', 1);
        if(strcmp(nT8_PCC word, "program_size") == 0) { init_token(lex, uT8_PC word, program_size_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "stack_access") == 0) { init_token(lex, uT8_PC word, stack_access_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "true") == 0) { init_token(lex, uT8_PC word, boolean_true); goto do_rest; }
        if(strcmp(nT8_PCC word, "false") == 0) { init_token(lex, uT8_PC word, boolean_false); goto do_rest; }
        if(strcmp(nT8_PCC word, "sections") == 0) { init_token(lex, uT8_PC word, sections_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "variable") == 0) { init_token(lex, uT8_PC word, variable_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "store_in") == 0) { init_token(lex, uT8_PC word, store_in_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "data") == 0) { init_token(lex, uT8_PC word, data_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "rodata") == 0) { init_token(lex, uT8_PC word, rodata_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "stack") == 0) { init_token(lex, uT8_PC word, stack_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "type") == 0) { init_token(lex, uT8_PC word, type_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "preset_data") == 0) { init_token(lex, uT8_PC word, preset_data_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "liked_size") == 0) { init_token(lex, uT8_PC word, liked_size_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "byte") == 0) { init_token(lex, uT8_PC word, byte_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "word") == 0) { init_token(lex, uT8_PC word, word_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "dword") == 0) { init_token(lex, uT8_PC word, dword_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "none") == 0) { init_token(lex, uT8_PC word, none_KW); goto do_rest; }
        if(strcmp(nT8_PCC word, "emptyArray") == 0) { init_token(lex, uT8_PC word, emptyArray_builtin); goto do_rest; }
        if(strcmp(nT8_PCC word, "byteArray") == 0) { init_token(lex, uT8_PC word, byteArray_builtin); goto do_rest; }
        init_token(lex, uT8_PC word, DM_word);

        do_rest:
        free(word);
        return lex;
    }

    if(is_number(lex->val))
    {
        uT8 *number = calloc(1, sizeof(*number));
        lang_assert(number,
            "Error allocating memory to obtain number from %s.\n\tTry rerunning the program.\n",
            OOC_allocation_error, lex->path)
        uT8 index = 0;
        bool is_hex = false;

        while(true)
        {
            number[index] = lex->val;
            index++;

            number = realloc(
                number,
                (index + 1) * sizeof(*number)
            );
            
            if(peek(lex, ' ') || peek(lex, '\n') || peek(lex, ',')) { advance(lex); break; }
            if(peek(lex, '\0')) goto end;
            if(peek(lex, 'x')) is_hex = true;

            advance(lex);
        }

        memset(&number[index], '\0', 1);
        
        if(is_hex) init_token(lex, uT8_PC number, hex);
        else init_token(lex, uT8_PC number, decimal);

        free(number);

        return lex;
    }

    check_char:
    switch(lex->val)
    {
        case '\'': {
            advance(lex);

            uT8 *byte_value;
            if(lex->val == '\\')
            {
                if(peek(lex, '0')) 
                { advance(lex); byte_value = uT8_PC make_uT8_ptr('\0'); }
            }
            else byte_value = uT8_PC make_uT8_ptr(lex->val);
            lang_assert(peek(lex, '\''),
                "Error on line %d in %s.\n\tExpected closing single-quote for byte value.\n",
                grammar_mismatch_error, lex->line, lex->path)
            advance(lex);
            
            init_token(lex, uT8_PC byte_value, char_value);
            advance(lex);
            free(byte_value);
            return lex;
        }
        case '/': {
            if(peek(lex, '/'))
            {
                advance(lex);
                advance(lex);
                while(lex->val != '\n' && !(lex->val == '\0')) advance(lex);

                goto top;
            }
            lang_error("Error on line %d in %s.\n\tUnexpected `/`.\n",
                invalid_grammar_error, lex->line, lex->path)
            
        }
        case 'B':
        case 'b': { init_token(lex, uT8_PC "b", t_bytes); advance(lex); return lex; }
        case 'G':
        case 'g': { init_token(lex, uT8_PC "g", t_gb); advance(lex); return lex; }
        case 'M':
        case 'm': { init_token(lex, uT8_PC "m", t_mb); advance(lex); return lex; }
        case '(': { init_token(lex, uT8_PC "(", left_par); advance(lex); return lex; }
        case ')': { init_token(lex, uT8_PC ")", right_par); advance(lex); return lex; }
        case '{': { init_token(lex, uT8_PC "{", left_brack); advance(lex); return lex; }
        case '}': { init_token(lex, uT8_PC "}", right_brack); advance(lex); return lex; }
        case ':': { init_token(lex, uT8_PC ":", colon); advance(lex); return lex; }
        case ',': { init_token(lex, uT8_PC ",", comma); advance(lex); return lex; }
        default: {
            uT8 *char_value_as_string = uT8_PC make_uT8_ptr(lex->val);

            if(is_ascii(lex->val)) { init_token(lex, char_value_as_string, t_unknown); advance(lex); free(char_value_as_string); return lex; }

            if(lex->val == '\0') { init_token(lex, uT8_PC "\0", DM_EOF); free(char_value_as_string); return lex; }
            init_token(lex, char_value_as_string, t_unknown);
            advance(lex);
            free(char_value_as_string);
            return lex;
            break;
        }
    }
    
    end:
    init_token(lex, uT8_PC "\0", DM_EOF);
    return lex;
}

#endif