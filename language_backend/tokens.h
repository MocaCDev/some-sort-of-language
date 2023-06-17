#ifndef tokens
#define tokens

/* Keyword tokens. */
enum keyword_tokens
{
    KW_print       = 0x0,
    KW_exit,
    KW_include,
    KW_incmem,       // #incmem
    KW_unknown       // returned in `decipher_KTT` if the type cannot be determined
};

/* Data type tokens(e.g. strings, integers, characters, hex). */
enum DT_tokens
{
    DT_string       = 0x05,
    DT_integer,
    DT_float,
    DT_hex,
    DT_char,
    DT_word        // variable declarations
};

/* Grammar tokens(e.g. `'`, `:`, `(` & `)`, `{` & `}` etc..)*/
enum grammar_tokens
{
    G_single_quote  = 0x0B,     // `'`
    G_colon,            // `:`
    G_left_par,         // `(`
    G_right_par,        // `)`
    G_end_of_file,
    G_equals,           // `=`
    G_hashtag,          // `#`
    G_comma,
    G_double_quote,
    G_unknown           // returned in `decipher_GTT` if the type cannot be determined
};

/* Type of token.
 * KW - Keyword
 * GR - grammar
 * */
enum token_type
{
    KW             = 0x16,
    GR,
    DT,
    VD,  // Variable Declaration
    DEF, // "init" token
    END, // end token
    NONE // Used for `token_name`
};

typedef struct token
{
    enum token_type     type_of_token;

    /* Token information. */
    union {
        struct {
            uT8     *keyword_value;
            enum keyword_tokens keyword_token;
        } KW_token_info;

        struct {
            uT8     *data_type_value;
            enum DT_tokens data_type_token;
        } DT_token_info;

        struct {
            uT8     grammar_value;
            enum grammar_tokens grammar_token;
        } GR_token_info;
    } token_info;
} _token;

_token *token_data = NULL;

void clear_out_token(enum token_type);

/* Decipher the GTT.
 * GTT - Grammar Token Type
 * */
enum grammar_tokens decipher_GTT(uT8 val)
{
    switch(val)
    {
        case '(': return G_left_par;break;
        case ')': return G_right_par;break;
        case '\'': return G_single_quote;break;
        case '"': return G_double_quote;break;
        case ':': return G_colon;break;
        case '=': return G_equals;break;
        case '#': return G_hashtag;break;
        case ',': return G_comma;break;
        default: break;
    }
    return G_unknown;
}

/* Decipher the KTT.
 * KTT - Keyword Token Type
 * */
enum keyword_tokens decipher_KTT(uT8 *val)
{
    if(strcmp(nT8_PCC val, "print") == 0) return KW_print;
    if(strcmp(nT8_PCC val, "include") == 0) return KW_include;
    if(strcmp(nT8_PCC val, "incmem") == 0) return KW_incmem;
    if(strcmp(nT8_PCC val, "exit") == 0) return KW_exit;

    return KW_unknown;
}

/* Decipher the DTT.
 * DTT - Data Token Type
 * */
enum DT_tokens decipher_DTT(uT8 *val)
{
    if(strcmp(nT8_PCC val, "int") == 0) return DT_integer;
    if(strcmp(nT8_PCC val, "string") == 0) return DT_string;
    if(strcmp(nT8_PCC val, "float") == 0) return DT_float;
    if(strcmp(nT8_PCC val, "hex") == 0) return DT_hex;
    if(strcmp(nT8_PCC val, "char") == 0) return DT_char;

    return DT_word;
}

