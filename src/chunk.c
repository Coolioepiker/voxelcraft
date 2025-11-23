#include "chunk.h"
#include <stdlib.h>
#include <string.h>

Chunk* chunk_create(int x, int z) {
    Chunk* chunk = (Chunk*)malloc(sizeof(Chunk));
    if (!chunk) return NULL;
    
    chunk->x = x;
    chunk->z = z;
    chunk->is_generated = false;
    chunk->is_dirty = true;
    chunk->mesh = NULL;
    
    // Initialize neighbors to NULL
    chunk->north = NULL;
    chunk->south = NULL;
    chunk->east = NULL;
    chunk->west = NULL;
    
    // Clear blocks
    memset(chunk->blocks, 0, sizeof(chunk->blocks));
    
    return chunk;
}

void chunk_destroy(Chunk* chunk) {
    if (chunk) {
        // Mesh is freed by renderer
        free(chunk);
    }
}

BlockType chunk_get_block(Chunk* chunk, int x, int y, int z) {
    if (!chunk) return BLOCK_AIR;
    
    if (x >= 0 && x < CHUNK_SIZE && 
        y >= 0 && y < CHUNK_HEIGHT && 
        z >= 0 && z < CHUNK_SIZE) {
        return chunk->blocks[x][y][z];
    }
    
    return BLOCK_AIR;
}

void chunk_set_block(Chunk* chunk, int x, int y, int z, BlockType type) {
    if (!chunk) return;
    
    if (x >= 0 && x < CHUNK_SIZE && 
        y >= 0 && y < CHUNK_HEIGHT && 
        z >= 0 && z < CHUNK_SIZE) {
        
        if (chunk->blocks[x][y][z] != type) {
            chunk->blocks[x][y][z] = type;
            chunk->is_dirty = true;
            
            // Mark neighboring chunks dirty if on edge
            if (x == 0 && chunk->west) {
                chunk->west->is_dirty = true;
            } else if (x == CHUNK_SIZE - 1 && chunk->east) {
                chunk->east->is_dirty = true;
            }
            
            if (z == 0 && chunk->north) {
                chunk->north->is_dirty = true;
            } else if (z == CHUNK_SIZE - 1 && chunk->south) {
                chunk->south->is_dirty = true;
            }
        }
    }
}

BlockType chunk_get_neighbor_block(Chunk* chunk, int x, int y, int z) {
    if (!chunk) return BLOCK_AIR;
    
    // Check if out of vertical bounds
    if (y < 0 || y >= CHUNK_HEIGHT) {
        return BLOCK_AIR;
    }
    
    // If within this chunk, return directly
    if (x >= 0 && x < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
        return chunk->blocks[x][y][z];
    }
    
    // Check neighboring chunks
    Chunk* neighbor = NULL;
    int local_x = x;
    int local_z = z;
    
    if (x < 0) {
        neighbor = chunk->west;
        local_x = CHUNK_SIZE - 1;
    } else if (x >= CHUNK_SIZE) {
        neighbor = chunk->east;
        local_x = 0;
    }
    
    if (z < 0) {
        neighbor = chunk->north;
        local_z = CHUNK_SIZE - 1;
    } else if (z >= CHUNK_SIZE) {
        neighbor = chunk->south;
        local_z = 0;
    }
    
    if (neighbor && neighbor->is_generated) {
        return chunk_get_block(neighbor, local_x, y, local_z);
    }
    
    return BLOCK_AIR;
}

bool chunk_is_block_visible(Chunk* chunk, int x, int y, int z) {
    BlockType block = chunk_get_block(chunk, x, y, z);
    
    if (block == BLOCK_AIR) {
        return false;
    }
    
    // Check all 6 faces
    int dx[] = {1, -1, 0, 0, 0, 0};
    int dy[] = {0, 0, 1, -1, 0, 0};
    int dz[] = {0, 0, 0, 0, 1, -1};
    
    for (int i = 0; i < 6; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        int nz = z + dz[i];
        
        BlockType neighbor = chunk_get_neighbor_block(chunk, nx, ny, nz);
        
        // Face is visible if neighbor is air or transparent
        if (neighbor == BLOCK_AIR || block_is_transparent(neighbor)) {
            return true;
        }
    }
    
    return false;
}