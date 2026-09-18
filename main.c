#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// --- 3D Math Types ---

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    int x, y;
} Point2D;

typedef struct {
    int a, b, c;
    uint32_t color;
} Face;

// --- Basic Vector Math Helpers ---

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_normalize(Vec3 v) {
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 0.0001f) {
        return (Vec3){ v.x / len, v.y / len, v.z / len };
    }
    return v;
}


void draw_pixel(uint32_t *buffer, int x, int y, uint32_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    buffer[(y * SCREEN_WIDTH) + x] = color;
}

void clear_screen(uint32_t *buffer, uint32_t color) {
    int total = SCREEN_WIDTH * SCREEN_HEIGHT;
    for (int i = 0; i < total; i++) buffer[i] = color;
}

void draw_horizontal_line(uint32_t *buffer, int x_start, int x_end, int y, uint32_t color) {
    if (x_start > x_end) {
        int temp = x_start;
        x_start = x_end;
        x_end = temp;
    }
    for (int x = x_start; x <= x_end; x++) {
        draw_pixel(buffer, x, y, color);
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
    float cur_x1 = x0;
    float cur_x2 = x0;

    for (int y = y0; y <= y1; y++) {
        draw_horizontal_line(buffer, (int)cur_x1, (int)cur_x2, y, color);
        cur_x1 += inv_slope_left;
        cur_x2 += inv_slope_right;
    }
}

void fill_flat_top_triangle(uint32_t *buffer, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    float inv_slope_left = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_right = (float)(x2 - x1) / (y2 - y1);
    float cur_x1 = x0;
    float cur_x2 = x1;

    for (int y = y0; y <= y2; y++) {
        draw_horizontal_line(buffer, (int)cur_x1, (int)cur_x2, y, color);
        cur_x1 += inv_slope_left;
        cur_x2 += inv_slope_right;
    }
}

void draw_filled_triangle(uint32_t *buffer, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    if (y0 > y1) { swap_ints(&x0, &x1); swap_ints(&y0, &y1); }
    if (y1 > y2) { swap_ints(&x1, &x2); swap_ints(&y1, &y2); }
    if (y0 > y1) { swap_ints(&x0, &x1); swap_ints(&y0, &y1); }

    if (y1 == y2) {
        fill_flat_bottom_triangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        fill_flat_top_triangle(buffer, x0, y0, x1, y1, x2, y2, color);
    } else {
        int x_mid = x0 + ((float)(y1 - y0) / (float)(y2 - y0)) * (x2 - x0);
        fill_flat_bottom_triangle(buffer, x0, y0, x1, y1, x_mid, y1, color);
        fill_flat_top_triangle(buffer, x1, y1, x_mid, y1, x2, y2, color);
    }
}


Vec3 rotate_x(Vec3 p, float a) {
    return (Vec3){ p.x, p.y * cosf(a) - p.z * sinf(a), p.y * sinf(a) + p.z * cosf(a) };
}

Vec3 rotate_y(Vec3 p, float a) {
    return (Vec3){ p.x * cosf(a) - p.z * sinf(a), p.y, p.x * sinf(a) + p.z * cosf(a) };
}

Vec3 rotate_z(Vec3 p, float a) {
    return (Vec3){ p.x * cosf(a) - p.y * sinf(a), p.x * sinf(a) + p.y * cosf(a), p.z };
}

Point2D project_perspective(Vec3 p, float fov) {
    if (p.z <= 0.1f) p.z = 0.1f;
    return (Point2D){
        (int)((p.x * fov) / p.z + SCREEN_WIDTH / 2),
        (int)((p.y * fov) / p.z + SCREEN_HEIGHT / 2)
    };
}


uint32_t apply_light_intensity(uint32_t color, float intensity) {
    if (intensity < 0.15f) intensity = 0.15f; 
    if (intensity > 1.0f)  intensity = 1.0f;

    uint32_t r = ((color >> 24) & 0xFF) * intensity;
    uint32_t g = ((color >> 16) & 0xFF) * intensity;
    uint32_t b = ((color >> 8)  & 0xFF) * intensity;

    return (r << 24) | (g << 16) | (b << 8) | 0xFF;
}


int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window *window = SDL_CreateWindow(
        "Low-Level Graphics Engine - Module 5: Solid Shaded 3D",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture *screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    uint32_t *framebuffer = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

    Vec3 cube_vertices[8] = {
        {-1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
        {-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}
    };

    Face cube_faces[12] = {
        {0, 1, 2, 0x00FF88FF}, {0, 2, 3, 0x00FF88FF},
        {5, 4, 7, 0x0088FFFF}, {5, 7, 6, 0x0088FFFF},
        {1, 5, 6, 0xFF5500FF}, {1, 6, 2, 0xFF5500FF},
        {4, 0, 3, 0xFFFF00FF}, {4, 3, 7, 0xFFFF00FF},
        {4, 5, 1, 0xFF0055FF}, {4, 1, 0, 0xFF0055FF},
        {3, 2, 6, 0xAA00FFFF}, {3, 6, 7, 0xAA00FFFF}
    };

    Vec3 light_dir = vec3_normalize((Vec3){ 0.0f, 0.0f, -1.0f });

    int running = 1;
    SDL_Event event;
    float angle_x = 0.0f, angle_y = 0.0f;
    float camera_dist = 3.5f;
    float fov_factor = 450.0f;
    uint32_t last_time = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        uint32_t current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        angle_x += 0.7f * dt;
        angle_y += 1.0f * dt;

        clear_screen(framebuffer, 0xFFFFFFFF);

        Vec3 transformed_vertices[8];
        for (int i = 0; i < 8; i++) {
            Vec3 v = cube_vertices[i];
            v = rotate_x(v, angle_x);
            v = rotate_y(v, angle_y);
            v.z += camera_dist; 
            transformed_vertices[i] = v;
        }

        for (int i = 0; i < 12; i++) {
            Face face = cube_faces[i];
            Vec3 a = transformed_vertices[face.a];
            Vec3 b = transformed_vertices[face.b];
            Vec3 c = transformed_vertices[face.c];

            Vec3 ab = vec3_sub(b, a);
            Vec3 ac = vec3_sub(c, a);
            Vec3 normal = vec3_normalize(vec3_cross(ab, ac));

            Vec3 cam_ray = (Vec3){ -a.x, -a.y, -a.z };

            if (vec3_dot(normal, cam_ray) > 0.0f) {

                float light_intensity = -vec3_dot(normal, light_dir);
                uint32_t shaded_color = apply_light_intensity(face.color, light_intensity);

                Point2D p0 = project_perspective(a, fov_factor);
                Point2D p1 = project_perspective(b, fov_factor);
                Point2D p2 = project_perspective(c, fov_factor);

                draw_filled_triangle(framebuffer, p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, shaded_color);
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