#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "engine.h"
#include "config.h"

Engine* g_engine = NULL;

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (g_engine) {
        engine_key_callback(g_engine, key, action, mods);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (g_engine) {
        engine_mouse_button_callback(g_engine, button, action);
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (g_engine) {
        engine_mouse_move_callback(g_engine, xpos, ypos);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (g_engine) {
        engine_resize(g_engine, width, height);
    }
}

int main(void) {
    printf("VoxelCraft - Starting...\n");
    printf("Controls:\n");
    printf("  WASD - Move\n");
    printf("  Space - Jump\n");
    printf("  Shift - Sprint\n");
    printf("  Mouse - Look around\n");
    printf("  Left Click - Break block\n");
    printf("  Right Click - Place block\n");
    printf("  1-9 - Select block type\n");
    printf("  ESC - Release mouse\n");
    printf("  F3 - Toggle debug info\n");
    printf("  F5 - Save world\n");
    printf("  F9 - Load world\n\n");

    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "VoxelCraft", NULL, NULL);
    
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    g_engine = engine_create(window);
    if (!g_engine) {
        fprintf(stderr, "Failed to create game engine\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    printf("Engine initialized successfully\n");

    double last_time = glfwGetTime();
    double last_fps_time = last_time;
    int frame_count = 0;

    while (!glfwWindowShouldClose(window)) {
        double current_time = glfwGetTime();
        float dt = (float)(current_time - last_time);
        last_time = current_time;

        frame_count++;
        if (current_time - last_fps_time >= 1.0) {
            g_engine->fps = frame_count;
            frame_count = 0;
            last_fps_time = current_time;
        }

        engine_update(g_engine, dt);
        engine_render(g_engine);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    printf("Shutting down...\n");
    engine_destroy(g_engine);
    glfwDestroyWindow(window);
    glfwTerminate();

    printf("Goodbye!\n");
    return 0;
}