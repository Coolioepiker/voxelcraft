import os
import msgpack
import numpy as np
from collections import deque
from core.config import Config
from world.chunk import Chunk
from world.terrain_gen import TerrainGenerator
from world.block_types import BLOCKS


class WorldManager:
    """Manages chunks and world state"""
    
    def __init__(self, seed=None):
        self.chunks = {}  # (x, z) -> Chunk
        self.terrain_gen = TerrainGenerator(seed)
        self.load_queue = deque()
        self.mesh_queue = deque()
    
    def get_chunk(self, chunk_x, chunk_z):
        """Get or create chunk at chunk coordinates"""
        key = (chunk_x, chunk_z)
        if key not in self.chunks:
            chunk = Chunk(chunk_x, chunk_z)
            self.chunks[key] = chunk
            self.load_queue.append(chunk)
        return self.chunks[key]
    
    def get_block(self, x, y, z):
        """Get block at world coordinates"""
        if y < 0 or y >= Config.CHUNK_HEIGHT:
            return 0
        
        chunk_x = x // Config.CHUNK_SIZE
        chunk_z = z // Config.CHUNK_SIZE
        local_x = x % Config.CHUNK_SIZE
        local_z = z % Config.CHUNK_SIZE
        
        chunk = self.chunks.get((chunk_x, chunk_z))
        if chunk and chunk.is_generated:
            return chunk.get_block(local_x, y, local_z)
        return 0
    
    def set_block(self, x, y, z, block_id):
        """Set block at world coordinates"""
        if y < 0 or y >= Config.CHUNK_HEIGHT:
            return False
        
        chunk_x = x // Config.CHUNK_SIZE
        chunk_z = z // Config.CHUNK_SIZE
        local_x = x % Config.CHUNK_SIZE
        local_z = z % Config.CHUNK_SIZE
        
        chunk = self.get_chunk(chunk_x, chunk_z)
        if chunk.is_generated:
            chunk.set_block(local_x, y, local_z, block_id)
            return True
        return False
    
    def update_chunks_around_player(self, player_x, player_z):
        """Load/unload chunks based on player position"""
        player_chunk_x = int(player_x // Config.CHUNK_SIZE)
        player_chunk_z = int(player_z // Config.CHUNK_SIZE)
        
        # Load chunks in render distance
        for dx in range(-Config.RENDER_DISTANCE, Config.RENDER_DISTANCE + 1):
            for dz in range(-Config.RENDER_DISTANCE, Config.RENDER_DISTANCE + 1):
                chunk_x = player_chunk_x + dx
                chunk_z = player_chunk_z + dz
                self.get_chunk(chunk_x, chunk_z)
        
        # Unload distant chunks
        to_unload = []
        for (cx, cz), chunk in self.chunks.items():
            dist = max(abs(cx - player_chunk_x), abs(cz - player_chunk_z))
            if dist > Config.RENDER_DISTANCE + 2:
                to_unload.append((cx, cz))
        
        for key in to_unload:
            del self.chunks[key]
    
    def process_load_queue(self, max_chunks=1):
        """Generate terrain for queued chunks"""
        processed = 0
        while self.load_queue and processed < max_chunks:
            chunk = self.load_queue.popleft()
            if not chunk.is_generated:
                self.terrain_gen.generate_chunk(chunk)
                self._setup_chunk_neighbors(chunk)
                processed += 1
    
    def _setup_chunk_neighbors(self, chunk):
        """Set up neighbor references for a chunk"""
        cx, cz = chunk.x, chunk.z
        
        chunk.neighbors['north'] = self.chunks.get((cx, cz - 1))
        chunk.neighbors['south'] = self.chunks.get((cx, cz + 1))
        chunk.neighbors['east'] = self.chunks.get((cx + 1, cz))
        chunk.neighbors['west'] = self.chunks.get((cx - 1, cz))
        
        # Update neighbors' references
        if chunk.neighbors['north']:
            chunk.neighbors['north'].neighbors['south'] = chunk
        if chunk.neighbors['south']:
            chunk.neighbors['south'].neighbors['north'] = chunk
        if chunk.neighbors['east']:
            chunk.neighbors['east'].neighbors['west'] = chunk
        if chunk.neighbors['west']:
            chunk.neighbors['west'].neighbors['east'] = chunk
    
    def get_dirty_chunks(self):
        """Get list of chunks needing mesh regeneration"""
        return [chunk for chunk in self.chunks.values() 
                if chunk.is_dirty and chunk.is_generated]
    
    def save_world(self, filename):
        """Save world to file"""
        os.makedirs(Config.SAVES_DIR, exist_ok=True)
        filepath = os.path.join(Config.SAVES_DIR, filename)
        
        world_data = {
            'seed': self.terrain_gen.seed,
            'chunks': {}
        }
        
        for (cx, cz), chunk in self.chunks.items():
            if chunk.is_generated:
                # Compress chunk data
                chunk_data = {
                    'blocks': chunk.blocks.tobytes(),
                    'shape': chunk.blocks.shape
                }
                world_data['chunks'][f"{cx},{cz}"] = chunk_data
        
        with open(filepath, 'wb') as f:
            msgpack.pack(world_data, f)
        
        print(f"World saved: {len(world_data['chunks'])} chunks")
    
    def load_world(self, filename):
        """Load world from file"""
        filepath = os.path.join(Config.SAVES_DIR, filename)
        
        if not os.path.exists(filepath):
            print(f"Save file not found: {filepath}")
            return False
        
        try:
            with open(filepath, 'rb') as f:
                world_data = msgpack.unpack(f, raw=False)
            
            self.terrain_gen.seed = world_data['seed']
            self.chunks.clear()
            
            for key, chunk_data in world_data['chunks'].items():
                cx, cz = map(int, key.split(','))
                chunk = Chunk(cx, cz)
                
                # Decompress chunk data
                chunk.blocks = np.frombuffer(
                    chunk_data['blocks'],
                    dtype=np.uint8
                ).reshape(chunk_data['shape'])
                
                chunk.is_generated = True
                chunk.is_dirty = True
                self.chunks[(cx, cz)] = chunk
            
            # Setup neighbors
            for chunk in self.chunks.values():
                self._setup_chunk_neighbors(chunk)
            
            print(f"World loaded: {len(self.chunks)} chunks")
            return True
            
        except Exception as e:
            print(f"Error loading world: {e}")
            return False
    
    def raycast(self, origin, direction, max_distance=5.0):
        """Raycast to find block intersection"""
        step = 0.1
        distance = 0
        
        while distance < max_distance:
            x = origin[0] + direction[0] * distance
            y = origin[1] + direction[1] * distance
            z = origin[2] + direction[2] * distance
            
            bx, by, bz = int(np.floor(x)), int(np.floor(y)), int(np.floor(z))
            
            block = self.get_block(bx, by, bz)
            if block != 0 and BLOCKS.is_solid(block):
                # Find the face that was hit
                prev_distance = distance - step
                px = origin[0] + direction[0] * prev_distance
                py = origin[1] + direction[1] * prev_distance
                pz = origin[2] + direction[2] * prev_distance
                
                return (bx, by, bz), (int(np.floor(px)), int(np.floor(py)), int(np.floor(pz)))
            
            distance += step
        
        return None, None