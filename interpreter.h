#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"

#define KeySet unsigned long int
#define E 1 <<

int errors;
int *error_position;

short flags[LEX_IDS_MAX][2];

void declare(KeySet K);

void program(KeySet K);

void command(KeySet K);

int match(Symbol expected, KeySet K);

void mul_div(KeySet K);

void expr(KeySet K);

void term(KeySet K);

void print(KeySet K);

void read(KeySet K);

void error(const char *msg, KeySet K);

void check(const char *msg, KeySet K);

void repeat(KeySet K);

void branch(KeySet K);

void cond(KeySet K);

void set(KeySet K);

#endif //INTERPRETER_H
