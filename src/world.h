#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include "chunk.h"
#include "config.h"

#define MAX_CHUNKS 1024

typedef struct {
    Chunk* chunks[MAX_CHUNKS];
    int chunk_count;
    int seed;
    void* terrain_gen;
} World;

World* world_create(int seed);
void world_destroy(World* world);
Chunk* world_get_chunk(World* world, int chunk_x, int chunk_z);
Chunk* world_find_chunk(World* world, int chunk_x, int chunk_z);
void world_add_chunk(World* world, Chunk* chunk);
BlockType world_get_block(World* world, int x, int y, int z);
bool world_set_block(World* world, int x, int y, int z, BlockType type);
void world_update_chunks(World* world, float player_x, float player_z);
int world_get_dirty_chunks(World* world, Chunk** out_chunks, int max_count);
bool world_raycast(World* world, float* origin, float* direction, 
                   int* hit_x, int* hit_y, int* hit_z,
                   int* prev_x, int* prev_y, int* prev_z);
bool world_save(World* world, const char* filename);
bool world_load(World* world, const char* filename);

#endif