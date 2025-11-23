#include "blocks.h"
#include <string.h>

BlockInfo BLOCK_REGISTRY[BLOCK_COUNT];

void blocks_init(void) {
    memset(BLOCK_REGISTRY, 0, sizeof(BLOCK_REGISTRY));

    BLOCK_REGISTRY[BLOCK_AIR] = (BlockInfo){
        .type = BLOCK_AIR, .name = "air",
        .color = {0.0f, 0.0f, 0.0f},
        .is_solid = false, .is_transparent = true
    };

    BLOCK_REGISTRY[BLOCK_GRASS] = (BlockInfo){
        .type = BLOCK_GRASS, .name = "grass",
        .color = {0.4f, 0.8f, 0.2f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_DIRT] = (BlockInfo){
        .type = BLOCK_DIRT, .name = "dirt",
        .color = {0.6f, 0.4f, 0.2f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_STONE] = (BlockInfo){
        .type = BLOCK_STONE, .name = "stone",
        .color = {0.5f, 0.5f, 0.5f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_SAND] = (BlockInfo){
        .type = BLOCK_SAND, .name = "sand",
        .color = {0.9f, 0.9f, 0.6f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_WATER] = (BlockInfo){
        .type = BLOCK_WATER, .name = "water",
        .color = {0.2f, 0.4f, 0.8f},
        .is_solid = false, .is_transparent = true
    };

    BLOCK_REGISTRY[BLOCK_COAL_ORE] = (BlockInfo){
        .type = BLOCK_COAL_ORE, .name = "coal_ore",
        .color = {0.2f, 0.2f, 0.2f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_IRON_ORE] = (BlockInfo){
        .type = BLOCK_IRON_ORE, .name = "iron_ore",
        .color = {0.7f, 0.5f, 0.4f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_GOLD_ORE] = (BlockInfo){
        .type = BLOCK_GOLD_ORE, .name = "gold_ore",
        .color = {0.9f, 0.8f, 0.2f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_DIAMOND_ORE] = (BlockInfo){
        .type = BLOCK_DIAMOND_ORE, .name = "diamond_ore",
        .color = {0.3f, 0.8f, 0.9f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_WOOD] = (BlockInfo){
        .type = BLOCK_WOOD, .name = "wood",
        .color = {0.6f, 0.4f, 0.2f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_PLANKS] = (BlockInfo){
        .type = BLOCK_PLANKS, .name = "planks",
        .color = {0.8f, 0.6f, 0.3f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_GLASS] = (BlockInfo){
        .type = BLOCK_GLASS, .name = "glass",
        .color = {0.8f, 0.9f, 1.0f},
        .is_solid = true, .is_transparent = true
    };

    BLOCK_REGISTRY[BLOCK_BRICK] = (BlockInfo){
        .type = BLOCK_BRICK, .name = "brick",
        .color = {0.7f, 0.3f, 0.2f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_COBBLESTONE] = (BlockInfo){
        .type = BLOCK_COBBLESTONE, .name = "cobblestone",
        .color = {0.6f, 0.6f, 0.6f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_LEAVES] = (BlockInfo){
        .type = BLOCK_LEAVES, .name = "leaves",
        .color = {0.2f, 0.6f, 0.2f},
        .is_solid = true, .is_transparent = true
    };

    BLOCK_REGISTRY[BLOCK_SNOW] = (BlockInfo){
        .type = BLOCK_SNOW, .name = "snow",
        .color = {0.95f, 0.95f, 1.0f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_ICE] = (BlockInfo){
        .type = BLOCK_ICE, .name = "ice",
        .color = {0.7f, 0.85f, 1.0f},
        .is_solid = true, .is_transparent = true
    };

    BLOCK_REGISTRY[BLOCK_GRAVEL] = (BlockInfo){
        .type = BLOCK_GRAVEL, .name = "gravel",
        .color = {0.5f, 0.5f, 0.5f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_BEDROCK] = (BlockInfo){
        .type = BLOCK_BEDROCK, .name = "bedrock",
        .color = {0.2f, 0.2f, 0.2f},
        .is_solid = true, .is_transparent = false
    };

    BLOCK_REGISTRY[BLOCK_LAVA] = (BlockInfo){
        .type = BLOCK_LAVA, .name = "lava",
        .color = {1.0f, 0.3f, 0.0f},
        .is_solid = false, .is_transparent = true
    };
}

const BlockInfo* block_get_info(BlockType type) {
    if (type >= 0 && type < BLOCK_COUNT) {
        return &BLOCK_REGISTRY[type];
    }
    return &BLOCK_REGISTRY[BLOCK_AIR];
}

bool block_is_solid(BlockType type) {
    return block_get_info(type)->is_solid;
}

bool block_is_transparent(BlockType type) {
    return block_get_info(type)->is_transparent;
}