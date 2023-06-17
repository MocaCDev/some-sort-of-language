#ifndef dot_mem_parser
#define dot_mem_parser

typedef struct DotMemParser
{
    _MemLexer       *DM_lexer;
    _DotMemToken    token_data;
} _DotMemParser;

_DotMemParser *init_dot_mem_parser(_MemLexer *DM_lexer)
{
    _DotMemParser *DM_parser = calloc(1, sizeof(*DM_parser));

    DM_parser->DM_lexer = DM_lexer;
    return DM_parser;
}

#endif