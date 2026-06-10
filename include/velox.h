#ifndef VELOX_H
#define VELOX_H

#include <stdio.h>

#define MAX_VARS 256

typedef struct {
    int var_count;
    int running;
} VM;

void vm_init(VM *vm);
void run_file(VM *vm, const char *fname);
void exec_line(VM *vm, const char *line);
void compile_file(const char *fname);
void run_binary(const char *fname);

#endif
