#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>

void init_generator(FILE *output);
void generate_output();
short get_address();
void write_begin(short num_vars);
void write_end();
void write_result();
void write_number(short value);
void write_var(short index);
void write_add();
void write_sub();
void write_mul();
void write_div();
void write_ask_var(short index, char *name);
void write_string(const char *str);

#endif /* GENERATOR_H */
