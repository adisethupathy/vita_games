#!/usr/bin/env bash
# Build varia-depths.vpk using the official VitaSDK Docker image.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="${VITASDK_IMAGE:-vitasdk/vitasdk:2026.08}"

echo "Using image: ${IMAGE}"
echo "Project:     ${ROOT}"

# Regenerate pixel assets if Pillow is available
if python3 -c "from PIL import Image" >/dev/null 2>&1; then
  python3 "${ROOT}/scripts/gen_assets.py"
fi

DOCKER=(docker)
if ! docker info >/dev/null 2>&1; then
  if sudo -n docker info >/dev/null 2>&1; then
    DOCKER=(sudo -n docker)
  else
    echo "Docker daemon not reachable. Start dockerd or fix /var/run/docker.sock permissions." >&2
    exit 1
  fi
fi

"${DOCKER[@]}" run --rm \
  -v "${ROOT}:/workspace" \
  -w /workspace \
  "${IMAGE}" \
  bash -lc '
    set -euo pipefail
    if vdpm list 2>/dev/null | grep -q "^sdl2_vitagl "; then
      vdpm pacman -- --noconfirm -Rdd sdl2_vitagl </dev/null
    fi
    if ! vdpm list 2>/dev/null | grep -q "^sdl2 "; then
      vdpm pacman -- --noconfirm -S sdl2 </dev/null
    fi
    if ! vdpm list 2>/dev/null | grep -q "^sdl2_image "; then
      vdpm pacman -- --noconfirm -S sdl2_image </dev/null
    fi
    test -f "$VITASDK/arm-vita-eabi/include/SDL2/SDL.h"
    test -f "$VITASDK/arm-vita-eabi/include/SDL2/SDL_image.h"
    rm -rf build-vita
    cmake -S . -B build-vita \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake"
    cmake --build build-vita -j"$(nproc)"
    echo
    echo "Built:"
    ls -la build-vita/*.vpk
  '

echo
echo "VPK on host: ${ROOT}/build-vita/varia-depths.vpk"
echo "Copy to Vita (VitaShell FTP on Select):"
echo "  curl -T ${ROOT}/build-vita/varia-depths.vpk \"ftp://VITA_IP:1337/ux0:/data/\""
