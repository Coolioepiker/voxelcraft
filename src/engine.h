#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <GLFW/glfw3.h>
#include "world.h"
#include "player.h"
#include "renderer.h"

typedef struct {
    GLFWwindow* window;
    World* world;
    Player* player;
    Renderer* renderer;
    bool mouse_captured;
    double last_mouse_x;
    double last_mouse_y;
    bool first_mouse;
    int fps;
    bool show_debug;
    double last_block_action;
} Engine;

Engine* engine_create(GLFWwindow* window);
void engine_destroy(Engine* engine);
void engine_update(Engine* engine, float dt);
void engine_render(Engine* engine);
void engine_resize(Engine* engine, int width, int height);
void engine_key_callback(Engine* engine, int key, int action, int mods);
void engine_mouse_button_callback(Engine* engine, int button, int action);
void engine_mouse_move_callback(Engine* engine, double xpos, double ypos);

#endif