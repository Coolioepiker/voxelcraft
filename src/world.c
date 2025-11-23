#include "world.h"
#include "terrain.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

World* world_create(int seed) {
    World* world = (World*)malloc(sizeof(World));
    if (!world) return NULL;
    
    world->chunk_count = 0;
    world->seed = seed;
    world->terrain_gen = terrain_create(seed);
    
    for (int i = 0; i < MAX_CHUNKS; i++) {
        world->chunks[i] = NULL;
    }
    
    return world;
}

void world_destroy(World* world) {
    if (!world) return;
    
    // Free all chunks
    for (int i = 0; i < world->chunk_count; i++) {
        chunk_destroy(world->chunks[i]);
    }
    
    // Free terrain generator
    terrain_destroy((TerrainGenerator*)world->terrain_gen);
    
    free(world);
}

Chunk* world_find_chunk(World* world, int chunk_x, int chunk_z) {
    if (!world) return NULL;
    
    for (int i = 0; i < world->chunk_count; i++) {
        Chunk* chunk = world->chunks[i];
        if (chunk && chunk->x == chunk_x && chunk->z == chunk_z) {
            return chunk;
        }
    }
    
    return NULL;
}

static void setup_chunk_neighbors(World* world, Chunk* chunk) {
    chunk->north = world_find_chunk(world, chunk->x, chunk->z - 1);
    chunk->south = world_find_chunk(world, chunk->x, chunk->z + 1);
    chunk->east = world_find_chunk(world, chunk->x + 1, chunk->z);
    chunk->west = world_find_chunk(world, chunk->x - 1, chunk->z);
    
    // Update neighbors' references
    if (chunk->north) chunk->north->south = chunk;
    if (chunk->south) chunk->south->north = chunk;
    if (chunk->east) chunk->east->west = chunk;
    if (chunk->west) chunk->west->east = chunk;
}

void world_add_chunk(World* world, Chunk* chunk) {
    if (!world || !chunk || world->chunk_count >= MAX_CHUNKS) return;
    
    world->chunks[world->chunk_count++] = chunk;
    setup_chunk_neighbors(world, chunk);
}

Chunk* world_get_chunk(World* world, int chunk_x, int chunk_z) {
    if (!world) return NULL;
    
    // Check if chunk already exists
    Chunk* chunk = world_find_chunk(world, chunk_x, chunk_z);
    if (chunk) return chunk;
    
    // Create new chunk
    chunk = chunk_create(chunk_x, chunk_z);
    if (!chunk) return NULL;
    
    world_add_chunk(world, chunk);
    
    // Generate terrain if not loaded
    if (!chunk->is_generated) {
        terrain_generate_chunk((TerrainGenerator*)world->terrain_gen, chunk);
    }
    
    return chunk;
}

BlockType world_get_block(World* world, int x, int y, int z) {
    if (!world || y < 0 || y >= CHUNK_HEIGHT) {
        return BLOCK_AIR;
    }
    
    int chunk_x = (int)floor((float)x / CHUNK_SIZE);
    int chunk_z = (int)floor((float)z / CHUNK_SIZE);
    int local_x = x - chunk_x * CHUNK_SIZE;
    int local_z = z - chunk_z * CHUNK_SIZE;
    
    // Handle negative coordinates
    if (x < 0 && local_x != 0) {
        local_x = CHUNK_SIZE + local_x;
        chunk_x--;
    }
    if (z < 0 && local_z != 0) {
        local_z = CHUNK_SIZE + local_z;
        chunk_z--;
    }
    
    Chunk* chunk = world_find_chunk(world, chunk_x, chunk_z);
    if (chunk && chunk->is_generated) {
        return chunk_get_block(chunk, local_x, y, local_z);
    }
    
    return BLOCK_AIR;
}

bool world_set_block(World* world, int x, int y, int z, BlockType type) {
    if (!world || y < 0 || y >= CHUNK_HEIGHT) {
        return false;
    }
    
    int chunk_x = (int)floor((float)x / CHUNK_SIZE);
    int chunk_z = (int)floor((float)z / CHUNK_SIZE);
    int local_x = x - chunk_x * CHUNK_SIZE;
    int local_z = z - chunk_z * CHUNK_SIZE;
    
    // Handle negative coordinates
    if (x < 0 && local_x != 0) {
        local_x = CHUNK_SIZE + local_x;
        chunk_x--;
    }
    if (z < 0 && local_z != 0) {
        local_z = CHUNK_SIZE + local_z;
        chunk_z--;
    }
    
    Chunk* chunk = world_get_chunk(world, chunk_x, chunk_z);
    if (chunk && chunk->is_generated) {
        chunk_set_block(chunk, local_x, y, local_z, type);
        return true;
    }
    
    return false;
}

