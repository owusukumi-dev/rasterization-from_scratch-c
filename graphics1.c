#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Low-Level Graphics Engine - Module 1",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    SDL_Texture *screen_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );


    uint32_t *framebuffer = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    if (!framebuffer) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = 0x000000FF;
    }

    int center_x = SCREEN_WIDTH / 2;   
    int center_y = SCREEN_HEIGHT / 2;  
    uint32_t red_color = 0xFF0000FF;  

    for (int x = center_x - 20; x <= center_x + 20; x++) {
        int index = (center_y * SCREEN_WIDTH) + x;
        framebuffer[index] = red_color;
    }


    for (int y = center_y - 20; y <= center_y + 20; y++) {
        int index = (y * SCREEN_WIDTH) + center_x;
        framebuffer[index] = red_color;
    }


    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }


        SDL_UpdateTexture(screen_texture, NULL, framebuffer, SCREEN_WIDTH * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    
    free(framebuffer);
    SDL_DestroyTexture(screen_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}