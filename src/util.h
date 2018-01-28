#ifndef _UTIL_H_
#define _UTIL_H_

int INT_MIN(int a, int b);
int INT_MAX(int a, int b);

int INT_POW(int base, int exp);
int INT_QUO(int a, int b);
int INT_REM(int a, int b);

#define LOG() printf("%s: %i: ", __func__, __LINE__)
#define ERR_INFO() fprintf(stderr, "%s: %i: ", __func__, __LINE__)

#define RET_IF_NZ(x) {int e=(x); if(e){ERR_INFO(); fprintf(stderr, "RET_IF_NZ caught %i\n", e); return e;}}

#define RET_NULL_IF_NZ(x) {int e=(x); if(e){ERR_INFO(); fprintf(stderr, "RET_NULL_IF_NZ caught %i\n", e); return NULL;}}

#define RET_IF_SDL_ERR(x) {int e=(x); if(e < 0){ERR_INFO(); fprintf(stderr, "SDL error: %s\n", SDL_GetError()); return 2;}}

#define REPR_FIELD(obj, field, fmt, depth) print_tabs(depth); printf(#field "=" fmt "\n", obj->field);
#define REPR_FIELD_EXT(obj, field, field_ext, fmt, depth) print_tabs(depth); printf(#field "=" fmt "\n", obj->field_ext);
#define REPR_FIELD_MULTI(field, depth) print_tabs(depth); printf(#field "=\n");

size_t strnlen(const char *s, size_t maxlen);
char *strndup(const char *s1, size_t len);
char *load_file(const char *filename);
void print_tabs(int depth);
int parse_item(char **fdata, char **key, int *key_len, char **val, int *val_len);
int parse_intmap(char **fdata, int *data, int w, int h, int base);

#endif