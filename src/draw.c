#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL.h>
#include <time.h>

#include "life.h"

static inline void set_pixel(uint8_t *data, int x, int y, int w, int val) {
    uint8_t *color = data + y * w * 4 + x * 4;
    color[3] = val;
    color[2] = val;
    color[1] = val;
    color[0] = 255;
}

void main_loop(life_t *life, size_t gutter_radius) {
    size_t w = life->grid_w - gutter_radius * 2;
    size_t h = life->grid_h - gutter_radius * 2;

    int scale;
    if (w < h)
        scale = 900 / h;
    else
        scale = 900 / w;

    if (scale == 0)
        scale = 1;

    int window_width = scale * w;
    int window_height = scale * h;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Texture *texture;

    window = SDL_CreateWindow(
        "Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window_width,
        window_height,
        0
    );

    if (window == NULL) {
        fprintf(stderr, "SDL window creation failed: %s\n", SDL_GetError());
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        fprintf(stderr, "SDL renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    if (texture == NULL) {
        fprintf(stderr, "SDL texture creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        return;
    }

    uint8_t *data = malloc(window_height * window_width * 4);

    if (data == NULL) {
        fprintf(stderr, "malloc failed\n");
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        return;
    }

    Uint32 target_ms = 1000 / 60;

    for (int running = 1, speed = 1; running;) {
        Uint32 t1 = SDL_GetTicks();

        SDL_Event event;

        if (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN: {
                SDL_KeyboardEvent *key_event = (SDL_KeyboardEvent *) &event;
                switch (key_event->keysym.sym) {
                case SDLK_UP:
                    speed++;
                    printf("%d\n", speed);
                    break;
                case SDLK_DOWN:
                    if (speed > 1)
                        speed--;
                    printf("%d\n", speed);
                    break;
                }
                break; }
            }
        }
        if (!running)
            break;

        {
            for (int i = 0; i < speed; i++)
                life_next_gen_omp(life);

            /* SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); */
            /* SDL_RenderClear(renderer); */

            memset(data, 255, window_height * window_width * 4);
            for (size_t y = 0; y < h; y++) {
                for (size_t x = 0; x < w; x++) {
                    int isset = life->grid[y + gutter_radius][x + gutter_radius] != 0;
                    for (size_t fy = y * scale; fy < (y + 1) * scale; fy++) {
                        /* if (isset) */
                        /*     memset(data + fy * window_width * 4 + x * scale * 4, 0, scale * 4); */
                        for (size_t fx = x * scale; fx < (x + 1) * scale; fx++) {
                            if (isset) {
                                set_pixel(data, fx, fy, window_width, 0);
                            } else if (scale > 1 && (fx % scale == 0 || fy % scale == 0)) {
                                set_pixel(data, fx, fy, window_width, 0);
                            }
                        }
                    }
                }
            }
            /* SDL_SetRenderTarget(renderer, NULL); */
            SDL_UpdateTexture(texture, NULL, (void *) data, 4 * window_width);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        Uint32 t2 = SDL_GetTicks();
        Uint32 elapsed = t2 - t1;

        if (elapsed > target_ms)
            continue;

        SDL_Delay(target_ms - elapsed);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(data);
    SDL_Quit();
}
