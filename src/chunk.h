#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <stdbool.h>
#include "blocks.h"
#include "config.h"

typedef struct Chunk Chunk;

struct Chunk {
    int x, z;
    uint8_t blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    bool is_generated;
    bool is_dirty;
    Chunk* north;
    Chunk* south;
    Chunk* east;
    Chunk* west;
    void* mesh;
};

Chunk* chunk_create(int x, int z);
void chunk_destroy(Chunk* chunk);
BlockType chunk_get_block(Chunk* chunk, int x, int y, int z);
void chunk_set_block(Chunk* chunk, int x, int y, int z, BlockType type);
BlockType chunk_get_neighbor_block(Chunk* chunk, int x, int y, int z);
bool chunk_is_block_visible(Chunk* chunk, int x, int y, int z);

#endif