#ifndef BLOCKS_H
#define BLOCKS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 1,
    BLOCK_DIRT = 2,
    BLOCK_STONE = 3,
    BLOCK_SAND = 4,
    BLOCK_WATER = 5,
    BLOCK_COAL_ORE = 6,
    BLOCK_IRON_ORE = 7,
    BLOCK_GOLD_ORE = 8,
    BLOCK_DIAMOND_ORE = 9,
    BLOCK_WOOD = 10,
    BLOCK_PLANKS = 11,
    BLOCK_GLASS = 12,
    BLOCK_BRICK = 13,
    BLOCK_COBBLESTONE = 14,
    BLOCK_LEAVES = 15,
    BLOCK_SNOW = 16,
    BLOCK_ICE = 17,
    BLOCK_GRAVEL = 18,
    BLOCK_BEDROCK = 19,
    BLOCK_LAVA = 20,
    BLOCK_COUNT = 21
} BlockType;

typedef struct {
    BlockType type;
    const char* name;
    float color[3];
    bool is_solid;
    bool is_transparent;
} BlockInfo;

extern BlockInfo BLOCK_REGISTRY[BLOCK_COUNT];

void blocks_init(void);
const BlockInfo* block_get_info(BlockType type);
bool block_is_solid(BlockType type);
bool block_is_transparent(BlockType type);

#endif