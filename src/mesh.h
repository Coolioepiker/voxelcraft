#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include "chunk.h"

typedef struct {
    GLuint vao;
    GLuint vbo;
    int vertex_count;
} ChunkMesh;

ChunkMesh* mesh_build(Chunk* chunk);
void mesh_destroy(ChunkMesh* mesh);
void mesh_render(ChunkMesh* mesh);

#endif