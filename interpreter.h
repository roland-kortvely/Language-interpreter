#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"

#define KeySet unsigned long long int
#define E 1ULL <<

int errors;
int *error_position;

short flags[LEX_IDS_MAX][2];

int match(Symbol expected, KeySet K);

void error(const char *msg, KeySet K);

void check(const char *msg, KeySet K);

void declare(KeySet K);

void program(KeySet K);

void command(KeySet K);

void mul_div(KeySet K);

void expr(KeySet K);

void term(KeySet K);

void print(KeySet K);

void read(KeySet K);

void _incr(KeySet K);

void _decr(KeySet K);

void _while(KeySet K);

void _for(KeySet K);

void _if(KeySet K);

void condition(KeySet K);

void set(KeySet K);

#endif //INTERPRETER_H
