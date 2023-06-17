#ifndef ast
#define ast

enum action
{
    print_statement = 0x0,
    variable_decl,
    ast_tree_init
};

enum var_decl_DT
{
    Char = 0x03,
    Str,
    Int,
    Hex
};

enum ast_state
{
    stuck = 0x07,
    comitted,
    adding_variable_decl,
    adding_print_statement,
    adding_function,
    ready
};

typedef struct ast_tree
{
    enum action action_occurred;

    union {
        struct {
            uT8 *value_to_print;
        } print;

        struct {
            enum var_decl_DT variable_datatype;
            uT8 *variable_name;
            
            union {
                uT8 *string_value;
                uT32 integer_value;
                uT8 char_value;
                uT8 *hex_value;
            } variable_value;
        } var_declaration;
    } action_data;

    /* What is the "state" of the ast at the current index?
     * `stuck` - an error ocurred. The ast is trying to fix it.
     * `comitted` - the program is done, the ast is complete.
     * `adding_variable_decl` - the ast is creating a "tree entry" for a variable declaration.
     * `adding_print_statement` - the ast is creating a "tree entry" for a print statement.
     * `adding_function` - the ast is creating a "tree entry" for a function.
     * `ready` - the current index of the ast is ready to be filled with data.
     * */
    enum ast_state      state;

    /* When `state` is `comitted`, `entire_tree` will be assigned to `tree`. */
    struct ast_tree     **entire_tree;
} _ast_tree;

_ast_tree **tree = NULL;
static uT32 tree_index = 0;

/* Create a new "entry" in the tree.
 * AO - Action Ocurred
 * */
void new_tree_entry(enum action AO)
{
    /* Make sure we have memory allocated. */
    if(!(tree)) tree = calloc((tree_index + 1), sizeof(*tree));
    lang_assert(tree, 
        "Error allocating memory for the AST tree.\n", 
        OOC_allocation_error)

    if(!(tree[tree_index])) tree[tree_index] = calloc(1, sizeof(*tree[tree_index]));
    lang_assert(tree[tree_index], 
        "Error allocating memory for the AST tree entry %d.\n", 
        OOC_allocation_error, tree_index)

    switch(AO)
    {
        case print_statement: {
            tree[tree_index]->action_data.print.value_to_print = token_data->token_info.DT_token_info.data_type_value;
            tree[tree_index]->state = adding_print_statement;
            tree_index++;

            tree = realloc(
                tree,
                (tree_index + 1) * sizeof(*tree)
            );
            tree[tree_index] = calloc(1, sizeof(*tree[tree_index]));
            lang_assert(tree[tree_index], 
                "Error allocating memory for the AST tree entry %d.\n", 
                OOC_allocation_error, tree_index)
            tree[tree_index]->state = ready;

            break;
        }
        case variable_decl: {
            tree[tree_index]->state = adding_variable_decl;
            exit(0);
            break;
        }
        case ast_tree_init: {
            tree[tree_index]->state = ready;
            break;
        }
        default: break;
    }
}

/* Commit the ast.
 * Set the state of the ast to "official".
 * */
void commit_ast()
{
    /* Make sure there is valid memory allocated for the last index of the ast. */
    if(!tree[tree_index])
    {
        tree[tree_index] = calloc(1, sizeof(*tree[tree_index]));
        lang_assert(tree[tree_index], 
            "Error allocating memory for the AST tree entry %d.\n", 
            OOC_allocation_error, tree_index)
    }
    
    /* Set the state of the ast to `comitted`. Store the entire "tree" in `entire_tree`. */
    tree[tree_index]->state = comitted;
    tree[tree_index]->entire_tree = tree;
}
bool ast_has_been_comitted()
{
    return tree[tree_index]->state == comitted ? true : false;
}
bool check_is_EOF(_parser *p)
{
    if(lexer_peek_and_return(p->lang_lexer) == '\0')
        commit_ast();
    
    return ast_has_been_comitted();
}

void destroy_tree()
{
    for(uT32 i = 0; i < tree_index; i++)
    {
        switch(tree[i]->action_occurred)
        {
            case print_statement: {
                free(tree[i]->action_data.print.value_to_print);
                tree[i]->action_data.print.value_to_print = NULL;
                
                free(tree[i]);
                tree[i] = NULL;
                break;
            }
            case variable_decl: {
                switch(tree[i]->action_data.var_declaration.variable_datatype)
                {
                    case Str: {
                        free(tree[i]->action_data.var_declaration.variable_value.string_value);
                        tree[i]->action_data.var_declaration.variable_value.string_value = NULL;
                        break;
                    }
                    case Char: {
                        tree[i]->action_data.var_declaration.variable_value.char_value = '\0';
                        break;
                    }
                    case Int: break;
                    case Hex: {
                        free(tree[i]->action_data.var_declaration.variable_value.hex_value);
                        tree[i]->action_data.var_declaration.variable_value.hex_value = NULL;
                    }
                }

                free(tree[i]);
                tree[i] = NULL;
                break;
            }
            default: break;
        }
    }
}

#endif