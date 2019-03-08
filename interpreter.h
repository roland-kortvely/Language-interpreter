#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"

int match(Symbol expected);

int expr();

int term();

void print();
void read();

void crash(char * expected);

#endif //INTERPRETER_H
