#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "interpreter.h"
#include "lexer.h"
#include "generator.h"

/*** Syntakticky analyzator a interpretator ***/

//int variables[LEX_IDS_MAX];

void error(const char *msg, KeySet K)
{
    fprintf(stderr, "CHYBA na pozicii %d: %s, vstup bol %s\n", position, msg, symbol_name(lex_symbol));

    error_position = realloc(error_position, (size_t) ++errors);
    error_position[errors - 1] = position;

    while (!(E lex_symbol & K)) {
        next_symbol();
    }
}

void check(const char *msg, KeySet K)
{
    if ((E lex_symbol & K)) {
        return;
    }

    error(msg, K);
}

/* Overenie symbolu na vstupe a precitanie dalsieho.
 * Vracia atribut overeneho symbolu. */
int match(const Symbol expected, KeySet K)
{
    if (lex_symbol != expected) {

        char *msg = malloc(255);
        snprintf(msg, 255, "CHYBA: Chyba %s, namiesto toho sa vyskytol %s.\n", symbol_name(expected),
                 symbol_name(lex_symbol));

        error(msg, K);
        return 0;
    }

    int attr = lex_attr;
    next_symbol();
    return attr;
}

//void read_variable(int id_idx)
//{
//
//    /*
//    int value;
//    printf("ID <%d> -> %s = ", id_idx, lex_ids[id_idx]);
//    scanf("%d", &value);
//    variables[id_idx] = value;
//    */
//
//    write_ask_var((short) id_idx, lex_ids[id_idx]);
//}

/* Gramatika:
*
* Declare -> [declare id{"," id}] Program
* Program -> Process {";" Process}
* Process -> (Read | Save | Print | While | If)
* Read -> "read" id{"," id}
* Save -> "save" id "=" Expr {"," id Expr}
* Print -> "write" ("bool" Cond | Expr ) {"&" ("bool" Cond | Expr )}
* While → "while" Cond "{" Program "}"
* If → "if" Cond "{" Program ["@" Program] "}"
* Cond -> Expr [("<" | ">" | "<=" | ">=" | "==" | "!=") Expr]
* Expr -> Mul {("+" | "-") Mul}
* Mul -> Power {("*" | "/") Power}
* Power -> Term ["^" Power]
* Term -> VALUE | "(" Expr ")" | ID
*/

/* Declare -> [id{"," id}] Program */
void declare(KeySet K)
{
    if (lex_symbol != DECLARE) {
        write_begin((short) lex_ids_size);
        program(K);
        return;
    }

    match(DECLARE, K);

    check("Ocakavane ID", (E ID) | K);

    if ((E lex_symbol) & E ID) {

        flags[match(ID, K)][0] = 1;

        check("Ocakavana \",\"", E COMMA | E SEP | K);

        while ((E lex_symbol) & (E COMMA | E ID)) {

            if ((E lex_symbol) & E COMMA) {
                next_symbol();
            }

            if ((E lex_symbol) & E ID) {
                if (flags[lex_attr][0] == 0) {
                    flags[lex_attr][0] = 1;
                    next_symbol();
                } else {
                    error("Premenna bola deklarovana", E COMMA | E SEP | K);
                }
            } else {
                error("Ocakavane ID", E ID | K);
            }

            check("Ocakavana \",\"", E COMMA | E SEP | E ID | K);
        }
    }

    match(SEP, K);

    write_begin((short) lex_ids_size);
    program(K);
}

/* Program -> Process {";" Process} */
void program(KeySet K)
{

    do {
        check("Ocakava sa prikaz", E SEP | E READ | E PRINT | E SAVE | E WHILE | E IF | K);

        if (lex_symbol == SEP) {
            next_symbol();
        }

        if (lex_symbol == SEOF) {
            return;
        }

        process((E SEP) | K);
    } while ((E lex_symbol) & E SEP);

    /*
    check("Ocakavany tag", E READ | E PRINT | E SAVE | E WHILE | E IF | K);
    process((E SEP) | K);
    check("Ocakava sa \";\", \"|\", \"]\", \"}\" alebo koniec suboru", (E SEP) | K);

    while ((E lex_symbol) & E SEP) {
        next_symbol();

        if (lex_symbol & SEOF) {
            return;
        }

        check("Ocakavany tag", E READ | E PRINT | E SAVE | E WHILE | E IF | K);
        process((E SEP) | K);
        check("Ocakavany \"|\", \"]\", \"}\" alebo koniec suboru", (E SEP) | K);
    }
     */
}

/* Process -> (Read | Store | Print | While | If) */
void process(KeySet K)
{
    switch (lex_symbol) {
        case READ:
            read((E COMMA) | (E SEP) | K);
            break;
        case PRINT:
            print(K);
            break;
        case SAVE:
            save((E COMMA) | K);
            break;
        case WHILE:
            repeat(K);
            break;
        case IF:
            branch(K);
            break;
        default:
            error("Ocakavany tag", E READ | E PRINT | E SAVE | E WHILE | E IF | K);
    }
}

