#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "world.h"
#include "blocks.h"

typedef struct {
    float position[3];
    float velocity[3];
    float pitch;
    float yaw;
    bool move_forward;
    bool move_backward;
    bool move_left;
    bool move_right;
    bool jump;
    bool sprint;
    bool on_ground;
    BlockType selected_block;
    World* world;
} Player;

Player* player_create(World* world, float x, float y, float z);
void player_destroy(Player* player);
void player_update(Player* player, float dt);
void player_set_movement(Player* player, bool forward, bool backward, 
                        bool left, bool right, bool jump, bool sprint);
void player_rotate(Player* player, float dx, float dy);
void player_get_view_matrix(Player* player, float* eye, float* center, float* up);
void player_get_look_direction(Player* player, float* direction);

#endif