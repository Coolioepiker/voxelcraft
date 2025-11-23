#ifndef TERRAIN_H
#define TERRAIN_H

#include "chunk.h"

typedef struct {
    int seed;
} TerrainGenerator;

TerrainGenerator* terrain_create(int seed);
void terrain_destroy(TerrainGenerator* gen);
void terrain_generate_chunk(TerrainGenerator* gen, Chunk* chunk);

#endif