import noise
import numpy as np
from core.config import Config
from world.block_types import BLOCKS


class TerrainGenerator:
    """Generates terrain using Perlin noise"""
    
    def __init__(self, seed=None):
        self.seed = seed if seed is not None else np.random.randint(0, 1000000)
    
    def generate_chunk(self, chunk):
        """Generate terrain for a chunk"""
        chunk_x = chunk.x * Config.CHUNK_SIZE
        chunk_z = chunk.z * Config.CHUNK_SIZE
        
        # Generate height map
        for x in range(Config.CHUNK_SIZE):
            for z in range(Config.CHUNK_SIZE):
                world_x = chunk_x + x
                world_z = chunk_z + z
                
                # Generate terrain height using Perlin noise
                height = self._get_height(world_x, world_z)
                
                # Fill blocks based on height
                self._generate_column(chunk, x, z, height)
        
        chunk.is_generated = True
        chunk.is_dirty = True
    
    def _get_height(self, x, z):
        """Get terrain height at world coordinates"""
        # Multiple octaves of Perlin noise for natural terrain
        value = noise.pnoise2(
            x * Config.TERRAIN_SCALE,
            z * Config.TERRAIN_SCALE,
            octaves=Config.TERRAIN_OCTAVES,
            persistence=Config.TERRAIN_PERSISTENCE,
            lacunarity=Config.TERRAIN_LACUNARITY,
            repeatx=1024,
            repeaty=1024,
            base=self.seed
        )
        
        # Convert to height
        height = int(Config.TERRAIN_BASE + value * Config.TERRAIN_HEIGHT_MULTIPLIER)
        return max(1, min(Config.CHUNK_HEIGHT - 1, height))
    
    def _generate_column(self, chunk, x, z, height):
        """Generate a vertical column of blocks"""
        # Bedrock at bottom
        chunk.set_block(x, 0, z, BLOCKS.get_by_name("bedrock").id)
        
        # Stone layer
        stone_height = max(1, height - 4)
        for y in range(1, stone_height):
            # Add ores randomly
            if np.random.random() < 0.01:
                chunk.set_block(x, y, z, self._get_ore_type(y))
            else:
                chunk.set_block(x, y, z, BLOCKS.get_by_name("stone").id)
        
        # Dirt layer
        dirt_height = max(stone_height, height - 1)
        for y in range(stone_height, dirt_height):
            chunk.set_block(x, y, z, BLOCKS.get_by_name("dirt").id)
        
        # Surface block
        if height >= Config.SEA_LEVEL:
            # Grass above sea level
            chunk.set_block(x, height, z, BLOCKS.get_by_name("grass").id)
            
            # Add snow on high peaks
            if height > Config.SEA_LEVEL + 30:
                chunk.set_block(x, height, z, BLOCKS.get_by_name("snow").id)
        else:
            # Sand near water
            if height > Config.SEA_LEVEL - 3:
                for y in range(max(1, height - 2), height + 1):
                    chunk.set_block(x, y, z, BLOCKS.get_by_name("sand").id)
            else:
                chunk.set_block(x, height, z, BLOCKS.get_by_name("dirt").id)
        
        # Fill water
        if height < Config.SEA_LEVEL:
            for y in range(height + 1, Config.SEA_LEVEL + 1):
                chunk.set_block(x, y, z, BLOCKS.get_by_name("water").id)
        
        # Add occasional trees
        if height >= Config.SEA_LEVEL and np.random.random() < 0.02:
            self._add_tree(chunk, x, height + 1, z)
    
    def _get_ore_type(self, y):
        """Get ore type based on depth"""
        if y < 16 and np.random.random() < 0.3:
            return BLOCKS.get_by_name("diamond_ore").id
        elif y < 32 and np.random.random() < 0.4:
            return BLOCKS.get_by_name("gold_ore").id
        elif y < 64 and np.random.random() < 0.5:
            return BLOCKS.get_by_name("iron_ore").id
        else:
            return BLOCKS.get_by_name("coal_ore").id
    
    def _add_tree(self, chunk, x, y, z):
        """Add a simple tree"""
        tree_height = 5
        
        # Trunk
        wood_id = BLOCKS.get_by_name("wood").id
        for dy in range(tree_height):
            if y + dy < Config.CHUNK_HEIGHT:
                chunk.set_block(x, y + dy, z, wood_id)
        
        # Leaves (simple sphere)
        leaves_id = BLOCKS.get_by_name("leaves").id
        top_y = y + tree_height
        
        if top_y < Config.CHUNK_HEIGHT - 2:
            for dx in range(-2, 3):
                for dz in range(-2, 3):
                    for dy in range(-2, 3):
                        if 0 <= x + dx < Config.CHUNK_SIZE and 0 <= z + dz < Config.CHUNK_SIZE:
                            if abs(dx) + abs(dy) + abs(dz) <= 3:
                                leaf_y = top_y + dy
                                if 0 <= leaf_y < Config.CHUNK_HEIGHT:
                                    if chunk.get_block(x + dx, leaf_y, z + dz) == 0:
                                        chunk.set_block(x + dx, leaf_y, z + dz, leaves_id)