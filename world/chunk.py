import numpy as np
from core.config import Config
from world.block_types import BLOCKS


class Chunk:
    """Represents a 16x256x16 chunk of blocks"""
    
    def __init__(self, x, z):
        self.x = x  # Chunk X coordinate
        self.z = z  # Chunk Z coordinate
        self.blocks = np.zeros(
            (Config.CHUNK_SIZE, Config.CHUNK_HEIGHT, Config.CHUNK_SIZE),
            dtype=np.uint8
        )
        self.is_dirty = True  # Needs mesh regeneration
        self.is_generated = False
        self.mesh_data = None
        self.neighbors = {
            'north': None,
            'south': None,
            'east': None,
            'west': None
        }
    
    def get_block(self, x, y, z):
        """Get block at local coordinates"""
        if 0 <= x < Config.CHUNK_SIZE and 0 <= y < Config.CHUNK_HEIGHT and 0 <= z < Config.CHUNK_SIZE:
            return self.blocks[x, y, z]
        return 0  # Air for out of bounds
    
    def set_block(self, x, y, z, block_id):
        """Set block at local coordinates"""
        if 0 <= x < Config.CHUNK_SIZE and 0 <= y < Config.CHUNK_HEIGHT and 0 <= z < Config.CHUNK_SIZE:
            if self.blocks[x, y, z] != block_id:
                self.blocks[x, y, z] = block_id
                self.is_dirty = True
                
                # Mark neighboring chunks dirty if block is on edge
                if x == 0 and self.neighbors['west']:
                    self.neighbors['west'].is_dirty = True
                elif x == Config.CHUNK_SIZE - 1 and self.neighbors['east']:
                    self.neighbors['east'].is_dirty = True
                
                if z == 0 and self.neighbors['north']:
                    self.neighbors['north'].is_dirty = True
                elif z == Config.CHUNK_SIZE - 1 and self.neighbors['south']:
                    self.neighbors['south'].is_dirty = True
    
    def is_block_visible(self, x, y, z):
        """Check if a block face should be rendered"""
        block_id = self.get_block(x, y, z)
        if block_id == 0:  # Air blocks not visible
            return False
        
        # Check all 6 faces
        faces_visible = 0
        
        # Check each neighbor
        for dx, dy, dz in [(1,0,0), (-1,0,0), (0,1,0), (0,-1,0), (0,0,1), (0,0,-1)]:
            nx, ny, nz = x + dx, y + dy, z + dz
            
            # Get neighbor block (might be from adjacent chunk)
            if 0 <= nx < Config.CHUNK_SIZE and 0 <= ny < Config.CHUNK_HEIGHT and 0 <= nz < Config.CHUNK_SIZE:
                neighbor_id = self.blocks[nx, ny, nz]
            else:
                # Edge of chunk - check neighbor chunk
                neighbor_id = self._get_neighbor_block(nx, ny, nz)
            
            # Face is visible if neighbor is air or transparent
            if neighbor_id == 0 or BLOCKS.is_transparent(neighbor_id):
                faces_visible += 1
        
        return faces_visible > 0
    
    def _get_neighbor_block(self, x, y, z):
        """Get block from neighboring chunk if on edge"""
        if y < 0 or y >= Config.CHUNK_HEIGHT:
            return 0  # Air above/below world
        
        chunk = None
        local_x, local_z = x, z
        
        if x < 0:
            chunk = self.neighbors['west']
            local_x = Config.CHUNK_SIZE - 1
        elif x >= Config.CHUNK_SIZE:
            chunk = self.neighbors['east']
            local_x = 0
        
        if z < 0:
            chunk = self.neighbors['north']
            local_z = Config.CHUNK_SIZE - 1
        elif z >= Config.CHUNK_SIZE:
            chunk = self.neighbors['south']
            local_z = 0
        
        if chunk and chunk.is_generated:
            return chunk.get_block(local_x, y, local_z)
        
        return 0  # Air for unloaded chunks
    
    def get_world_position(self):
        """Get world position of chunk origin"""
        return (
            self.x * Config.CHUNK_SIZE,
            0,
            self.z * Config.CHUNK_SIZE
        )
    
    def __repr__(self):
        return f"Chunk({self.x}, {self.z})"