import numpy as np
from pyglet.graphics import Batch
from pyglet import gl
from world.block_types import BLOCKS
from core.config import Config


class ChunkMesh:
    """Mesh data for a chunk"""
    
    def __init__(self, chunk):
        self.chunk = chunk
        self.vertex_list = None
        self.vertex_count = 0
    
    def build(self, batch):
        """Build optimized mesh for chunk"""
        vertices = []
        colors = []
        
        # Iterate through all blocks
        for x in range(Config.CHUNK_SIZE):
            for y in range(Config.CHUNK_HEIGHT):
                for z in range(Config.CHUNK_SIZE):
                    block_id = self.chunk.get_block(x, y, z)
                    
                    if block_id == 0:  # Skip air
                        continue
                    
                    block_type = BLOCKS.get(block_id)
                    
                    # World position
                    wx = self.chunk.x * Config.CHUNK_SIZE + x
                    wy = y
                    wz = self.chunk.z * Config.CHUNK_SIZE + z
                    
                    # Add visible faces
                    self._add_visible_faces(
                        x, y, z, wx, wy, wz,
                        block_type, vertices, colors
                    )
        
        # Create vertex list
        if self.vertex_list:
            self.vertex_list.delete()
        
        if len(vertices) > 0:
            self.vertex_count = len(vertices) // 3
            self.vertex_list = batch.add(
                self.vertex_count,
                gl.GL_TRIANGLES,
                None,
                ('v3f/static', vertices),
                ('c3f/static', colors)
            )
        else:
            self.vertex_list = None
            self.vertex_count = 0
    
    def _add_visible_faces(self, x, y, z, wx, wy, wz, block_type, vertices, colors):
        """Add faces for visible sides of a block"""
        # Check each face
        faces = [
            # (dx, dy, dz, face_vertices)
            (0, 1, 0, self._top_face(wx, wy, wz)),      # Top
            (0, -1, 0, self._bottom_face(wx, wy, wz)),  # Bottom
            (1, 0, 0, self._east_face(wx, wy, wz)),     # East
            (-1, 0, 0, self._west_face(wx, wy, wz)),    # West
            (0, 0, 1, self._south_face(wx, wy, wz)),    # South
            (0, 0, -1, self._north_face(wx, wy, wz))    # North
        ]
        
        for dx, dy, dz, face_verts in faces:
            neighbor = self.chunk.get_block(x + dx, y + dy, z + dz)
            
            # Show face if neighbor is air or transparent
            if neighbor == 0 or BLOCKS.is_transparent(neighbor):
                # Add vertices
                vertices.extend(face_verts)
                
                # Add colors with simple lighting
                brightness = self._get_face_brightness(dx, dy, dz)
                color = [c * brightness for c in block_type.color[:3]]
                
                # 6 vertices per face (2 triangles)
                for _ in range(6):
                    colors.extend(color)
    
    def _get_face_brightness(self, dx, dy, dz):
        """Get brightness multiplier for face based on direction"""
        if dy == 1:  # Top
            return 1.0
        elif dy == -1:  # Bottom
            return 0.5
        elif dx != 0:  # East/West
            return 0.8
        else:  # North/South
            return 0.7
    
    # Face vertex definitions (2 triangles per face)
    def _top_face(self, x, y, z):
        return [
            x, y+1, z,     x+1, y+1, z,     x+1, y+1, z+1,
            x, y+1, z,     x+1, y+1, z+1,   x, y+1, z+1
        ]
    
    def _bottom_face(self, x, y, z):
        return [
            x, y, z,       x+1, y, z+1,     x+1, y, z,
            x, y, z,       x, y, z+1,       x+1, y, z+1
        ]
    
    def _east_face(self, x, y, z):
        return [
            x+1, y, z,     x+1, y+1, z,     x+1, y+1, z+1,
            x+1, y, z,     x+1, y+1, z+1,   x+1, y, z+1
        ]
    
    def _west_face(self, x, y, z):
        return [
            x, y, z,       x, y+1, z+1,     x, y+1, z,
            x, y, z,       x, y, z+1,       x, y+1, z+1
        ]
    
    def _south_face(self, x, y, z):
        return [
            x, y, z+1,     x+1, y+1, z+1,   x+1, y, z+1,
            x, y, z+1,     x, y+1, z+1,     x+1, y+1, z+1
        ]
    
    def _north_face(self, x, y, z):
        return [
            x, y, z,       x+1, y, z,       x+1, y+1, z,
            x, y, z,       x+1, y+1, z,     x, y+1, z
        ]
    
    def delete(self):
        """Clean up mesh"""
        if self.vertex_list:
            self.vertex_list.delete()
            self.vertex_list = None