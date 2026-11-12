#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
export DEVKITPRO=/opt/devkitpro
export PATH="$DEVKITPRO/devkitARM/bin:$DEVKITPRO/tools/bin:$PATH"
OUT="$ROOT/work/arm"
# Validate serialized name lookup, not just the CIA's container hashes.
"${PYTHON:-python3}" -X utf8 "$ROOT/tools/check_banner.py" "$ROOT/assets/banner/banner.cgfx"
"${PYTHON:-python3}" -X utf8 "$ROOT/tests/banner_motion.py"
"$ROOT/work/cia-tools/bannertool/bannertool-1.2.3-windows/bannertool.exe" makebanner -ci "$ROOT/assets/banner/banner.cgfx" -a "$ROOT/assets/banner/banner.wav" -o "$ROOT/assets/banner/banner.bnr"
# Build common native objects and the standalone Homebrew Launcher version.
bash "$ROOT/build-native.sh"
ARCH=(-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft)
FLAGS=("${ARCH[@]}" -O2 -D__3DS__ -DJAVA3DS_CIA -D__JVM_PRI_ALLOC__=0 -ffunction-sections -fdata-sections -I"$ROOT/third-party/minijvm/minijvm/c/jvm" -I"$ROOT/third-party/minijvm/minijvm/c/utils" -I"$DEVKITPRO/libctru/include" -I"$DEVKITPRO/portlibs/3ds/include" -DCURL_STATICLIB)
for FILE in main updater cia_heap; do arm-none-eabi-g++ "${FLAGS[@]}" -std=gnu++17 -fno-exceptions -fno-rtti -c "$ROOT/native/$FILE.cpp" -o "$OUT/cia-$FILE.o"; done
arm-none-eabi-g++ "${ARCH[@]}" -specs=3dsx.specs -Wl,--gc-sections -Wl,--wrap=__system_allocateHeaps "$OUT/cia-cia_heap.o" "$OUT/cia-main.o" "$OUT/catalog.o" "$OUT/cia-updater.o" "$OUT/midp_native.o" "$OUT/raster.o" "$OUT/midi_engine.o" "$OUT/audio_device.o" "$OUT/ui_audio.o" "$ROOT/work/runtime-arm/libminijvm-offline.a" "$ROOT/work/amr-arm/libamrnb.a" -L"$DEVKITPRO/portlibs/3ds/lib" -L"$DEVKITPRO/portlibs/armv6k/lib" -L"$DEVKITPRO/libctru/lib" -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lz -lctru -lm -o "$OUT/java3ds-cia.elf"
"$ROOT/work/cia-tools/bannertool/bannertool-1.2.3-windows/bannertool.exe" makesmdh -s Java3DS -l "JAR + JAD / MIDI / Wi-Fi" -p Faefolk333 -f visible,allow3d,recordusage,extendedbanner -i "$ROOT/assets/icon48.png" -o "$OUT/java3ds-cia.smdh"
"$ROOT/work/cia-tools/makerom184/makerom.exe" -f cia -target t -exefslogo -o "$OUT/java3ds.cia" -elf "$OUT/java3ds-cia.elf" -rsf "$ROOT/assets/java3ds.rsf" -icon "$OUT/java3ds-cia.smdh" -banner "$ROOT/assets/banner/banner.bnr" "-DAPP_ROMFS=$(cygpath -m "$ROOT/romfs")" -ver 112
