"""
VoxelCraft - A Minecraft-like Voxel Game
Main entry point for the game
"""

import sys
import pyglet
from core.engine import GameEngine
from core.config import Config

def main():
    """Initialize and run the game"""
    try:
        # Create game window
        config = Config()
        window = pyglet.window.Window(
            width=config.WINDOW_WIDTH,
            height=config.WINDOW_HEIGHT,
            caption="VoxelCraft",
            resizable=True,
            vsync=True
        )
        
        # Initialize game engine
        engine = GameEngine(window)
        
        # Set up event handlers
        @window.event
        def on_draw():
            engine.on_draw()
        
        @window.event
        def on_resize(width, height):
            engine.on_resize(width, height)
        
        @window.event
        def on_mouse_motion(x, y, dx, dy):
            engine.on_mouse_motion(x, y, dx, dy)
        
        @window.event
        def on_mouse_press(x, y, button, modifiers):
            engine.on_mouse_press(x, y, button, modifiers)
        
        @window.event
        def on_key_press(symbol, modifiers):
            engine.on_key_press(symbol, modifiers)
        
        @window.event
        def on_key_release(symbol, modifiers):
            engine.on_key_release(symbol, modifiers)
        
        # Schedule update loop
        pyglet.clock.schedule_interval(engine.update, 1.0 / 60.0)
        
        # Run the application
        print("VoxelCraft starting...")
        print("Controls:")
        print("  WASD - Move")
        print("  Space - Jump")
        print("  Shift - Sprint")
        print("  Mouse - Look around")
        print("  Left Click - Break block")
        print("  Right Click - Place block")
        print("  1-9 - Select block type")
        print("  ESC - Toggle mouse capture")
        print("  F3 - Toggle debug info")
        print("  F5 - Save world")
        print("  F9 - Load world")
        
        pyglet.app.run()
        
    except Exception as e:
        print(f"Error starting game: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()