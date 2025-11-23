#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

GLuint shader_load(const char* vertex_path, const char* fragment_path);
void shader_delete(GLuint program);
void shader_set_mat4(GLuint program, const char* name, float* matrix);
void shader_set_vec3(GLuint program, const char* name, float x, float y, float z);
void shader_set_float(GLuint program, const char* name, float value);
void shader_set_int(GLuint program, const char* name, int value);

#endif