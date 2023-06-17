#ifndef memory_outline
#define memory_outline

/* Depending on the type of PD Variable, each element that the PD variable takes up
 * will be multiplied by one of the following to get the correct amount of bytes
 * that will be used.
 * */
#define byte_size       0x01
#define word_size       0x02
#define dword_size      0x04

enum memory_types
{
    byte,
    MB,
    GB
};

enum predefined_variable_types
{
    T_data = 0x20,         // `.data` in assembly, so to say
    T_rodata = 0x21,       // `.rodata` in assembly (read-only)
    T_stack_based = 0x22   // lives on the stack
};

/* Users can predefine variables in the `.mem` file. */
typedef struct predefined_variables
{
    /* Predefined variable name. */
    uT8         *PD_var_name;

    /* Predefined variable size (in bytes only).
     * This cannot surpass the overall size of the program.
     * With `byte_data`, this will be 1 (byte)
     * With `ptr_byte_data`, this will be however much bytes the predefined variables takes up
     * With `word_data`, this will be 2 (bytes)
     * With `ptr_word_data`, this will be however much bytes (in multiples of 2) the predefined variable takes up
     * With `dword_data`, this will be 4 (bytes)
     * With `ptr_dword_data`, this will be however much bytes (in multiples of 4) the predefined variables takes up
     * */
    uT32        PD_var_size;

    /* Predefined variable data, if any. */
    union {
        /*
         * byte_data - strictly a single byte of occupied memory
         * ptr_byte_data - X amount of byte sized elements occupied memory
         * */
        uT8     byte_data;
        uT8     *ptr_byte_data;

        /*
         * word_data - strictly 2 bytes of occupied memory
         * ptr_word_data - X amount of word sized elements occupied in memory
         * */
        uT16    word_data;
        uT16    *ptr_word_data;

        /*
         * dword_data - strictly 4 bytes of occupied memory
         * ptr_dword_data - X amount of dword sized elements occupied in memory
         * */
        uT32    dword_data;
        uT32    *ptr_dword_data;
    } PD_var_data;

    /* If the user wants the predefined variable to be constant, they'll specify this variable
     * "lives" in rodata. If they want it to be mutable throughout the program, they'll specify
     * the variable "lives" in data. If they want to store it and use it only when needed they'll
     * specify the variable "lives" on the stakc.
     *
     * A pro about a variable living on the stack: it can switch from being "constant" to being "mutable".
     * */
    enum predefined_variable_types PD_var_type;
} _predefined_variables;

/* `.mem` files explain how to use memory in a `.sum` file. */
typedef struct memory_info
{
    /* Total memory the program is allowed to use. */
    uT32                    total_memory;

    /* Program size in bytes, MB and GB. */
    uT32                    mem_in_bytes;
    float                   mem_in_MB;
    float                   mem_in_GB;

    /* What is `total_memory` representing? */
    enum memory_types       mem_type;

    /* Can this program push/pop on the stack? */
    bool                    stack_access;

    /* Do variables HAVE to be initialized? */
    bool                    require_initialized_variables;

    /* Predefined variables. */
    _predefined_variables   **PD_vars;

    /* Size of `PD_vars`. */
    uT32                    PD_vars_size;
} _memory_info;

static _memory_info *program_memory_info = NULL;

/* Default values just in case user does not create there own `.mem` file.
 * `default_program_bytesize` - allow programs up to 1MB.
 * */
#define default_program_bytesize        0x100000

void init_program_memory_info()
{
    program_memory_info = calloc(1, sizeof(*program_memory_info));

    program_memory_info->PD_vars = NULL;
    program_memory_info->PD_vars_size = 0;

    program_memory_info->total_memory = default_program_bytesize;
    program_memory_info->mem_type = byte;

    /* Different memory representation of program size. */
    program_memory_info->mem_in_bytes = program_memory_info->total_memory;
    program_memory_info->mem_in_MB = program_memory_info->mem_in_bytes / 1024 / 1024;
    program_memory_info->mem_in_GB = program_memory_info->mem_in_MB / 1024;

    program_memory_info->stack_access = true;
    program_memory_info->require_initialized_variables = false;
}

void try_init_PD_vars()
{
    if(program_memory_info->PD_vars == NULL && program_memory_info->PD_vars_size == 0)
    {
        program_memory_info->PD_vars = calloc((program_memory_info->PD_vars_size + 1), sizeof(*program_memory_info->PD_vars));
        lang_assert(program_memory_info->PD_vars,
            "Error allocating memory for PD Variables.\n\tTry rerunning the program.\n",
            OOC_allocation_error)
    }

    if(!(program_memory_info->PD_vars[program_memory_info->PD_vars_size]))
    {
        program_memory_info->PD_vars[program_memory_info->PD_vars_size] = calloc(1, sizeof(*program_memory_info->PD_vars[program_memory_info->PD_vars_size]));
        lang_assert(program_memory_info->PD_vars[program_memory_info->PD_vars_size],
            "Error allocating new PD Variable index.\n\tTry rerunning the program.\n",
            OOC_allocation_error)
    }
}

void create_next_PD_var_element()//(uT8 *var_name, enum predefined_variable_types var_type, uT32 memory_needed, void *data)
{
    program_memory_info->PD_vars_size++;
    program_memory_info->PD_vars = realloc(
        program_memory_info->PD_vars,
        (program_memory_info->PD_vars_size + 1) * sizeof(*program_memory_info->PD_vars)
    );

    program_memory_info->PD_vars[program_memory_info->PD_vars_size] = calloc(1, sizeof(*program_memory_info->PD_vars[program_memory_info->PD_vars_size]));
    lang_assert(program_memory_info->PD_vars[program_memory_info->PD_vars_size],
        "Error allocating memory for new PD Variable index.\n\tTry rerunning the program.\n",
        OOC_allocation_error)
}

void assign_PD_var_name(uT8 *name)
{
    /* Make sure we have enough memory. */
    program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_name = calloc(strlen(nT8_PCC name), sizeof(*program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_name));
    lang_assert(program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_name, 
        "Error allocating memory for new PD variable name.\n\tTry rerunning the program.\n", 
        OOC_allocation_error)

    memcpy(program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_name, name, strlen(nT8_PCC name));
    memset(&program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_name[strlen(nT8_PCC name)], 0, 1);
}
uT8 *get_curr_PD_var_name()
{
    return program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_name;
}

void assign_PD_storage_place(enum predefined_variable_types place)
{
    program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_type = place;
}
uT8 *get_curr_PD_placement()
{
    switch(program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_type)
    {
        case T_data: return uT8_PC "Data";break;
        case T_rodata: return uT8_PC "Rodata";break;
        case T_stack_based: return uT8_PC "Stack";break;
        default: break;
    }
    return uT8_PC "Unknown placement";
}

void assign_PD_var_size(uT32 size)
{
    program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_size = size;
}

void assign_PD_var_value_byte(uT8 value, uT16 index)
{
    if(program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_size > 1)
    {
        if(!(program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_data.ptr_byte_data))
            program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_data.ptr_byte_data = calloc(program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_size, sizeof(*program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_data.ptr_byte_data));
    
        memset(&program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_data.ptr_byte_data[index], value, 1);
        return;
    }

    program_memory_info->PD_vars[program_memory_info->PD_vars_size]->PD_var_data.byte_data = value;
}

#endif