# Varia Depths

Early-Metroid-inspired **2D platformer** for a hacked PS Vita.

Explore linked cavern rooms, fire a beam, unlock **morph**, crawl through sealed tunnels, and reach the exit.

## Controls

| Vita | Action |
|------|--------|
| Left stick / D-pad | Move |
| D-pad Down | Morph (after pickup) |
| Cross | Jump / confirm |
| Square or R | Fire beam |
| Start | Title / quit |

Desktop (if you build host-side later): arrows/WASD, Z/Space jump, X shoot, Down morph.

## Build (Docker)

```bash
chmod +x scripts/build-docker.sh
./scripts/build-docker.sh
```

Produces `build-vita/varia-depths.vpk` (title ID `VARI00001`).

## Install

```bash
curl -T build-vita/varia-depths.vpk "ftp://VITA_IP:1337/ux0:/data/"
```

Install in VitaShell, then launch **Varia Depths**.

## Layout

| Path | Role |
|------|------|
| `src/` | Game loop, player, world, entities, render |
| `assets/` | Pixel sprites + artistic background layers |
| `scripts/gen_assets.py` | Regenerates PNGs |
| `scripts/build-docker.sh` | VitaSDK container build |

## Goal

1. Clear the landing shaft and corridor.  
2. Grab the morph pickup.  
3. Crawl through the magenta tunnel gate.  
4. Climb the ascent and touch the green exit.
