#!/usr/bin/env python3
"""Generate pixel-art sprites and artistic backgrounds for Varia Depths."""
from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1] / "assets"
ROOT.mkdir(parents=True, exist_ok=True)

BG_DEEP = (8, 10, 22, 255)
BG_MID = (18, 40, 52, 255)
BG_MIST = (40, 78, 88, 255)
TEAL = (56, 140, 148, 255)
TEAL_DK = (28, 72, 80, 255)
METAL = (70, 78, 92, 255)
METAL_HI = (120, 130, 148, 255)
SUIT = (220, 170, 70, 255)
SUIT_DK = (160, 110, 40, 255)
VISOR = (80, 220, 255, 255)
MAGENTA = (200, 60, 120, 255)
RED = (200, 70, 70, 255)
WHITE = (240, 240, 245, 255)
GOLD = (255, 210, 90, 255)
GREEN = (80, 200, 120, 255)
TRANSPARENT = (0, 0, 0, 0)


def save(img: Image.Image, name: str) -> None:
    path = ROOT / name
    img.save(path)
    print(f"wrote {path}")


def gen_player() -> None:
    frames = 5
    tw, th = 32, 32
    img = Image.new("RGBA", (tw * frames, th), TRANSPARENT)
    d = ImageDraw.Draw(img)

    def suit_body(ox: int, lean: int = 0, arms_up: bool = False) -> None:
        d.rectangle([ox + 10, 26, ox + 14, 30], fill=METAL)
        d.rectangle([ox + 17, 26, ox + 21, 30], fill=METAL)
        d.rectangle([ox + 11, 20, ox + 14, 26], fill=SUIT_DK)
        d.rectangle([ox + 17, 20, ox + 20, 26], fill=SUIT_DK)
        d.rectangle([ox + 10 + lean, 10, ox + 21 + lean, 20], fill=SUIT)
        d.rectangle([ox + 12 + lean, 12, ox + 19 + lean, 18], fill=SUIT_DK)
        d.ellipse([ox + 11 + lean, 4, ox + 20 + lean, 13], fill=SUIT)
        d.rectangle([ox + 13 + lean, 7, ox + 18 + lean, 10], fill=VISOR)
        ay = 12 if not arms_up else 8
        d.rectangle([ox + 20 + lean, ay, ox + 26 + lean, ay + 4], fill=METAL_HI)
        d.rectangle([ox + 25 + lean, ay + 1, ox + 28 + lean, ay + 3], fill=TEAL)

    suit_body(0, 0)
    suit_body(32, 1)
    d.rectangle([32 + 10, 24, 32 + 13, 30], fill=METAL)
    d.rectangle([32 + 18, 22, 32 + 21, 28], fill=METAL)
    suit_body(64, -1)
    d.rectangle([64 + 10, 22, 64 + 13, 28], fill=METAL)
    d.rectangle([64 + 18, 24, 64 + 21, 30], fill=METAL)
    suit_body(96, 0, arms_up=True)
    ox = 128
    d.ellipse([ox + 8, 10, ox + 23, 25], fill=SUIT)
    d.ellipse([ox + 11, 13, ox + 20, 22], fill=SUIT_DK)
    d.ellipse([ox + 13, 15, ox + 18, 20], fill=VISOR)
    d.point([(ox + 10, 12), (ox + 21, 12), (ox + 10, 23), (ox + 21, 23)], fill=METAL_HI)
    save(img, "player.png")


def gen_enemy() -> None:
    img = Image.new("RGBA", (64, 24), TRANSPARENT)
    d = ImageDraw.Draw(img)
    for i, ox in enumerate((0, 32)):
        yoff = 0 if i == 0 else 2
        d.ellipse([ox + 4, 6 + yoff, ox + 27, 20 + yoff], fill=MAGENTA)
        d.ellipse([ox + 8, 8 + yoff, ox + 23, 18 + yoff], fill=RED)
        for lx in (8, 14, 20):
            d.rectangle([ox + lx, 18 + yoff, ox + lx + 2, 22 + yoff], fill=METAL)
        d.rectangle([ox + 18, 10 + yoff, ox + 22, 14 + yoff], fill=GOLD)
    save(img, "enemy_crawler.png")


