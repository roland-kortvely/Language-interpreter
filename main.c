#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "interpreter.h"
#include "generator.h"

#define MAX_INPUT_SIZE 20000

int main(int argc, char **argv)
{
    printf("Vstupny program \n");
    printf("--------------- \n");

    char *source = (char *) calloc(MAX_INPUT_SIZE, 1);

    int c;

    for (int i = 0; i < MAX_INPUT_SIZE; ++i) {
        c = getchar();
        if (c == ':') { break; }
        if (c == '\n') { c = ' '; }

        source[i] = (char) c;
    }

    init_lexer(source);
    print_tokens();

    FILE *output_file = fopen("../program.bin", "wb");
    init_generator(output_file);

    printf("\nZaciatok syntaxou riadenej interpretacie\n\n");
    init_lexer(source);
    next_symbol();

    declare(E SEOF);

    if (lex_symbol != SEOF) {
        error("Ocakavany je koniec suboru", E SEOF);
    }

    write_end();

    if (errors > 0) {
        printf("\n%s\n", source);

        int len = (int) strlen(source) + 2;

        for (int i = 0, h = 0; i < len; i++) {

            for (int j = 0; j < errors; j++) {
                if ((i + 1) == error_position[j]) {
                    printf("^");
                    h = 1;
                }
            }
            if (h == 0) {
                printf(" ");
            }
        }
        printf("\n");


        printf("Program nebol vygenerovany. Pocet chyb: %d\n", errors);
        fclose(output_file);
        exit(1);
    }

    generate_output();
    fclose(output_file);
    printf("Program vygenerovany v program.bin\n");

    free(source);
    return 0;
}
