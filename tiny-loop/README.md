# tiny-loop

Minimal **SDL2 game loop** for a hacked PS Vita. No engine — just:

`init → input → update(dt) → render → (repeat) → quit`

A yellow square moves with the left stick / D-pad. Cross, Circle, or Start quits.

Project path in this repo: `dev/vita/tiny-loop`  
(Linux `/dev` is for device nodes, so the sources live in the git workspace.)

## Requirements

- A CFW Vita (HENkaku / Ensō) with VitaShell
- Either:
  - **Docker** + image `vitasdk/vitasdk:2026.08`, or
  - A local [VitaSDK](https://vitasdk.org/) install with `vdpm install sdl2`

## Build with Docker (recommended)

From this directory:

```bash
chmod +x scripts/build-docker.sh
./scripts/build-docker.sh
```

The script swaps the image’s `sdl2_vitagl` package for classic `sdl2` (2D GXM renderer — what this sample needs), then builds.

Produces: `build-vita/tiny-loop.vpk`

Pin a dated image if you want bit-identical rebuilds:

```bash
VITASDK_IMAGE=vitasdk/vitasdk:2026.08-20260815 ./scripts/build-docker.sh
```

## Build with a local VitaSDK

```bash
export VITASDK=/usr/local/vitasdk
export PATH="$VITASDK/bin:$PATH"
vdpm install sdl2   # once

cmake -S . -B build-vita -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
cmake --build build-vita
```

## Install on your Vita

1. Open VitaShell → press **SELECT** to start FTP (or use USB mode).
2. Copy the VPK:

   ```bash
   curl -T build-vita/tiny-loop.vpk "ftp://VITA_IP:1337/ux0:/data/"
   ```

3. In VitaShell, open `ux0:data/tiny-loop.vpk` → install.
4. Launch the **tiny-loop** bubble (title ID `TINY00001`).

## Code map

| File | Role |
|------|------|
| `src/main.c` | Entire game: `Game` struct + init / input / update / render / run |
| `CMakeLists.txt` | Cross-compile + pack `.self` / `.vpk` |
| `scripts/build-docker.sh` | One-shot build in the official VitaSDK container |

Grow the game by adding more fields to `Game` and logic inside `game_update` / `game_render`. Keep the loop shape the same.
