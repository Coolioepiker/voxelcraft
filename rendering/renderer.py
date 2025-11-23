import pyglet
from pyglet import gl
from pyglet.graphics import Batch
from rendering.mesh import ChunkMesh
from core.config import Config
import math


class Renderer:
    """Handles all rendering"""
    
    def __init__(self, window):
        self.window = window
        self.batch = Batch()
        self.chunk_meshes = {}
        
        # Setup OpenGL
        self._setup_opengl()
        
        # FPS display
        self.fps_display = pyglet.text.Label(
            '',
            font_name='Arial',
            font_size=14,
            x=10, y=window.height - 10,
            anchor_x='left', anchor_y='top',
            color=(255, 255, 255, 255)
        )
        
        # Debug info
        self.debug_label = pyglet.text.Label(
            '',
            font_name='Arial',
            font_size=12,
            x=10, y=window.height - 40,
            anchor_x='left', anchor_y='top',
            color=(255, 255, 255, 255)
        )
        
        self.show_debug = False
    
    def _setup_opengl(self):
        """Setup OpenGL state"""
        gl.glEnable(gl.GL_DEPTH_TEST)
        gl.glEnable(gl.GL_CULL_FACE)
        gl.glCullFace(gl.GL_BACK)
        gl.glClearColor(0.5, 0.7, 1.0, 1.0)  # Sky blue
        
        # Enable blending for transparency
        gl.glEnable(gl.GL_BLEND)
        gl.glBlendFunc(gl.GL_SRC_ALPHA, gl.GL_ONE_MINUS_SRC_ALPHA)
    
    def update_projection(self, width, height):
        """Update projection matrix on window resize"""
        gl.glViewport(0, 0, width, height)
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        
        aspect = width / height
        self._gluPerspective(Config.FOV, aspect, Config.NEAR_PLANE, Config.FAR_PLANE)
        
        gl.glMatrixMode(gl.GL_MODELVIEW)
        
        # Update label positions
        self.fps_display.y = height - 10
        self.debug_label.y = height - 40
    
    def _gluPerspective(self, fov, aspect, near, far):
        """Implement gluPerspective"""
        fov_rad = math.radians(fov)
        f = 1.0 / math.tan(fov_rad / 2.0)
        
        matrix = [
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (far + near) / (near - far), -1,
            0, 0, (2 * far * near) / (near - far), 0
        ]
        
        gl.glMultMatrixf((gl.GLfloat * 16)(*matrix))
    
    def set_view_matrix(self, view_matrix):
        """Set camera view matrix"""
        eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z = view_matrix
        
        gl.glMatrixMode(gl.GL_MODELVIEW)
        gl.glLoadIdentity()
        
        # Implement gluLookAt
        f_x = center_x - eye_x
        f_y = center_y - eye_y
        f_z = center_z - eye_z
        
        # Normalize forward
        f_len = math.sqrt(f_x**2 + f_y**2 + f_z**2)
        f_x, f_y, f_z = f_x/f_len, f_y/f_len, f_z/f_len
        
        # Right = forward x up
        r_x = f_y * up_z - f_z * up_y
        r_y = f_z * up_x - f_x * up_z
        r_z = f_x * up_y - f_y * up_x
        
        # Normalize right
        r_len = math.sqrt(r_x**2 + r_y**2 + r_z**2)
        r_x, r_y, r_z = r_x/r_len, r_y/r_len, r_z/r_len
        
        # Up = right x forward
        u_x = r_y * f_z - r_z * f_y
        u_y = r_z * f_x - r_x * f_z
        u_z = r_x * f_y - r_y * f_x
        
        matrix = [
            r_x, u_x, -f_x, 0,
            r_y, u_y, -f_y, 0,
            r_z, u_z, -f_z, 0,
            0, 0, 0, 1
        ]
        
        gl.glMultMatrixf((gl.GLfloat * 16)(*matrix))
        gl.glTranslatef(-eye_x, -eye_y, -eye_z)
    
    def render_chunk(self, chunk):
        """Add or update chunk mesh"""
        key = (chunk.x, chunk.z)
        
        if chunk.is_dirty:
            # Rebuild mesh
            if key in self.chunk_meshes:
                self.chunk_meshes[key].delete()
            
            mesh = ChunkMesh(chunk)
            mesh.build(self.batch)
            self.chunk_meshes[key] = mesh
            
            chunk.is_dirty = False
    
    def unload_chunk(self, chunk_x, chunk_z):
        """Remove chunk mesh"""
        key = (chunk_x, chunk_z)
        if key in self.chunk_meshes:
            self.chunk_meshes[key].delete()
            del self.chunk_meshes[key]
    
    def draw(self, player_pos, chunks_loaded, fps):
        """Main draw call"""
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
        
        # Draw all chunks
        self.batch.draw()
        
        # Draw UI
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glPushMatrix()
        gl.glLoadIdentity()
        gl.glOrtho(0, self.window.width, 0, self.window.height, -1, 1)
        gl.glMatrixMode(gl.GL_MODELVIEW)
        gl.glPushMatrix()
        gl.glLoadIdentity()
        
        # Disable depth test for UI
        gl.glDisable(gl.GL_DEPTH_TEST)
        
        # Draw crosshair
        self._draw_crosshair()
        
        # Draw FPS
        if Config.SHOW_FPS:
            self.fps_display.text = f'FPS: {fps:.1f}'
            self.fps_display.draw()
        
        # Draw debug info
        if self.show_debug:
            self.debug_label.text = (
                f'Position: ({player_pos[0]:.1f}, {player_pos[1]:.1f}, {player_pos[2]:.1f})\n'
                f'Chunks: {chunks_loaded}\n'
                f'Meshes: {len(self.chunk_meshes)}'
            )
            self.debug_label.draw()
        
        # Restore matrices
        gl.glEnable(gl.GL_DEPTH_TEST)
        gl.glPopMatrix()
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glPopMatrix()
        gl.glMatrixMode(gl.GL_MODELVIEW)
    
    def _draw_crosshair(self):
        """Draw simple crosshair"""
        cx = self.window.width // 2
        cy = self.window.height // 2
        size = 10
        
        gl.glColor3f(1, 1, 1)
        gl.glBegin(gl.GL_LINES)
        
        # Horizontal line
        gl.glVertex2f(cx - size, cy)
        gl.glVertex2f(cx + size, cy)
        
        # Vertical line
        gl.glVertex2f(cx, cy - size)
        gl.glVertex2f(cx, cy + size)
        
        gl.glEnd()
    
    def cleanup(self):
        """Clean up resources"""
        for mesh in self.chunk_meshes.values():
            mesh.delete()
        self.chunk_meshes.clear()