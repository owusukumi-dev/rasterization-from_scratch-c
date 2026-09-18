#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define NUM_TRIANGLES 2

typedef struct {
    float x, y;          
    float vx, vy;        
    uint32_t color;      
    int inverted;        
    float radius;       
} TriangleEntity;


void draw_pixel_alpha(uint32_t *buffer, int x, int y, uint32_t src_color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;

    uint32_t src_a = src_color & 0xFF;
    if (src_a == 0) return;

    int index = (y * SCREEN_WIDTH) + x;
    if (src_a == 255) {
        buffer[index] = src_color;
        return;
    }

    uint32_t dst_color = buffer[index];

    uint32_t src_r = (src_color >> 24) & 0xFF;
    uint32_t src_g = (src_color >> 16) & 0xFF;
    uint32_t src_b = (src_color >> 8)  & 0xFF;

    uint32_t dst_r = (dst_color >> 24) & 0xFF;
    uint32_t dst_g = (dst_color >> 16) & 0xFF;
    uint32_t dst_b = (dst_color >> 8)  & 0xFF;

    uint32_t inv_a = 255 - src_a;
    uint32_t out_r = (src_r * src_a + dst_r * inv_a) / 255;
    uint32_t out_g = (src_g * src_a + dst_g * inv_a) / 255;
    uint32_t out_b = (src_b * src_a + dst_b * inv_a) / 255;

    buffer[index] = (out_r << 24) | (out_g << 16) | (out_b << 8) | 0xFF;
}

void clear_screen(uint32_t *buffer, uint32_t color) {
    int total_pixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    for (int i = 0; i < total_pixels; i++) {
        buffer[i] = color;
    }
}

void draw_horizontal_line(uint32_t *buffer, int x_start, int x_end, int y, uint32_t color) {
    if (x_start > x_end) {
        int temp = x_start;
        x_start = x_end;
        x_end = temp;
    }
    for (int x = x_start; x <= x_end; x++) {
        draw_pixel_alpha(buffer, x, y, color);
    }
}

void swap_ints(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void fill_flat_bottom_triangle(uint32_t *buffer, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    float inv_slope_left = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_right = (float)(x2 - x0) / (y2 - y0);

    float current_x_left = x0;
    float current_x_right = x0;

    for (int scanline_y = y0; scanline_y <= y1; scanline_y++) {
        draw_horizontal_line(buffer, (int)current_x_left, (int)current_x_right, scanline_y, color);
        current_x_left += inv_slope_left;
        current_x_right += inv_slope_right;
    }
}

void fill_flat_top_triangle(uint32_t *buffer, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    float inv_slope_left = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_right = (float)(x2 - x1) / (y2 - y1);

    float current_x_left = x0;
    float current_x_right = x1;

    for (int scanline_y = y0; scanline_y <= y2; scanline_y++) {
        draw_horizontal_line(buffer, (int)current_x_left, (int)current_x_right, scanline_y, color);
        current_x_left += inv_slope_left;
        current_x_right += inv_slope_right;
    }
}

void draw_triangle(uint32_t *buffer, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    if (y0 > y1) { swap_ints(&x0, &x1); swap_ints(&y0, &y1); }
    if (y1 > y2) { swap_ints(&x1, &x2); swap_ints(&y1, &y2); }
    if (y0 > y1) { swap_ints(&x0, &x1); swap_ints(&y0, &y1); }

    if (y1 == y2) {
        fill_flat_bottom_triangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        fill_flat_top_triangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } else {
        int x_mid = x0 + ((float)(y1 - y0) / (float)(y2 - y0)) * (x2 - x0);
        int y_mid = y1;

        fill_flat_bottom_triangle(buffer, x0, y0, x1, y1, x_mid, y_mid, color);
        fill_flat_top_triangle(buffer, x1, y1, x_mid, y_mid, x2, y2, color);
    }
}



int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window *window = SDL_CreateWindow(
        "Low-Level Graphics Engine - Entity Collisions",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Texture *screen_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    uint32_t *framebuffer = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    if (!framebuffer) return 1;

    TriangleEntity triangles[NUM_TRIANGLES] = {
        { .x = 150.0f, .y = 200.0f, .vx = 220.0f,  .vy = 160.0f,  .color = 0x00FFCC80, .inverted = 0, .radius = 55.0f },
        { .x = 650.0f, .y = 250.0f, .vx = -200.0f, .vy = -180.0f, .color = 0xFF4400AA, .inverted = 1, .radius = 55.0f }
    };

    int running = 1;
    SDL_Event event;
    uint32_t last_time = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        uint32_t current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        // 1. Move & Wall Bounce
        for (int i = 0; i < NUM_TRIANGLES; i++) {
            TriangleEntity *t = &triangles[i]; 
            t->x += t->vx * dt;
            t->y += t->vy * dt;

            if (t->x < 60.0f) {
                t->x = 60.0f;
                t->vx = -t->vx;
            } else if (t->x > (float)(SCREEN_WIDTH - 60)) {
                t->x = (float)(SCREEN_WIDTH - 60);
                t->vx = -t->vx;
            }

            if (t->y < 50.0f) {
                t->y = 50.0f;
                t->vy = -t->vy;
            } else if (t->y > (float)(SCREEN_HEIGHT - 100)) {
                t->y = (float)(SCREEN_HEIGHT - 100);
                t->vy = -t->vy;
            }
        }

        TriangleEntity *t1 = &triangles[0];
        TriangleEntity *t2 = &triangles[1];

        float c1_y = t1->inverted ? (t1->y + 35.0f) : (t1->y + 65.0f);
        float c2_y = t2->inverted ? (t2->y + 35.0f) : (t2->y + 65.0f);

        float dx = t2->x - t1->x;
        float dy = c2_y - c1_y;
        float dist_sq = (dx * dx) + (dy * dy);
        float min_dist = t1->radius + t2->radius;

        if (dist_sq < (min_dist * min_dist) && dist_sq > 0.0001f) {
            float dist = sqrtf(dist_sq);
            
            float nx = dx / dist;
            float ny = dy / dist;

            float overlap = 0.5f * (min_dist - dist);
            t1->x -= nx * overlap;
            c1_y  -= ny * overlap;
            t2->x += nx * overlap;
            c2_y  += ny * overlap;

            t1->y = t1->inverted ? (c1_y - 35.0f) : (c1_y - 65.0f);
            t2->y = t2->inverted ? (c2_y - 35.0f) : (c2_y - 65.0f);

            float kx = t1->vx - t2->vx;
            float ky = t1->vy - t2->vy;
            float impulse = 2.0f * (kx * nx + ky * ny) / 2.0f;

            t1->vx -= impulse * nx;
            t1->vy -= impulse * ny;
            t2->vx += impulse * nx;
            t2->vy += impulse * ny;
        }

        clear_screen(framebuffer, 0x111122FF);

        for (int x = 0; x < SCREEN_WIDTH; x += 80) {
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                draw_pixel_alpha(framebuffer, x, y, 0x222233FF);
            }
        }

        for (int i = 0; i < NUM_TRIANGLES; i++) {
            TriangleEntity *t = &triangles[i];
            if (!t->inverted) {
                draw_triangle(
                    framebuffer,
                    (int)t->x,      (int)t->y,
                    (int)t->x - 60, (int)t->y + 100,
                    (int)t->x + 60, (int)t->y + 100,
                    t->color
                );
            } else {
                draw_triangle(
                    framebuffer,
                    (int)t->x,      (int)t->y + 100,
                    (int)t->x - 60, (int)t->y,
                    (int)t->x + 60, (int)t->y,
                    t->color
                );
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