#include "terrain.h"
#include "config.h"
#include <stdlib.h>
#include <math.h>

static float simple_noise(float x, float z, int seed) {
    int n = (int)(x * 57 + z * 131 + seed);
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

static float fbm(float x, float z, int seed) {
    float total = 0.0f;
    float frequency = TERRAIN_SCALE;
    float amplitude = 1.0f;
    float max_value = 0.0f;
    
    for (int i = 0; i < TERRAIN_OCTAVES; i++) {
        total += simple_noise(x * frequency, z * frequency, seed) * amplitude;
        max_value += amplitude;
        amplitude *= TERRAIN_PERSISTENCE;
        frequency *= TERRAIN_LACUNARITY;
    }
    
    return total / max_value;
}

static int get_height(TerrainGenerator* gen, int x, int z) {
    float noise_val = fbm((float)x, (float)z, gen->seed);
    int height = TERRAIN_BASE + (int)(noise_val * TERRAIN_HEIGHT_MULTIPLIER);
    
    if (height < 1) height = 1;
    if (height >= CHUNK_HEIGHT) height = CHUNK_HEIGHT - 1;
    
    return height;
}

static BlockType get_ore_type(int y) {
    int rand_val = rand() % 100;
    
    if (y < 16) {
        if (rand_val < 30) return BLOCK_DIAMOND_ORE;
    } else if (y < 32) {
        if (rand_val < 40) return BLOCK_GOLD_ORE;
    } else if (y < 64) {
        if (rand_val < 50) return BLOCK_IRON_ORE;
    }
    
    return BLOCK_COAL_ORE;
}

static void generate_column(TerrainGenerator* gen, Chunk* chunk, int x, int z, int height) {
    chunk_set_block(chunk, x, 0, z, BLOCK_BEDROCK);
    
    int stone_height = height - 4;
    if (stone_height < 1) stone_height = 1;
    
    for (int y = 1; y < stone_height; y++) {
        if (rand() % 100 < 1) {
            chunk_set_block(chunk, x, y, z, get_ore_type(y));
        } else {
            chunk_set_block(chunk, x, y, z, BLOCK_STONE);
        }
    }
    
    int dirt_height = height - 1;
    if (dirt_height < stone_height) dirt_height = stone_height;
    
    for (int y = stone_height; y < dirt_height; y++) {
        chunk_set_block(chunk, x, y, z, BLOCK_DIRT);
    }
    
    if (height >= SEA_LEVEL) {
        chunk_set_block(chunk, x, height, z, BLOCK_GRASS);
        
        if (height > SEA_LEVEL + 30) {
            chunk_set_block(chunk, x, height, z, BLOCK_SNOW);
        }
        
        if (rand() % 100 < 2 && height < CHUNK_HEIGHT - 10) {
            for (int dy = 1; dy < 6; dy++) {
                chunk_set_block(chunk, x, height + dy, z, BLOCK_WOOD);
            }
            
            for (int dx = -2; dx <= 2; dx++) {
                for (int dz = -2; dz <= 2; dz++) {
                    for (int dy = 4; dy <= 7; dy++) {
                        if (x + dx >= 0 && x + dx < CHUNK_SIZE &&
                            z + dz >= 0 && z + dz < CHUNK_SIZE &&
                            height + dy < CHUNK_HEIGHT) {
                            if (abs(dx) + abs(dz) <= 3) {
                                if (chunk_get_block(chunk, x + dx, height + dy, z + dz) == BLOCK_AIR) {
                                    chunk_set_block(chunk, x + dx, height + dy, z + dz, BLOCK_LEAVES);
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        if (height > SEA_LEVEL - 3) {
            for (int y = height - 2; y <= height; y++) {
                if (y >= 0) {
                    chunk_set_block(chunk, x, y, z, BLOCK_SAND);
                }
            }
        } else {
            chunk_set_block(chunk, x, height, z, BLOCK_DIRT);
        }
        
        for (int y = height + 1; y <= SEA_LEVEL; y++) {
            chunk_set_block(chunk, x, y, z, BLOCK_WATER);
        }
    }
}

TerrainGenerator* terrain_create(int seed) {
    TerrainGenerator* gen = (TerrainGenerator*)malloc(sizeof(TerrainGenerator));
    if (!gen) return NULL;
    
    gen->seed = seed;
    srand(seed);
    
    return gen;
}

void terrain_destroy(TerrainGenerator* gen) {
    free(gen);
}

void terrain_generate_chunk(TerrainGenerator* gen, Chunk* chunk) {
    if (!gen || !chunk) return;
    
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            int world_x = chunk->x * CHUNK_SIZE + x;
            int world_z = chunk->z * CHUNK_SIZE + z;
            
            int height = get_height(gen, world_x, world_z);
            generate_column(gen, chunk, x, z, height);
        }
    }
    
    chunk->is_generated = true;
    chunk->is_dirty = true;
}