void world_update_chunks(World* world, float player_x, float player_z) {
    if (!world) return;
    
    int player_chunk_x = (int)floor(player_x / CHUNK_SIZE);
    int player_chunk_z = (int)floor(player_z / CHUNK_SIZE);
    
    // Load chunks in render distance
    for (int dx = -RENDER_DISTANCE; dx <= RENDER_DISTANCE; dx++) {
        for (int dz = -RENDER_DISTANCE; dz <= RENDER_DISTANCE; dz++) {
            world_get_chunk(world, player_chunk_x + dx, player_chunk_z + dz);
        }
    }
    
    // Unload distant chunks (simple version - don't unload for now)
    // In a real implementation, you'd remove chunks beyond render distance + margin
}

int world_get_dirty_chunks(World* world, Chunk** out_chunks, int max_count) {
    if (!world || !out_chunks) return 0;
    
    int count = 0;
    for (int i = 0; i < world->chunk_count && count < max_count; i++) {
        Chunk* chunk = world->chunks[i];
        if (chunk && chunk->is_dirty && chunk->is_generated) {
            out_chunks[count++] = chunk;
        }
    }
    
    return count;
}

bool world_raycast(World* world, float* origin, float* direction,
                   int* hit_x, int* hit_y, int* hit_z,
                   int* prev_x, int* prev_y, int* prev_z) {
    if (!world) return false;
    
    float step = 0.1f;
    float distance = 0.0f;
    float max_distance = REACH_DISTANCE;
    
    int last_x = (int)floor(origin[0]);
    int last_y = (int)floor(origin[1]);
    int last_z = (int)floor(origin[2]);
    
    while (distance < max_distance) {
        float x = origin[0] + direction[0] * distance;
        float y = origin[1] + direction[1] * distance;
        float z = origin[2] + direction[2] * distance;
        
        int bx = (int)floor(x);
        int by = (int)floor(y);
        int bz = (int)floor(z);
        
        BlockType block = world_get_block(world, bx, by, bz);
        
        if (block != BLOCK_AIR && block_is_solid(block)) {
            *hit_x = bx;
            *hit_y = by;
            *hit_z = bz;
            *prev_x = last_x;
            *prev_y = last_y;
            *prev_z = last_z;
            return true;
        }
        
        last_x = bx;
        last_y = by;
        last_z = bz;
        
        distance += step;
    }
    
    return false;
}

bool world_save(World* world, const char* filename) {
    if (!world) return false;
    
    FILE* file = fopen(filename, "wb");
    if (!file) return false;
    
    // Write seed
    fwrite(&world->seed, sizeof(int), 1, file);
    
    // Write chunk count
    fwrite(&world->chunk_count, sizeof(int), 1, file);
    
    // Write each chunk
    for (int i = 0; i < world->chunk_count; i++) {
        Chunk* chunk = world->chunks[i];
        if (chunk && chunk->is_generated) {
            fwrite(&chunk->x, sizeof(int), 1, file);
            fwrite(&chunk->z, sizeof(int), 1, file);
            fwrite(chunk->blocks, sizeof(chunk->blocks), 1, file);
        }
    }
    
    fclose(file);
    printf("World saved: %d chunks\n", world->chunk_count);
    return true;
}

bool world_load(World* world, const char* filename) {
    if (!world) return false;
    
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Save file not found: %s\n", filename);
        return false;
    }
    
    // Clear existing chunks
    for (int i = 0; i < world->chunk_count; i++) {
        chunk_destroy(world->chunks[i]);
        world->chunks[i] = NULL;
    }
    world->chunk_count = 0;
    
    // Read seed
    fread(&world->seed, sizeof(int), 1, file);
    
    // Read chunk count
    int chunk_count;
    fread(&chunk_count, sizeof(int), 1, file);
    
    // Read each chunk
    for (int i = 0; i < chunk_count; i++) {
        int x, z;
        fread(&x, sizeof(int), 1, file);
        fread(&z, sizeof(int), 1, file);
        
        Chunk* chunk = chunk_create(x, z);
        fread(chunk->blocks, sizeof(chunk->blocks), 1, file);
        chunk->is_generated = true;
        chunk->is_dirty = true;
        
        world_add_chunk(world, chunk);
    }
    
    fclose(file);
    printf("World loaded: %d chunks\n", world->chunk_count);
    return true;
}