#include <stdio.h>

#include "lexer.h"

#define MAX_INPUT_SIZE 160

int main(int argc, char** argv)
{
    // Citanie vstupneho retazca
    printf("Vstupny retazec: ");
    char source[MAX_INPUT_SIZE];
    fgets(source, MAX_INPUT_SIZE, stdin);

    init_lexer(source);
    print_tokens();

    //getchar();
    return 0;
}
