#include "mesh.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES (CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE * 6 * 6 * 6)

static void add_face(float* vertices, int* vertex_count, 
                    float x, float y, float z,
                    int face, const float* color) {
    float brightness = 1.0f;
    
    // Adjust brightness based on face direction
    if (face == 0) brightness = 1.0f;      // Top
    else if (face == 1) brightness = 0.5f; // Bottom
    else if (face == 2 || face == 3) brightness = 0.8f; // East/West
    else brightness = 0.7f; // North/South
    
    float r = color[0] * brightness;
    float g = color[1] * brightness;
    float b = color[2] * brightness;
    
    float face_vertices[6][6]; // 6 vertices, each with 6 floats (x,y,z,r,g,b)
    
    switch (face) {
        case 0: // Top
            memcpy(face_vertices, (float[][6]){
                {x, y+1, z,     r, g, b},
                {x+1, y+1, z,   r, g, b},
                {x+1, y+1, z+1, r, g, b},
                {x, y+1, z,     r, g, b},
                {x+1, y+1, z+1, r, g, b},
                {x, y+1, z+1,   r, g, b}
            }, sizeof(face_vertices));
            break;
        case 1: // Bottom
            memcpy(face_vertices, (float[][6]){
                {x, y, z,       r, g, b},
                {x+1, y, z+1,   r, g, b},
                {x+1, y, z,     r, g, b},
                {x, y, z,       r, g, b},
                {x, y, z+1,     r, g, b},
                {x+1, y, z+1,   r, g, b}
            }, sizeof(face_vertices));
            break;
        case 2: // East
            memcpy(face_vertices, (float[][6]){
                {x+1, y, z,     r, g, b},
                {x+1, y+1, z,   r, g, b},
                {x+1, y+1, z+1, r, g, b},
                {x+1, y, z,     r, g, b},
                {x+1, y+1, z+1, r, g, b},
                {x+1, y, z+1,   r, g, b}
            }, sizeof(face_vertices));
            break;
        case 3: // West
            memcpy(face_vertices, (float[][6]){
                {x, y, z,       r, g, b},
                {x, y+1, z+1,   r, g, b},
                {x, y+1, z,     r, g, b},
                {x, y, z,       r, g, b},
                {x, y, z+1,     r, g, b},
                {x, y+1, z+1,   r, g, b}
            }, sizeof(face_vertices));
            break;
        case 4: // South
            memcpy(face_vertices, (float[][6]){
                {x, y, z+1,     r, g, b},
                {x+1, y+1, z+1, r, g, b},
                {x+1, y, z+1,   r, g, b},
                {x, y, z+1,     r, g, b},
                {x, y+1, z+1,   r, g, b},
                {x+1, y+1, z+1, r, g, b}
            }, sizeof(face_vertices));
            break;
        case 5: // North
            memcpy(face_vertices, (float[][6]){
                {x, y, z,       r, g, b},
                {x+1, y, z,     r, g, b},
                {x+1, y+1, z,   r, g, b},
                {x, y, z,       r, g, b},
                {x+1, y+1, z,   r, g, b},
                {x, y+1, z,     r, g, b}
            }, sizeof(face_vertices));
            break;
    }
    
    // Copy to vertex buffer
    memcpy(&vertices[*vertex_count * 6], face_vertices, sizeof(face_vertices));
    *vertex_count += 6;
}

ChunkMesh* mesh_build(Chunk* chunk) {
    if (!chunk || !chunk->is_generated) return NULL;
    
    // Allocate temporary vertex buffer
    float* vertices = (float*)malloc(MAX_VERTICES * sizeof(float));
    if (!vertices) return NULL;
    
    int vertex_count = 0;
    
    // Iterate through all blocks
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                BlockType block = chunk_get_block(chunk, x, y, z);
                
                if (block == BLOCK_AIR) continue;
                
                const BlockInfo* info = block_get_info(block);
                
                // World position
                float wx = (float)(chunk->x * CHUNK_SIZE + x);
                float wy = (float)y;
                float wz = (float)(chunk->z * CHUNK_SIZE + z);
                
                // Check each face
                int directions[][3] = {
                    {0, 1, 0},   // Top
                    {0, -1, 0},  // Bottom
                    {1, 0, 0},   // East
                    {-1, 0, 0},  // West
                    {0, 0, 1},   // South
                    {0, 0, -1}   // North
                };
                
                for (int face = 0; face < 6; face++) {
                    int nx = x + directions[face][0];
                    int ny = y + directions[face][1];
                    int nz = z + directions[face][2];
                    
                    BlockType neighbor = chunk_get_neighbor_block(chunk, nx, ny, nz);
                    
                    // Render face if neighbor is air or transparent
                    if (neighbor == BLOCK_AIR || block_is_transparent(neighbor)) {
                        add_face(vertices, &vertex_count, wx, wy, wz, face, info->color);
                    }
                }
            }
        }
    }
    
    // Create mesh if we have vertices
    if (vertex_count == 0) {
        free(vertices);
        return NULL;
    }
    
    ChunkMesh* mesh = (ChunkMesh*)malloc(sizeof(ChunkMesh));
    if (!mesh) {
        free(vertices);
        return NULL;
    }
    
    mesh->vertex_count = vertex_count;
    
    // Create VAO and VBO
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 6 * sizeof(float), 
                 vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
                         (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    free(vertices);
    
    chunk->is_dirty = false;
    
    return mesh;
}

void mesh_destroy(ChunkMesh* mesh) {
    if (mesh) {
        glDeleteVertexArrays(1, &mesh->vao);
        glDeleteBuffers(1, &mesh->vbo);
        free(mesh);
    }
}

void mesh_render(ChunkMesh* mesh) {
    if (mesh && mesh->vertex_count > 0) {
        glBindVertexArray(mesh->vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
        glBindVertexArray(0);
    }
}