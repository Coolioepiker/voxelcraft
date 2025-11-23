import math
import numpy as np
from pyglet.window import key
from core.config import Config


class Player:
    """Player character with physics and controls"""
    
    def __init__(self, world_manager, spawn_position=(0, 80, 0)):
        self.world = world_manager
        
        # Position and orientation
        self.position = list(spawn_position)
        self.velocity = [0, 0, 0]
        self.rotation = [0, 0]  # [pitch, yaw] in degrees
        
        # Movement state
        self.keys = {
            'forward': False,
            'backward': False,
            'left': False,
            'right': False,
            'jump': False,
            'sprint': False
        }
        
        self.on_ground = False
        self.selected_block = 1  # Grass by default
        
    def update(self, dt):
        """Update player physics and movement"""
        # Apply gravity
        self.velocity[1] -= Config.GRAVITY * dt
        self.velocity[1] = max(-Config.TERMINAL_VELOCITY, self.velocity[1])
        
        # Calculate movement direction
        move_x, move_z = 0, 0
        
        forward = self._get_forward_vector()
        right = self._get_right_vector()
        
        if self.keys['forward']:
            move_x += forward[0]
            move_z += forward[2]
        if self.keys['backward']:
            move_x -= forward[0]
            move_z -= forward[2]
        if self.keys['right']:
            move_x += right[0]
            move_z += right[2]
        if self.keys['left']:
            move_x -= right[0]
            move_z -= right[2]
        
        # Normalize movement
        length = math.sqrt(move_x**2 + move_z**2)
        if length > 0:
            move_x /= length
            move_z /= length
        
        # Apply speed
        speed = Config.PLAYER_SPEED
        if self.keys['sprint']:
            speed *= Config.PLAYER_SPRINT_MULTIPLIER
        
        self.velocity[0] = move_x * speed
        self.velocity[2] = move_z * speed
        
        # Jump
        if self.keys['jump'] and self.on_ground:
            self.velocity[1] = Config.PLAYER_JUMP_SPEED
            self.on_ground = False
        
        # Apply friction when on ground
        if self.on_ground:
            self.velocity[0] *= Config.FRICTION
            self.velocity[2] *= Config.FRICTION
        
        # Move with collision detection
        self._move_with_collision(dt)
    
    def _move_with_collision(self, dt):
        """Move player with collision detection"""
        # Move X
        self.position[0] += self.velocity[0] * dt
        if self._check_collision():
            self.position[0] -= self.velocity[0] * dt
            self.velocity[0] = 0
        
        # Move Y
        self.position[1] += self.velocity[1] * dt
        if self._check_collision():
            if self.velocity[1] < 0:
                self.on_ground = True
            self.position[1] -= self.velocity[1] * dt
            self.velocity[1] = 0
        else:
            self.on_ground = False
        
        # Move Z
        self.position[2] += self.velocity[2] * dt
        if self._check_collision():
            self.position[2] -= self.velocity[2] * dt
            self.velocity[2] = 0
    
    def _check_collision(self):
        """Check if player collides with blocks"""
        # Check blocks around player
        x, y, z = self.position
        
        # Player bounding box
        for dx in [-Config.PLAYER_RADIUS, Config.PLAYER_RADIUS]:
            for dy in [0, Config.PLAYER_HEIGHT]:
                for dz in [-Config.PLAYER_RADIUS, Config.PLAYER_RADIUS]:
                    bx = int(np.floor(x + dx))
                    by = int(np.floor(y + dy))
                    bz = int(np.floor(z + dz))
                    
                    block = self.world.get_block(bx, by, bz)
                    if block != 0 and self.world.terrain_gen.seed:  # Solid block
                        from world.block_types import BLOCKS
                        if BLOCKS.is_solid(block):
                            return True
        
        return False
    
    def get_view_matrix(self):
        """Get view matrix for rendering"""
        # Eye position
        eye_x = self.position[0]
        eye_y = self.position[1] + Config.PLAYER_EYE_HEIGHT
        eye_z = self.position[2]
        
        # Calculate look direction
        pitch_rad = math.radians(self.rotation[0])
        yaw_rad = math.radians(self.rotation[1])
        
        look_x = math.cos(pitch_rad) * math.sin(yaw_rad)
        look_y = math.sin(pitch_rad)
        look_z = math.cos(pitch_rad) * math.cos(yaw_rad)
        
        # Look at point
        center_x = eye_x + look_x
        center_y = eye_y + look_y
        center_z = eye_z + look_z
        
        # Up vector
        up_x, up_y, up_z = 0, 1, 0
        
        return (eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z)
    
    def _get_forward_vector(self):
        """Get forward direction vector"""
        yaw_rad = math.radians(self.rotation[1])
        return [
            math.sin(yaw_rad),
            0,
            math.cos(yaw_rad)
        ]
    
    def _get_right_vector(self):
        """Get right direction vector"""
        yaw_rad = math.radians(self.rotation[1])
        return [
            math.cos(yaw_rad),
            0,
            -math.sin(yaw_rad)
        ]
    
    def get_look_direction(self):
        """Get look direction for raycasting"""
        pitch_rad = math.radians(self.rotation[0])
        yaw_rad = math.radians(self.rotation[1])
        
        return [
            math.cos(pitch_rad) * math.sin(yaw_rad),
            math.sin(pitch_rad),
            math.cos(pitch_rad) * math.cos(yaw_rad)
        ]
    
    def rotate(self, dx, dy):
        """Rotate camera"""
        self.rotation[1] += dx * Config.MOUSE_SENSITIVITY
        self.rotation[0] -= dy * Config.MOUSE_SENSITIVITY
        
        # Clamp pitch
        self.rotation[0] = max(-89, min(89, self.rotation[0]))
        
        # Wrap yaw
        self.rotation[1] %= 360
    
    def on_key_press(self, symbol, modifiers):
        """Handle key press"""
        if symbol == key.W:
            self.keys['forward'] = True
        elif symbol == key.S:
            self.keys['backward'] = True
        elif symbol == key.A:
            self.keys['left'] = True
        elif symbol == key.D:
            self.keys['right'] = True
        elif symbol == key.SPACE:
            self.keys['jump'] = True
        elif symbol == key.LSHIFT:
            self.keys['sprint'] = True
        elif key._1 <= symbol <= key._9:
            self.selected_block = symbol - key._1 + 1
    
    def on_key_release(self, symbol, modifiers):
        """Handle key release"""
        if symbol == key.W:
            self.keys['forward'] = False
        elif symbol == key.S:
            self.keys['backward'] = False
        elif symbol == key.A:
            self.keys['left'] = False
        elif symbol == key.D:
            self.keys['right'] = False
        elif symbol == key.SPACE:
            self.keys['jump'] = False
        elif symbol == key.LSHIFT:
            self.keys['sprint'] = False