#ifndef CAMERA_H
#define CAMERA_H

void mat4_identity(float* m);
void mat4_perspective(float* m, float fov, float aspect, float near, float far);
void mat4_look_at(float* m, float* eye, float* center, float* up);
void mat4_translate(float* m, float x, float y, float z);
void mat4_multiply(float* result, float* a, float* b);

#endif