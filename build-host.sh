#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
export PATH="/mingw64/bin:$PATH"
SRC="$ROOT/third-party/minijvm/minijvm/c"
OUT="$ROOT/work/runtime-host"
mkdir -p "$OUT"
FLAGS=(-std=gnu99 -O2 -fno-strict-aliasing -fwrapv -DCRIMSON_OFFLINE=1 -DJIT_ENABLE=0 -D__JVM_PRI_ALLOC__=0 -I"$SRC/jvm" -I"$SRC/utils" -I"$SRC/utils/https" -I"$SRC/utils/https/mbedtls/include")
OBJS=()
REBUILD=0
STAMP="$OUT/flags.txt"
if [[ ! -f "$STAMP" || "$(cat "$STAMP")" != "${FLAGS[*]}" ]]; then REBUILD=1; fi
for file in "$SRC"/jvm/*.c "$SRC"/utils/*.c; do
 case "$file" in */os_posix.c|*/jdwp.c) continue;; esac
 name="$(basename "$(dirname "$file")")_$(basename "$file" .c)"
 obj="$OUT/$name.o"
 if [[ "$REBUILD" == 1 || ! -f "$obj" || "$file" -nt "$obj" || "$SRC/jvm/jvm.h" -nt "$obj" ]]; then gcc "${FLAGS[@]}" -c "$file" -o "$obj"; fi
 OBJS+=("$obj")
done
gcc "${FLAGS[@]}" -c "$ROOT/native/jdwp_disabled.c" -o "$OUT/jdwp_disabled.o"
TEST_OBJECTS=()
if [[ "${JAVA_ALLOC_FAILURE_TEST:-0}" == 1 ]]; then
 gcc -c "$ROOT/tests/failed_array_alloc.c" -o "$OUT/failed-array-alloc.o"
 TEST_OBJECTS=("$OUT/failed-array-alloc.o" -Wl,--wrap=calloc)
fi
gcc "${FLAGS[@]}" -O2 "${TEST_OBJECTS[@]}" "${OBJS[@]}" "$OUT/jdwp_disabled.o" "$ROOT/native/midp_native.c" "$ROOT/native/raster.c" "$ROOT/native/audio/midi_engine.c" "$ROOT/native/host.c" -lpthread -lm -lws2_32 -lrpcrt4 -ldbghelp -o "$ROOT/work/${JAVA_HOST_OUTPUT:-java-host.exe}"
printf "%s" "${FLAGS[*]}" > "$STAMP"
