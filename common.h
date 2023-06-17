#ifndef language
#define language
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* My own types.
 * n - normal(not signed/unsigned)
 * s - signed
 * u - unsigned
 * T - Type
 * L - Long
 * */
typedef char            nT8;
typedef nT8             bool;
typedef signed char     sT8;
typedef unsigned char   uT8;
typedef short           nT16;
typedef signed short    sT16;
typedef unsigned short  uT16;
typedef int             nT32;
typedef signed int      sT32;
typedef unsigned int    uT32;
typedef long            nLNG;
typedef signed long     sLNG;
typedef unsigned long   uLNG;

/* size_t. */
typedef long int            nTL32;
typedef long long           nSIZE;
typedef signed long long    sSIZE;
typedef unsigned long long  uSIZE;

/* Booleans. */
#define true  1
#define false 0

/* Max size of filename(excluding extension). Length of extension. */
#define filename_max_size 10
#define extension_length  3

/* Useful casts.
 * PCC - Pointer Const Cast
 * PC - Pointer Cast
 * */
#define nT8_PCC      (const nT8 *)
#define nT8_PC       (nT8 *)
#define uT8_PCC      (const uT8 *)
#define uT8_PC       (uT8 *)

/* Custom Exit statuses. */
enum custom_errors
{
    /* "Base" errors. */
    unknown_error                   = 0x02,
    no_file_given_error             = 0x03,
    filename_too_long_error         = 0x04,
    wrong_extension_error           = 0x05,
    no_extension_error              = 0x06,
    extension_too_long_error        = 0x07,
    /* Lexer related errors. */
    lexing_error                    = 0x08,
    lexing_grammar_error            = 0x09, // Keyword misspelt
    lexing_tokenization_error       = 0x0A,
    lexing_invalid_hex_value_error  = 0x0B, // Example: 0xABT <- `T` is not a hex value
    lexing_too_large_number_error   = 0x0C, // We'll see about this one
    /* Parser related errors. */
    parser_invalid_syntax           = 0x0D,
    parser_unknown_error            = 0x0E,
    /* File specific erros. */
    file_has_no_data_error          = 0x0F,
    /* OOC(Out Of Context) errors. e.g. an allocation error. */
    OOC_allocation_error            = 0x10,
    OOC_memory_free_error           = 0x11,
    OOC_source_code_read_error      = 0x12,
    /* Token related errors. */
    not_a_keyword_error             = 0x13,
    /* Variable errors. */
    no_variable_name_error          = 0x14,
    /* Grammar errors. */
    invalid_grammar_error           = 0x15,
    missing_equals_error            = 0x16,
    missing_quote_error             = 0x17,
    grammar_mismatch_error          = 0x18,
    /* Macro errors. */
    incmem_not_on_line_1_error      = 0x19,
    /* Others. */
    unexpected_EOF                  = 0x1A,
    unexpect_value_error            = 0x1B,
    trailing_numbers_after_eoh_error= 0x1C, // eoh = end of hex
    unexpected_end_of_hex_error     = 0x1D,
    unexpected_end_of_decimal_error = 0x1E,
    expected_DQ_error               = 0x1F,
    unexpected_new_line_error       = 0x20,
    file_not_exist_error            = 0x21,
    /* Error specifically for `.mem` files. */
    invalid_hex_type_error          = 0x22,
    missing_mem_size_type_error     = 0x23,
    missing_parts_error             = 0x24,
};

/* Colors for printing. */
#define error_color     "\e[0;31m"
#define warning_color   "\e[0;33m"
#define reset           "\e[0m"