nT8 *token_name(uT32 token_val, uT8 *val, enum token_type tt)
{
    if(tt != NONE)
    {
        switch(tt)
        {
            case KW: token_val = decipher_KTT(val);break;
            case GR: token_val = decipher_GTT(val[0]);break;
            case DT: token_val = decipher_DTT(val);break;
            case VD: token_val = DT_word;break;
            default: break;
        }

        if(val) free(val);
    }

    switch(token_val)
    {
        /* Keywords. */
        case KW_print: return nT8_PC "Built-in `print` function";break;
        case KW_exit: return nT8_PC "Built-in `exit` function";break;
        case KW_include: return nT8_PC "Built-in `include` keyword";break;
        case KW_incmem: return nT8_PC "Built-in `incmem` keyword";break;
        /* Grammar. */
        case G_single_quote: return nT8_PC "Grammar Single Quote";break;
        case G_double_quote: return nT8_PC "Double Quote";break;
        case G_equals: return nT8_PC "Grammar Equals";break;
        case G_comma: return nT8_PC "Grammar Comma";break;
        case G_colon: return nT8_PC "Grammar Colon";break;
        case G_hashtag: return nT8_PC "Grammar Hashtag";break;
        case G_left_par: return nT8_PC "Left Parenthesis";break;
        case G_right_par: return nT8_PC "Right Parenthesis";break;
        /* Data Types. */
        case DT_string: return nT8_PC "Built-in data type String";break;
        case DT_integer: return nT8_PC "Built-in data type Integer";break;
        case DT_hex: return nT8_PC "Built-in data type Hexadecimal";break;
        case DT_char: return nT8_PC "Built-in data type Character";break;
        case DT_float: return nT8_PC "Built-in data type float";break;
        /* Token types (GR - Grammar, VD - Variable Declaration, KW - Keyword, DT - Data Type, DEF - Default, END - EOF)*/
        case GR: return nT8_PC "Token-Parent GR (Grammar)";break;
        case VD: return nT8_PC "Token-Parent VD (Variable Declaration)";break;
        case KW: return nT8_PC "Token-Parent KW (Keyword)";break;
        case DT: return nT8_PC "Token-Parent DT (Data Type)";break;
        case DEF: return nT8_PC "Token-Parent DEF (Default Token)";break;
        case END: return nT8_PC "Token-Parent END (EOF - End Of File)";break;
        default: break;
    }
    return nT8_PC "Unknown";
}

/* Get the TOT.
 * TOT - Type Of Token
 * */
enum token_type get_TOT()
{
    return token_data->type_of_token;
}

/* Get the KTT.
 * KTT - Keyword Token Type
 * */
enum keyword_tokens get_KTT()
{
    return token_data->token_info.KW_token_info.keyword_token;
}

/* Get the DTT.
 * DTT - Data Token Type
 * */
enum DT_tokens get_DTT()
{
    return token_data->token_info.DT_token_info.data_type_token;
}

/* Get the GTT.
 * GTT - Grammar Token Type
 * */
enum grammar_tokens get_GTT()
{
    return token_data->token_info.GR_token_info.grammar_token;
}

/* Get the TOTN.
 * TOTN - Type Of Token Name
 * */
uT8 *get_TOTN()
{
    switch(get_TOT())
    {
        case KW: return uT8_PC "KW";break;
        case GR: return uT8_PC "GR";break;
        case VD: return uT8_PC "VD";break;
        case DT: return uT8_PC "DT";break;
        default: break;
    }
    return uT8_PC "Unknown TOT (Type Of Token)";
}

/* Get the DTTN.
 * DTTN - Data Token Type Name
 * */
uT8 *get_DTTN()
{
    switch(get_DTT())
    {
        case DT_string:     return uT8_PC "DT_string";break;
        case DT_integer:    return uT8_PC "DT_integer";break;
        case DT_float:      return uT8_PC "DT_float";break;
        case DT_hex:        return uT8_PC "DT_hex";break;
        case DT_char:       return uT8_PC "DT_char";break;
        case DT_word:       return uT8_PC "DT_word";break;
        default: break;
    }
    return uT8_PC "Unknown DTT (Data Token Type)";
}

/* Get the KTTN.
 * KTTN - Keyword Token Type Name
 * */
