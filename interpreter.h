#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"

int match(Symbol expected);

void mul_div();
void expr();

void term();

void print();
void read();

void program();

void crash(char * expected);

#endif //INTERPRETER_H
