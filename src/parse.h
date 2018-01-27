#ifndef _PARSE_H_
#define _PARSE_H_


int parse_item(char **fdata, char **key, int *key_len, char **val, int *val_len);
int parse_intmap(char **fdata, int *data, int w, int h, int base);


#endif