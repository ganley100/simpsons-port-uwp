# Name:        make_logos.py
# Description: the UWP package's tiles and splash screen (ports/uwp/Assets): a pink donut on the show's yellow, drawn
#              here with no library (zlib's PNG alone), so the package needs no picture of anyone's. With --picture, the
#              player's own picture instead (local/icon_uwp.png: never in the repository), filling each tile, what
#              is past its sides cut, kept from the top (Pillow's)
# Usage:       python3 ports/uwp/make_logos.py [--picture PICTURE] [FOLDER]    (FOLDER: ports/uwp/Assets by default)
import math, struct, sys, zlib, os

YELLOW = (255, 217, 15)
PINK = (240, 98, 170)
DOUGH = (214, 150, 72)
HOLE = YELLOW
SPRINKLES = [(255, 255, 255), (80, 190, 255), (120, 220, 90), (255, 150, 40)]

def png(path, width, height, pixel):
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        for x in range(width):
            rows.extend(pixel(x, y))
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    data = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
    data += chunk(b"IDAT", zlib.compress(bytes(rows), 9)) + chunk(b"IEND", b"")
    with open(path, "wb") as out:
        out.write(data)

def donut(width, height, size):
    cx, cy = width / 2, height / 2
    outer, inner = size * 0.42, size * 0.16
    icing = size * 0.36
    def pixel(x, y):
        dx, dy = x + 0.5 - cx, y + 0.5 - cy
        r = math.hypot(dx, dy)
        if r > outer or r < inner:
            return YELLOW
        if r > icing or r < inner + size * 0.04:
            return DOUGH
        # (a few sprinkles on the icing, by the angle and the radius)
        angle = math.atan2(dy, dx)
        slot = int((angle + math.pi) / (2 * math.pi) * 14)
        middle = (slot + 0.5) / 14 * 2 * math.pi - math.pi
        ring = inner + (icing - inner) * (0.35 if slot % 2 else 0.7)
        sx, sy = cx + math.cos(middle) * ring, cy + math.sin(middle) * ring
        if math.hypot(x + 0.5 - sx, y + 0.5 - sy) < max(1.0, size * 0.025):
            return SPRINKLES[slot % len(SPRINKLES)]
        return PINK
    return pixel

def fitted(picture, width, height):
    # the tile filled with the picture, as small as covers it, what is past its sides cut (centred across, kept from
    # the top down: a poster's title stays)
    from PIL import Image
    scale = max(width / picture.width, height / picture.height)
    size = (max(width, round(picture.width * scale)), max(height, round(picture.height * scale)))
    left = (size[0] - width) // 2
    return picture.resize(size, Image.LANCZOS).crop((left, 0, left + width, height))

arguments = sys.argv[1:]
picture = None
if len(arguments) >= 2 and arguments[0] == "--picture":
    from PIL import Image
    picture = Image.open(arguments[1]).convert("RGB")
    arguments = arguments[2:]
folder = arguments[0] if arguments else os.path.join(os.path.dirname(__file__), "Assets")
os.makedirs(folder, exist_ok=True)
for name, width, height in [("Square44x44Logo", 44, 44), ("Square150x150Logo", 150, 150), ("StoreLogo", 50, 50),
                            ("Wide310x150Logo", 310, 150), ("SplashScreen", 620, 300)]:
    path = os.path.join(folder, name + ".png")
    if picture:
        fitted(picture, width, height).save(path)
    else:
        png(path, width, height, donut(width, height, min(width, height)))
    print(name, width, height)
if picture:
    # (the square, at the picture's own size: what the tiles were made of)
    side = min(picture.width, picture.height)
    fitted(picture, side, side).save(os.path.join(folder, "square.png"))
