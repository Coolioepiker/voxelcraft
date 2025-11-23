#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    
    fclose(file);
    return buffer;
}

static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Shader compilation failed:\n%s\n", info_log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint shader_load(const char* vertex_path, const char* fragment_path) {
    char* vertex_source = read_file(vertex_path);
    char* fragment_source = read_file(fragment_path);
    
    if (!vertex_source || !fragment_source) {
        free(vertex_source);
        free(fragment_source);
        return 0;
    }
    
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);
    
    free(vertex_source);
    free(fragment_source);
    
    if (!vertex_shader || !fragment_shader) {
        if (vertex_shader) glDeleteShader(vertex_shader);
        if (fragment_shader) glDeleteShader(fragment_shader);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Shader linking failed:\n%s\n", info_log);
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

void shader_delete(GLuint program) {
    if (program) {
        glDeleteProgram(program);
    }
}

void shader_set_mat4(GLuint program, const char* name, float* matrix) {
    GLint location = glGetUniformLocation(program, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }
}

void shader_set_vec3(GLuint program, const char* name, float x, float y, float z) {
    GLint location = glGetUniformLocation(program, name);
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void shader_set_float(GLuint program, const char* name, float value) {
    GLint location = glGetUniformLocation(program, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void shader_set_int(GLuint program, const char* name, int value) {
    GLint location = glGetUniformLocation(program, name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}