def gen_tiles() -> None:
    img = Image.new("RGBA", (32 * 6, 32), TRANSPARENT)
    d = ImageDraw.Draw(img)

    d.rectangle([0, 0, 31, 31], fill=TEAL_DK)
    d.rectangle([2, 2, 29, 29], fill=TEAL)
    for x in range(0, 32, 8):
        d.line([(x, 0), (x, 31)], fill=TEAL_DK)
    for y in range(0, 32, 8):
        d.line([(0, y), (31, y)], fill=TEAL_DK)
    d.rectangle([4, 4, 10, 10], fill=METAL_HI)

    ox = 32
    d.rectangle([ox + 6, 0, ox + 25, 31], fill=METAL)
    d.rectangle([ox + 8, 2, ox + 23, 29], fill=(40, 20, 60, 255))
    d.rectangle([ox + 10, 4, ox + 21, 27], fill=MAGENTA)
    d.ellipse([ox + 14, 14, ox + 18, 18], fill=GOLD)

    ox = 64
    for i in range(4):
        x0 = ox + i * 8
        d.polygon([(x0 + 4, 8), (x0, 31), (x0 + 8, 31)], fill=METAL_HI)
        d.polygon([(x0 + 4, 10), (x0 + 1, 31), (x0 + 7, 31)], fill=WHITE)

    ox = 96
    d.rectangle([ox, 0, ox + 31, 31], fill=(20, 30, 40, 255))
    d.rectangle([ox + 2, 2, ox + 29, 12], fill=TEAL_DK)
    d.rectangle([ox + 2, 20, ox + 29, 29], fill=TEAL_DK)
    d.rectangle([ox + 4, 14, ox + 27, 18], fill=(12, 16, 24, 255))

    ox = 128
    d.rectangle([ox + 4, 0, ox + 27, 31], fill=GREEN)
    d.rectangle([ox + 8, 4, ox + 23, 27], fill=(20, 60, 40, 255))
    d.rectangle([ox + 12, 10, ox + 19, 22], fill=GOLD)

    ox = 160
    d.rectangle([ox, 0, ox + 31, 31], fill=METAL)
    d.rectangle([ox + 3, 3, ox + 28, 28], fill=METAL_HI)
    d.rectangle([ox + 8, 8, ox + 23, 23], fill=TEAL_DK)
    save(img, "tiles.png")


def gen_beam() -> None:
    img = Image.new("RGBA", (16, 6), TRANSPARENT)
    d = ImageDraw.Draw(img)
    d.rectangle([0, 1, 15, 4], fill=VISOR)
    d.rectangle([2, 0, 13, 5], fill=WHITE)
    d.rectangle([4, 2, 11, 3], fill=GOLD)
    save(img, "beam.png")


def gen_pickup() -> None:
    img = Image.new("RGBA", (24, 24), TRANSPARENT)
    d = ImageDraw.Draw(img)
    d.ellipse([2, 2, 21, 21], fill=GOLD)
    d.ellipse([5, 5, 18, 18], fill=SUIT)
    d.ellipse([8, 8, 15, 15], fill=VISOR)
    save(img, "pickup_morph.png")


def gen_hud() -> None:
    img = Image.new("RGBA", (12, 12), TRANSPARENT)
    d = ImageDraw.Draw(img)
    d.ellipse([1, 1, 10, 10], fill=GREEN)
    d.ellipse([3, 3, 8, 8], fill=(40, 120, 70, 255))
    save(img, "hud_energy.png")


def gen_bg_gradient() -> None:
    w, h = 480, 272
    img = Image.new("RGBA", (w, h), BG_DEEP)
    for y in range(h):
        t = y / (h - 1)
        if t < 0.55:
            u = t / 0.55
            r = int(BG_DEEP[0] + (BG_MID[0] - BG_DEEP[0]) * u)
            g = int(BG_DEEP[1] + (BG_MID[1] - BG_DEEP[1]) * u)
            b = int(BG_DEEP[2] + (BG_MID[2] - BG_DEEP[2]) * u)
        else:
            u = (t - 0.55) / 0.45
            r = int(BG_MID[0] + (BG_MIST[0] - BG_MID[0]) * u)
            g = int(BG_MID[1] + (BG_MIST[1] - BG_MID[1]) * u)
            b = int(BG_MID[2] + (BG_MIST[2] - BG_MID[2]) * u)
        for x in range(w):
            n = ((x * 17 + y * 3) % 23) - 11
            img.putpixel(
                (x, y),
                (
                    max(0, min(255, r + n // 3)),
                    max(0, min(255, g + n // 4)),
                    max(0, min(255, b + n // 5)),
                    255,
                ),
            )
    save(img, "bg_gradient.png")


def gen_bg_silhouettes() -> None:
    w, h = 960, 544
    img = Image.new("RGBA", (w, h), TRANSPARENT)
    d = ImageDraw.Draw(img)
    sil = (6, 8, 16, 180)
    sil2 = (10, 14, 24, 140)
    pillars = [
        (40, 180, 70, 544),
        (120, 220, 155, 544),
        (280, 160, 320, 544),
        (500, 200, 545, 544),
        (700, 140, 740, 544),
        (820, 210, 870, 544),
        (900, 170, 940, 544),
    ]
    for x0, y0, x1, y1 in pillars:
        d.rectangle([x0, y0, x1, y1], fill=sil)
        d.rectangle([x0 - 6, y0, x1 + 6, y0 + 18], fill=sil2)
    for cx, cy, rad in ((200, 320, 80), (620, 300, 100), (780, 340, 60)):
        d.ellipse([cx - rad, cy - rad, cx + rad, cy + rad // 3], outline=sil2, width=10)
    for x in (360, 380, 400, 640, 660):
        d.rectangle([x, 0, x + 8, 160 + (x % 40)], fill=sil2)
        d.ellipse([x - 4, 150 + (x % 40), x + 12, 170 + (x % 40)], fill=sil)
    for y in range(480, 544):
        a = int(40 + (y - 480) * 2)
        d.line([(0, y), (w, y)], fill=(12, 24, 32, min(200, a)))
    save(img, "bg_silhouettes.png")


def main() -> None:
    gen_player()
    gen_enemy()
    gen_tiles()
    gen_beam()
    gen_pickup()
    gen_hud()
    gen_bg_gradient()
    gen_bg_silhouettes()
    print("done")


if __name__ == "__main__":
    main()
