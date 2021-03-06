#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "interpreter.h"
#include "lexer.h"
#include "generator.h"

/*** Syntakticky analyzator a interpretator ***/

short jsr[LEX_POINTERS_MAX];

void error(const char *msg, KeySet K)
{
    fprintf(stderr, "CHYBA na pozicii %d: %s\n", position, msg);

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

/* Gramatika:
*
* Declare → [declare id{"," id};] Program
* Program → Command {"}" Command}
* Command → (Read | Print | Set | While | For | If)
* Read → "read" id{"," id};
* Set → "set" id "=" Expr {"," id Expr};
* Incr → "incr" id;
* Decr → "decr" id;
* Print → "print" ("bool" Condition | [char] Expr ) {"&" ("bool" Condition | [char] Expr )};
* While → "while" (Condition) "{" Program "}"
* For → "for" (set "=" Expr {"," id Expr}; Condition; (Set | Incr | Decr); "{" Program "}"
* If → "if" (Condition) "{" Program [Program] "}" ["else" "{" Program [Program] "}"]
* Void → void POINTER () "{" Program [Program] "}"
* Exec → "exec"(POINTER);
* Condition → Expr [("<" | ">" | "<=" | ">=" | "==" | "!=") Expr]
* Expr → Mul {("+" | "-") Mul}
* Mul → Power {("*" | "/") Power}
* Power → Term ["^" Power]
* Term → VALUE | "(" Expr ")" | ID
* Exit → "exit"
*/

/* Declare → [id{"," id}] Program */
void declare(KeySet K)
{
    if (lex_symbol == DECLARE) {
        match(DECLARE, K);

        check("Ocakavane ID", (E ID) | K);
    }

    if ((E lex_symbol) & E ID) {

        flags[match(ID, K)][0] = 1;

        check("Ocakavana \",\"", E COMMA | E EOC | K);

        while ((E lex_symbol) & (E COMMA | E ID)) {

            if ((E lex_symbol) & E COMMA) {
                next_symbol();
            }

            if ((E lex_symbol) & E ID) {
                if (flags[lex_attr][0] == 0) {
                    flags[lex_attr][0] = 1;
                    next_symbol();
                } else {
                    error("Premenna bola deklarovana", E COMMA | E EOC | K);
                }
            } else {
                error("Ocakavane ID", E ID | K);
            }

            check("Ocakavana \",\"", E COMMA | E EOC | E ID | K);
        }

        match(EOC, K);
    }

    write_begin((short) lex_ids_size);
    write_string("\n");
    program(K);
}

/* Program → Command {"}" Command} */
void program(KeySet K)
{
    do {
        check("Ocakava sa prikaz",
              E VOID | E EXEC | E EXIT | E EOC | E READ | E PRINT | E SET | E INCR | E DECR | E WHILE | E FOR | E IF |
              K);

        if (lex_symbol == EOC) {
            next_symbol();
        }

        if (lex_symbol == SEOF) {
            return;
        }

        command(E EOC | K);

    } while (lex_symbol != SEOF && lex_symbol != RCB);
}

/* Command → (Read | Print | Set | While | For | If) */
void command(KeySet K)
{
    switch (lex_symbol) {
        case READ:
            read(E COMMA | K);
            break;
        case PRINT:
            print(E COMMA | K);
            break;
        case SET:
            set(E COMMA | K);
            break;
        case INCR:
            _incr(K);
            break;
        case DECR:
            _decr(K);
            break;
        case WHILE:
            _while(K);
            break;
        case FOR:
            _for(K);
            break;
        case IF:
            _if(K);
            break;
        case VOID:
            _void(K);
            break;
        case EXEC:
            _exec(K);
            break;
        case EXIT:
            match(EXIT, K);
            write_exit();
            break;
        default:
            error("Ocakavany prikaz", E EXIT | E READ | E PRINT | E SET | E INCR | E DECR | E WHILE | E FOR | E IF | K);
            break;
    }
}

/* Incr → "incr" id; */
void _incr(KeySet K)
{
    match(INCR, K);

    write_incr((short) match(ID, K));

    match(EOC, K);
}

/* Incr → "decr" id; */
void _decr(KeySet K)
{
    match(DECR, K);

    write_decr((short) match(ID, K));

    match(EOC, K);
}

/* While → "while" (Condition) "{" Program "}" */
void _while(KeySet K)
{
    match(WHILE, K);

    match(LPAR, K);
    short _condition = get_address();
    condition(E RPAR | K);
    short _condition_end = write_bze_begin();

    match(RPAR, K);
    match(LCB, K);

    program((E RCB) | K);

    match(RCB, K);

    write_jmp_addr(_condition);
    write_flag(_condition_end, get_address());
}

/* For → "for" (set "=" Expr {"," id Expr}; Condition; (Set | Incr | Decr); "{" Program "}" */
void _for(KeySet K)
{
    match(FOR, K);

    match(LPAR, K);

    check("Ocakava sa set", E SET | K);
    set(K);

    short _condition = get_address();
    condition(E EOC | K);
    short _condition_end = write_bze_begin();
    match(EOC, K);
    short _program = write_branch_jmp();

    check("Ocakava sa set, incr alebo decr", E SET | E INCR | E DECR | K);

    short _flag = write_branch_jmp();

    if (E lex_symbol & E SET) {
        set(K);
    } else if (E lex_symbol & E INCR) {
        _incr(K);
    } else if (E lex_symbol & E DECR) {
        _decr(K);
    }

    write_jmp_addr(_condition);

    match(RPAR, K);
    match(LCB, K);

    write_flag(_program, get_address());
    program((E RCB) | K);

    match(RCB, K);

    write_jmp_addr(_flag);
    write_flag(_condition_end, get_address());
}

/* If → "if" (Condition) "{" Program [Program] "}" ["else" "{" Program [Program] "}"] */
void _if(KeySet K)
{
    match(IF, K);

    match(LPAR, K);
    condition(E RPAR | K);
    match(RPAR, K);

    match(LCB, K);

    short branch_1 = write_bze_begin();

    program(E RCB | K);

    short branch_2 = write_branch_jmp();

    write_flag(branch_1, get_address());

    match(RCB, K);

    if ((E lex_symbol) & E ELSE) {

        match(ELSE, K);
        match(LCB, K);

        program((E RCB) | K);

        match(RCB, K);
    }

    write_flag(branch_2, get_address() + 1);
}

/* Void → void POINTER () "{" Program [Program] "}"   */
void _void(KeySet K)
{
    match(VOID, K);

    check("Ocakava sa pointer", E POINTER | K);

    short _pointer = (short) lex_attr;

    jsr[_pointer] = write_branch_jmp();

//    printf("New pointer addr [%d] = {%d}\n", lex_attr, jsr[_pointer]);

    match(POINTER, K);

    match(LPAR, K);
    match(RPAR, K);

    match(LCB, K);


    program(E RCB | K);
    write_rts();

    write_flag(jsr[_pointer], get_address());

    match(RCB, K);
}

/* Exec → "exec"(POINTER); */
void _exec(KeySet K)
{
    match(EXEC, K);
    match(LPAR, K);

    check("Ocakava sa pointer", E POINTER | K);

    write_jsr_addr(jsr[lex_attr] + 1);

    match(POINTER, K);
    match(RPAR, K);
    match(EOC, K);
}

/* Read → "read" ID {"," ID}; */
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

    check("Ocakavana \",\" alebo \";\"", (E EOC) | K);
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

    match(EOC, K);
}


