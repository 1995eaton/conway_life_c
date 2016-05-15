#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "./life.h"

void print_arr(life_t *life) {
    for (size_t y = 0; y < life->grid_h; y++) {
        for (size_t x = 0; x < life->grid_w; x++)
            printf("%c ", life->grid[y][x] ? '*' : ' ');
        puts("");
    }
}

void read_105(life_t *life, const char *path) {
    FILE *fp = fopen(path, "r");

    size_t grid_w = 0;
    size_t grid_h = 0;
    size_t x = 0, y = 0;
    int is_cmt = 0;
    for (char c; (c = fgetc(fp)) != EOF;) {
        if (x == 0 && c == '#')
            is_cmt = 1;
        if (is_cmt) {
            if (c == '\n')
                is_cmt = 0;
        } else if (c == '\n') {
            grid_h++;
            if (x > grid_w)
                grid_w = x;
            x = 0;
        } else {
            ++x;
        }
    }

    size_t padding = 5;
    grid_w += padding << 1;
    grid_h += padding << 1;

    life_create(life, grid_w, grid_h);
    fseek(fp, 0, SEEK_SET);
    x = 0;
    y = 0;
    is_cmt = 0;
    for (char c; (c = fgetc(fp)) != EOF;) {
        if (x == 0 && c == '#') {
            is_cmt = 1;
        }
        if (is_cmt) {
            if (c == '\n')
                is_cmt = 0;
        } else if (c == '\n') {
            x = 0;
            ++y;
        } else {
            *life_get(life, x + padding, y + padding) = c == '*';
            ++x;
        }
    }
    fclose(fp);
}

char skip_ws(FILE *fp, char c) {
    while (c != EOF && isblank(c)) c = fgetc(fp);
    return c;
}

char skip_until_char(FILE *fp, char c, char u) {
    while (c != EOF && c != u) c = fgetc(fp);
    return c;
}

void read_rle(life_t *life, const char *path) {
    FILE *fp = fopen(path, "r");

    char c;
    size_t grid_w;
    size_t grid_h;
    int has_header = 0;
    size_t x = 0, y = 0;
    size_t pad = 5;

    for (;;) {
        c = fgetc(fp);
        c = skip_ws(fp, c);
        if (c == EOF)
            break;
        if (c == '#') {
            skip_until_char(fp, c, '\n');
            continue;
        }
        if (!has_header) {
            if (c == 'x') {
                fscanf(fp, " = %lu, y = %lu", &grid_w, &grid_h);
                skip_until_char(fp, 0, '\n');
                has_header = 1;
                grid_w += pad << 1;
                grid_h += pad << 1;
                life_create(life, grid_w, grid_h);
                continue;
            }
        } else {
            int rep = 0;
            if (isdigit(c)) {
                while (c != EOF && isdigit(c)) {
                    rep = rep * 10 + c - '0';
                    c = fgetc(fp);
                }
            }
            if (rep == 0)
                rep = 1;
            switch (c) {
            case 'b':
                while (rep--)
                    *life_get(life, x++ + pad, y + pad) = 0;
                break;
            case 'o':
                while (rep--)
                    *life_get(life, x++ + pad, y + pad) = 1;
                break;
            case '$':
                x = 0;
                y += rep;
                break;
            }
        }
    }

    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc < 2)
        return 0;
    life_t life;

    const char *path = argv[1];
    size_t len = strlen(path);

    if (len >= 4 && strcmp(path + (len - 4), ".rle") == 0) {
        read_rle(&life, path);
    } else if (len >= 8 && strcmp(path + (len - 8), "_105.lif") == 0) {
        read_105(&life, path);
    } else {
        fprintf(stderr, "invalid file: %s\n", path);
    }

    for (;;) {
        usleep(50 * 1000);
        printf("\x1b[0;0H\x1b[0J");
        life_next_gen_omp(&life);
        print_arr(&life);
    }
    life_destroy(&life);
}
