#include "player.h"
#include "config.h"
#include <stdlib.h>
#include <math.h>

Player* player_create(World* world, float x, float y, float z) {
    Player* player = (Player*)malloc(sizeof(Player));
    if (!player) return NULL;
    
    player->position[0] = x;
    player->position[1] = y;
    player->position[2] = z;
    
    player->velocity[0] = 0.0f;
    player->velocity[1] = 0.0f;
    player->velocity[2] = 0.0f;
    
    player->pitch = 0.0f;
    player->yaw = 0.0f;
    
    player->move_forward = false;
    player->move_backward = false;
    player->move_left = false;
    player->move_right = false;
    player->jump = false;
    player->sprint = false;
    
    player->on_ground = false;
    player->selected_block = BLOCK_GRASS;
    player->world = world;
    
    return player;
}

void player_destroy(Player* player) {
    free(player);
}

static bool check_collision(Player* player) {
    float x = player->position[0];
    float y = player->position[1];
    float z = player->position[2];
    
    float offsets[] = {-PLAYER_RADIUS, PLAYER_RADIUS};
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                int bx = (int)floor(x + offsets[i]);
                int by = (int)floor(y + (k == 0 ? 0 : PLAYER_HEIGHT));
                int bz = (int)floor(z + offsets[j]);
                
                BlockType block = world_get_block(player->world, bx, by, bz);
                if (block != BLOCK_AIR && block_is_solid(block)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void player_update(Player* player, float dt) {
    if (!player) return;
    
    player->velocity[1] -= GRAVITY * dt;
    if (player->velocity[1] < -TERMINAL_VELOCITY) {
        player->velocity[1] = -TERMINAL_VELOCITY;
    }
    
    float move_x = 0.0f;
    float move_z = 0.0f;
    
    float yaw_rad = player->yaw * M_PI / 180.0f;
    float forward_x = sinf(yaw_rad);
    float forward_z = cosf(yaw_rad);
    float right_x = cosf(yaw_rad);
    float right_z = -sinf(yaw_rad);
    
    if (player->move_forward) {
        move_x += forward_x;
        move_z += forward_z;
    }
    if (player->move_backward) {
        move_x -= forward_x;
        move_z -= forward_z;
    }
    if (player->move_right) {
        move_x += right_x;
        move_z += right_z;
    }
    if (player->move_left) {
        move_x -= right_x;
        move_z -= right_z;
    }
    
    float length = sqrtf(move_x * move_x + move_z * move_z);
    if (length > 0.0f) {
        move_x /= length;
        move_z /= length;
    }
    
    float speed = PLAYER_SPEED;
    if (player->sprint) {
        speed *= PLAYER_SPRINT_MULTIPLIER;
    }
    
    player->velocity[0] = move_x * speed;
    player->velocity[2] = move_z * speed;
    
    if (player->jump && player->on_ground) {
        player->velocity[1] = PLAYER_JUMP_SPEED;
        player->on_ground = false;
    }
    
    if (player->on_ground) {
        player->velocity[0] *= FRICTION;
        player->velocity[2] *= FRICTION;
    }
    
    player->position[0] += player->velocity[0] * dt;
    if (check_collision(player)) {
        player->position[0] -= player->velocity[0] * dt;
        player->velocity[0] = 0.0f;
    }
    
    player->position[1] += player->velocity[1] * dt;
    if (check_collision(player)) {
        if (player->velocity[1] < 0.0f) {
            player->on_ground = true;
        }
        player->position[1] -= player->velocity[1] * dt;
        player->velocity[1] = 0.0f;
    } else {
        player->on_ground = false;
    }
    
    player->position[2] += player->velocity[2] * dt;
    if (check_collision(player)) {
        player->position[2] -= player->velocity[2] * dt;
        player->velocity[2] = 0.0f;
    }
}

void player_set_movement(Player* player, bool forward, bool backward,
                        bool left, bool right, bool jump, bool sprint) {
    if (!player) return;
    
    player->move_forward = forward;
    player->move_backward = backward;
    player->move_left = left;
    player->move_right = right;
    player->jump = jump;
    player->sprint = sprint;
}

void player_rotate(Player* player, float dx, float dy) {
    if (!player) return;
    
    // Invert dx so mouse movement matches look direction
    player->yaw -= dx * MOUSE_SENSITIVITY;  // Changed from += to -=
    player->pitch -= dy * MOUSE_SENSITIVITY;
    
    if (player->pitch > 89.0f) player->pitch = 89.0f;
    if (player->pitch < -89.0f) player->pitch = -89.0f;
    
    while (player->yaw >= 360.0f) player->yaw -= 360.0f;
    while (player->yaw < 0.0f) player->yaw += 360.0f;
}

void player_get_view_matrix(Player* player, float* eye, float* center, float* up) {
    if (!player) return;
    
    eye[0] = player->position[0];
    eye[1] = player->position[1] + PLAYER_EYE_HEIGHT;
    eye[2] = player->position[2];
    
    float pitch_rad = player->pitch * M_PI / 180.0f;
    float yaw_rad = player->yaw * M_PI / 180.0f;
    
    float look_x = cosf(pitch_rad) * sinf(yaw_rad);
    float look_y = sinf(pitch_rad);
    float look_z = cosf(pitch_rad) * cosf(yaw_rad);
    
    center[0] = eye[0] + look_x;
    center[1] = eye[1] + look_y;
    center[2] = eye[2] + look_z;
    
    up[0] = 0.0f;
    up[1] = 1.0f;
    up[2] = 0.0f;
}

void player_get_look_direction(Player* player, float* direction) {
    if (!player) return;
    
    float pitch_rad = player->pitch * M_PI / 180.0f;
    float yaw_rad = player->yaw * M_PI / 180.0f;
    
    direction[0] = cosf(pitch_rad) * sinf(yaw_rad);
    direction[1] = sinf(pitch_rad);
    direction[2] = cosf(pitch_rad) * cosf(yaw_rad);
}