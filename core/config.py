class Config:
    """Central configuration for the game"""
    
    # Window settings
    WINDOW_WIDTH = 1280
    WINDOW_HEIGHT = 720
    FOV = 70
    NEAR_PLANE = 0.1
    FAR_PLANE = 500.0
    
    # World generation
    CHUNK_SIZE = 16  # Blocks per chunk dimension
    CHUNK_HEIGHT = 256  # Maximum world height
    RENDER_DISTANCE = 8  # Chunks
    SEA_LEVEL = 64
    
    # Terrain generation
    TERRAIN_SCALE = 0.02
    TERRAIN_OCTAVES = 6
    TERRAIN_PERSISTENCE = 0.5
    TERRAIN_LACUNARITY = 2.0
    TERRAIN_HEIGHT_MULTIPLIER = 40
    TERRAIN_BASE = 50
    
    # Player settings
    PLAYER_HEIGHT = 1.8
    PLAYER_EYE_HEIGHT = 1.62
    PLAYER_RADIUS = 0.3
    PLAYER_SPEED = 4.3
    PLAYER_SPRINT_MULTIPLIER = 1.3
    PLAYER_JUMP_SPEED = 8.0
    GRAVITY = 32.0
    MOUSE_SENSITIVITY = 0.15
    
    # Physics
    TERMINAL_VELOCITY = 50.0
    FRICTION = 0.9
    
    # Block interaction
    REACH_DISTANCE = 5.0
    BLOCK_PLACE_COOLDOWN = 0.25
    
    # Rendering
    VSYNC = True
    SHOW_FPS = True
    AMBIENT_LIGHT = 0.4
    
    # File paths
    SAVES_DIR = "saves"
    WORLD_FILE = "world.dat"
    
    # Performance
    MAX_CHUNKS_PER_FRAME = 4  # Max chunks to mesh per frame
    CHUNK_UPDATE_DISTANCE = 2  # Priority distance for chunk updates