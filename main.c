#include <stdio.h>

#include "lexer.h"
#include "interpreter.h"

#define MAX_INPUT_SIZE 160

int main(int argc, char** argv)
{
    printf("Vstupny retazec: ");

    char source[MAX_INPUT_SIZE];
    fgets(source, MAX_INPUT_SIZE, stdin);

    init_lexer(source);
    print_tokens();

    printf("\nZaciatok syntaxou riadenej interpretacie\n\n");
    init_lexer(source);
    next_symbol();

    read();
    print();

    return 0;
}