uT8 *get_KTTN()
{
    switch(get_KTT())
    {
        case KW_print:  return uT8_PC "KW_print";break;
        case KW_exit:   return uT8_PC "KW_exit";break;
        case KW_include:return uT8_PC "KW_include";break;
        case KW_incmem: return uT8_PC "KW_incmem";break;
        default: break;
    }
    return uT8_PC "Unknown KTT (Keyword Token Type)";
}

/* Get the GTTN.
 * GTTN - Grammar Token Type Name
 * */
uT8 *get_GTTN()
{
    switch(get_GTT())
    {
        case G_single_quote:        return uT8_PC "G_single_quote";break;
        case G_colon:               return uT8_PC "G_colon";break;
        case G_left_par:            return uT8_PC "G_left_par";break;
        case G_right_par:           return uT8_PC "G_right_par";break;
        case G_end_of_file:         return uT8_PC "G_end_of_file";break;
        case G_equals:              return uT8_PC "G_equals";break;
        case G_hashtag:             return uT8_PC "G_hashtag";break;
        case G_comma:               return uT8_PC "G_comma";break;
        case G_double_quote:        return uT8_PC "G_double_quote";break;
        default: break;
    }
    return uT8_PC "Unknown GTT (Grammar Token Type)";
}

/* Get the DTV.
 * DTV - Data Token Value
 * */
uT8 *get_DTV()
{
    if(token_data->token_info.DT_token_info.data_type_value)
        return uT8_PC token_data->token_info.DT_token_info.data_type_value;
    
    return uT8_PC "\0";
}

/* Get the KTV.
 * KTV - Keyword Token Value
 * */
uT8 *get_KTV()
{
    return uT8_PC token_data->token_info.KW_token_info.keyword_value;
}

/* Get the GTV.
 * GTV - Grammar Token Value
 * */
uT8 *get_GTV()
{
    return uT8_PC make_uT8_ptr(token_data->token_info.GR_token_info.grammar_value);
}

/* Create new data for a token.
 * TT - Token Type
 * */
void make_new_token(enum token_type TT, uT8 *value, uT32 token_value)
{
    /* Make sure we have valid memory. */
    if(!(token_data))
    {
        token_data = calloc(1, sizeof(*token_data));
        lang_assert(token_data,
            "Error allocating memory for token data.\n\tTry rerunning the program.\n",
            OOC_allocation_error)
    }
    else clear_out_token(token_data->type_of_token);

    /* Make sure the allocation occurred. */
    lang_assert(token_data, "Error allocating memory for tokens.\n", OOC_allocation_error)

    token_data->type_of_token = TT;

    /* What type of token is it? */
    switch(TT)
    {
        case KW: {
            token_data->token_info.KW_token_info.keyword_token = token_value;

            token_data->token_info.KW_token_info.keyword_value = calloc(strlen(nT8_PCC value), sizeof(uT8));
            lang_assert(token_data->token_info.KW_token_info.keyword_value, 
                "Error allocating memory for token value.\n\tToken Type(TT): %d\n\tToken Value: %s\n", 
                OOC_allocation_error, TT, value)

            /* Copy over from `value` to `keyword_value`. */
            memcpy(token_data->token_info.KW_token_info.keyword_value, value, strlen(nT8_PCC value));
            memset(&token_data->token_info.KW_token_info.keyword_value[strlen(nT8_PCC value)], '\0', 1);
            
            printf("Created new KW(Keyword) token:\n\tKeyword Token: %d (%s)\n\tKeyword Value: %s\n", token_value, token_name(token_value, NULL, NONE), value);
            break;
        }
        case GR: {
            token_data->token_info.GR_token_info.grammar_token = token_value;
            token_data->token_info.GR_token_info.grammar_value = value[0];
            printf("Created new GR(Grammar) token:\n\tGrammar Token: %d (%s)\n\tGrammar Value: %c\n", token_value, token_name(token_value, NULL, NONE), value[0]);
            break;
        }
        case VD: {
            token_data->token_info.DT_token_info.data_type_token = token_value;
            printf("Created new VD(Variable Declaration) token:\n\tData Type Token: %d (%s)\n\n", token_value, token_name(token_value, NULL, NONE));
            break;
        }
        case DT: {
            token_data->token_info.DT_token_info.data_type_token = token_value;

            token_data->token_info.DT_token_info.data_type_value = calloc(strlen(nT8_PCC value), sizeof(uT8));
            lang_assert(token_data->token_info.DT_token_info.data_type_value, 
                "Error allocating memory for token value.\n\tToken Type(TT): %d\n\tToken Value: %s\n", 
                OOC_allocation_error, TT, value)

            /* Copy over from `value` to `data_type_value`. */
            memcpy(token_data->token_info.DT_token_info.data_type_value, value, strlen(nT8_PCC value));
            memset(&token_data->token_info.DT_token_info.data_type_value[strlen(nT8_PCC value)], '\0', 1);
            
            printf("Created new DT(Datatype) token:\n\tDatatype Token: %d (%s)\n\tDatatype Value: %s\n", token_value, token_name(token_value, NULL, NONE), value);
            break;
        }
        case DEF: break;
        case END: break;
        default: break;
    }

    /* "Free" the memory from `value`. */
    value = NULL;
}

