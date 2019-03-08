#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

/* Nazvy symbolov (len pre ich jednoduchsi vypis) */
const char *SYM_NAMES[] = {
        [VALUE]="VALUE", [ID]="ID", [READ]="READ", [PRINT]="PRINT",
        [PLUS]="PLUS", [MINUS]="MINUS", [MUL]="MUL", [DIV]="DIV", [POWER]="POWER",
        [LPAR]="LPAR", [RPAR]="RPAR", [COMMA]="COMMA",
        [SEOF]="SEOF", [SERROR]="SERROR"
};

/* Globalne premenne, "public" */
Symbol lex_symbol;
int lex_attr;

char *lex_ids[LEX_IDS_MAX];
int lex_ids_size; // Pocet ulozenych identifikatorov


/* Vstupne premenne */
static char *input;     // Vstupny retazec
static char c;          // Spracovavany vstupny znak
static int ic;          // Index dalsieho znaku vo vstupnom retazci


/* Inicializacia lex. analyzatora. Parametrom je vstupny retazec. */
void init_lexer(char *string) {
    input = string;
    ic = 0;
    lex_ids_size = 0;
}


/* Ulozenie identifikatora `id` do tabulky identifikatorov ak tam este nie je.
 * Vracia index, na ktorom je identifikator ulozeny. */
int store_id(char *id) {
    int i = 0;
    while (i < lex_ids_size) {
        if (strcmp(id, lex_ids[i]) == 0)
            return i;
        i++;
    }
    lex_ids[i] = strdup(id);
    lex_ids_size++;
    return i;
}


/* Precitanie dalsieho symbolu.
 * Volanie nastavi nove hodnoty lex_symbol a lex_attr. */
void next_symbol() {

    c = input[ic];
    ic++;

    while (isspace(c)) { // Preskocenie medzier
        c = input[ic];
        ic++;
    }

    switch (c) {
        case ',':
            lex_symbol = COMMA;
            break;
        case '+':
            lex_symbol = PLUS;
            break;
        case '-':
            lex_symbol = MINUS;
            break;
        case '*':
            lex_symbol = MUL;
            break;
        case '/':
            lex_symbol = DIV;
            break;
        case '^':
            lex_symbol = POWER;
            break;
        case '(':
            lex_symbol = LPAR;
            break;
        case ')':
            lex_symbol = RPAR;
            break;
        case '\0':
            lex_symbol = SEOF;
            break; // Koniec retazce
        default:
            if (isdigit(c)) {

                lex_symbol = VALUE;

                do {
                    lex_attr *= 10;
                    lex_attr += c - 48;

                    c = input[ic];
                    ic++;
                } while (isdigit(c));

                //Release char
                ic--;

            } else if (isalpha(c)) {
                int id_start = ic - 1; // Index zaciatku identifikatora
                do {
                    c = input[ic];
                    ic++;
                } while (isalnum(c));
                ic--; // "Vratenie" posledneho znaku

                // Skopirovanie identifikatora
                // char *id = strndup(&input[id_start], ic - id_start);
                int id_len = ic - id_start;
                char *id = (char *) (malloc((size_t) (id_len + 1)));
                memcpy(id, &input[id_start], (size_t) (id_len));
                id[id_len] = 0;

                // Kontrola klucovych slov
                if (strcmp(id, "read") == 0) {
                    lex_symbol = READ;
                } else if (strcmp(id, "print") == 0) {
                    lex_symbol = PRINT;
                } else { // Ulozenie do tabulky identifikatorov
                    lex_attr = store_id(id);
                    lex_symbol = ID;
                }
                free(id);
            } else {
                lex_symbol = SERROR;
            }
    }
}

/* Nazov lexikalnej jednotky */
const char *symbol_name(Symbol symbol) {
    return SYM_NAMES[symbol];
}

/* Vypis vsetky lexikalnych jednotiek zo vstupu */
void print_tokens() {
    printf("\nVystup lexikalnej analyzy (retazec symbolov)\n");
    do {
        next_symbol();

        printf("  [%2d] %s", lex_symbol, symbol_name(lex_symbol));

        if (lex_symbol == VALUE) {
            printf(" <%d>", lex_attr);
        }

        if (lex_symbol == ID) {
            printf(" <%d> -> %s", lex_attr, lex_ids[lex_attr]);
        }

        printf("\n");
    } while (lex_symbol != SEOF);
}
