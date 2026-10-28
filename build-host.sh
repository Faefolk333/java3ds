#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
export PATH="/mingw64/bin:$PATH"
SRC="$ROOT/third-party/minijvm/minijvm/c"
OUT="$ROOT/work/runtime-host"
mkdir -p "$OUT"
FLAGS=(-std=gnu99 -O1 -DCRIMSON_OFFLINE=1 -DJIT_ENABLE=0 -D__JVM_PRI_ALLOC__=0 -I"$SRC/jvm" -I"$SRC/utils" -I"$SRC/utils/https" -I"$SRC/utils/https/mbedtls/include")
OBJS=()
for file in "$SRC"/jvm/*.c "$SRC"/utils/*.c; do
 case "$file" in */os_posix.c|*/jdwp.c) continue;; esac
 name="$(basename "$(dirname "$file")")_$(basename "$file" .c)"
 obj="$OUT/$name.o"
 if [[ ! -f "$obj" || "$file" -nt "$obj" || "$SRC/jvm/jvm.h" -nt "$obj" ]]; then gcc "${FLAGS[@]}" -c "$file" -o "$obj"; fi
 OBJS+=("$obj")
done
gcc "${FLAGS[@]}" -c "$ROOT/native/jdwp_disabled.c" -o "$OUT/jdwp_disabled.o"
gcc "${FLAGS[@]}" -O2 "${OBJS[@]}" "$OUT/jdwp_disabled.o" "$ROOT/native/midp_native.c" "$ROOT/native/raster.c" "$ROOT/native/audio/midi_engine.c" "$ROOT/native/host.c" -lpthread -lm -lws2_32 -lrpcrt4 -ldbghelp -o "$ROOT/work/java-host.exe"