/* Assertion/Error. */
#define lang_error(err_msg, error_code, ...)               \
{                                                          \
    fprintf(stderr, "\n%s[ERROR]%s ", error_color, reset); \
    fprintf(stderr, err_msg, ##__VA_ARGS__);               \
    fprintf(stderr, "\n");                                 \
    exit(error_code);                                      \
}

#define lang_assert(cond, err_msg, error_code, ...)     \
    if(!(cond))                                         \
        lang_error(err_msg, error_code, ##__VA_ARGS__)  \

/* Is it still ASCII? */
#define is_ascii(c)     (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ? true : false

/* Is it ASCII, with an exception? */
#define is_ascii_with_exception(c, exc) is_ascii(c) || c == exc ? true : false

/* Is it a number? */
#define is_number(c)    (c >= '0' && c <= '9') ? true : false

/* Is it a "hex valid" ascii value? (A-F) */
#define is_hex_valid_ascii(c) ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) || is_number(c) ? true : false

/* */

// Dumb little function to shorten the amount of code
uT8 *initiate_path(uT8 data1[], uT8 data2[])
{
	/* `data1`, at least, has to be valid and not NULL. */
	lang_assert(data1, "Cannot initiate the path. No data given to configure the path.\n", unknown_error)

	/* Initiate `array` and concat `data1` to it. */
	uT8 *array = calloc(strlen(nT8_PCC data1), sizeof(*array));
    lang_assert(array,
        "Error allocating memory for `initiate_path`.\n\tTry rerunning the program.\n",
        OOC_allocation_error)
	strcat(nT8_PC array, nT8_PC data1);

	/* If `data2` is not NULL, go ahead and perform the same action as above. */
	if(data2 != NULL)
	{
		array = realloc(array, ((strlen(nT8_PCC data1) + 1) + strlen(nT8_PCC data2)) * sizeof(*array));
		lang_assert(array,
            "Error reallocating memory.\n\tThe heap might be getting overused, this is a internal problem.\n\tReach out to the developers for this issue to be fixed.\n",
            OOC_allocation_error)
    
		lang_assert(strlen(nT8_PCC data1) + strlen(nT8_PCC data2) <= 80, 
			"Path is too large: %s.\nFAMP only allows up to 50 characters for a path.\n", unknown_error, strcat(nT8_PC array, nT8_PC data2))
		
		strcat(nT8_PC array, nT8_PC data2);
	}

	return array;
}

/* ---------- Language Backend Code. ---------- */
#include "language_backend/sum.h"
/* -------------------------------------------- */

/* Check file extension and length. */
bool check_file(nT8 *filename)
{
    uT8 i = 0;

    /* Where does the extension start in the filename? */
    nT8 extension_start_index = 0;

    /* If `filename` is a path(e.g. `dir/file.sum`) we want to know where the filename starts. */
    nT8 file_name_start_index = 0;

    nT8 *extension = malloc(4 * sizeof(nT8));
    lang_assert(extension,
        "Error allocating memory for `check_file`.\n\tTry rerunning the program.\n",
        OOC_allocation_error)
    uT8 index = 0;
    memset(extension, 0, 4);

    /* Before we check the length, check if it is a full given path. */
    for(; i < strlen(nT8_PCC filename); i++)
    {
        if(filename[i] == '/') { file_name_start_index = i; break; }
        if(i == strlen(nT8_PCC filename) - 1) i = 0;
    }

    /* Get the start of the extension. */
    for(i = file_name_start_index + 1; i < strlen(nT8_PCC filename); i++)
        if(filename[i] == '.') { extension_start_index = i + 1; break; }
    
    /* Make sure the filename does not exceed the max length. */
    lang_assert(strlen(nT8_PCC filename) - extension_start_index <= 10, "The file `%s` has a length > 10.\n", filename_too_long_error, filename)

    for(i = 0; i < strlen(nT8_PCC filename); i++)
        if(filename[i] == '.') { extension_start_index = i + 1; break; }
    
    lang_assert(extension_start_index != 0, "The file `%s` has no extension.\n\tAdd the extension `.sum` to the file.\n", no_extension_error, filename)
    lang_assert((strlen(nT8_PCC filename) - (i + 1)) == extension_length, "The extension for the file `%s` is too long.\n\tThe extension should be `.sum`.\n", extension_too_long_error, filename)
    
    /* Get the extension. */
    for(i = i+1; i < strlen(nT8_PCC filename); i++)
    { extension[index] = filename[i]; index++; }
    
    if(strcmp(extension, "sum") == 0) { free(extension); return true; }

    free(extension);
    return false;
}

#endif