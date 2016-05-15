#include <stdlib.h>
#include <string.h>

#include <omp.h>

#include "./life.h"

static inline int is_set(life_t *life, size_t x, size_t y) {
    if (x >= life->grid_w || y >= life->grid_h)
        return 0;
    return life->grid[y][x];
}

static inline unsigned count_neighbors(life_t *life, size_t x, size_t y) {
    register unsigned c = 0;
    c += is_set(life, x - 1, y - 1);
    c += is_set(life, x,     y - 1);
    c += is_set(life, x + 1, y - 1);
    c += is_set(life, x - 1,     y);
    c += is_set(life, x + 1,     y);
    c += is_set(life, x - 1, y + 1);
    c += is_set(life, x,     y + 1);
    c += is_set(life, x + 1, y + 1);
    return c;
}

static void life_copy_to_ngrid(life_t *life) {
    memcpy((char *) (life->_ngrid + life->grid_h),
           (char *) (life->grid   + life->grid_h),
           life->grid_a - life->grid_h * sizeof (char *));
}

static inline void do_next_gen_at(life_t *life, size_t x, size_t y) {
    int count = count_neighbors(life, x, y);
    if (life->grid[y][x]) {
        if (count < 2 || count > 3)
            life->_ngrid[y][x] = 0;
    } else if (count == 3) {
        life->_ngrid[y][x] = 1;
    }
}

void life_create(life_t *life, size_t grid_w, size_t grid_h) {
    life->grid_w = grid_w;
    life->grid_h = grid_h;
    life->grid_a = grid_h * sizeof (char *) +
                   grid_h * grid_w * sizeof (char);
    life->grid = malloc(life->grid_a);
    life->_ngrid = malloc(life->grid_a);
    bzero(life->grid + grid_h, life->grid_a - grid_h * sizeof (char *));
    char *data = (char *) (life->grid + grid_h);
    for (size_t i = 0; i < grid_h; i++, data += grid_w)
        life->grid[i] = data;
    data = (char *) (life->_ngrid + grid_h);
    for (size_t i = 0; i < grid_h; i++, data += grid_w)
        life->_ngrid[i] = data;
    life->gen_num = 0;
}

void life_destroy(life_t *life) {
    if (life->grid   != NULL) free(life->grid);
    if (life->_ngrid != NULL) free(life->_ngrid);
}

void life_next_gen(life_t *life) {
    life_copy_to_ngrid(life);
    if (life->grid_w > life->grid_h)
        for (size_t y = 0; y < life->grid_h; y++)
            for (size_t x = 0; x < life->grid_w; x++)
                do_next_gen_at(life, x, y);
    else
        for (size_t x = 0; x < life->grid_w; x++)
            for (size_t y = 0; y < life->grid_h; y++)
                do_next_gen_at(life, x, y);
    char **temp = life->grid;
    life->grid = life->_ngrid;
    life->_ngrid = temp;
}

void life_next_gen_omp(life_t *life) {
    life_copy_to_ngrid(life);
    if (life->grid_w > life->grid_h)
#pragma omp parallel for
        for (size_t y = 0; y < life->grid_h; y++)
            for (size_t x = 0; x < life->grid_w; x++)
                do_next_gen_at(life, x, y);
    else
#pragma omp parallel for
        for (size_t x = 0; x < life->grid_w; x++)
            for (size_t y = 0; y < life->grid_h; y++)
                do_next_gen_at(life, x, y);
    char **temp = life->grid;
    life->grid = life->_ngrid;
    life->_ngrid = temp;
}

char *life_get(life_t *life, size_t x, size_t y) {
    if (x >= life->grid_w || y >= life->grid_h)
        return NULL;
    return life->grid[y] + x;
}
