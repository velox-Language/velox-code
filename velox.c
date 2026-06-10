// src/velox.c - полный интерпретатор Velox
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VARS 1024
#define MAX_LINE 4096
#define MAX_INCLUDE 16

typedef struct {
    char name[64];
    int value;
    int is_ptr;
    void *ptr;
} Var;

Var vars[MAX_VARS];
int var_count = 0;

Var* find_var(const char *name) {
    for (int i = 0; i < var_count; i++)
        if (strcmp(vars[i].name, name) == 0)
            return &vars[i];
    return NULL;
}

Var* new_var(const char *name) {
    Var *v = &vars[var_count++];
    strcpy(v->name, name);
    v->value = 0;
    v->is_ptr = 0;
    v->ptr = NULL;
    return v;
}

void set_var(const char *name, int val) {
    Var *v = find_var(name);
    if (!v) v = new_var(name);
    v->value = val;
}

int get_var(const char *name) {
    Var *v = find_var(name);
    return v ? v->value : 0;
}

// парсинг выражения (простая версия)
int eval_expr(const char *expr) {
    if (isdigit(expr[0]) || expr[0] == '-')
        return atoi(expr);
    
    // переменная
    char var[64];
    int i = 0;
    while (expr[i] && expr[i] != ' ' && expr[i] != '\t' && expr[i] != '+' && expr[i] != '-' && expr[i] != '*' && expr[i] != '/')
        var[i++] = expr[i];
    var[i] = '\0';
    
    int left = get_var(var);
    
    // пропуск пробелов
    while (expr[i] == ' ' || expr[i] == '\t') i++;
    
    if (expr[i] == '+') return left + eval_expr(expr + i + 1);
    if (expr[i] == '-') return left - eval_expr(expr + i + 1);
    if (expr[i] == '*') return left * eval_expr(expr + i + 1);
    if (expr[i] == '/') return left / eval_expr(expr + i + 1);
    
    return left;
}

void exec_line(const char *line) {
    // пропуск пробелов
    while (*line == ' ' || *line == '\t') line++;
    
    // комментарий
    if (*line == '/' && *(line+1) == '/') return;
    if (*line == '\0') return;
    
    // include
    if (strncmp(line, "include", 7) == 0 && (line[7] == ' ' || line[7] == '\t')) {
        const char *start = strchr(line, '"');
        if (start) {
            start++;
            char fname[256];
            int i = 0;
            while (*start && *start != '"') fname[i++] = *start++;
            fname[i] = '\0';
            FILE *f = fopen(fname, "r");
            if (f) {
                char buf[MAX_LINE];
                while (fgets(buf, sizeof(buf), f))
                    exec_line(buf);
                fclose(f);
            }
        }
        return;
    }
    
    // if
    if (strncmp(line, "if", 2) == 0 && (line[2] == ' ' || line[2] == '\t')) {
        const char *p = line + 2;
        while (*p == ' ' || *p == '\t') p++;
        char expr[256];
        int i = 0;
        while (*p && *p != ' ' && *p != '\t') expr[i++] = *p++;
        expr[i] = '\0';
        
        int cond = eval_expr(expr);
        
        // найти then
        while (*p == ' ' || *p == '\t') p++;
        if (strncmp(p, "then", 4) == 0) p += 4;
        while (*p == ' ' || *p == '\t') p++;
        
        if (cond) {
            exec_line(p);
        } else {
            // пропустить до else или end
            // упрощённо: просто игнорируем
        }
        return;
    }
    
    // while
    if (strncmp(line, "while", 5) == 0 && (line[5] == ' ' || line[5] == '\t')) {
        // упрощённая реализация
        return;
    }
    
    // jump
    if (strncmp(line, "jump", 4) == 0 && (line[4] == ' ' || line[4] == '\t')) {
        int addr = strtol(line + 5, NULL, 0);
        printf("; jump to 0x%x (simulated)\n", addr);
        return;
    }
    
    // asm { ... }
    if (strncmp(line, "asm", 3) == 0 && (line[3] == ' ' || line[3] == '\t')) {
        printf("; asm block (ignored in interpreter)\n");
        return;
    }
    
    // print
    if (strncmp(line, "print", 5) == 0 && (line[5] == ' ' || line[5] == '\t')) {
        const char *p = line + 5;
        while (*p == ' ' || *p == '\t') p++;
        
        if (*p == '"') {
            p++;
            while (*p && *p != '"') putchar(*p++);
        } else {
            printf("%d", eval_expr(p));
        }
        putchar('\n');
        return;
    }
    
    // присваивание
    if (strchr(line, '=')) {
        char var_name[64];
        int i = 0;
        while (line[i] && line[i] != ' ' && line[i] != '\t' && line[i] != '=')
            var_name[i++] = line[i];
        var_name[i] = '\0';
        
        const char *p = line + i;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '=') p++;
        while (*p == ' ' || *p == '\t') p++;
        
        // ссылка &var
        if (*p == '&') {
            p++;
            char ref[64];
            i = 0;
            while (*p && *p != ' ' && *p != '\t') ref[i++] = *p++;
            ref[i] = '\0';
            Var *v = find_var(ref);
            if (v) {
                Var *newv = find_var(var_name);
                if (!newv) newv = new_var(var_name);
                newv->is_ptr = 1;
                newv->ptr = &v->value;
            }
            return;
        }
        
        // разыменование *ptr
        if (*p == '*') {
            p++;
            char ptr_name[64];
            i = 0;
            while (*p && *p != ' ' && *p != '\t') ptr_name[i++] = *p++;
            ptr_name[i] = '\0';
            Var *ptr_var = find_var(ptr_name);
            if (ptr_var && ptr_var->is_ptr) {
                int val = eval_expr(p);
                *(int*)ptr_var->ptr = val;
            }
            return;
        }
        
        int val = eval_expr(p);
        set_var(var_name, val);
        return;
    }
    
    // неизвестная команда
    printf("; unknown: %s\n", line);
}

void run_file(const char *fname) {
    FILE *f = fopen(fname, "r");
    if (!f) {
        perror("fopen");
        return;
    }
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f))
        exec_line(line);
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Velox v1.0\nUsage: velox <file.vx>\n");
        return 1;
    }
    run_file(argv[1]);
    return 0;
}