/* Print → "print" ("bool" Condition | [char] Expr ) {"&" ("bool" Condition | [char] Expr )}; */
void print(KeySet K)
{
    match(PRINT, K);

    if (!(E lex_symbol & (E CHAR | E BOOL | E VALUE | E ID | E LPAR))) {
        error("Ocakavany bool, char alebo vyraz", (E CHAR | E BOOL | E VALUE | E ID | E LPAR) | K);
    }

    if ((E lex_symbol) & E BOOL) {
        next_symbol();
        condition((E AND) | K);
        write_bool();
    } else if ((E lex_symbol) & E CHAR) {
        next_symbol();
        condition((E AND) | K);
        write_result_char();
    } else if ((E lex_symbol) & (E VALUE | E ID | E LPAR)) {
        condition((E AND) | K);
        write_result();
    }

    check("Ocakava sa \"&\"", (E AND) | K);
    while (lex_symbol == AND) {
        next_symbol();

        if (!(E lex_symbol & (E BOOL | E CHAR | E VALUE | E ID | E LPAR))) {
            error("Ocakavany bool, char alebo vyraz", (E BOOL | E VALUE | E ID | E LPAR) | K);
        }

        if ((E lex_symbol) & E BOOL) {
            next_symbol();
            condition((E AND) | K);
            write_bool();
        } else if ((E lex_symbol) & E CHAR) {
            next_symbol();
            condition((E AND) | K);
            write_result_char();
        } else if ((E lex_symbol) & (E VALUE | E ID | E LPAR)) {
            condition((E AND) | K);
            write_result();
        }

        check("Ocakava sa \"&\"", (E AND) | K);
    }

    match(EOC, K);
}

/* Condition → Expr [("<" | ">" | "<=" | ">=" | "==" | "!=") Expr] */
void condition(KeySet K)
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

/* Set → "set" id "=" Expr {"," id "=" Expr} */
void set(KeySet K)
{
    match(SET, K);

    if (!(E lex_symbol & E ID)) {
        error("Ocakavane ID", (E ID) | K);
    }

    if ((E lex_symbol) & E ID) {
        if (flags[lex_attr][0] == 0) {
            error("Nedeklarovana premenna", K);
        } else {
            int attr = match(ID, K);

            if (!(E lex_symbol & E ASSIGN)) {
                error("Ocakava sa priradenie \"=\"", (E ASSIGN) | K);
            }

            if ((E lex_symbol) & E ASSIGN) {
                match(ASSIGN, K);
                condition(K);
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

                if (!(E lex_symbol & E ASSIGN)) {
                    error("Ocakavana \"=\"", (E ASSIGN) | K);
                }

                if ((E lex_symbol) & E ASSIGN) {
                    match(ASSIGN, K);
                    condition(K);
                    flags[attr][1] = 1;
                    write_save_var((short) attr);
                }
            }
        }
    }

    match(EOC, K);
}

/* Term → VALUE | "(" Expr ")" */
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

            if ((E lex_symbol) & E RPAR) {
                match(RPAR, K);
            } else {
                error("Ocakava sa \")\"", (E RPAR) | K);
            }
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

/* Power → Term {"^"  Power} */
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

/* Mul → Power  {("*"|"/")  Power } */
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

/* Expr → Mul {("+" | "-") Mul} */
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
