#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
SRC="$ROOT/third-party/minijvm/minijvm/c"
export DEVKITPRO=/opt/devkitpro
export DEVKITARM="$DEVKITPRO/devkitARM"
export PATH="$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH"
OUT="$ROOT/work/runtime-arm"
mkdir -p "$OUT"
cd "$SRC"
FLAGS=(-std=gnu99 -O1 -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
  -D__3DS__ -DJIT_ENABLE=0 -D__JVM_PRI_ALLOC__=0 -ffunction-sections -fdata-sections
  -Ijvm -Iutils -Iutils/https -Iutils/https/mbedtls/include -I"$DEVKITPRO/libctru/include")
SOURCES=(jvm/*.c utils/*.c)
OBJECTS=()
for src in "${SOURCES[@]}"; do
  case "$src" in jvm/os_posix.c|jvm/os_win.c|jvm/jdwp.c) continue;; esac
  obj="$OUT/${src//\//_}.o"
  echo "ARM $src"
  arm-none-eabi-gcc "${FLAGS[@]}" -c "$src" -o "$obj"
  OBJECTS+=("$obj")
done
arm-none-eabi-gcc "${FLAGS[@]}" -c "$ROOT/native/os_3ds.c" -o "$OUT/os_3ds.o"
arm-none-eabi-gcc "${FLAGS[@]}" -c "$ROOT/native/jdwp_disabled.c" -o "$OUT/jdwp_disabled.o"
# A new archive name prevents obsolete trial members from being retained.
arm-none-eabi-ar rcs "$OUT/libminijvm-offline.a" "${OBJECTS[@]}" "$OUT/os_3ds.o" "$OUT/jdwp_disabled.o"
