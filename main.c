#include <stdio.h>
#include "common.h"

int main(int args, char *argv[])
{
    lang_assert(args > 1, "Expected file as argument.\n", no_file_given_error)

    lang_assert(check_file(argv[1]), "The file `%s` has the wrong extension.\n\tThe extension should be `.sum`.\n", wrong_extension_error, argv[1])

    run(argv[1]);

    return 0;
}