#include <stdio.h>

#include "lexer.h"
#include "interpreter.h"
#include "generator.h"

#define MAX_INPUT_SIZE 160

int main(int argc, char **argv)
{
    printf("Vstupny retazec: ");

    char source[MAX_INPUT_SIZE];
    fgets(source, MAX_INPUT_SIZE, stdin);

    init_lexer(source);
    print_tokens();

    FILE *output_file = fopen("../program.bin", "wb");
    init_generator(output_file);

    write_begin((short) lex_ids_size);

    write_string("\n---START OF EXECUTION---\n");
    write_string(source);

    printf("\nZaciatok syntaxou riadenej interpretacie\n\n");
    init_lexer(source);
    next_symbol();

    program();

    write_string("\n---END OF EXECUTION---");

    write_end();

    generate_output();
    fclose(output_file);
    printf("Program vygenerovany v program.bin\n");


    /*
     * printf("Spustit Computron? [Y|n]");
     * if (getchar() != 'n') {
     * system("java -jar ../Computron_VM.jar");
     * }
     */

    return 0;
}
