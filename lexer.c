#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "lexer.h"

/* Nazvy symbolov (len pre ich jednoduchsi vypis) */
const char *SYM_NAMES[] = {
        [VALUE] = "VALUE", [ID] = "ID",
        [READ] = "READ", [PRINT] = "PRINT", [SET] = "SET",
        [PLUS] = "PLUS", [MINUS] = "MINUS", [MUL] = "MUL", [DIV] = "DIV", [POWER] = "POWER",
        [LPAR] ="(", [RPAR]=")",
        [LCB] = "{", [RCB] = "}",
        [LSB] = "[", [RSB] = "]",
        [COMMA] = "COMMA",
        [AND] = "AND",
        [POINTER] = "POINTER", [VOID] = "VOID", [EXEC] = "EXEC",
        [INCR] = "INCR", [DECR] = "DECR",
        [LT] = "LT", [LE] = "LE", [GT] = "GT", [GE] = "GE", [EQ] = "EQ", [NE] = "NE",
        [ASSIGN] = "ASSIGN", [BOOL] = "BOOL", [CHAR] = "CHAR",
        [WHILE] = "WHILE", [FOR] = "FOR",
        [IF] = "IF", [ELSE] = "ELSE",
        [DECLARE] = "DECLARE",
        [EOC] = "EOC", [SEOF]="SEOF", [SERROR]="SERROR", [EXIT] = "EXIT"
};

/* Globalne premenne, "public" */
Symbol lex_symbol;
int lex_attr;

char *lex_ids[LEX_IDS_MAX];
int lex_ids_size; // Pocet ulozenych identifikatorov

char *lex_pointers[LEX_POINTERS_MAX];
int lex_pointers_size; // Pocet ulozenych identifikatorov


/* Vstupne premenne */
static char *input;     // Vstupny retazec
static char c;          // Spracovavany vstupny znak
static int ic;          // Index dalsieho znaku vo vstupnom retazci


/* Inicializacia lex. analyzatora. Parametrom je vstupny retazec. */
void init_lexer(char *string)
{
    input = string;
    ic = 0;
    lex_ids_size = 0;
}


/* Ulozenie identifikatora `id` do tabulky identifikatorov ak tam este nie je.
 * Vracia index, na ktorom je identifikator ulozeny. */
int store_id(char *id)
{
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

int store_pointer(char *id)
{
    int i = 0;
    while (i < lex_pointers_size) {
        if (strcmp(id, lex_pointers[i]) == 0)
            return i;
        i++;
    }
    lex_pointers[i] = strdup(id);
    lex_pointers_size++;
    return i;
}


/* Precitanie dalsieho symbolu.
 * Volanie nastavi nove hodnoty lex_symbol a lex_attr. */
void next_symbol()
{
    c = input[ic];
    ic++;

    while (isspace(c)) { // Preskocenie medzier
        c = input[ic];
        ic++;
    }

    position = ic;

    switch (c) {
        case '&':
            lex_symbol = AND;
            break;
        case ';':
            lex_symbol = EOC;
            break;
        case ',':
            lex_symbol = COMMA;
            break;
        case '+':
            lex_symbol = PLUS;
            break;
        case '-':
            lex_symbol = MINUS;
            break;
//        case '*':
//            lex_symbol = MUL;
//            break;
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
        case '[':
            lex_symbol = LSB;
            break;
        case ']':
            lex_symbol = RSB;
            break;
        case '{':
            lex_symbol = LCB;
            break;
        case '}':
            lex_symbol = RCB;
            break;
        case '\0':
            lex_symbol = SEOF;
            break;

        default:
            if (isdigit(c)) {

                lex_symbol = VALUE;
                lex_attr = 0;

                //convert string to int
                do {
                    lex_attr *= 10;
                    lex_attr += c - 48;

                    c = input[ic];
                    ic++;
                } while (isdigit(c));

                //Release char
                ic--;

            } else if (isalpha(c) || (c == '*' && isalpha(input[ic]))) {
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
                } else if (strcmp(id, "if") == 0) {
                    lex_symbol = IF;
                } else if (strcmp(id, "else") == 0) {
                    lex_symbol = ELSE;
                } else if (strcmp(id, "while") == 0) {
                    lex_symbol = WHILE;
                } else if (strcmp(id, "for") == 0) {
                    lex_symbol = FOR;
                } else if (strcmp(id, "set") == 0) {
                    lex_symbol = SET;
                } else if (strcmp(id, "bool") == 0) {
                    lex_symbol = BOOL;
                } else if (strcmp(id, "char") == 0) {
                    lex_symbol = CHAR;
                } else if (strcmp(id, "declare") == 0) {
                    lex_symbol = DECLARE;
                } else if (strcmp(id, "incr") == 0) {
                    lex_symbol = INCR;
                } else if (strcmp(id, "decr") == 0) {
                    lex_symbol = DECR;
                } else if (strcmp(id, "exit") == 0) {
                    lex_symbol = EXIT;
                } else if (strcmp(id, "void") == 0) {
                    lex_symbol = VOID;
                } else if (strcmp(id, "exec") == 0) {
                    lex_symbol = EXEC;
                } else if (id[0] == '*') {
                    lex_attr = store_pointer(id);
                    lex_symbol = POINTER;
                } else {
                    // Ulozenie do tabulky identifikatorov
                    lex_attr = store_id(id);
                    lex_symbol = ID;
                }
                free(id);
            } else if (c == '=' && input[ic] == '=') {
                lex_symbol = EQ;
                ic++;
            } else if (c == '=' && input[ic] != '=') {
                lex_symbol = ASSIGN;
            } else if (c == '!' && input[ic] == '=') {
                lex_symbol = NE;
                ic++;
            } else if (c == '<') {
                if (input[ic] == '=') {
                    ic++;
                    lex_symbol = LE;
                } else {
                    lex_symbol = LT;
                }
            } else if (c == '>') {
                if (input[ic] == '=') {
                    ic++;
                    lex_symbol = GE;
                } else {
                    lex_symbol = GT;
                }
            } else if (c == '*') {
                lex_symbol = MUL;
            } else {
                lex_symbol = SERROR;
            }
    }
}

/* Nazov lexikalnej jednotky */
const char *symbol_name(Symbol symbol)
{
    return SYM_NAMES[symbol];
}

/* Vypis vsetky lexikalnych jednotiek zo vstupu */
void print_tokens()
{
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

        if (lex_symbol == POINTER) {
            printf(" <%d> -> %s", lex_attr, lex_pointers[lex_attr]);
        }

        printf("\n");
    } while (lex_symbol != SEOF);
}
