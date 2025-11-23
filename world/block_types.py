class BlockType:
    """Base class for block types"""
    
    def __init__(self, id, name, color, is_solid=True, is_transparent=False):
        self.id = id
        self.name = name
        self.color = color
        self.is_solid = is_solid
        self.is_transparent = is_transparent
    
    def __repr__(self):
        return f"BlockType({self.name})"


class BlockRegistry:
    """Registry for all block types"""
    
    def __init__(self):
        self.blocks = {}
        self._register_default_blocks()
    
    def _register_default_blocks(self):
        """Register all default block types"""
        # Air (empty space)
        self.register(BlockType(0, "air", (0, 0, 0, 0), is_solid=False, is_transparent=True))
        
        # Terrain blocks
        self.register(BlockType(1, "grass", (0.4, 0.8, 0.2)))
        self.register(BlockType(2, "dirt", (0.6, 0.4, 0.2)))
        self.register(BlockType(3, "stone", (0.5, 0.5, 0.5)))
        self.register(BlockType(4, "sand", (0.9, 0.9, 0.6)))
        self.register(BlockType(5, "water", (0.2, 0.4, 0.8), is_transparent=True))
        
        # Ores and resources
        self.register(BlockType(6, "coal_ore", (0.2, 0.2, 0.2)))
        self.register(BlockType(7, "iron_ore", (0.7, 0.5, 0.4)))
        self.register(BlockType(8, "gold_ore", (0.9, 0.8, 0.2)))
        self.register(BlockType(9, "diamond_ore", (0.3, 0.8, 0.9)))
        
        # Building blocks
        self.register(BlockType(10, "wood", (0.6, 0.4, 0.2)))
        self.register(BlockType(11, "planks", (0.8, 0.6, 0.3)))
        self.register(BlockType(12, "glass", (0.8, 0.9, 1.0), is_transparent=True))
        self.register(BlockType(13, "brick", (0.7, 0.3, 0.2)))
        self.register(BlockType(14, "cobblestone", (0.6, 0.6, 0.6)))
        
        # Natural blocks
        self.register(BlockType(15, "leaves", (0.2, 0.6, 0.2), is_transparent=True))
        self.register(BlockType(16, "snow", (0.95, 0.95, 1.0)))
        self.register(BlockType(17, "ice", (0.7, 0.85, 1.0), is_transparent=True))
        self.register(BlockType(18, "gravel", (0.5, 0.5, 0.5)))
        
        # Special blocks
        self.register(BlockType(19, "bedrock", (0.2, 0.2, 0.2)))
        self.register(BlockType(20, "lava", (1.0, 0.3, 0.0)))
    
    def register(self, block_type):
        """Register a new block type"""
        self.blocks[block_type.id] = block_type
    
    def get(self, block_id):
        """Get block type by ID"""
        return self.blocks.get(block_id, self.blocks[0])
    
    def get_by_name(self, name):
        """Get block type by name"""
        for block in self.blocks.values():
            if block.name == name:
                return block
        return self.blocks[0]
    
    def is_solid(self, block_id):
        """Check if a block is solid"""
        return self.get(block_id).is_solid
    
    def is_transparent(self, block_id):
        """Check if a block is transparent"""
        return self.get(block_id).is_transparent


# Global block registry instance
BLOCKS = BlockRegistry()