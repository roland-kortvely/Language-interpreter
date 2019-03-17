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

void write_power();

void write_ask_var(short index, char *name);

void write_string(const char *str);

void write_save_var(short index);

short write_bze_begin();

short write_branch_jmp();

void write_jmp_addr(int address);

void write_flag(short list, short address);

void write_bool();

void write_eq();

void write_ne();

void write_lt();

void write_le();

void write_gt();

void write_ge();

void write_incr(short index);

void write_decr(short index);

#endif /* GENERATOR_H */
