#ifndef LIFE_H
#define LIFE_H

#include <stdio.h>

typedef struct life_t {
    char **grid;
    char **_ngrid;
    size_t gen_num;
    size_t grid_w;
    size_t grid_h;
    size_t grid_a;
} life_t;

void life_create(life_t *, size_t, size_t);
void life_destroy(life_t *);
void life_next_gen(life_t *);
void life_next_gen_omp(life_t *);

char *life_get(life_t *, size_t, size_t);

#endif
