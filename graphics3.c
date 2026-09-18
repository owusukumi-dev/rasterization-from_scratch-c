#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    int x, y;
} Point2D;

typedef struct {
    int v_start;
    int v_end;
} Edge;


void draw_pixel(uint32_t *buffer, int x, int y, uint32_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    buffer[(y * SCREEN_WIDTH) + x] = color;
}

void clear_screen(uint32_t *buffer, uint32_t color) {
    int total_pixels = SCREEN_WIDTH * SCREEN_HEIGHT;
    for (int i = 0; i < total_pixels; i++) {
        buffer[i] = color;
    }
}

void draw_line(uint32_t *buffer, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw_pixel(buffer, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}


Vec3 rotate_y(Vec3 p, float angle) {
    Vec3 out;
    out.x = p.x * cosf(angle) - p.z * sinf(angle);
    out.y = p.y;
    out.z = p.x * sinf(angle) + p.z * cosf(angle);
    return out;
}

Vec3 rotate_x(Vec3 p, float angle) {
    Vec3 out;
    out.x = p.x;
    out.y = p.y * cosf(angle) - p.z * sinf(angle);
    out.z = p.y * sinf(angle) + p.z * cosf(angle);
    return out;
}

Vec3 rotate_z(Vec3 p, float angle) {
    Vec3 out;
    out.x = p.x * cosf(angle) - p.y * sinf(angle);
    out.y = p.x * sinf(angle) + p.y * cosf(angle);
    out.z = p.z;
    return out;
}

Point2D project_perspective(Vec3 p, float fov_factor) {
    Point2D out;
    
    if (p.z <= 0.1f) p.z = 0.1f;

    float projected_x = (p.x * fov_factor) / p.z;
    float projected_y = (p.y * fov_factor) / p.z;

    out.x = (int)(projected_x + (SCREEN_WIDTH / 2));
    out.y = (int)(projected_y + (SCREEN_HEIGHT / 2));

    return out;
}


int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window *window = SDL_CreateWindow(
        "Low-Level Graphics Engine - Module 4: 3D Rotating Cube",
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

    Vec3 cube_vertices[8] = {
        { -1.0f, -1.0f, -1.0f }, 
        {  1.0f, -1.0f, -1.0f }, 
        { -1.0f,  1.0f, -1.0f }, 
        {  1.0f,  1.0f, -1.0f }, 
        { -1.0f, -1.0f,  1.0f }, 
        {  1.0f, -1.0f,  1.0f }, 
        { -1.0f,  1.0f,  1.0f }, 
        {  1.0f,  1.0f,  1.0f }  
    };

    Edge cube_edges[12] = {
        {0, 1}, {1, 3}, {3, 2}, {2, 0},
        {4, 5}, {5, 7}, {7, 6}, {6, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    int running = 1;
    SDL_Event event;

    float angle_x = 0.0f;
    float angle_y = 0.0f;
    float angle_z = 0.0f;

    float fov_factor = 400.0f; 
    float camera_distance = 3.5f; 

    uint32_t last_time = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        uint32_t current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        angle_x += 0.8f * dt;
        angle_y += 1.2f * dt;
        angle_z += 0.4f * dt;
         
        clear_screen(framebuffer, 0x111122FF);

        Point2D projected_points[8];

        for (int i = 0; i < 8; i++) {
            Vec3 v = cube_vertices[i];

            v = rotate_x(v, angle_x);
            v = rotate_y(v, angle_y);
            v = rotate_z(v, angle_z);

            v.z += camera_distance;

            projected_points[i] = project_perspective(v, fov_factor);
        }

        for (int i = 0; i < 12; i++) {
            Point2D p0 = projected_points[cube_edges[i].v_start];
            Point2D p1 = projected_points[cube_edges[i].v_end];

            draw_line(framebuffer, p0.x, p0.y, p1.x, p1.y, 0x00FFCCFF); 
        }

        for (int i = 0; i < 8; i++) {
            int px = projected_points[i].x;
            int py = projected_points[i].y;

            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    draw_pixel(framebuffer, px + dx, py + dy, 0xFFFFFFFF);
                }
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