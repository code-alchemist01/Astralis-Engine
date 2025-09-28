from PIL import Image, ImageDraw
import os

def create_checkerboard_png():
    """Create a checkerboard pattern PNG"""
    size = 256
    tile_size = 32
    img = Image.new('RGB', (size, size), 'white')
    draw = ImageDraw.Draw(img)
    
    for y in range(0, size, tile_size):
        for x in range(0, size, tile_size):
            # Checkerboard pattern
            if (x // tile_size + y // tile_size) % 2 == 1:
                draw.rectangle([x, y, x + tile_size, y + tile_size], fill='#cccccc')
    
    img.save('assets/textures/checkerboard.png')
    print("Created checkerboard.png")

def create_brick_png():
    """Create a brick pattern PNG"""
    size = 256
    brick_width = 30
    brick_height = 14
    mortar = 2
    
    img = Image.new('RGB', (size, size), '#654321')  # Mortar color
    draw = ImageDraw.Draw(img)
    
    y = 0
    row = 0
    while y < size:
        x_offset = (brick_width // 2) if row % 2 == 1 else 0
        x = x_offset
        
        while x < size + brick_width:
            color = '#8B4513' if row % 2 == 0 else '#A0522D'
            draw.rectangle([x, y, x + brick_width, y + brick_height], fill=color)
            x += brick_width + mortar
        
        y += brick_height + mortar
        row += 1
    
    img.save('assets/textures/brick.png')
    print("Created brick.png")

def create_wood_png():
    """Create a wood grain pattern PNG"""
    size = 256
    img = Image.new('RGB', (size, size), '#8B4513')
    draw = ImageDraw.Draw(img)
    
    # Create wood grain lines
    for i in range(0, size, 8):
        color = '#A0522D' if i % 16 == 0 else '#654321'
        draw.line([(0, i), (size, i)], fill=color, width=2)
    
    # Add some vertical variation
    for i in range(0, size, 32):
        for j in range(0, size, 4):
            if j % 8 == 0:
                draw.point((i + (j % 16), j), fill='#D2691E')
    
    img.save('assets/textures/wood.png')
    print("Created wood.png")

if __name__ == "__main__":
    os.makedirs('assets/textures', exist_ok=True)
    create_checkerboard_png()
    create_brick_png()
    create_wood_png()
    print("All textures created successfully!")