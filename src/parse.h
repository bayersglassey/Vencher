#ifndef _PARSE_H_
#define _PARSE_H_


void parse_whitespace(char **fdata, bool eat_comments, bool eat_newlines);
int parse_string(char **fdata, char **val, int *val_len);
int parse_item(char **fdata, char **key, int *key_len, char **val, int *val_len);
int parse_intmap(char **fdata, int *data, int w, int h, int base);
void repr_intmap(int *data, int w, int h, const char *fmt_s, const char *fmt_i, int depth);


#endif