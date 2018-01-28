
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "settings.h"
#include "parse.h"


void parse_whitespace(char **fdata, bool eat_comments, bool eat_newlines){
    char c = '\0';
    do{
        /* EAT WHITESPACE */
        while(c = *(*fdata), c != '\0' && (eat_newlines || c != '\n') && isspace(c)){
            (*fdata)++;
        }

        /* EAT COMMENT */
        if(eat_comments && c == '#'){
            while(c = *(*fdata), c != '\n' && c != '\0'){
                (*fdata)++;
            }
        }
    }while(eat_newlines && c == '\n');
}

int parse_string(char **fdata, char **val, int *val_len){
    /*
        The value goes right to the newline: all whitespace and '#'
        comments will be included in the value!
        TODO: add support for trailing whitespace & comments?..
    */

    char c = '\0';

    /* EAT WHITESPACE */
    parse_whitespace(fdata, true, true);

    /* PARSE STRING TO NEWLINE */
    *val = *fdata;
    *val_len = 0;
    while(c = *(*fdata), c != '\0' && c != '\n'){
        (*val_len)++;
        (*fdata)++;
    }
    if(c != '\0'){
        (*fdata)++;
    }

    return 0;
}

int parse_item(char **fdata, char **key, int *key_len, char **val, int *val_len){
    /*
        Parses one "item", i.e. key/value pair.
        The expected format is:
            "key=value\n"
        The key can contain letters, numbers, and underscores.
        No whitespace is allowed between the key and '='.
        The value starts immediately after the '=' and goes right to the
        newline: all whitespace and '#' comments will be included in it!
    */

    char c = '\0';
    *key = NULL;
    *val = NULL;
    *key_len = 0;
    *val_len = 0;

    /* EAT WHITESPACE */
    parse_whitespace(fdata, true, true);

    /* PARSE KEY TO '=' */
    *key = *fdata;
    while(c = *(*fdata), c == '_' || isalpha(c)){
        (*key_len)++;
        (*fdata)++;
    }

    if(*key_len == 0){
        /* No key: so we're at end of file. That's fine: we return success.
        Caller should know that an empty key means end of file. */
        return 0;
    }else if(c != '='){
        LOG(); printf("Parse error: expected '='\n");
        printf("\n----\n%s\n----\n", *fdata);
        return 2;
    }
    (*fdata)++;

    /* PARSE VAL TO NEWLINE */
    *val = *fdata;
    while(c = *(*fdata), c != '\0' && c != '\n'){
        (*val_len)++;
        (*fdata)++;
    }
    if(c != '\0'){
        (*fdata)++;
    }

    if(DEBUG_PARSE >= 1){
        LOG(); printf("Parsed: %.*s=%.*s\n", *key_len, *key, *val_len, *val);
    }

    return 0;
}

int parse_intmap(char **fdata, int *data, int w, int h, int base){
    /*
        Parses a 2d map of non-negative integers, e.g.:
            0 1 2
            . 2 .
            4 5 .
        The special value '.' is treated as -1.
        Digits greater than 9 are represented by uppercase letters.
        The 'base' parameter is the parsed number's base, e.g. 10 for
        decimal.
    */

    char c = '\0';

    /* EAT WHITESPACE */
    parse_whitespace(fdata, true, true);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){

            /* EAT WHITESPACE ON THIS LINE */
            parse_whitespace(fdata, false, false);

            /* PARSE AN INT */
            int n = 0;
            if(*(*fdata) == '.'){
                /* The special character */
                n = -1;
                (*fdata)++;
            }else{
                while(c = *(*fdata), !isspace(c)){
                    int digit = 0;
                    if(c >= '0' && c <= '9'){
                        digit = c - '0';
                    }else if(c >= 'A' && c <= 'Z'){
                        digit = c - 'A' + 10;
                    }else{
                        LOG(); printf("Parse error: expected [.0-9A-Z]\n");
                        return 2;
                    }
                    n *= base;
                    n += digit;
                    (*fdata)++;
                }
            }
            if(DEBUG_PARSE >= 2){
                LOG(); printf("Parsed: %i\n", n);
            }
            *data = n;
            data++;
        }

        /* EAT WHITESPACE */
        parse_whitespace(fdata, true, true);
    }

    return 0;
}


void repr_intmap(int *data, int w, int h, const char *fmt_s, const char *fmt_i, int depth){
    for(int i = 0; i < h; i++){
        print_tabs(depth);
        for(int j = 0; j < w; j++){
            int data_i = data[i * w + j];
            if(data_i == -1)printf(fmt_s, ".");
            else printf(fmt_i, data_i);
            printf(" ");
        }
        printf("\n");
    }
}

