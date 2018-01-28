
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "settings.h"
#include "parse.h"


int parse_item(char **fdata, char **key, int *key_len, char **val, int *val_len){
    /*
        Parses one "item", i.e. key/value pair.
        The expected data is like this:
            x=1
            name=Johnny Q
    */

    char c = '\0';
    *key = NULL;
    *val = NULL;
    *key_len = 0;
    *val_len = 0;

    /* EAT WHITESPACE */
    while(c = *(*fdata), isspace(c)){
        (*fdata)++;
    }

    /* PARSE KEY */
    *key = *fdata;
    while(c = *(*fdata), c == '_' || isalpha(c)){
        (*key_len)++;
        (*fdata)++;
    }
    if(c != '='){
        LOG(); printf("Parse error: expected '='\n");
        return 2;
    }
    (*fdata)++;

    /* PARSE VAL */
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

    for(int i = 0; i < h; i++){
        /* EAT WHITESPACE, INCLUDING EMPTY LINES */
        while(c = *(*fdata), c != '\0' && isspace(c)){
            (*fdata)++;
        }

        if(c != '#'){
            for(int j = 0; j < w; j++){

                /* EAT WHITESPACE ON THIS LINE */
                while(c = *(*fdata), c != '\n' && c != '\0' && isspace(c)){
                    (*fdata)++;
                }

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

            /* EAT WHITESPACE TO NEWLINE */
            while(c = *(*fdata), c != '\n' && isspace(c)){
                (*fdata)++;
            }
        }

        /* EAT COMMENT TO NEWLINE */
        if(c == '#'){
            while(c = *(*fdata), c != '\n' && c != '\0'){
                (*fdata)++;
            }
        }

        if(c != '\n'){
            LOG(); printf("Parse error: expected newline\n");
            return 2;
        }
        (*fdata)++;
    }

    return 0;
}
