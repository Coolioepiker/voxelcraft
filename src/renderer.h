#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>
#include "chunk.h"
#include "player.h"

typedef struct {
    unsigned int shader_program;
    int width;
    int height;
    int u_projection;
    int u_view;
    int u_model;
    bool show_debug;
} Renderer;

Renderer* renderer_create(int width, int height);
void renderer_destroy(Renderer* renderer);
void renderer_begin(Renderer* renderer, Player* player);
void renderer_render_chunk(Renderer* renderer, Chunk* chunk);
void renderer_end(Renderer* renderer);
void renderer_build_chunk_mesh(Renderer* renderer, Chunk* chunk);
void renderer_destroy_chunk_mesh(Chunk* chunk);
void renderer_draw_crosshair(Renderer* renderer);
void renderer_draw_debug_info(Renderer* renderer, Player* player, 
                              int chunk_count, int fps);
void renderer_resize(Renderer* renderer, int width, int height);

#endif