#include "camera.h"
#include "config.h"
#include <math.h>
#include <string.h>

void mat4_identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov * 0.5f * M_PI / 180.0f);
    
    memset(m, 0, 16 * sizeof(float));
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
}

static void vec3_normalize(float* v) {
    float len = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len > 0.0f) {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

static void vec3_cross(float* result, float* a, float* b) {
    result[0] = a[1]*b[2] - a[2]*b[1];
    result[1] = a[2]*b[0] - a[0]*b[2];
    result[2] = a[0]*b[1] - a[1]*b[0];
}

void mat4_look_at(float* m, float* eye, float* center, float* up) {
    float f[3] = {
        center[0] - eye[0],
        center[1] - eye[1],
        center[2] - eye[2]
    };
    vec3_normalize(f);
    
    float r[3];
    vec3_cross(r, f, up);
    vec3_normalize(r);
    
    float u[3];
    vec3_cross(u, r, f);
    
    memset(m, 0, 16 * sizeof(float));
    m[0] = r[0];
    m[4] = r[1];
    m[8] = r[2];
    
    m[1] = u[0];
    m[5] = u[1];
    m[9] = u[2];
    
    m[2] = -f[0];
    m[6] = -f[1];
    m[10] = -f[2];
    
    m[12] = -(r[0]*eye[0] + r[1]*eye[1] + r[2]*eye[2]);
    m[13] = -(u[0]*eye[0] + u[1]*eye[1] + u[2]*eye[2]);
    m[14] = f[0]*eye[0] + f[1]*eye[1] + f[2]*eye[2];
    m[15] = 1.0f;
}

void mat4_translate(float* m, float x, float y, float z) {
    mat4_identity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

void mat4_multiply(float* result, float* a, float* b) {
    float temp[16];
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i*4 + j] = 
                a[i*4 + 0] * b[0*4 + j] +
                a[i*4 + 1] * b[1*4 + j] +
                a[i*4 + 2] * b[2*4 + j] +
                a[i*4 + 3] * b[3*4 + j];
        }
    }
    
    memcpy(result, temp, 16 * sizeof(float));
}