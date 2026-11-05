#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
export DEVKITPRO=/opt/devkitpro
export PATH="$DEVKITPRO/devkitARM/bin:$DEVKITPRO/tools/bin:$PATH"
OUT="$ROOT/work/arm"; mkdir -p "$OUT"
"${PYTHON:-python3}" -X utf8 "$ROOT/tools/build_symbian_probe.py"
"${PYTHON:-python3}" -X utf8 "$ROOT/tools/build-amr.py" arm
ARCH=(-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft)
FLAGS=("${ARCH[@]}" -O2 -D__3DS__ -D__JVM_PRI_ALLOC__=0 -ffunction-sections -fdata-sections -I"$ROOT/third-party/minijvm/minijvm/c/jvm" -I"$ROOT/third-party/minijvm/minijvm/c/utils" -I"$DEVKITPRO/libctru/include" -I"$DEVKITPRO/portlibs/3ds/include" -DCURL_STATICLIB)
for FILE in midp_native raster; do arm-none-eabi-gcc "${FLAGS[@]}" -std=gnu99 -c "$ROOT/native/$FILE.c" -o "$OUT/$FILE.o"; done
arm-none-eabi-gcc "${FLAGS[@]}" -std=gnu99 -c "$ROOT/native/audio/midi_engine.c" -o "$OUT/midi_engine.o"
for FILE in main catalog updater; do arm-none-eabi-g++ "${FLAGS[@]}" -std=gnu++17 -fno-exceptions -fno-rtti -c "$ROOT/native/$FILE.cpp" -o "$OUT/$FILE.o"; done
arm-none-eabi-g++ "${FLAGS[@]}" -std=gnu++17 -fno-exceptions -fno-rtti -c "$ROOT/native/audio/device_3ds.cpp" -o "$OUT/audio_device.o"
arm-none-eabi-g++ "${FLAGS[@]}" -std=gnu++17 -fno-exceptions -fno-rtti -c "$ROOT/native/audio/ui_3ds.cpp" -o "$OUT/ui_audio.o"
arm-none-eabi-g++ "${ARCH[@]}" -specs=3dsx.specs -Wl,--gc-sections "$OUT/main.o" "$OUT/catalog.o" "$OUT/updater.o" "$OUT/midp_native.o" "$OUT/raster.o" "$OUT/midi_engine.o" "$OUT/audio_device.o" "$OUT/ui_audio.o" "$ROOT/work/runtime-arm/libminijvm-offline.a" "$ROOT/work/amr-arm/libamrnb.a" -L"$DEVKITPRO/portlibs/3ds/lib" -L"$DEVKITPRO/portlibs/armv6k/lib" -L"$DEVKITPRO/libctru/lib" -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lz -lctru -lm -o "$OUT/java_pocket.elf"
smdhtool --create "Java3DS" "JAR + JAD / MIDI / Wi-Fi" "Faefolk333" "$ROOT/assets/icon48.png" "$OUT/java_pocket.smdh"
3dsxtool "$OUT/java_pocket.elf" "$OUT/java_pocket.3dsx" --smdh="$OUT/java_pocket.smdh" --romfs="$ROOT/romfs"