void make_new_token_alone(uT8 *value, _lexer *l)
{
    if(strcmp(nT8_PCC value, "print") == 0) { make_new_token(KW, uT8_PC value, KW_print); return; }
    if(strcmp(nT8_PCC value, "exit") == 0) { make_new_token(KW, uT8_PC value, KW_exit); return; }
    if(strcmp(nT8_PCC value, "int") == 0) { make_new_token(VD, uT8_PC value, DT_integer); return; }
    if(strcmp(nT8_PCC value, "str") == 0) { make_new_token(VD, uT8_PC value, DT_string); return; }
    if(strcmp(nT8_PCC value, "hex") == 0) { make_new_token(VD, uT8_PC value, DT_hex); return; }
    if(strcmp(nT8_PCC value, "include") == 0) { make_new_token(KW, uT8_PC value, KW_include); return; }
    if(strcmp(nT8_PCC value, "incmem") == 0) { make_new_token(KW, uT8_PC value, KW_incmem); return; }

    make_new_token(DT, uT8_PC value, DT_word);
    //lang_error("Unknown keyword `%s` on line %ld.\n", not_a_keyword_error, value, l->line)
}

/* Free any allocation made on the previous token.
 * LT - Last Token
 * */
void clear_out_token(enum token_type LT)
{
    switch(LT)
    {
        case KW: {
            printf("\t\tReleased:\n\t\t\tToken: %d\n\t\t\tValue: %s\n\n", token_data->token_info.KW_token_info.keyword_token, token_data->token_info.KW_token_info.keyword_value);
            free(token_data->token_info.KW_token_info.keyword_value);
            token_data->token_info.KW_token_info.keyword_value = NULL;
            break;
        }
        case GR: {
            printf("\t\tReleased:\n\t\t\tToken: %d\n\t\t\tValue: %c\n\n", token_data->token_info.GR_token_info.grammar_token, token_data->token_info.GR_token_info.grammar_value);
            token_data->token_info.GR_token_info.grammar_value = '\0';
            break;
        }
        case DT: {
            printf("\t\tReleased:\n\t\t\tToken: %d\n\t\t\tValue: %s\n\n", token_data->token_info.DT_token_info.data_type_token, token_data->token_info.DT_token_info.data_type_value);
            free(token_data->token_info.DT_token_info.data_type_value);
            token_data->token_info.DT_token_info.data_type_value = NULL;
            break;
        }
        case DEF: break;
        case END: break;
        default: break;
    }
}

/* Destroy the `token_data` variable.
 * LT - Last Token
 * */
void destroy_token_reference(enum token_type LT)
{
    /* Make sure all data is cleared. */
    clear_out_token(LT);

    free(token_data);
    token_data = NULL;
}

#endif