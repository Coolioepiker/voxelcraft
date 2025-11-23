/*
 * Game engine implementation
 */

#include "engine.h"
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

Engine* engine_create(GLFWwindow* window) {
    Engine* engine = (Engine*)malloc(sizeof(Engine));
    if (!engine) return NULL;
    
    engine->window = window;
    engine->mouse_captured = true;
    engine->first_mouse = true;
    engine->last_mouse_x = 0.0;
    engine->last_mouse_y = 0.0;
    engine->fps = 0;
    engine->show_debug = false;
    engine->last_block_action = 0.0;
    
    blocks_init();
    
    engine->world = world_create(12345);
    if (!engine->world) {
        free(engine);
        return NULL;
    }
    
    engine->player = player_create(engine->world, 0.0f, 100.0f, 0.0f);
    if (!engine->player) {
        world_destroy(engine->world);
        free(engine);
        return NULL;
    }
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    engine->renderer = renderer_create(width, height);
    if (!engine->renderer) {
        player_destroy(engine->player);
        world_destroy(engine->world);
        free(engine);
        return NULL;
    }
    
    return engine;
}

void engine_destroy(Engine* engine) {
    if (engine) {
        renderer_destroy(engine->renderer);
        player_destroy(engine->player);
        world_destroy(engine->world);
        free(engine);
    }
}

void engine_update(Engine* engine, float dt) {
    if (!engine) return;
    
    player_update(engine->player, dt);
    
    world_update_chunks(engine->world, 
                       engine->player->position[0],
                       engine->player->position[2]);
    
    Chunk* dirty_chunks[MAX_CHUNKS_PER_FRAME];
    int dirty_count = world_get_dirty_chunks(engine->world, dirty_chunks, 
                                            MAX_CHUNKS_PER_FRAME);
    
    for (int i = 0; i < dirty_count; i++) {
        renderer_build_chunk_mesh(engine->renderer, dirty_chunks[i]);
    }
}

void engine_render(Engine* engine) {
    if (!engine) return;
    
    renderer_begin(engine->renderer, engine->player);
    
    for (int i = 0; i < engine->world->chunk_count; i++) {
        Chunk* chunk = engine->world->chunks[i];
        if (chunk && chunk->is_generated && chunk->mesh) {
            renderer_render_chunk(engine->renderer, chunk);
        }
    }
    
    renderer_end(engine->renderer);
    
    renderer_draw_crosshair(engine->renderer);
    renderer_draw_debug_info(engine->renderer, engine->player,
                            engine->world->chunk_count, engine->fps);
}

void engine_resize(Engine* engine, int width, int height) {
    if (engine) {
        renderer_resize(engine->renderer, width, height);
    }
}

void engine_key_callback(Engine* engine, int key, int action, int mods) {
    if (!engine) return;
    
    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    
    if (key == GLFW_KEY_W) {
        engine->player->move_forward = pressed;
    } else if (key == GLFW_KEY_S) {
        engine->player->move_backward = pressed;
    } else if (key == GLFW_KEY_A) {
        engine->player->move_left = pressed;
    } else if (key == GLFW_KEY_D) {
        engine->player->move_right = pressed;
    } else if (key == GLFW_KEY_SPACE) {
        engine->player->jump = pressed;
    } else if (key == GLFW_KEY_LEFT_SHIFT) {
        engine->player->sprint = pressed;
    }
    
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            engine->mouse_captured = !engine->mouse_captured;
            glfwSetInputMode(engine->window, GLFW_CURSOR,
                           engine->mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            engine->first_mouse = true;
            printf("Mouse %s\n", engine->mouse_captured ? "captured" : "released");
        } else if (key == GLFW_KEY_F3) {
            engine->show_debug = !engine->show_debug;
            engine->renderer->show_debug = engine->show_debug;
        } else if (key == GLFW_KEY_F5) {
            world_save(engine->world, "saves/world.dat");
        } else if (key == GLFW_KEY_F9) {
            if (world_load(engine->world, "saves/world.dat")) {
                for (int i = 0; i < engine->world->chunk_count; i++) {
                    Chunk* chunk = engine->world->chunks[i];
                    if (chunk && chunk->is_generated) {
                        chunk->is_dirty = true;
                    }
                }
            }
        }
        
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
            int block_index = key - GLFW_KEY_1 + 1;
            if (block_index < BLOCK_COUNT) {
                engine->player->selected_block = (BlockType)block_index;
                printf("Selected block: %s\n", 
                       block_get_info(engine->player->selected_block)->name);
            }
        }
    }
}

void engine_mouse_button_callback(Engine* engine, int button, int action) {
    if (!engine || !engine->mouse_captured) return;
    
    if (action != GLFW_PRESS) return;
    
    double current_time = glfwGetTime();
    if (current_time - engine->last_block_action < BLOCK_PLACE_COOLDOWN) {
        return;
    }
    
    float eye[3] = {
        engine->player->position[0],
        engine->player->position[1] + PLAYER_EYE_HEIGHT,
        engine->player->position[2]
    };
    
    float direction[3];
    player_get_look_direction(engine->player, direction);
    
    int hit_x, hit_y, hit_z;
    int prev_x, prev_y, prev_z;
    
    if (world_raycast(engine->world, eye, direction,
                     &hit_x, &hit_y, &hit_z,
                     &prev_x, &prev_y, &prev_z)) {
        
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            world_set_block(engine->world, hit_x, hit_y, hit_z, BLOCK_AIR);
            engine->last_block_action = current_time;
            printf("Broke block at (%d, %d, %d)\n", hit_x, hit_y, hit_z);
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            int player_x = (int)floor(engine->player->position[0]);
            int player_y = (int)floor(engine->player->position[1]);
            int player_z = (int)floor(engine->player->position[2]);
            int player_y_top = (int)floor(engine->player->position[1] + PLAYER_HEIGHT);
            
            bool overlaps = (prev_x == player_x && prev_z == player_z &&
                           (prev_y == player_y || prev_y == player_y_top));
            
            if (!overlaps) {
                world_set_block(engine->world, prev_x, prev_y, prev_z,
                              engine->player->selected_block);
                engine->last_block_action = current_time;
                printf("Placed block at (%d, %d, %d)\n", prev_x, prev_y, prev_z);
            }
        }
    }
}

void engine_mouse_move_callback(Engine* engine, double xpos, double ypos) {
    if (!engine || !engine->mouse_captured) return;
    
    if (engine->first_mouse) {
        engine->last_mouse_x = xpos;
        engine->last_mouse_y = ypos;
        engine->first_mouse = false;
        return;
    }
    
    double dx = xpos - engine->last_mouse_x;
    double dy = ypos - engine->last_mouse_y;
    
    engine->last_mouse_x = xpos;
    engine->last_mouse_y = ypos;
    
    player_rotate(engine->player, (float)dx, (float)dy);
}