/* While → "while" Cond "{" Program "}" */
void repeat(KeySet K)
{
    match(WHILE, K);

    short begin = get_address();
    cond((E LCB) | K);

    if ((E lex_symbol) & E LCB) {
        match(LCB, K);
    } else {
        error("Ocakava sa \"{\"", (E LCB) | E READ | E PRINT | E SAVE | E WHILE | E IF | K);
    }

    short addr_begin = write_bze_begin();
    program((E RCB) | K);

    if ((E lex_symbol) & E RCB)
        match(RCB, K);
    else
        error("Ocakava sa \"}\"", (E RCB) | K);

    write_jmp_addr(begin);
    write_finish(addr_begin, get_address());
}

/* If → "if" Cond "{" Program ["@" Program] "}" */
void branch(KeySet K)
{
    match(IF, K);

    cond((E LCB) | K);

    if ((E lex_symbol) & E LCB) {
        match(LCB, K);
    } else {
        error("Ocakava sa \"{\"", (E LCB | E ELSE) | E READ | E PRINT | E SAVE | E WHILE | E IF | K);
    }

    short codelist_1 = write_bze_begin();
    program((E RCB | E ELSE) | K);
    short codelist_2 = write_branch_jmp();

    if ((E lex_symbol) & E ELSE) {
        next_symbol();
        write_finish(codelist_1, get_address());
        program((E RCB) | K);

        if ((E lex_symbol) & E RCB) {
            match(RCB, K);
        } else {
            error("Ocakava sa \"}\"", (E RCB) | K);
        }
        write_finish(codelist_2, get_address());
    } else {
        if ((E lex_symbol) & E RCB) {
            match(RCB, K);
        } else {
            error("Ocakava sa \"}\"", (E RCB) | K);
        }
        write_finish(codelist_2, get_address());
    }
}

/* Cond -> Expr [("<" | ">" | "<=" | ">=" | "==" | "!=") Expr] */
void cond(KeySet K)
{
    Symbol operator;
    expr((E EQ | E NE | E LT | E LE | E GT | E GE) | K);

    if ((E lex_symbol) & (E EQ | E NE | E LT | E LE | E GT | E GE)) {

        operator = lex_symbol;
        next_symbol();

        expr((E EQ | E NE | E LT | E LE | E GT | E GE) | K);

        switch (operator) {
            case EQ:
                write_eq();
                break;
            case NE:
                write_ne();
                break;
            case LT:
                write_lt();
                break;
            case LE:
                write_le();
                break;
            case GE:
                write_ge();
                break;
            case GT:
                write_gt();
                break;
            default:
                error("Ocakava sa symbol porovnavania", (E EQ | E NE | E LT | E LE | E GT | E GE) | K);
        }
    }
}

/* Save -> "save" id "=" Expr {"," id "=" Expr} */
void save(KeySet K)
{
    match(SAVE, K);

    if (!(E lex_symbol & E ID)) {
        error("Ocakavane ID", (E ID) | K);
    }

    if ((E lex_symbol) & E ID) {
        if (flags[lex_attr][0] == 0) {
            error("Nedeklarovana premenna", K);
        } else {
            int attr = match(ID, K);

            if (!(E lex_symbol & E VAR)) {
                error("Ocakavana \"~\"", (E VAR) | K);
            }

            if ((E lex_symbol) & E VAR) {
                match(VAR, K);
                cond(K);
                flags[attr][1] = 1;
                write_save_var((short) attr);
            }
        }
    }

    while ((E lex_symbol) & E COMMA) {
        next_symbol();

        if (!(E lex_symbol & E ID)) {
            error("Ocakavane ID", (E ID) | K);
        }

        if ((E lex_symbol) & E ID) {
            if (flags[lex_attr][0] == 0) {
                error("Nedeklarovana premenna", K);
            } else {
                int attr = match(ID, K);

                if (!(E lex_symbol & E VAR)) {
                    error("Ocakavana \"~\"", (E VAR) | K);
                }

                if ((E lex_symbol) & E VAR) {
                    match(VAR, K);
                    cond(K);
                    flags[attr][1] = 1;
                    write_save_var((short) attr);
                }
            }
        }
    }
}

/* Term -> VALUE | "(" Expr ")" */
void term(KeySet K)
{
    check("Ocakava sa VALUE, ID alebo \"(\"", E VALUE | E ID | E LPAR | K);

    switch (lex_symbol) {
        case VALUE:
            write_number((short) lex_attr);
            match(VALUE, K);
            break;
        case LPAR:
            next_symbol();
            expr((E RPAR) | K);

            if ((E lex_symbol) & E RPAR)
                match(RPAR, K);
            else
                error("Ocakava sa \")\"", (E RPAR) | K);

            break;
        case ID:
            if (flags[lex_attr][0] == 0) {
                error("Nedeklarovana premenna", K);
            } else if (flags[lex_attr][1] == 0) {
                error("Nedefinovana premenna", K);
            } else {
                write_var((short) lex_attr);
                match(ID, K);
            }
            break;
        default:
            error("Ocakava sa VALUE, ID alebo \"(\"", E VALUE | E ID | E LPAR | K);
    }
}

