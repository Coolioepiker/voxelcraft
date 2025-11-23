#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

Renderer* renderer_create(int width, int height) {
    Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
    if (!renderer) return NULL;
    
    renderer->width = width;
    renderer->height = height;
    renderer->show_debug = false;
    
    // Load shaders
    renderer->shader_program = shader_load("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (!renderer->shader_program) {
        fprintf(stderr, "Failed to load shaders\n");
        free(renderer);
        return NULL;
    }
    
    // Get uniform locations
    renderer->u_projection = glGetUniformLocation(renderer->shader_program, "projection");
    renderer->u_view = glGetUniformLocation(renderer->shader_program, "view");
    renderer->u_model = glGetUniformLocation(renderer->shader_program, "model");
    
    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    
    printf("Renderer initialized\n");
    
    return renderer;
}

void renderer_destroy(Renderer* renderer) {
    if (renderer) {
        shader_delete(renderer->shader_program);
        free(renderer);
    }
}

void renderer_begin(Renderer* renderer, Player* player) {
    if (!renderer || !player) return;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(renderer->shader_program);
    
    // Setup projection matrix
    float projection[16];
    mat4_perspective(projection, FOV, 
                    (float)renderer->width / (float)renderer->height,
                    NEAR_PLANE, FAR_PLANE);
    shader_set_mat4(renderer->shader_program, "projection", projection);
    
    // Setup view matrix
    float eye[3], center[3], up[3];
    player_get_view_matrix(player, eye, center, up);
    
    float view[16];
    mat4_look_at(view, eye, center, up);
    shader_set_mat4(renderer->shader_program, "view", view);
    
    // Identity model matrix
    float model[16];
    mat4_identity(model);
    shader_set_mat4(renderer->shader_program, "model", model);
}

void renderer_render_chunk(Renderer* renderer, Chunk* chunk) {
    if (!renderer || !chunk) return;
    
    ChunkMesh* mesh = (ChunkMesh*)chunk->mesh;
    if (mesh) {
        mesh_render(mesh);
    }
}

void renderer_end(Renderer* renderer) {
    // Cleanup
    glUseProgram(0);
}

void renderer_build_chunk_mesh(Renderer* renderer, Chunk* chunk) {
    if (!renderer || !chunk) return;
    
    // Destroy old mesh if exists
    if (chunk->mesh) {
        mesh_destroy((ChunkMesh*)chunk->mesh);
        chunk->mesh = NULL;
    }
    
    // Build new mesh
    chunk->mesh = mesh_build(chunk);
}

void renderer_destroy_chunk_mesh(Chunk* chunk) {
    if (chunk && chunk->mesh) {
        mesh_destroy((ChunkMesh*)chunk->mesh);
        chunk->mesh = NULL;
    }
}

void renderer_draw_crosshair(Renderer* renderer) {
    if (!renderer) return;
    
    // TODO: Implement crosshair drawing
    // This requires a separate 2D rendering system
}

void renderer_draw_debug_info(Renderer* renderer, Player* player,
                              int chunk_count, int fps) {
    if (!renderer || !player || !renderer->show_debug) return;
    
    // TODO: Implement text rendering
    // This requires a text rendering system
    
    // For now, just print to console occasionally
    static int frame_counter = 0;
    if (frame_counter++ % 60 == 0) {
        printf("FPS: %d | Pos: (%.1f, %.1f, %.1f) | Chunks: %d\n",
               fps, player->position[0], player->position[1], 
               player->position[2], chunk_count);
    }
}

void renderer_resize(Renderer* renderer, int width, int height) {
    if (renderer) {
        renderer->width = width;
        renderer->height = height;
        glViewport(0, 0, width, height);
    }
}