#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "interpreter.h"
#include "lexer.h"

/*** Syntakticky analyzator a interpretator ***/

int variables[LEX_IDS_MAX];

//
void crash(char *expected) {
    fprintf(stderr, "CHYBA: Chyba %s, namiesto toho sa vyskytol %s.\n", expected, symbol_name(lex_symbol));
    exit(1);
}

void read_variable(int id_idx) {
    int value;
    printf("ID <%d> -> %s = ", id_idx, lex_ids[id_idx]);
    scanf("%d", &value);
    variables[id_idx] = value;
}

/* Overenie symbolu na vstupe a precitanie dalsieho.
 * Vracia atribut overeneho symbolu. */
int match(const Symbol expected) {

    if (lex_symbol != expected) {
        crash((char *) SYM_NAMES[expected]);
    }

    int attr = lex_attr;
    next_symbol();
    return attr;
}

/* Term -> VALUE | "(" Expr ")" */
int term() {
    int value = 0;

    switch (lex_symbol) {

        case VALUE:
            value = lex_attr;
            next_symbol();
            break;

        case ID:
            next_symbol();
            value = variables[lex_attr];
            break;

        case LPAR:
            next_symbol();
            value = expr();
            match(RPAR);
            break;

        default:
            crash("operand");
            break;
    }

    return value;
}

/* Expr -> Term {("+"|"-") Term} */
int expr() {

    int leftOp, rightOp;

    Symbol operator;
    leftOp = term();

    while (lex_symbol == PLUS || lex_symbol == MINUS) {
        operator = lex_symbol;
        next_symbol();
        rightOp = term();
        switch (operator) {
            case PLUS:
                leftOp = leftOp + rightOp;
                break;
            case MINUS:
                leftOp = leftOp - rightOp;
                break;
            default:
                assert("Neocakavany operator v expr()");
        }
    }

    return leftOp;
}

// read -> "read" ID {"," ID}
void read() {

    match(READ);

    read_variable(match(ID));

    while (lex_symbol == COMMA) {
        next_symbol();
        read_variable(match(ID));
    }
}

// print → "print" expr
void print() {
    match(PRINT);
    printf("Vysledok: %d\n", expr());
}
