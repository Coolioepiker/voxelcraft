import time
from pyglet.window import key, mouse
from world.world_manager import WorldManager
from player.controller import Player
from rendering.renderer import Renderer
from core.config import Config


class GameEngine:
    """Main game engine coordinating all systems"""
    
    def __init__(self, window):
        self.window = window
        
        # Initialize systems
        self.world = WorldManager(seed=12345)
        self.renderer = Renderer(window)
        self.player = Player(self.world, spawn_position=(0, 100, 0))
        
        # Game state
        self.mouse_captured = True
        self.window.set_exclusive_mouse(True)
        
        # Performance tracking
        self.fps = 60.0
        self.frame_times = []
        self.last_frame_time = time.time()
        
        # Block interaction cooldown
        self.last_block_action = 0
        
        print("Game engine initialized")
    
    def update(self, dt):
        """Update game state"""
        # Calculate FPS
        current_time = time.time()
        frame_time = current_time - self.last_frame_time
        self.last_frame_time = current_time
        
        self.frame_times.append(frame_time)
        if len(self.frame_times) > 60:
            self.frame_times.pop(0)
        
        if len(self.frame_times) > 0:
            avg_frame_time = sum(self.frame_times) / len(self.frame_times)
            self.fps = 1.0 / avg_frame_time if avg_frame_time > 0 else 60
        
        # Update player
        self.player.update(dt)
        
        # Update chunks around player
        self.world.update_chunks_around_player(
            self.player.position[0],
            self.player.position[2]
        )
        
        # Process chunk generation
        self.world.process_load_queue(max_chunks=2)
        
        # Update chunk meshes
        dirty_chunks = self.world.get_dirty_chunks()
        chunks_to_mesh = min(Config.MAX_CHUNKS_PER_FRAME, len(dirty_chunks))
        
        for i in range(chunks_to_mesh):
            if i < len(dirty_chunks):
                self.renderer.render_chunk(dirty_chunks[i])
    
    def on_draw(self):
        """Render frame"""
        # Set view matrix
        view_matrix = self.player.get_view_matrix()
        self.renderer.set_view_matrix(view_matrix)
        
        # Draw world
        self.renderer.draw(
            self.player.position,
            len(self.world.chunks),
            self.fps
        )
    
    def on_resize(self, width, height):
        """Handle window resize"""
        self.renderer.update_projection(width, height)
    
    def on_mouse_motion(self, x, y, dx, dy):
        """Handle mouse movement"""
        if self.mouse_captured:
            self.player.rotate(dx, dy)
    
    def on_mouse_press(self, x, y, button, modifiers):
        """Handle mouse clicks"""
        if not self.mouse_captured:
            return
        
        current_time = time.time()
        if current_time - self.last_block_action < Config.BLOCK_PLACE_COOLDOWN:
            return
        
        # Raycast to find target block
        eye_pos = [
            self.player.position[0],
            self.player.position[1] + Config.PLAYER_EYE_HEIGHT,
            self.player.position[2]
        ]
        look_dir = self.player.get_look_direction()
        
        hit_block, prev_block = self.world.raycast(eye_pos, look_dir, Config.REACH_DISTANCE)
        
        if hit_block:
            if button == mouse.LEFT:
                # Break block
                self.world.set_block(*hit_block, 0)
                self.last_block_action = current_time
                print(f"Broke block at {hit_block}")
            
            elif button == mouse.RIGHT and prev_block:
                # Place block
                # Check if placement would overlap player
                px, py, pz = prev_block
                player_blocks = [
                    (int(self.player.position[0]), int(self.player.position[1]), int(self.player.position[2])),
                    (int(self.player.position[0]), int(self.player.position[1]) + 1, int(self.player.position[2]))
                ]
                
                if prev_block not in player_blocks:
                    self.world.set_block(*prev_block, self.player.selected_block)
                    self.last_block_action = current_time
                    print(f"Placed block at {prev_block}")
    
    def on_key_press(self, symbol, modifiers):
        """Handle key press"""
        # Player movement
        self.player.on_key_press(symbol, modifiers)
        
        # Game controls
        if symbol == key.ESCAPE:
            # Toggle mouse capture
            self.mouse_captured = not self.mouse_captured
            self.window.set_exclusive_mouse(self.mouse_captured)
            print(f"Mouse {'captured' if self.mouse_captured else 'released'}")
        
        elif symbol == key.F3:
            # Toggle debug info
            self.renderer.show_debug = not self.renderer.show_debug
        
        elif symbol == key.F5:
            # Save world
            self.world.save_world(Config.WORLD_FILE)
        
        elif symbol == key.F9:
            # Load world
            if self.world.load_world(Config.WORLD_FILE):
                # Regenerate meshes
                for chunk in self.world.chunks.values():
                    chunk.is_dirty = True
    
    def on_key_release(self, symbol, modifiers):
        """Handle key release"""
        self.player.on_key_release(symbol, modifiers)