/* Power -> Term {"^"  Power} */
void power(KeySet K)
{

    Symbol operator;
    term((E POWER) | K);

    check("Ocakavana matematicka operacia", (E POWER) | K);

    if ((E lex_symbol) & (E POWER)) {

        operator = lex_symbol;
        match(POWER, K);

        power(K);

        switch (operator) {
            case POWER:
                write_power();
                break;
            default:
                error("Ocakava sa matematicka operacia", (E POWER) | K);
        }
    }
}

/* Mul -> Power  {("*"|"/")  Power } */
void mul_div(KeySet K)
{
    Symbol operator;
    power((E MUL) | (E DIV) | K);

    check("Ocakava sa \"*\" alebo \"/\"", (E MUL) | (E DIV) | K);

    while ((E lex_symbol) & (E DIV | E MUL)) {

        operator = lex_symbol;
        next_symbol();

        power((E MUL) | (E DIV) | K);

        switch (operator) {
            case MUL:
                write_mul();
                break;
            case DIV:
                write_div();
                break;
            default:
                error("Ocakava sa \"*\" alebo \"/\"", (E MUL) | (E DIV) | K);
        }
    }
}

/* Expr -> Mul {("+" | "-") Mul} */
void expr(KeySet K)
{
    Symbol operator;
    mul_div((E PLUS) | (E MINUS) | K);

    check("Ocakava sa \"+\" alebo \"-\"", (E PLUS) | (E MINUS) | K);

    while ((E lex_symbol) & (E PLUS | E MINUS)) {

        operator = lex_symbol;
        next_symbol();

        mul_div((E PLUS) | (E MINUS) | K);

        switch (operator) {
            case PLUS:
                write_add();
                break;
            case MINUS:
                write_sub();
                break;
            default:
                error("Ocakava sa \"+\" alebo \"-\"", (E PLUS) | (E MINUS) | K);
        }
    }
}

// read -> "read" ID {"," ID};
void read(KeySet K)
{
    match(READ, K);

    if (!(E lex_symbol & E ID)) {
        error("Ocakavane ID", (E ID) | K);
    }

    if ((E lex_symbol) & E ID) {
        if (flags[lex_attr][0] == 0) {
            error("Nedeklarovana premenna", K);
        } else {
            write_ask_var((short) lex_attr, lex_ids[lex_attr]);
            write_var((short) lex_attr);
            flags[lex_attr][1] = 1;
            match(ID, K);
        }
    }

    check("Ocakavana \",\" alebo \";\"", (E SEP) | K);
    while ((E lex_symbol) & E COMMA) {
        next_symbol();

        if (!(E lex_symbol & E ID)) {
            error("Ocakavane ID", (E ID) | K);
        }

        if ((E lex_symbol) & E ID) {
            if (flags[lex_attr][0] == 0) {
                error("Nedeklarovana premenna", K);
            } else {
                write_ask_var((short) lex_attr, lex_ids[lex_attr]);
                write_var((short) lex_attr);
                flags[lex_attr][1] = 1;
                match(ID, K);
            }
        }
    }

    check("Ocakava sa \";\"", E SEP | K);
}


/* Print -> "write" ("bool" Cond | Expr ) {"&" ("bool" Cond | Expr )} */
void print(KeySet K)
{
    match(PRINT, K);

    if (!(E lex_symbol & (E BOOL | E VALUE | E ID | E LPAR)))
        error("Ocakavany BOOL alebo vyraz", (E BOOL | E VALUE | E ID | E LPAR) | K);

    if ((E lex_symbol) & E BOOL) {
        next_symbol();
        cond((E AND) | K);
        write_bool();
    } else if ((E lex_symbol) & (E VALUE | E ID | E LPAR)) {
        cond((E AND) | K);
        write_result();
    }

    check("Ocakava sa \"&\" alebo \"|\"", (E AND) | K);
    while (lex_symbol == AND) {
        next_symbol();

        if (!(E lex_symbol & (E BOOL | E VALUE | E ID | E LPAR)))
            error("Ocakavany BOOL alebo vyraz", (E BOOL | E VALUE | E ID | E LPAR) | K);

        if ((E lex_symbol) & E BOOL) {
            next_symbol();
            cond((E AND) | K);
            write_bool();
        } else if ((E lex_symbol) & (E VALUE | E ID | E LPAR)) {
            cond((E AND) | K);
            write_result();
        }

        check("Ocakava sa \"&\" alebo \"|\"", (E AND) | K);
    }

    check("Ocakava sa \";\"", E SEP | K);
}
