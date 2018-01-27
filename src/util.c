
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "util.h"


int INT_MIN(int a, int b){
    return a <= b? a: b;
}

int INT_MAX(int a, int b){
    return a >= b? a: b;
}


int INT_POW(int base, int exp){
    int result = 1;
    while (exp)
    {
        if (exp & 1)
           result *= base;
        exp /= 2;
        base *= base;
    }
    return result;
}

int INT_QUO(int a, int b){
    div_t d = div(a, b);
    if(a<0 && d.rem != 0){
        d.quot += b<0? 1: -1;
    }
    return d.quot;
}

int INT_REM(int a, int b){
    div_t d = div(a, b);
    if(a<0 && d.rem != 0){
        d.rem += b<0? -b: b;
    }
    return d.rem;
}

size_t strnlen(const char *s, size_t maxlen){
    size_t len = 0;
    while(len < maxlen && s[len] != '\0')len++;
    return len;
}

char *strndup(const char *s1, size_t len){
    size_t s_len = strnlen(s1, len);
    char *s2 = malloc(s_len + 1);
    if(s2 == NULL)return NULL;
    strncpy(s2, s1, len);
    s2[s_len] = '\0';
    return s2;
}

char *load_file(const char *filename){
    FILE *f = fopen(filename, "r");
    long f_size;
    char *f_buffer;
    size_t n_read_bytes;
    if(f == NULL){
        ERR_INFO(); fprintf(stderr, "Could not open file: %s\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    f_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    f_buffer = malloc(f_size + 1);
    if(f_buffer == NULL){
        ERR_INFO(); fprintf(stderr, "Could not allocate buffer for file: %s (%li bytes)\n", filename, f_size);
        fclose(f);
        return NULL;
    }
    n_read_bytes = fread(f_buffer, 1, f_size, f);
    fclose(f);

    return f_buffer;
}

void print_tabs(int depth){
    for(int i = 0; i < depth; i++){
        printf("  ");
    }
}

